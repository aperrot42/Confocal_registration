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

/*
*  This program is intended to plot the value of the cost function depending
*  on the transformation parameters.
*  It performs an exhaustive search. This is usefull to determine
*  the optimization method, later on.
*/


#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif


#include "itkImageRegistrationMethod.h"
#include "itkTranslationTransform.h"
#include "itkMeanSquaresImageToImageMetric.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkImage.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include "itkResampleImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkSubtractImageFilter.h"


#include "itkExhaustiveOptimizer.h"


class CommandIterationUpdate : public itk::Command
{
public:
  typedef  CommandIterationUpdate   Self;
  typedef  itk::Command             Superclass;
  typedef itk::SmartPointer<Self>  Pointer;
  itkNewMacro( Self );
  static const unsigned int Dimension = 2;

protected:
  CommandIterationUpdate() {};

public:
  typedef itk::Image< double, Dimension >  metricImageType;
  typedef itk::ExhaustiveOptimizer  OptimizerType;
  typedef const OptimizerType*      OptimizerPointer;

  void Execute(itk::Object *caller, const itk::EventObject & event)
  {
    Execute( (const itk::Object *)caller, event);
  }

  void Execute(const itk::Object * object, const itk::EventObject & event)
  {
    OptimizerPointer optimizer =
                         dynamic_cast< OptimizerPointer >( object );

    if( itk::IterationEvent().CheckEvent(& event ) )
      {
      metricImageType::IndexType index;
      index[0] = optimizer->GetCurrentIndex()[0];
      index[1] = optimizer->GetCurrentIndex()[1];
      index[2] = optimizer->GetCurrentIndex()[2];

      std::cout << optimizer->GetCurrentValue() << std::endl;
      image->SetPixel( index, optimizer->GetCurrentValue() );
    }
  }

  metricImageType::Pointer image;
};


int main( int argc, char *argv[] )
{
  if( argc < 4 )
    {
    std::cerr << "Missing Parameters " << std::endl;
    std::cerr << "Usage: " << argv[0];
    std::cerr << " fixedImageFile  movingImageFile ";
    std::cerr << "outputmetricImagefile";
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
  typedef itk::Image< double, Dimension >  metricImageType;

  //  The transform that will map the fixed image space into the moving image
  //  space is defined below.
  typedef itk::TranslationTransform< double, Dimension > TransformType;


  //  An optimizer is required to explore the parameter space of the transform
  //  in search of optimal values of the metric.
// typedef itk::RegularStepGradientDescentOptimizer       OptimizerType;
typedef itk::ExhaustiveOptimizer OptimizerType;

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


  //  In this example, the fixed and moving images are read from files. This
  //  requires the ImageRegistrationMethod to acquire its inputs from
  //  the output of the readers.
  registration->SetFixedImage(    fixedImageReader->GetOutput()    );
  registration->SetMovingImage(   movingImageReader->GetOutput()   );



  //  The registration can be restricted to consider only a particular region
  //  of the fixed image as input to the metric computation. This region is
  //  defined with the SetFixedImageRegion() method.  You could use this
  //  feature to reduce the computational time of the registration or to avoid
  //  unwanted objects present in the image from affecting the registration outcome.
  //  In this example we use the full available content of the image. This
  //  region is identified by the BufferedRegion of the fixed image.
  //  Note that for this region to be valid the reader must first invoke its
  //  Update() method.
  //
  fixedImageReader->Update();
  registration->SetFixedImageRegion(
                    fixedImageReader->GetOutput()->GetBufferedRegion() );


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

  initialParameters[0] = 0.0;  // Initial offset in mm along X
  initialParameters[1] = 0.0;  // Initial offset in mm along Y

  registration->SetInitialTransformParameters( initialParameters );



  //  At this point the registration method is ready for execution. The
  //  optimizer is the component that drives the execution of the
  //  registration.  However, the ImageRegistrationMethod class
  //  orchestrates the ensemble to make sure that everything is in place
  //  before control is passed to the optimizer.
  //
  //  It is usually desirable to fine tune the parameters of the optimizer.
  //  Each optimizer has particular parameters that must be interpreted in the
  //  context of the optimization strategy it implements.
  OptimizerType::StepsType steps( transform->GetNumberOfParameters() );
  steps[0] = 10;
  steps[1] = 10;
  optimizer->SetNumberOfSteps( steps );
  optimizer->SetStepLength( 1 );

  // create the image containing the result of the exhaustive computation of
  // metric
  metricImageType::Pointer metricImage = metricImageType::New();

  metricImageType::SizeType size;
  size[0] = steps[0]+1;
  size[1] = steps[1]+1;

  metricImageType::IndexType start;
  start[0] = 0;
  start[1] = 0;

  metricImageType::RegionType region;
  region.SetSize( size );
  region.SetIndex( start );

  metricImage->SetRegions( region );
  metricImage->Allocate();

  // Connect an observer
  CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
  observer->image = metricImage;

  optimizer->AddObserver( itk::IterationEvent(), observer );


  //  The registration process is triggered by an invocation to the
  //  Update() method. If something goes wrong during the
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
  //  performed to reach convergence.  The \code{GetCurrentIteration()}
  //  method returns this value. A large number of iterations may be an
  //  indication that the maximum step length has been set too small, which
  //  is undesirable since it results in long computational times.
//  const unsigned int numberOfIterations = optimizer->GetCurrentIteration();


  //  The value of the image metric corresponding to the last set of parameters
  //  can be obtained with the \code{GetValue()} method of the optimizer.
  const double bestValue = optimizer->GetCurrentValue();


  // Print out results
  //
  std::cout << "Result = " << std::endl;
  std::cout << " Translation X = " << TranslationAlongX  << std::endl;
  std::cout << " Translation Y = " << TranslationAlongY  << std::endl;
  //std::cout << " Iterations    = " << numberOfIterations << std::endl;
  std::cout << " Metric value  = " << bestValue          << std::endl;

  typedef itk::ImageFileWriter< metricImageType >  WriterType;
  WriterType::Pointer      writer =  WriterType::New();
  writer->SetFileName( argv[3] );
  writer->SetInput( metricImage );

  try
    {
    writer->Update();
    }
  catch( itk::ExceptionObject & err )
    {
    std::cerr << "ExceptionObject caught !" << std::endl;
    std::cerr << err << std::endl;
    return EXIT_FAILURE;
    }


  return EXIT_SUCCESS;
}

