/* MorphOpStretcher.cpp -- Stretches image by a specified factor.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */

#include "FirstIncludes.h"
#include <stdlib.h>
#include <iostream>
#include <map>
#include <vector>
#include <math.h>
#include "MemoryDebug.h"
using namespace std;

#include "MorphOp.h"
#include "MorphOpStretcher.h"
#include "Raster.h"
using namespace KKB;



class  MorphOpStretcher::CellFactor
{
public:
  CellFactor ():  
     destCellCount   (0),
     destCellIdxs    (NULL),
     destCellFracts  (NULL)
  {}


  ~CellFactor ()
  {
    delete  destCellIdxs;    destCellIdxs   = NULL;
    delete  destCellFracts;  destCellFracts = NULL;
  }


  void  UpdateForSourceCellIdx (kkuint32  cellIdx,
                                float   factor
                               )
  {
    float  cellStart = (float)cellIdx * factor;
    float  cellEnd   = (float)(cellIdx + 1) * factor;

    kkuint32 cellStartIdx = (kkuint32)floor (cellStart);
    kkuint32 cellEndIdx   = (kkuint32)floor (cellEnd);
    destCellCount = cellEndIdx - cellStartIdx;
    if  (cellEnd > floor(cellEnd))
      ++destCellCount;
    
    delete  destCellIdxs;
    destCellIdxs = new kkuint32[destCellCount];

    delete  destCellFracts;
    destCellFracts = new float[destCellCount];

    float cellValue     = cellStart;
    float nextCellValue = cellStart;
    for  (kkuint32  idx = 0;  idx < destCellCount;  ++idx)
    {
      if  (cellValue < ceil (cellValue))
        nextCellValue = Min (cellEnd, ceilf (cellValue));
      else
        nextCellValue = Min (cellEnd, ceilf (cellValue + 1.0f));

      float  fract = nextCellValue - cellValue;
      destCellIdxs[idx]   = cellStartIdx + idx;
      destCellFracts[idx] = fract;
      cellValue = nextCellValue;
    }
  }

  kkuint32   destCellCount;
  kkuint32*  destCellIdxs;
  float*     destCellFracts; 
};  /* UpdateForSourceCellIdx */



MorphOpStretcher::MorphOpStretcher (float  _rowFactor,
                                    float  _colFactor
                                   ):
    MorphOp (),
    rowFactor        (_rowFactor),
    colFactor        (_colFactor),
    rowFactorsCount  (0),
    rowFactors       (NULL),
    colFactorsCount  (0),
    colFactors       (NULL)
{
}

    
    
MorphOpStretcher::~MorphOpStretcher ()
{
  delete[]  rowFactors;   rowFactors = NULL;
  delete[]  colFactors;   colFactors = NULL;
}



kkMemSize  MorphOpStretcher::MemoryConsumedEstimated ()
{
  kkMemSize  result = sizeof (*this) + (kkMemSize)(rowFactorsCount * sizeof (CellFactor) + colFactorsCount * sizeof (CellFactor));
  return  result;
}



RasterPtr   MorphOpStretcher::PerformOperation (RasterConstPtr  _image)
{
  this->SetSrcRaster (_image);

  kkuint32  destHeight = (kkuint32)ceil (0.5f + (float)srcHeight * rowFactor);
  kkuint32  destWidth  = (kkuint32)ceil (0.5f + (float)srcWidth  * colFactor);

  bool  color = _image->Color ();

  UpdateFactors (srcHeight, srcWidth);

  RasterPtr  result = _image->AllocateARasterInstance(destHeight, destWidth, color);

  result->BackgroundPixelTH    (_image->BackgroundPixelTH    ());
  result->BackgroundPixelValue (_image->BackgroundPixelValue ());
  result->ForegroundPixelValue (_image->ForegroundPixelValue ());

  uchar**  destRed   = result->Red   ();
  uchar**  destGreen = result->Green ();
  uchar**  destBlue  = result->Blue  ();

  for  (kkint32  srcRow = 0;  srcRow < srcHeight;  ++srcRow)
  {
    uchar*  srcRedRow   = NULL;
    uchar*  srcGreenRow = srcGreen[srcRow];
    uchar*  srcBlueRow  = NULL;
    if  (color)
    {
      srcRedRow  = srcRed [srcRow];
      srcBlueRow = srcBlue[srcRow];
    }

    CellFactor&  srcRowFactor = rowFactors[srcRow];

    for  (kkuint32  rowFactorIdx = 0;  rowFactorIdx < srcRowFactor.destCellCount;  ++rowFactorIdx)
    {
      kkuint32  destRow      = srcRowFactor.destCellIdxs  [rowFactorIdx];
      float     destRowFract = srcRowFactor.destCellFracts[rowFactorIdx];

      for  (kkint32 srcCol = 0;  srcCol < srcWidth;  ++srcCol)
      {
        uchar  srcPixelRed   = 0;
        uchar  srcPixelGreen = srcGreenRow[srcCol];
        uchar  srcPixelBlue  = 0;
        if  (color)
        {
          srcPixelRed   = srcRedRow [srcCol];
          srcPixelBlue  = srcBlueRow[srcCol];
        }

        CellFactor&  srcColFactor = colFactors[srcCol];

        for  (kkuint32  colFactorIdx = 0;  colFactorIdx < srcColFactor.destCellCount;  ++colFactorIdx)
        {
          kkuint32  destCol      = srcColFactor.destCellIdxs  [colFactorIdx];
          float     destColFract = srcColFactor.destCellFracts[colFactorIdx];

          uchar newDestG = (uchar)(destGreen[destRow][destCol] + (uchar)Min (255.0f, (float)srcPixelGreen * destRowFract * destColFract));
          destGreen[destRow][destCol] = newDestG;
          if  (color)
          {
            destRed [destRow][destCol] = (uchar)(destRed[destRow][destCol]  + (uchar)Min (255.0f, (float)srcPixelRed  * destRowFract * destColFract));
            destBlue[destRow][destCol] = (uchar)(destBlue[destRow][destCol] + (uchar)Min (255.0f, (float)srcPixelBlue * destRowFract * destColFract));
          }
        }
      }
    }
  }

  return  result;
}  /* PerformOperation */




MorphOpStretcher::CellFactorPtr  
         MorphOpStretcher::BuildCellFactors (float     factor,
                                             kkuint32  cellFactorsCount
                                            )
{
  CellFactorPtr  cellFactors = new CellFactor[cellFactorsCount];

  for  (kkuint32 x = 0;  x < cellFactorsCount;  ++x)
  {
    cellFactors[x].UpdateForSourceCellIdx (x, factor);
  }

  return  cellFactors; 
}  /* BuildCellFactors */



void  MorphOpStretcher::UpdateFactors (kkuint32  height,
                                       kkuint32  width
                                      )
{
  if  ((height + 1) > rowFactorsCount)
  {
    delete[]  rowFactors;
    rowFactors = NULL;
    rowFactorsCount = 0;
    rowFactors = BuildCellFactors (rowFactor, height + 5);
    rowFactorsCount = height + 5;
  }

  if  ((width + 1) > colFactorsCount)
  {
    delete[]  colFactors;
    colFactors = NULL;
    colFactors = BuildCellFactors (colFactor, width + 5);
    colFactorsCount = width + 5;
  }
}  /* UpdateFactors */
