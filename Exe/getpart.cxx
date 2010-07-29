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
*  This program is intended to save a tile of an image
*/


#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRegionOfInterestImageFilter.h"

#include "itkImage.h"


int main( int argc, char ** argv )
{
  // Verify the number of parameters in the command line
  if( argc < 7 )
    {
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << " inputImageFile  outputImageFile " << std::endl;
    std::cerr << " startX startY sizeX sizeY" << std::endl;
    return EXIT_FAILURE;
    }

  //  Image types are defined below.

  typedef unsigned char        InputPixelType;
  typedef unsigned char        OutputPixelType;
  const   unsigned int        Dimension = 2;

  typedef itk::Image< InputPixelType,  Dimension >    InputImageType;
  typedef itk::Image< OutputPixelType, Dimension >    OutputImageType;

  //  The types for the \doxygen{ImageFileReader} and \doxygen{ImageFileWriter}
  //  are instantiated using the image types.

  typedef itk::ImageFileReader< InputImageType  >  ReaderType;
  typedef itk::ImageFileWriter< OutputImageType >  WriterType;

  //  The RegionOfInterestImageFilter type is instantiated using
  //  the input and output image types. A filter object is created with the
  //  New() method and assigned to a \doxygen{SmartPointer}.

  typedef itk::RegionOfInterestImageFilter< InputImageType,
                                            OutputImageType > FilterType;

  FilterType::Pointer filter = FilterType::New();

  //  The RegionOfInterestImageFilter requires a region to be
  //  defined by the user. The region is specified by an \doxygen{Index}
  //  indicating the pixel where the region starts and an \doxygen{Size}
  //  indicating how many pixels the region has along each dimension. In this
  //  example, the specification of the region is taken from the command line
  //  arguments (this example assumes that a 2D image is being processed).
  OutputImageType::IndexType start;
  start[0] = atoi( argv[3] );
  start[1] = atoi( argv[4] );

  OutputImageType::SizeType size;
  size[0] = atoi( argv[5] );
  size[1] = atoi( argv[6] );

  //  An ImageRegion object is created and initialized with start
  //  and size obtained from the command line.

  OutputImageType::RegionType desiredRegion;
  desiredRegion.SetSize(  size  );
  desiredRegion.SetIndex( start );

  //  Then the region is passed to the filter using the
  //  SetRegionOfInterest() method.
  filter->SetRegionOfInterest( desiredRegion );

  //  Below, we create the reader and writer using the New() method and
  //  assigning the result to a SmartPointer.
  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();

  const char * inputFilename  = argv[1];
  const char * outputFilename = argv[2];

  reader->SetFileName( inputFilename  );
  writer->SetFileName( outputFilename );

  //  Below we connect the reader, filter and writer to form the data
  //  processing pipeline.

  filter->SetInput( reader->GetOutput() );
  writer->SetInput( filter->GetOutput() );




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

  // diplay output region
  std::cout << reader->GetOutput()->GetLargestPossibleRegion() << std::endl;
  std::cout << reader->GetOutput()->GetOrigin() << std::endl;
  std::cout << reader->GetOutput()->GetSpacing() << std::endl;

  std::cout << filter->GetOutput()->GetLargestPossibleRegion() << std::endl;
  std::cout << filter->GetOutput()->GetOrigin() << std::endl;
  std::cout << filter->GetOutput()->GetSpacing() << std::endl;

  return EXIT_SUCCESS;
}
