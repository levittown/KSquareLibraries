/* ContourFollower.cpp -- Used to find the contour of image in a Raster object.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include  "FirstIncludes.h"
#include  <stdlib.h>
#include  <memory>
#include  <math.h>
#include  <complex>
#include  <fstream>
#include  <iostream>
#include  <string>
#include  <vector>
#include  "MemoryDebug.h"
using namespace std;

#if defined(FFTW_AVAILABLE)
#  if   defined(WIN32)
#    include  <fftw.h>
#  else
#    include  <fftw3.h>
#  endif
#else
#  include  "kku_fftw.h"
#endif



#include "ContourFollower.h"
#include "Raster.h"
#include "KKStr.h"
using namespace KKB;



const 
MovDir   movements[8]  = {{-1,  0}, // Up
                          {-1,  1}, // Up-Right
                          { 0,  1}, // Right
                          { 1,  1}, // Down-Right
                          { 1,  0}, // Down
                          { 1, -1}, // Down-Left
                          { 0, -1}, // Left,
                          {-1, -1}  // Up-Left
                         };




ContourFollower::ContourFollower (Raster&  _raster,
                                  RunLog&  _log
                                 ):
  curCol  (-1),
  curRow  (-1),
  fromDir (),
  height  (_raster.Height ()),
  lastDir (0),
  log     (_log),
  raster  (_raster),
  rows    (_raster.Rows   ()),
  width   (_raster.Width  ())
{
}



uchar  ContourFollower::PixelValue (int32 row,
                                    int32 col
                                   )
{
  if  ((row < 0)  ||  (row >= height))  return 0;
  if  ((col < 0)  ||  (col >= width))   return 0;
  return  rows[row][col];
}


int32  ContourFollower::PixelCountIn9PixelNeighborhood (int32  row, 
                                                      int32  col
                                                     )
{
  int32  count = 0;
  if  (PixelValue (row - 1, col - 1) > 0)  ++count;
  if  (PixelValue (row - 1, col    ) > 0)  ++count;
  if  (PixelValue (row - 1, col + 1) > 0)  ++count;
  if  (PixelValue (row    , col - 1) > 0)  ++count;
  if  (PixelValue (row    , col    ) > 0)  ++count;
  if  (PixelValue (row    , col + 1) > 0)  ++count;
  if  (PixelValue (row + 1, col - 1) > 0)  ++count;
  if  (PixelValue (row + 1, col    ) > 0)  ++count;
  if  (PixelValue (row + 1, col + 1) > 0)  ++count;
  return  count;
}



void  ContourFollower::GetFirstPixel (int32&  startRow,
                                      int32&  startCol
                                     )
{
  lastDir = 1;
  fromDir = 0;

  startRow = height / 2;
  startCol = 0;

  while  ((startCol < width)  &&  (rows[startRow][startCol] == 0))
  {
    startCol++;
  }

  if  (startCol >= width)
  {
    // We did not find a Pixel in the Middle Row(height / 2) so now we will
    // scan the image row, by row, Left to Right until we find an occupied 
    // pixel.

    bool  found = false;
    int32  row;
    int32  col;

    for  (row = 0; ((row < height)  &&  (!found));  row++)
    {
      for  (col = 0; ((col < width)  &&  (!found)); col++)
      {
        if  (rows[row][col] != 0)
        {
          found = true;
          startRow = row;
          startCol = col;
        }
      }
    }

    if  (!found)
    {
      startRow = startCol = -1;
      return;
    }
  }

  curRow = startRow;
  curCol = startCol;

}  /* GetFirstPixel */



void  ContourFollower::GetNextPixel (int32&  nextRow,
                                     int32&  nextCol
                                    )
{
  fromDir = lastDir + 4;

  if  (fromDir > 7)
    fromDir = fromDir - 8;

  bool  nextPixelFound = false;

  int32  nextDir = fromDir + 2;

  while  (!nextPixelFound)
  {
    if  (nextDir > 7)
      nextDir = nextDir - 8;

    nextRow = curRow + movements[nextDir].row;
    nextCol = curCol + movements[nextDir].col;

    if  ((nextRow <  0)       ||  
         (nextRow >= height)  ||
         (nextCol <  0)       ||
         (nextCol >= width) 
        )
    {
      nextDir++;
    }
    else if  (rows[nextRow][nextCol] > 0)
    {
      nextPixelFound = true;
    }
    else
    {
      nextDir++;
    }
  }

  curRow = nextRow;
  curCol = nextCol;

  lastDir  = nextDir;
}  /* GetNextPixel */




float  CalcMagnitude (fftw_complex*  dest,
                       int32          index
                      )
{
  float  mag = 0.0f;

  #ifdef  FFTW3_H
    mag = (float)sqrt (dest[index][0] * dest[index][0] + dest[index][1] * dest[index][1]);
  #else
    mag = (float)sqrt (dest[index].re * dest[index].re + dest[index].im * dest[index].im);
  #endif

  return  mag;
}  /* CalcMagnitude */




int32  ContourFollower::FollowContour (float*  countourFreq,
                                       float   fourierDescriptors[15],
                                       int32   totalPixels,
                                       bool&   successful
                                      )
{
  // startRow and startCol is assumed to come from the left (6)

  // I am having a problem;  
  //     We sometimes locate a separate piece of the image and end of with a contour of only that
  //     one piece.  This results in two problems.
  //                    1) The Fourier Descriptors do not even begin to represent the image
  //                    2) If the number of border pixels are small enough; then we could end
  //                       up causing a memory access fault when we access a negative index.


  int32  numOfBuckets = 5;

  int32  startRow;
  int32  startCol;

  int32  scndRow;
  int32  scndCol;

  int32  lastRow;
  int32  lastCol;

  int32  nextRow;
  int32  nextCol;

  int32  absoluteMaximumEdgePixels = totalPixels * 3;

  int32  maxNumOfBorderPoints = 3 * (height + width);
  int32  numOfBorderPixels = 0;

  int32 x;

  successful = true;

  int32  totalRow = 0;
  int32  totalCol = 0;

  #ifdef  FFTW3_H
     fftw_complex*  src = (fftw_complex*)fftw_malloc (sizeof (fftw_complex) * maxNumOfBorderPoints);
  #else
     fftw_complex*  src = new fftw_complex[maxNumOfBorderPoints];
  #endif

  GetFirstPixel (startRow, startCol);
  if  ((startRow < 0)  ||  (startCol < 0)  ||  (PixelCountIn9PixelNeighborhood (startRow, startCol) < 2))
  {
    cout << "Very Bad Starting Point" << std::endl;
    successful = false;
    return 0;
  }

  GetNextPixel (scndRow, scndCol);

  nextRow = scndRow;
  nextCol = scndCol;

  lastRow = startRow;
  lastCol = startCol;


  while  (true)  
  {
    if  (numOfBorderPixels > absoluteMaximumEdgePixels)
    {
      // Something must be wrong;  somehow we missed the starting point.

      log.Level (-1) << endl << endl << endl
                     << "ContourFollower::FollowContour   ***ERROR***" << endl
                     << endl
                     << "     We exceeded the absolute maximum number of possible edge pixels." << endl
                     << endl
                     << "     FileName          [" << raster.FileName () << "]" << endl
                     << "     numOfBorderPixels [" << numOfBorderPixels  << "]" << endl
                     << endl;

      ofstream r ("c:\\temp\\ContourFollowerFollowContour.log", std::ios_base::ate);
      r << "FileName  [" << raster.FileName ()        << "]  ";  
      r << "totalPixels [" << totalPixels             << "]  ";
      r << "numOfBorderPixels [" << numOfBorderPixels << "]";
      r << std::endl;
      r.close ();
      break;
    }

    lastRow = nextRow;
    lastCol = nextCol;

    GetNextPixel (nextRow, nextCol);

    if  ((nextRow == scndRow)   &&  (nextCol == scndCol)  &&
         (lastRow == startRow)  &&  (lastCol == startCol))
    {
      break;
    }

    if  (numOfBorderPixels >= maxNumOfBorderPoints)
    {
      int32  newMaxNumOfAngles = maxNumOfBorderPoints * 2;

      #ifdef  FFTW3_H
        fftw_complex*  newSrc = (fftw_complex*)fftw_malloc (sizeof (fftw_complex) * newMaxNumOfAngles);
      #else
        fftw_complex*  newSrc = new fftw_complex[newMaxNumOfAngles];
      #endif
      
      if  (newSrc == NULL)
      {
        log.Level (-1) << endl << endl
                       << "FollowContour     ***ERROR***" << endl
                       << endl
                       << "Could not allocate memory needed for contour points." << endl
                       << endl;
        successful = false;
        return 0;
      }

      int32  x;
      for  (x = 0; x < maxNumOfBorderPoints; x++)
      {
        #ifdef  FFTW3_H
          newSrc[x][0] = src[x][0];
          newSrc[x][1] = src[x][1];
        #else
          newSrc[x].re = src[x].re;
          newSrc[x].im = src[x].im;
        #endif
      }
        
  
      #ifdef  FFTW3_H
        fftw_free (src);
      #else
        delete src;
      #endif
      
      src = newSrc;
      maxNumOfBorderPoints = newMaxNumOfAngles;
    }
  
    #ifdef  FFTW3_H
      src[numOfBorderPixels][0] = (float)nextRow; 
      src[numOfBorderPixels][1] = (float)nextCol;
    #else
      src[numOfBorderPixels].re = (float)nextRow; 
      src[numOfBorderPixels].im = (float)nextCol;
    #endif

    totalRow += nextRow;
    totalCol += nextCol;

    numOfBorderPixels++;
  }

  float  centerRow = (float)totalRow / (float)numOfBorderPixels;
  float  centerCol = (float)totalCol / (float)numOfBorderPixels;

  float  totalRe = 0.0;
  float  totalIm = 0.0;

  for  (x = 0;  x < numOfBorderPixels;  x++)
  {
    #ifdef  FFTW3_H
      src[x][0] = src[x][0] - centerRow;
      src[x][1] = src[x][1] - centerCol;
    #else
      src[x].re = src[x].re - centerRow;
      src[x].im = src[x].im - centerCol;
    #endif

    #ifdef  FFTW3_H
      totalRe+= (float)src[x][0];
      totalIm+= (float)src[x][1];
    #else
      totalRe+= (float)src[x].re;
      totalIm+= (float)src[x].im;
    #endif
  }

  #ifdef  FFTW3_H
    fftw_complex*   dest = (fftw_complex*)fftw_malloc (sizeof (fftw_complex) * maxNumOfBorderPoints);
  #else
    fftw_complex*   dest = new fftw_complex[maxNumOfBorderPoints];
  #endif

  fftw_plan       plan;

  int32  numOfedgePixels = numOfBorderPixels;

  #ifdef  FFTW3_H
    plan = fftw_plan_dft_1d (numOfBorderPixels, src, dest, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute (plan);
  #else
    plan = fftw_create_plan (numOfBorderPixels, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_one (plan, src, dest);
  #endif

  fftw_destroy_plan (plan);  

  int32*  count = new int32 [numOfBuckets];

  for  (x = 0;  x < numOfBuckets;  x++)
  {
    countourFreq[x] = 0.0f;
    count[x] = 0;
  }

  // Original Region Areas,  as reflected in ICPR paper
  float  middle = numOfBorderPixels / (float)2.0;
  float  r1 = middle / (float)2.0;
  float  r2 = middle * ( (float)3.0  /   (float)4.0);
  float  r3 = middle * ( (float)7.0  /   (float)8.0);
  float  r4 = middle * ((float)15.0  /  (float)16.0);

  //float  middle = numOfBorderPixels / (float)2.0;
  //float  r1 = middle / (float)2.0;
  //float  r2 = middle * ( (float)7.0  /   (float)8.0);
  //float  r3 = middle * ( (float)15.0  /   (float)16.0);
  //float  r4 = middle * ((float)31.0  /  (float)32.0);


//  // Format 14
//  float  middle = numOfBorderPixels / (float)2.0;
//  float  r1 = middle * ((float)16.0  / (float)64.0);
//  float  r2 = middle * ((float)32.0  / (float)64.0);
//  float  r3 = middle * ((float)48.0  / (float)64.0);
//  float  r4 = middle * ((float)56.0  / (float)64.0);
//  float  r5 = middle * ((float)60.0  / (float)64.0);
//  float  r6 = middle * ((float)62.0  / (float)64.0);
//  float  r7 = middle * ((float)63.0  / (float)64.0);

//  // Format 15
//  float  middle = numOfBorderPixels / (float)2.0;
//  float  r1 = middle * ((float)32.0  / (float)64.0);    // 1/2
//  float  r2 = middle * ((float)48.0  / (float)64.0);    // 3/4
//  float  r3 = middle * ((float)56.0  / (float)64.0);    
//  float  r4 = middle * ((float)60.0  / (float)64.0);
//  float  r5 = middle * ((float)61.0  / (float)64.0);
//  float  r6 = middle * ((float)62.0  / (float)64.0);
//  float  r7 = middle * ((float)63.0  / (float)64.0);

//  // Format 16
//  float  middle = numOfAngles / (float)2.0;
//  float  r1 = middle / (float)2.0;
//  float  r2 = middle * (  (float)3.0  /    (float)4.0);
//  float  r3 = middle * (  (float)7.0  /    (float)8.0);
//  float  r4 = middle * ( (float)15.0  /   (float)16.0);
//  float  r5 = middle * ( (float)31.0  /   (float)32.0);
//  float  r6 = middle * ( (float)63.0  /   (float)64.0);
//  float  r7 = middle * ((float)127.0  /  (float)128.0);


//  float  middle = numOfBorderPixels / (float)2.0;
//  float  r1 = middle * ((float)1.0 / (float)8.0);
//  float  r2 = middle * ((float)2.0 / (float)8.0);
//  float  r3 = middle * ((float)3.0 / (float)8.0);
//  float  r4 = middle * ((float)4.0 / (float)8.0);
//  float  r5 = middle * ((float)5.0 / (float)8.0);
//  float  r6 = middle * ((float)6.0 / (float)8.0);
//  float  r7 = middle * ((float)7.0 / (float)8.0);

  
//  // Format 17
//  float  middle = numOfBorderPixels / (float)2.0;
//  float  r1 = middle * ((float)43.0  / (float)50.0);    // 1/2
//  float  r2 = middle * ((float)44.0  / (float)50.0);    // 3/4
//  float  r3 = middle * ((float)45.0  / (float)50.0);    
//  float  r4 = middle * ((float)46.0  / (float)50.0);
//  float  r5 = middle * ((float)47.0  / (float)50.0);
//  float  r6 = middle * ((float)48.0  / (float)50.0);
//  float  r7 = middle * ((float)49.0  / (float)50.0);

//  // Format 25
//  float  middle = numOfBorderPixels / (float)2.0;
//  float  r0 = middle * ((float)50.0  /  (float)100.0);
//  float  r1 = middle * ((float)60.0  /  (float)100.0);
//  float  r2 = middle * ((float)70.0  /  (float)100.0);
//  float  r3 = middle * ((float)80.0  /  (float)100.0);
//  float  r4 = middle * ((float)86.0  /  (float)100.0);
//  float  r5 = middle * ((float)90.0  /  (float)100.0);
//  float  r6 = middle * ((float)94.0  /  (float)100.0);
//  float  r7 = middle * ((float)96.0  /  (float)100.0);
//  float  r8 = middle * ((float)98.0  /  (float)100.0);
//  float  r9 = middle * ((float)99.0  /  (float)100.0);


  // Format 26
  //float  middle = numOfBorderPixels / (float)2.0;
  //float  r0 = middle * ((float)50.0  /  (float)100.0);
  //float  r1 = middle * ((float)60.0  /  (float)100.0);
  //float  r2 = middle * ((float)70.0  /  (float)100.0);
  //float  r3 = middle * ((float)80.0  /  (float)100.0);
  //float  r4 = middle * ((float)86.0  /  (float)100.0);
  //float  r5 = middle * ((float)90.0  /  (float)100.0);
  //float  r6 = middle * ((float)94.0  /  (float)100.0);
  //float  r7 = middle * ((float)96.0  /  (float)100.0);
  //float  r8 = middle * ((float)98.0  /  (float)100.0);
  //float  r9 = middle * ((float)99.0  /  (float)100.0);


  float  deltaX;
  float  mag;

  if  (numOfBorderPixels < 8)
  {
    successful = false;
  }
  else
  {
    float  normalizationFactor = CalcMagnitude (dest, 1);

    fourierDescriptors[ 0] = CalcMagnitude (dest, numOfBorderPixels - 1) / normalizationFactor;

    fourierDescriptors[ 1] = CalcMagnitude (dest, 2)                     / normalizationFactor;
    fourierDescriptors[ 2] = CalcMagnitude (dest, numOfBorderPixels - 2) / normalizationFactor;

    fourierDescriptors[ 3] = CalcMagnitude (dest, 3)                     / normalizationFactor;
    fourierDescriptors[ 4] = CalcMagnitude (dest, numOfBorderPixels - 3) / normalizationFactor;

    fourierDescriptors[ 5] = CalcMagnitude (dest, 4)                     / normalizationFactor;
    fourierDescriptors[ 6] = CalcMagnitude (dest, numOfBorderPixels - 4) / normalizationFactor;

    fourierDescriptors[ 7] = CalcMagnitude (dest, 5)                     / normalizationFactor;
    fourierDescriptors[ 8] = CalcMagnitude (dest, numOfBorderPixels - 5) / normalizationFactor;

    fourierDescriptors[ 9] = CalcMagnitude (dest, 6)                     / normalizationFactor;
    fourierDescriptors[10] = CalcMagnitude (dest, numOfBorderPixels - 6) / normalizationFactor;

    fourierDescriptors[11] = CalcMagnitude (dest, 7)                     / normalizationFactor;
    fourierDescriptors[12] = CalcMagnitude (dest, numOfBorderPixels - 7) / normalizationFactor;

    fourierDescriptors[13] = CalcMagnitude (dest, 8)                     / normalizationFactor;
    fourierDescriptors[14] = CalcMagnitude (dest, numOfBorderPixels - 8) / normalizationFactor;
  }

  for  (x = 1; x < (numOfBorderPixels - 1); x++)
  {
    //mag = log (sqrt (dest[x].re * dest[x].re + dest[x].im * dest[x].im));
    //mag = log (sqrt (dest[x].re * dest[x].re + dest[x].im * dest[x].im) + 1.0);
 
    mag = CalcMagnitude (dest, x);

    deltaX =  ((float)x - middle);

    //if  (deltaX < r0)
    //  continue;

    if  (deltaX < r1)
    {
      countourFreq[0] = countourFreq[0] + mag;
      count[0]++;
    }

    else if  (deltaX < r2)
    {
      countourFreq[1] = countourFreq[1] + mag;
      count[1]++;
    }

    else if  (deltaX < r3)
    {
      countourFreq[2] = countourFreq[2] + mag;
      count[2]++;
    }

    else if  (deltaX < r4)
    {
      countourFreq[3] = countourFreq[3] + mag;
      count[3]++;
    }

    else
    {
      countourFreq[4] = countourFreq[4] + mag;
      count[4]++;
    }

/*
    else if  (deltaX < r5)
    {
      countourFreq[4] = countourFreq[4] + mag;
      count[4]++;
    }

    else if  (deltaX < r6)
    {
      countourFreq[5] = countourFreq[5] + mag;
      count[5]++;
    }

    else if  (deltaX < r7)
    {
      countourFreq[6] = countourFreq[6] + mag;
      count[6]++;
    }

    else if  (deltaX < r8)
    {
      countourFreq[7] = countourFreq[7] + mag;
      count[7]++;
    }

    else if  (deltaX < r9)
    {
      countourFreq[8] = countourFreq[8] + mag;
      count[8]++;
    }

    else
    {
      countourFreq[9] = countourFreq[9] + mag;
      count[9]++;
    }
*/
  }

  for  (x = 0; x < numOfBuckets; x++)
  {
    if  (count[x] <= 0)
    {
      countourFreq[x] = 0.0;
    }
    else
    {
      countourFreq[x] = countourFreq[x] / (float)count[x];
    }
  }

  delete  src;
  delete  dest;
  delete  count;

  return  numOfedgePixels;
}  /* FollowContour */





int32  ContourFollower::FollowContour2 (float*  countourFreq,
                                        bool&   successful
                                       )
{
  // A diff Version of FollowContour

  // startRow and startCol is assumed to come from the left (6)

  int32  numOfBuckets = 16;

  int32  startRow;
  int32  startCol;

  int32  scndRow;
  int32  scndCol;

  int32  lastRow;
  int32  lastCol;

  int32  nextRow;
  int32  nextCol;

  int32  maxNumOfBorderPoints = 3 * (height + width);
  int32  numOfBorderPixels = 0;

  int32 x;

  successful = true;

  int32  totalRow = 0;
  int32  totalCol = 0;

  fftw_complex*  src = new fftw_complex[maxNumOfBorderPoints];

  GetFirstPixel (startRow, startCol);
  if  ((startRow < 0)  ||  (startCol < 0)  ||  (PixelCountIn9PixelNeighborhood (startRow, startCol) < 2))
  {
    cout << "Very Bad Starting Point" << std::endl;
    successful = false;
    return 0;
  }

  GetNextPixel (scndRow, scndCol);

  nextRow = scndRow;
  nextCol = scndCol;

  lastRow = startRow;
  lastCol = startCol;

  while  (true)  
  {
    lastRow = nextRow;
    lastCol = nextCol;

    GetNextPixel (nextRow, nextCol);

    if  ((nextRow == scndRow)   &&  (nextCol == scndCol)  &&
         (lastRow == startRow)  &&  (lastCol == startCol))
    {
      break;
    }

    if  (numOfBorderPixels >= maxNumOfBorderPoints)
    {
      int32  newMaxNumOfAngles = maxNumOfBorderPoints * 2;
      fftw_complex*  newSrc = new fftw_complex[newMaxNumOfAngles];

      int32  x;
      for  (x = 0; x < maxNumOfBorderPoints; x++)
      {
        #ifdef  FFTW3_H
          newSrc[x][0] = src[x][0];
          newSrc[x][1] = src[x][1];
        #else
          newSrc[x].re = src[x].re;
          newSrc[x].im = src[x].im;
        #endif
      }
        
      delete src;
      src = newSrc;
      maxNumOfBorderPoints = newMaxNumOfAngles;
    }

    #ifdef  FFTW3_H
      src[numOfBorderPixels][0] = (float)nextRow; 
      src[numOfBorderPixels][1] = (float)nextCol;
    #else
      src[numOfBorderPixels].re = (float)nextRow; 
      src[numOfBorderPixels].im = (float)nextCol;
    #endif

    totalRow += nextRow;
    totalCol += nextCol;

    numOfBorderPixels++;
  }

  float  centerRow = (float)totalRow / (float)numOfBorderPixels;
  float  centerCol = (float)totalCol / (float)numOfBorderPixels;

  float  totalRe = 0.0;
  float  totalIm = 0.0;

  for  (x = 0;  x < numOfBorderPixels;  x++)
  {
    #ifdef  FFTW3_H
      src[x][0] = src[x][0] - centerRow;
      src[x][1] = src[x][1] - centerCol;
      totalRe+= (float)src[x][0];
      totalIm+= (float)src[x][1];
    #else
      src[x].re = src[x].re - centerRow;
      src[x].im = src[x].im - centerCol;
      totalRe+= (float)src[x].re;
      totalIm+= (float)src[x].im;
    #endif
  }

  int32  numOfedgePixels = numOfBorderPixels;

  #ifdef  FFTW3_H
    fftw_complex*   dest = (fftw_complex*)fftw_malloc (sizeof (fftw_complex) * maxNumOfBorderPoints);
    fftw_plan       plan;

    plan = fftw_plan_dft_1d (numOfBorderPixels, src, dest, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute (plan);
    fftw_destroy_plan (plan);
    plan = NULL;
  
#else

    fftw_complex*   dest = new fftw_complex[maxNumOfBorderPoints];
    fftw_plan       plan;

    plan = fftw_create_plan (numOfBorderPixels, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_one (plan, src, dest);
    fftw_destroy_plan (plan);
    plan = NULL;
  #endif


  int32*  count = new int32 [numOfBuckets];

  for  (x = 0; x < numOfBuckets; x++)
  {
    countourFreq[x] = 0.0;
    count[x] = 0;
  }



//  // Format 28
//  float  middle = (float)numOfBorderPixels / (float)2.0;

//  float  r0 = middle * (  (float)1.0  /    (float)4.0);
//  float  r1 = middle * (  (float)1.0  /    (float)2.0);
//  float  r2 = middle * (  (float)3.0  /    (float)4.0);
//  float  r3 = middle * (  (float)7.0  /    (float)8.0);
//  float  r4 = middle * ( (float)15.0  /   (float)16.0);
//  float  r5 = middle * ( (float)31.0  /   (float)32.0);
//  float  r6 = middle * ( (float)63.0  /   (float)64.0);
//  float  r7 = middle * ((float)127.0  /  (float)128.0);



  // Format 29
  float  middle = (float)numOfBorderPixels / (float)2.0;

  float  r0 = middle * ( (float) 8.0  /   (float)16.0);
  float  r1 = middle * ( (float)10.0  /   (float)16.0);
  float  r2 = middle * ( (float)12.0  /   (float)16.0);
  float  r3 = middle * ( (float)13.0  /   (float)16.0);



//  // Format ??   Never Used This One
//  float  middle = numOfBorderPixels / (float)2.0;
//  float  r0 = middle * ((float) 68.0  /  (float)100.0);
//  float  r1 = middle * ((float) 76.0  /  (float)100.0);
//  float  r2 = middle * ((float) 84.0  /  (float)128.0);
//  float  r3 = middle * ((float) 92.0  /  (float)128.0);
//  float  r4 = middle * ((float)100.0  /  (float)128.0);
//  float  r5 = middle * ((float)108.0  /  (float)128.0);
//  float  r6 = middle * ((float)116.0  /  (float)128.0);
//  float  r7 = middle * ((float)124.0  /  (float)128.0);
//  float  r8 = middle * ((float)126.0  /  (float)128.0);
//  float  r9 = middle * ((float)127.0  /  (float)128.0);


  float  deltaX;
  float  mag;

  int32  region = 0;

  for  (x = 1; x < numOfBorderPixels; x++)
  {
    //mag = log (sqrt (dest[x].re * dest[x].re + dest[x].im * dest[x].im));
    //mag = log (sqrt (dest[x].re * dest[x].re + dest[x].im * dest[x].im) + 1.0);
 

    #ifdef  FFTW3_H
      mag = (float)sqrt (dest[x][0] * dest[x][0] + dest[x][1] * dest[x][1]);
    #else
      mag = (float)sqrt (dest[x].re * dest[x].re + dest[x].im * dest[x].im);
    #endif


    deltaX = (float)x - middle;

    if  (FloatAbs (deltaX) < r0)
      continue;

    if  (deltaX < 0)
    {
      // We are on the Left half.
      deltaX = FloatAbs (deltaX);

      if       (x == 1)        region = 0;
      else if  (x == 2)        region = 1;
      else if  (x == 3)        region = 2;
      else if  (x == 4)        region = 3;
      else if  (deltaX >= r3)  region = 4;
      else if  (deltaX >= r2)  region = 5;
      else if  (deltaX >= r1)  region = 6;
      else                     region = 7;
    }
    else
    {
      // We are on Right Half of array
      if       (x == (numOfBorderPixels - 1))  region = 15;
      else if  (x == (numOfBorderPixels - 2))  region = 14;
      else if  (x == (numOfBorderPixels - 3))  region = 13;
      else if  (x == (numOfBorderPixels - 4))  region = 12;
      else if  (deltaX >= r3)                  region = 11;
      else if  (deltaX >= r2)                  region = 10;
      else if  (deltaX >= r1)                  region =  9;
      else                                     region =  8;
    }

    countourFreq[region] = countourFreq[region] + mag;
    count[region]++;
  }

  for  (x = 0; x < numOfBuckets; x++)
  {
    if  (count[x] <= 0)
    {
      countourFreq[x] = 0.0;
    }
    else
    {
      countourFreq[x] = countourFreq[x] / (float)count[x];
    }
  }

  #ifdef  FFTW3_H
    fftw_free (src);
    fftw_free (dest);
  #else
    delete  src;
    delete  dest;
  #endif


  delete  count;

  return  numOfedgePixels;
}  /* FollowContour2 */




PointListPtr  ContourFollower::GenerateContourList ()
{
  // startRow and startCol is assumed to come from the left (6)

  int32  startRow;
  int32  startCol;

  int32  scndRow;
  int32  scndCol;

  int32  lastRow;
  int32  lastCol;

  int32  nextRow;
  int32  nextCol;

  PointListPtr  points = new PointList (true);

  GetFirstPixel (startRow, startCol);
  if  ((startRow < 0)  ||  (startCol < 0)  ||  (PixelCountIn9PixelNeighborhood (startRow, startCol) < 2))
  {
    cout << "Very Bad Starting Point" << std::endl;
    return  NULL;
  }

  GetNextPixel (scndRow, scndCol);

  nextRow = scndRow;
  nextCol = scndCol;

  lastRow = startRow;
  lastCol = startCol;


  while  (true)  
  {
    lastRow = nextRow;
    lastCol = nextCol;

    GetNextPixel (nextRow, nextCol);

    if  ((nextRow == scndRow)   &&  (nextCol == scndCol)  &&
         (lastRow == startRow)  &&  (lastCol == startCol))
    {
      break;
    }


    points->PushOnBack (new Point (nextRow, nextCol));
  }

  return  points;
}  /* GenerateContourList */






ComplexDouble**  GetFourierOneDimMask (int32  size)
{
  static  
  int32  curMaskSize = 0;

  static  
  ComplexDouble**  fourierMask = NULL;

  if  (size == curMaskSize)
    return  fourierMask;

  ComplexDouble  N(size, 0);
  ComplexDouble  M(size, 0);

  int32  x;

  if  (fourierMask)
  {
    for  (x = 0;  x < curMaskSize;  x++)
    {
      delete  fourierMask[x];
      fourierMask[x] = NULL;
    }

    delete  fourierMask;
    fourierMask = NULL;
  }

  fourierMask = new ComplexDouble*[size];
  for  (x = 0;  x < size;  x++)
  {
    fourierMask[x] = new ComplexDouble[size];
  }
  curMaskSize = size;


  ComplexDouble  j;
  ComplexDouble  MinusOne (-1, 0);
  j = sqrt (MinusOne);

  ComplexDouble  Pi  (3.14159265359, 0);
  ComplexDouble  One (1.0, 0);
  ComplexDouble  Two (2.0, 0);

  for  (int32 m = 0;  m < size;  m++)
  {
    complex<double>  mc (m, 0);

    for  (int32 k = 0; k < size; k++)
    {
      complex<double>  kc (k, 0);

      // double  xxx = 2 * 3.14159265359 * (double)k * (double)m / (double)size;

      // fourierMask[m][k] = exp (MinusOne * j * Two * Pi * kc * mc / M);
      fourierMask[m][k] = exp (MinusOne * j * Two * Pi * kc * mc / M);

      double  exponetPart = 2.0 * 3.14159265359 * (double)k * (double)m / (double)size;
      double  realPart = cos (exponetPart);
      double  imgPart  = -sin (exponetPart);

      if  (realPart != fourierMask[m][k].real ())
      {
        continue;
      }

      if  (imgPart != fourierMask[m][k].imag ())
      {
        continue;
      }
    }
  }

  return  fourierMask;
}  /* GetFourierOneDimMask */




ComplexDouble**   GetRevFourierOneDimMask (int32  size)  // For reverse Fourier
{
  static  
  int32  curRevMaskSize = 0;

  static  
  ComplexDouble**  revFourierMask = NULL;


  if  (size == curRevMaskSize)
    return  revFourierMask;

  ComplexDouble  N(size, 0);
  ComplexDouble  M(size, 0);

  int32  x;
  

  if  (revFourierMask)
  {
    for  (x = 0;  x < curRevMaskSize;  x++)
    {
      delete  revFourierMask[x];
      revFourierMask[x] = NULL;
    }

    delete  revFourierMask;
    revFourierMask = NULL;
  }


  revFourierMask = new ComplexDouble*[size];
  for  (x = 0;  x < size;  x++)
  {
    revFourierMask[x] = new ComplexDouble[size];
  }
  curRevMaskSize = size;

  ComplexDouble  j;
  ComplexDouble  MinusOne (-1, 0);
  ComplexDouble  PositiveOne (1, 0);
  j = sqrt (MinusOne);

  ComplexDouble  Pi  (3.1415926, 0);
  ComplexDouble  One (1.0, 0);
  ComplexDouble  Two (2.0, 0);

  for  (int32 m = 0;  m < size;  m++)
  {
    complex<double>  mc (m, 0);

    for  (int32 k = 0; k < size; k++)
    {
      complex<double>  kc (k, 0);

      // double  xxx = 2 * 3.14159265359 * (double)k * (double)m / (double)size;

      // fourierMask[m][k] = exp (MinusOne * j * Two * Pi * kc * mc / M);
      revFourierMask[m][k] = exp (PositiveOne * j * Two * Pi * kc * mc / M);


      double  exponetPart = 2.0 * 3.14159265359 * (double)k * (double)m / (double)size;
      double  realPart = cos (exponetPart);
      double  imgPart  = sin (exponetPart);

      if  (realPart != revFourierMask[m][k].real ())
      {
        continue;
      }

      if  (imgPart != revFourierMask[m][k].imag ())
      {
        continue;
      }
    }
  }

  return  revFourierMask;
}  /* GetRevFourierOneDimMask */




int32  ContourFollower::CreateFourierDescriptorBySampling (int32    numOfBuckets,
                                                           float*  countourFreq,
                                                           bool&    successful
                                                          )
{
  // startRow and startCol is assumed to come from the left (6)
  // int32  numOfBuckets = 8;
  int32  startRow;
  int32  startCol;

  int32  scndRow;
  int32  scndCol;

  int32  lastRow;
  int32  lastCol;

  int32  nextRow;
  int32  nextCol;

  int32  numOfBorderPixels = 0;

  int32 x;

  successful = true;

  PointListPtr  points = new PointList (true);

  GetFirstPixel (startRow, startCol);
  if  ((startRow < 0)  ||  (startCol < 0)  ||  (PixelCountIn9PixelNeighborhood (startRow, startCol) < 2))
  {
    cout << "Vary Bad Starting Point" << std::endl;
    successful = false;
    return  0;
  }
   
  GetNextPixel (scndRow, scndCol);

  nextRow = scndRow;
  nextCol = scndCol;

  lastRow = startRow;
  lastCol = startCol;


  while  (true)  
  {
    lastRow = nextRow;
    lastCol = nextCol;

    GetNextPixel (nextRow, nextCol);

    if  ((nextRow == scndRow)   &&  (nextCol == scndCol)  &&
         (lastRow == startRow)  &&  (lastCol == startCol))
    {
      break;
    }
 
    points->PushOnBack (new Point (nextRow, nextCol));

    numOfBorderPixels++;
  }


  ComplexDouble*  src = new ComplexDouble[numOfBuckets];

  int32  totalRow = 0;
  int32  totalCol = 0;

  for  (x = 0;  x < numOfBuckets;  x++)
  {
    int32  borderPixelIdx = (int32)(((double)x * (double)numOfBorderPixels) /  (double)numOfBuckets);

    Point&  point = (*points)[borderPixelIdx];

    src[x] = ComplexDouble ((double)point.Row (), (double)point.Col ());

    totalRow += point.Row ();
    totalCol += point.Col ();
  }

  delete  points;
  points = NULL;

  double  centerRow = (double)totalRow / (double)numOfBuckets;
  double  centerCol = (double)totalCol / (double)numOfBuckets;

  for  (x = 0;  x < numOfBuckets;  x++)
  {
    src[x] = ComplexDouble ((double)(src[x].real () - centerRow), (double)(src[x].imag () - centerCol));
  }

  ComplexDouble*  dest = new ComplexDouble[numOfBuckets];

  ComplexDouble**  fourierMask = GetFourierOneDimMask (numOfBuckets);
  ComplexDouble    zero (0, 0);

  for  (int32  l = 0;  l < numOfBuckets;  l++)
  {
    dest[l] = zero;
    for  (int32 k = 0;  k < numOfBuckets;  k++)
    {
      dest[l] = dest[l] + src[k] * fourierMask[l][k];
    }
  }
  
  for  (x = 0;  x < numOfBuckets;  x++)
  {
    countourFreq[x] = (float)(sqrt (dest[x].real () * dest[x].real () + dest[x].imag () * dest[x].imag ()));
  }

  delete  src;
  delete  dest;

  return  numOfBorderPixels;
}  /* CreateFourierDescriptorBySampling */




void  ContourFollower::HistogramDistanceFromAPointOfEdge (float   pointRow,
                                                          float   pointCol,
                                                          int32   numOfBuckets,
                                                          int32*  buckets,
                                                          float&  minDistance,
                                                          float&  maxDistance,
                                                          int32&  numOfEdgePixels
                                                         )
{
  PointListPtr  points = GenerateContourList ();

  numOfEdgePixels = points->QueueSize ();
  
  int32  x;

  minDistance = FloatMax;
  maxDistance = FloatMin;

  for  (x = 0;  x < numOfBuckets;  x++)
    buckets[x] = 0;

  float*  distances = new float[points->QueueSize ()];

  for  (x = 0;  x < points->QueueSize ();  x++)
  {
    Point& point = (*points)[x];

    float  deltaCol = (float)point.Col () - pointCol;
    float  deltaRow = (float)point.Row () - pointRow;

    float  distance = (float)sqrt (deltaCol * deltaCol  +  deltaRow  * deltaRow);

    distances[x] = distance;

    minDistance = Min (minDistance, distance);
    maxDistance = Max (maxDistance, distance);
  }


  float  bucketSize = (maxDistance - minDistance) / numOfBuckets;

  if  (bucketSize == 0.0)
  {
     buckets[numOfBuckets / 2] = points->QueueSize ();
  }
  else
  {
    for  (x = 0;  x < points->QueueSize ();  x++)
    {
      int32 bucketIDX = (int32)((distances[x] - minDistance) / bucketSize);

      buckets[bucketIDX]++;
    }
  }

  delete  points;

  delete  distances;

  return;
}  /* HistogramDistanceFromAPoint */



vector<ComplexDouble>  ContourFollower::CreateFourierFromPointList (const PointList&  points)
{
  //ComplexDouble*  src = new ComplexDouble[points.QueueSize ()];

  #ifdef  FFTW3_H
     fftw_complex*  src = (fftw_complex*)fftw_malloc (sizeof (fftw_complex) * points.QueueSize ());
  #else
     fftw_complex*  src = new fftw_complex[points.QueueSize ()];
  #endif


  int32  totalRow = 0;
  int32  totalCol = 0;

  int32  x = 0;

  for  (x = 0;  x < points.QueueSize ();  x++)
  {
    Point&  point (points[x]);

    //src[x] = ComplexDouble ((double)point.Row (), (double)point.Col ());

    #ifdef  FFTW3_H 
      src[x][0] = (double)point.Row ();
      src[x][1] = (double)point.Col ();
    #else
      src[x].re = (double)point.Row ();
      src[x].im = (double)point.Col ();
    #endif

    totalRow += point.Row ();
    totalCol += point.Col ();
  }

  //double  centerRow = (double)totalRow / (double)(points.QueueSize ());
  //double  centerCol = (double)totalCol / (double)(points.QueueSize ());

//  for  (x = 0;  x < points.QueueSize ();  x++)
//  {
//    src[x] = ComplexDouble ((double)(src[x].real () - centerRow), (double)(src[x].imag () - centerCol));
//  }
//

  #ifdef  FFTW3_H
    fftw_complex*   destFFTW = (fftw_complex*)fftw_malloc (sizeof (fftw_complex) * points.QueueSize ());
  #else
    fftw_complex*   destFFTW = new fftw_complex[points.QueueSize ()];
  #endif

  fftw_plan       plan;

  #ifdef  FFTW3_H
    plan = fftw_plan_dft_1d (points.QueueSize (), src, destFFTW, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute (plan);
  #else
    plan = fftw_create_plan (points.QueueSize (), FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_one (plan, src, destFFTW);
  #endif
  fftw_destroy_plan (plan);  

  vector<ComplexDouble>  dest;

  for  (int32  l = 0;  l < points.QueueSize ();  l++)
  {
    #ifdef  FFTW3_H
    dest.push_back (ComplexDouble (destFFTW[l][0] / (double)(points.QueueSize ()), destFFTW[l][1] / (double)(points.QueueSize ())));
    #else
    dest.push_back (ComplexDouble (destFFTW[l].re / (double)(points.QueueSize ()), destFFTW[l].im / (double)(points.QueueSize ())));
    #endif
  }
  delete  destFFTW;
  delete  src;

  return  dest;
}  /* CreateFourierFromPointList */





PointListPtr  ContourFollower::CreatePointListFromFourier (vector<ComplexDouble>  fourier,
                                                           PointList&             origPointList
                                                          )
{
  int32  minRow, maxRow, minCol, maxCol;
  origPointList.BoxCoordinites (minRow, minCol, maxRow, maxCol);

  PointListPtr  points = new PointList (true);

  size_t  numOfEdgePixels = origPointList.size ();

  #ifdef  FFTW3_H
     fftw_complex*  src = (fftw_complex*)fftw_malloc (sizeof (fftw_complex) * numOfEdgePixels);
  #else
     fftw_complex*  src = new fftw_complex[numOfEdgePixels];
  #endif

  for  (int32  l = 0;  l < (int32)fourier.size ();  l++)
  {
    #ifdef  FFTW3_H
    src[l][0] = fourier[l].real ();
    src[l][1] = fourier[l].imag ();
    #else
    src[l].re = fourier[l].real ();
    src[l].im = fourier[l].imag ();
    #endif
  }


  #ifdef  FFTW3_H
    fftw_complex*   destFFTW = (fftw_complex*)fftw_malloc (sizeof (fftw_complex) * numOfEdgePixels);
  #else
    fftw_complex*   destFFTW = new fftw_complex[numOfEdgePixels];
  #endif

  fftw_plan       plan;

  #ifdef  FFTW3_H
    plan = fftw_plan_dft_1d (numOfEdgePixels, src, destFFTW, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute (plan);
  #else
    plan = fftw_create_plan ((int32)numOfEdgePixels, FFTW_BACKWARD, FFTW_ESTIMATE);
    fftw_one (plan, src, destFFTW);
  #endif
  fftw_destroy_plan (plan);  


  int32  largestRow  = -1;
  int32  largestCol  = -1;
  int32  smallestRow = 999999;
  int32  smallestCol = 999999;

  for  (int32  l = 0;  l < (int32)fourier.size ();  l++)
  {

    #ifdef  WIN32
      ComplexDouble  z;
      z.real (destFFTW[l].re);
      z.imag (destFFTW[l].im);
    #else
      #ifdef  FFTW3_H
         double  realPart = (double)destFFTW[l][0];
         double  imagPart = (double)destFFTW[l][1];
         ComplexDouble  z (realPart, imagPart);
      #else
         double  realPart = (double)destFFTW[l].re;
         double  imagPart = (double)destFFTW[l].im;
         ComplexDouble  z (realPart, imagPart);
      #endif
    #endif

    
    //  z = z / (double)(fourier.size ());

    int32  row = (int32)(z.real () + 0.5);
    if  (row > largestRow)
      largestRow = row;
    if  (row < smallestRow)
      smallestRow = row;

    int32  col = (int32)(z.imag () + 0.5);
    if  (col > largestCol)
      largestCol = col;
    if  (col < smallestCol)
      smallestCol = col;

    PointPtr p = new Point (row, col);
    points->PushOnBack (p);
  }

  delete  src;
  delete  destFFTW;

  return  points;
}  /* CreatePointListFromFourier */