/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: ImageRegistration1.cxx,v $
  Language:  C++
  Date:      $Date: 2007-11-22 00:30:16 $
  Version:   $Revision: 1.53 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif


#include "itkImageRegistrationMethod.h"
#include "itkTranslationTransform.h"
#include "itkMeanSquaresImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkRegularStepGradientDescentOptimizer.h"
#include "itkImage.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkSubtractImageFilter.h"

// I do a mean filtering, before shrinking, to lose less information
#include "itkMeanImageFilter.h"
#include "itkShrinkImageFilter.h"
// fabs may not have been used in other classes
#include "math.h"

class CommandIterationUpdate : public itk::Command
{
public:
  typedef  CommandIterationUpdate   Self;
  typedef  itk::Command             Superclass;
  typedef itk::SmartPointer<Self>  Pointer;
  itkNewMacro( Self );

protected:
  CommandIterationUpdate() {};

public:

  typedef itk::RegularStepGradientDescentOptimizer     OptimizerType;
  typedef const OptimizerType                         *OptimizerPointer;

  void Execute(itk::Object *caller, const itk::EventObject & event)
  {
    Execute( (const itk::Object *)caller, event);
  }

  void Execute(const itk::Object * object, const itk::EventObject & event)
  {
    OptimizerPointer optimizer =
                         dynamic_cast< OptimizerPointer >( object );

    if( ! itk::IterationEvent().CheckEvent( &event ) )
      {
      return;
      }

    std::cout << optimizer->GetCurrentIteration() << " = ";
    std::cout << optimizer->GetValue() ;
    std::cout << optimizer->GetCurrentPosition() << std::endl;

  }

};


int main( int argc, char *argv[] )
{
  if( argc < 7 )
    {
    std::cerr << "Missing Parameters " << std::endl;
    std::cerr << "Usage: " << argv[0];
    std::cerr << " fixedImageFile  movingImageFile ";
    std::cerr << " max_xtranslation max_ytranslation ";
    std::cerr << " previous_xtranslation previous_ytranslation ";
    std::cerr << " [outputImagefile] [differenceImageAfter]";
    std::cerr << " [differenceImageBefore]" << std::endl;

    //std::cerr << " xMaxTranslation yMaxTranslation zMaxTranslation ";

    return EXIT_FAILURE;
    }



  // The types of each one of the components in the registration methods should
  // be instantiated first. With that purpose, we start by selecting the image
  // dimension and the type used for representing image pixels.
  const    unsigned int    Dimension = 2;
  typedef  float           PixelType;

  //  The types of the input images are instantiated by the following lines.
  typedef itk::Image< PixelType, Dimension >  FixedImageType;
  typedef itk::Image< PixelType, Dimension >  MovingImageType;


  //  The transform that will map the fixed image space into the moving image
  //  space is defined below.
  typedef itk::TranslationTransform< double, Dimension > TransformType;


  //  An optimizer is required to explore the parameter space of the transform
  //  in search of optimal values of the metric.
 typedef itk::RegularStepGradientDescentOptimizer       OptimizerType;

  //  The metric will compare how well the two images match each other. Metric
  //  types are usually parameterized by the image types as it can be seen in
  //  the following type declaration.
  typedef itk::MeanSquaresImageToImageMetric<
                                    FixedImageType,
                                    MovingImageType >    MetricType;

  //  Finally, the type of the interpolator is declared. The interpolator will
  //  evaluate the intensities of the moving image at non-grid positions.
  typedef itk:: LinearInterpolateImageFunction<
                                    MovingImageType,
                                    double          >    InterpolatorType;
  //  The registration method type is instantiated using the types of the
  //  fixed and moving images. This class is responsible for interconnecting
  //  all the components that we have described so far.

  typedef itk::ImageRegistrationMethod<
                                    FixedImageType,
                                    MovingImageType >    RegistrationType;


  //  Each one of the registration components is created using its
  //  New() method and is assigned to its respective
  //  SmartPointer.
  MetricType::Pointer         metric        = MetricType::New();
  TransformType::Pointer      transform     = TransformType::New();
  OptimizerType::Pointer      optimizer     = OptimizerType::New();
  InterpolatorType::Pointer   interpolator  = InterpolatorType::New();
  RegistrationType::Pointer   registration  = RegistrationType::New();


  //  Each component is now connected to the instance of the registration method.
  registration->SetMetric(        metric        );
  registration->SetOptimizer(     optimizer     );
  registration->SetTransform(     transform     );
  registration->SetInterpolator(  interpolator  );


  typedef itk::ImageFileReader< FixedImageType  > FixedImageReaderType;
  typedef itk::ImageFileReader< MovingImageType > MovingImageReaderType;
  FixedImageReaderType::Pointer  fixedImageReader  = FixedImageReaderType::New();
  MovingImageReaderType::Pointer movingImageReader = MovingImageReaderType::New();

  fixedImageReader->SetFileName(  argv[1] );
  movingImageReader->SetFileName( argv[2] );

  //we blur the inputs to take care of general shape

  typedef itk::MeanImageFilter<
      FixedImageType, FixedImageType>  FixedFilterType;

  typedef itk::MeanImageFilter<
      MovingImageType, MovingImageType>  MovingFilterType;

  FixedFilterType::Pointer  fixedFilter  = FixedFilterType::New();
  MovingFilterType::Pointer movingFilter = MovingFilterType::New();

  FixedImageType::SizeType indexFRadius;

  indexFRadius[0] = 15; // radius along x
  indexFRadius[1] = 15; // radius along y

  fixedFilter->SetRadius(indexFRadius);

  MovingImageType::SizeType indexMRadius;

  indexMRadius[0] = 15; // radius along x
  indexMRadius[1] = 15; // radius along y

  movingFilter->SetRadius(indexMRadius);

  fixedFilter->SetInput(fixedImageReader->GetOutput());
  movingFilter->SetInput(movingImageReader->GetOutput());


  typedef itk::ShrinkImageFilter<  FixedImageType, FixedImageType >
    FixedShrinkType;
  typedef itk::ShrinkImageFilter<  MovingImageType, MovingImageType >
    MovingShrinkType;
  FixedShrinkType::Pointer  fixedShrink  = FixedShrinkType::New();
  MovingShrinkType::Pointer movingShrink = MovingShrinkType::New();

  fixedShrink->SetInput( fixedFilter->GetOutput() );
  movingShrink->SetInput( movingFilter->GetOutput() );


  fixedShrink->SetShrinkFactors(static_cast<unsigned int>(3));
  movingShrink->SetShrinkFactors(static_cast<unsigned int>(3));




  //  In this example, the fixed and moving images are read from files. This
  //  requires the ImageRegistrationMethod to acquire its inputs from
  //  the output of the readers.
  registration->SetFixedImage(    fixedShrink->GetOutput()    );
  registration->SetMovingImage(   movingShrink->GetOutput()   );



  //  The registration can be restricted to consider only a particular region
  //  of the fixed image as input to the metric computation. This region is
  //  defined with the SetFixedImageRegion() method.  You could use this
  //  feature to reduce the computational time of the registration or to avoid
  //  unwanted objects present in the image from affecting the registration outcome.
  //  In this example we use the full available content of the image. This
  //  region is identified by the BufferedRegion of the fixed image.
  //  Note that for this region to be valid the reader must first invoke its
  //  Update() method.
  fixedShrink->Update();
  registration->SetFixedImageRegion(
                    fixedShrink->GetOutput()->GetBufferedRegion() );




  //  The parameters of the transform are initialized by passing them in an
  //  array. This can be used to setup an initial known correction of the
  //  misalignment. In this particular case, a translation transform is
  //  being used for the registration. The array of parameters for this
  //  transform is simply composed of the translation values along each
  //  dimension. Setting the values of the parameters to zero
  //  initializes the transform to an Identity transform. Note that the
  //  array constructor requires the number of elements to be passed as an
  //  argument.
  typedef RegistrationType::ParametersType ParametersType;
  ParametersType initialParameters( transform->GetNumberOfParameters() );

  // Initial offset in mm along X
  float previousXtranslation = (float)atof(argv[5]);
  initialParameters[5] = previousXtranslation;
  // Initial offset in mm along Y
  float previousYtranslation = (float)atof(argv[6]);
  initialParameters[6] = previousYtranslation;

  registration->SetInitialTransformParameters( initialParameters );



  //  At this point the registration method is ready for execution. The
  //  optimizer is the component that drives the execution of the
  //  registration.  However, the ImageRegistrationMethod class
  //  orchestrates the ensemble to make sure that everything is in place
  //  before control is passed to the optimizer.
  //
  //  It is usually desirable to fine tune the parameters of the optimizer.
  //  Each optimizer has particular parameters that must be interpreted in the
  //  context of the optimization strategy it implements. The optimizer used in
  //  this example is a variant of gradient descent that attempts to prevent it
  //  from taking steps that are too large.  At each iteration, this optimizer
  //  will take a step along the direction of the \doxygen{ImageToImageMetric}
  //  derivative. The initial length of the step is defined by the user. Each
  //  time the direction of the derivative abruptly changes, the optimizer
  //  assumes that a local extrema has been passed and reacts by reducing the
  //  step length by a half. After several reductions of the step length, the
  //  optimizer may be moving in a very restricted area of the transform
  //  parameter space. The user can define how small the step length should be
  //  to consider convergence to have been reached. This is equivalent to defining
  //  the precision with which the final transform should be known.
  //
  //  The initial step length is defined with the method
  //  SetMaximumStepLength(), while the tolerance for convergence is
  //  defined with the method SetMinimumStepLength().
  //
  optimizer->SetMaximumStepLength( 5);
  optimizer->SetMinimumStepLength( 0.01 );


  //  In case the optimizer never succeeds reaching the desired
  //  precision tolerance, it is prudent to establish a limit on the number of
  //  iterations to be performed. This maximum number is defined with the
  //  method SetNumberOfIterations().
  optimizer->SetNumberOfIterations( 200 );

  // Connect an observer
  CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
  optimizer->AddObserver( itk::IterationEvent(), observer );


  //  The registration process is triggered by an invocation to the
  //  \code{Update()} method. If something goes wrong during the
  //  initialization or execution of the registration an exception will be
  //  thrown. We should therefore place the Update() method
  //  inside a try/catch block as illustrated in the following lines.
  try
    {
    registration->Update();
    }
  catch( itk::ExceptionObject & err )
    {
    std::cerr << "ExceptionObject caught !" << std::endl;
    std::cerr << err << std::endl;
    return EXIT_FAILURE;
    }


  //
  //  The result of the registration process is an array of parameters that
  //  defines the spatial transformation in an unique way. This final result is
  //  obtained using the GetLastTransformParameters() method.
  ParametersType finalParameters = registration->GetLastTransformParameters();

  //
  //  In the case of the \doxygen{TranslationTransform}, there is a
  //  straightforward interpretation of the parameters.  Each element of the
  //  array corresponds to a translation along one spatial dimension.
  const double TranslationAlongX = finalParameters[0];
  const double TranslationAlongY = finalParameters[1];

  //
  //  The optimizer can be queried for the actual number of iterations
  //  performed to reach convergence.  The GetCurrentIteration()
  //  method returns this value. A large number of iterations may be an
  //  indication that the maximum step length has been set too small, which
  //  is undesirable since it results in long computational times.
  const unsigned int numberOfIterations = optimizer->GetCurrentIteration();


  //  The value of the image metric corresponding to the last set of parameters
  //  can be obtained with the GetValue() method of the optimizer.
  const double bestValue = optimizer->GetValue();


  // Print out results
  //
  std::cout << "Result = " << std::endl;
  std::cout << " Translation X = " << TranslationAlongX  << std::endl;
  std::cout << " Translation Y = " << TranslationAlongY  << std::endl;
  std::cout << " Iterations    = " << numberOfIterations << std::endl;
  std::cout << " Metric value  = " << bestValue          << std::endl;

  float maxXtranslation = (float)atof(argv[3]);
  float maxYtranslation = (float)atof(argv[4]);


  // if the translation is within the accepted range :
  if (  ( fabs(TranslationAlongX) < maxXtranslation )
     && ( fabs(TranslationAlongY) < maxYtranslation ) )
    {
    // we use the format accepted by the visual basic macro
    std::cout << "success: ";
    std::cout << "Xshift = "<< TranslationAlongX <<"; ";
    std::cout << "Yshift = "<< TranslationAlongY <<"; ";
    std::cout << std::endl;
    }
  else
    {
    std::cerr << "registration failure : translation larger than limits"
      << std::endl;
    }


  //  It is common, as the last step of a registration task, to use the
  //  resulting transform to map the moving image into the fixed image space.
  //  This is easily done with the ResampleImageFilter. Please
  //  refer to ResampleImageFilter for details on the use
  //  of this filter.  First, a ResampleImageFilter type is instantiated
  //  using the image types. It is convenient to use the fixed image type as
  //  the output type since it is likely that the transformed moving image
  //  will be compared with the fixed image.
  typedef itk::ResampleImageFilter<
                            MovingImageType,
                            FixedImageType >    ResampleFilterType;




  //  A resampling filter is created and the moving image is connected as
  //  its input.
  ResampleFilterType::Pointer resampler = ResampleFilterType::New();
  resampler->SetInput( movingImageReader->GetOutput() );


  //  The Transform that is produced as output of the Registration method is
  //  also passed as input to the resampling filter. Note the use of the
  //  methods GetOutput() and Get(). This combination is needed
  //  here because the registration method acts as a filter whose output is a
  //  transform decorated in the form of a DataObject. For details in
  //  this construction you may want to read the documentation of the
  //  DataObjectDecorator.

  resampler->SetTransform( registration->GetOutput()->Get() );




  //  As described in Section \ref{sec:ResampleImageFilter}, the
  //  ResampleImageFilter requires additional parameters to be specified, in
  //  particular, the spacing, origin and size of the output image. The default
  //  pixel value is also set to a distinct gray level in order to highlight
  //  the regions that are mapped outside of the moving image.
  FixedImageType::Pointer fixedImage = fixedImageReader->GetOutput();
  resampler->SetSize( fixedImage->GetLargestPossibleRegion().GetSize() );
  resampler->SetOutputOrigin(  fixedImage->GetOrigin() );
  resampler->SetOutputSpacing( fixedImage->GetSpacing() );
  resampler->SetOutputDirection( fixedImage->GetDirection() );
  resampler->SetDefaultPixelValue( 100 );





  //  The output of the filter is passed to a writer that will store the
  //  image in a file. An CastImageFilter is used to convert the
  //  pixel type of the resampled image to the final type used by the
  //  writer. The cast and writer filters are instantiated below.
  typedef unsigned char OutputPixelType;
  typedef itk::Image< OutputPixelType, Dimension > OutputImageType;
  typedef itk::CastImageFilter<
                        FixedImageType,
                        OutputImageType > CastFilterType;
  typedef itk::ImageFileWriter< OutputImageType >  WriterType;

  WriterType::Pointer      writer =  WriterType::New();
  CastFilterType::Pointer  caster =  CastFilterType::New();



  //  The filters are connected together and the \code{Update()} method of the
  //  writer is invoked in order to trigger the execution of the pipeline.
  caster->SetInput( resampler->GetOutput() );
  writer->SetInput( caster->GetOutput()   );
  if( argc > 7 )
    {
    writer->SetFileName( argv[7] );
    writer->Update();
    }


  //  The fixed image and the transformed moving image can easily be compared
  //  using the SubtractImageFilter. This pixel-wise filter computes
  //  the difference between homologous pixels of its two input images.


  typedef itk::SubtractImageFilter<
                                  FixedImageType,
                                  FixedImageType,
                                  FixedImageType > DifferenceFilterType;

  DifferenceFilterType::Pointer difference = DifferenceFilterType::New();

  difference->SetInput1( fixedImageReader->GetOutput() );
  difference->SetInput2( resampler->GetOutput() );




  //  Note that the use of subtraction as a method for comparing the images is
  //  appropriate here because we chose to represent the images using a pixel
  //  type float. A different filter would have been used if the pixel
  //  type of the images were any of the unsigned integer type.




  //  Since the differences between the two images may correspond to very low
  //  values of intensity, we rescale those intensities with a
  //  RescaleIntensityImageFilter in order to make them more visible.
  //  This rescaling will also make possible to visualize the negative values
  //  even if we save the difference image in a file format that only support
  //  unsigned pixel values\footnote{This is the case of PNG, BMP, JPEG and
  //  TIFF among other common file formats.}.  We also reduce the
  //  \code{DefaultPixelValue} to ``1'' in order to prevent that value from
  //  absorbing the dynamic range of the differences between the two images.
  typedef itk::RescaleIntensityImageFilter<
                                  FixedImageType,
                                  OutputImageType >   RescalerType;

  RescalerType::Pointer intensityRescaler = RescalerType::New();

  intensityRescaler->SetInput( difference->GetOutput() );
  intensityRescaler->SetOutputMinimum(   0 );
  intensityRescaler->SetOutputMaximum( 255 );

  resampler->SetDefaultPixelValue( 1 );

  //
  //  Its output can be passed to another writer.


  WriterType::Pointer writer2 = WriterType::New();
  writer2->SetInput( intensityRescaler->GetOutput() );


  if( argc > 8 )
    {
    writer2->SetFileName( argv[8] );
    writer2->Update();
    }




  //  For the purpose of comparison, the difference between the fixed image and
  //  the moving image before registration can also be computed by simply
  //  setting the transform to an identity transform. Note that the resampling
  //  is still necessary because the moving image does not necessarily have the
  //  same spacing, origin and number of pixels as the fixed image. Therefore a
  //  pixel-by-pixel operation cannot in general be performed. The resampling
  //  process with an identity transform will ensure that we have a
  //  representation of the moving image in the grid of the fixed image.


  TransformType::Pointer identityTransform = TransformType::New();
  identityTransform->SetIdentity();
  resampler->SetTransform( identityTransform );


  if( argc > 9 )
    {
    writer2->SetFileName( argv[9] );
    writer2->Update();
    }


  //  It is always useful to keep in mind that registration is essentially an
  //  optimization problem. Figure \ref{fig:ImageRegistration1Trace} helps to
  //  reinforce this notion by showing the trace of translations and values of
  //  the image metric at each iteration of the optimizer. It can be seen from
  //  the top figure that the step length is reduced progressively as the
  //  optimizer gets closer to the metric extrema. The bottom plot clearly
  //  shows how the metric value decreases as the optimization advances. The
  //  log plot helps to highlight the normal oscillations of the optimizer
  //  around the extrema value.



  return EXIT_SUCCESS;
}

