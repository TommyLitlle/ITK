/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkPadImageFilter.txx
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#ifndef _itkPadImageFilter_txx
#define _itkPadImageFilter_txx

#include "itkPadImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkObjectFactory.h"

namespace itk
{

/**
 *
 */
template <class TInputImage, class TOutputImage>
PadImageFilter<TInputImage,TOutputImage>
::PadImageFilter()
{
  for( int j = 0; j < ImageDimension; j++ )
    {
    m_PadLowerBound[j] = 0;
    m_PadUpperBound[j] = 0;
    }
}


/**
 *
 */
template <class TInputImage, class TOutputImage>
void 
PadImageFilter<TInputImage,TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os,indent);

  os << indent << "Output Pad Lower Bounds: [";

  if (ImageDimension >= 1) 
    {
      os << m_PadLowerBound[0];
    }
  for( int j = 1; j < ImageDimension; j++ )
    {
      os << ", " << m_PadLowerBound[j];
    } 
  os << "]" << std::endl;

  os << indent << "Output Pad Upper Bounds: [";
  if (ImageDimension >= 1) 
    {
      os << m_PadUpperBound[0];
    }
  for( int j = 1; j < ImageDimension; j++ )
    {
      os << ", " << m_PadUpperBound[j];
    } 
  os << "]" << std::endl;
}


/** 
 * PadImageFilter needs a smaller input requested region than
 * output requested region.  As such, PadImageFilter needs to
 * provide an implementation for GenerateInputRequestedRegion() in
 * order to inform the pipeline execution model.
 *
 * \sa ProcessObject::GenerateInputRequestedRegion() 
 */
template <class TInputImage, class TOutputImage>
void 
PadImageFilter<TInputImage,TOutputImage>
::GenerateInputRequestedRegion()
{
  long sizeTemp;

  // call the superclass' implementation of this method
  // Superclass::GenerateInputRequestedRegion();

  // get pointers to the input and output
  InputImagePointer  inputPtr = this->GetInput();
  OutputImagePointer outputPtr = this->GetOutput();

  if ( !inputPtr || !outputPtr )
    {
      return;
    }

  // we need to compute the input requested region (size and start index)
  int i;
  const typename TOutputImage::SizeType& outputRequestedRegionSize
    = outputPtr->GetRequestedRegion().GetSize();
  const typename TOutputImage::IndexType& outputRequestedRegionStartIndex
    = outputPtr->GetRequestedRegion().GetIndex();
  const typename TInputImage::SizeType& inputWholeRegionSize
    = inputPtr->GetLargestPossibleRegion().GetSize();
  const typename TInputImage::IndexType& inputWholeRegionStartIndex
    = inputPtr->GetLargestPossibleRegion().GetIndex();
  
  typename TInputImage::SizeType  inputRequestedRegionSize;
  typename TInputImage::IndexType inputRequestedRegionStartIndex;
  
  for (i = 0; i < TInputImage::ImageDimension; i++)
    {
      if (outputRequestedRegionStartIndex[i] <= inputWholeRegionStartIndex[i]) 
	{
	  inputRequestedRegionStartIndex[i] = inputWholeRegionStartIndex[i];
	}
      else
	{
	  inputRequestedRegionStartIndex[i] = 
	    outputRequestedRegionStartIndex[i];
	}

      if ((inputWholeRegionStartIndex[i]+inputWholeRegionSize[i]) <= 
	  (outputRequestedRegionStartIndex[i]+outputRequestedRegionSize[i]))
	{
	  sizeTemp = inputWholeRegionSize[i] 
	    + inputWholeRegionStartIndex[i] - inputRequestedRegionStartIndex[i];
	}
      else
	{
	  sizeTemp = outputRequestedRegionSize[i]
	    + outputRequestedRegionStartIndex[i] - inputRequestedRegionStartIndex[i];
	}

      //
      // The previous statements correctly handle overlapped regions where
      // at least some of the pixels from the input image end up reflected 
      // in the output.  When there is no overlap, the size will be negative.
      // In that case we arbitrarily pick the start of the input region
      // as the start of the output region and zero for the size.
      // 
      if (sizeTemp < 0) 
	{
	  inputRequestedRegionSize[i] = 0;
	  inputRequestedRegionStartIndex[i] = inputWholeRegionStartIndex[i];
	} else {
	  inputRequestedRegionSize[i] = sizeTemp;
	}

    }

  typename TInputImage::RegionType inputRequestedRegion;
  inputRequestedRegion.SetSize( inputRequestedRegionSize );
  inputRequestedRegion.SetIndex( inputRequestedRegionStartIndex );

  inputPtr->SetRequestedRegion( inputRequestedRegion );
}


/** 
 * PadImageFilter produces an image which is a different resolution
 * than its input image.  As such, PadImageFilter needs to
 * provide an implementation for UpdateOutputInformation() in order
 * to inform the pipeline execution model.  The original
 * documentation of this method is below.
 *
 * \sa ProcessObject::UpdateOutputInformaton() 
 */
template <class TInputImage, class TOutputImage>
void 
PadImageFilter<TInputImage,TOutputImage>
::UpdateOutputInformation()
{
  // call the superclass' implementation of this method
  Superclass::UpdateOutputInformation();

  // get pointers to the input and output
  OutputImagePointer outputPtr = this->GetOutput();
  InputImagePointer inputPtr = this->GetInput();

  if ( !outputPtr || !inputPtr)
    {
    return;
    }

  // we need to compute the output image size, and the
  // output image start index
  int i;
  typename TOutputImage::SizeType     outputSize;
  typename TOutputImage::IndexType    outputStartIndex;
  typename TInputImage::SizeType      inputSize;
  typename TInputImage::IndexType     inputStartIndex;
  
  inputSize = inputPtr->GetLargestPossibleRegion().GetSize();
  inputStartIndex = inputPtr->GetLargestPossibleRegion().GetIndex();

  for (i = 0; i < TOutputImage::ImageDimension; i++)
    {
      outputSize[i] = inputSize[i] + m_PadLowerBound[i] + m_PadUpperBound [i];
      outputStartIndex[i] = inputStartIndex[i] - (long) m_PadLowerBound[i];
    }

  typename TOutputImage::RegionType outputLargestPossibleRegion;
  outputLargestPossibleRegion.SetSize( outputSize );
  outputLargestPossibleRegion.SetIndex( outputStartIndex );

  outputPtr->SetLargestPossibleRegion( outputLargestPossibleRegion );
}

} // end namespace itk

#endif
