/* SimpleCompressor.cpp -- A simple Run Length compression algorithm.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>

#include "MemoryDebug.h"

#include "KKBaseTypes.h"

using namespace std;
using namespace KKB;

#include "OSservices.h"
#include "SimpleCompressor.h"



SimpleCompressor::SimpleCompressor (kkuint32  estimatedMaxBuffSize)
{
  buffBytes = new uchar[estimatedMaxBuffSize];
  buffLens  = new uchar[estimatedMaxBuffSize];

  buffSize = estimatedMaxBuffSize;
  buffSpaceUsed = 0;
  lastBuffByteUsed = 0;
  growthRate = estimatedMaxBuffSize;
}



SimpleCompressor::~SimpleCompressor ()
{
  delete[]  buffBytes;
  delete[]  buffLens;
}



void  SimpleCompressor::Add16BitInt (kkuint32  i)
{
  AddByte ((uchar)(i / 256));
  AddByte ((uchar)(i % 256));
}



void  SimpleCompressor::AddByte (uchar  b)
{
  if  (buffSpaceUsed >= buffSize)
  {
    kkuint32  newBuffSize = buffSize + growthRate;
    uchar*  newBuffBytes = new uchar[newBuffSize];
    uchar*  newBuffLens = new uchar[newBuffSize];

    memcpy (newBuffBytes, buffBytes, buffSize);
    memcpy (newBuffLens,  buffLens, buffSize);
    delete  buffBytes;
    delete  buffLens;
    buffBytes = newBuffBytes;
    buffLens  = newBuffLens;
    newBuffBytes = NULL;
    newBuffLens  = NULL;
    buffSize     = newBuffSize;
  }

  if  (buffSpaceUsed == 0)
  {
    buffBytes[0]     = b;
    buffLens [0]     = 1;
    lastBuffByteUsed = 0;
    buffSpaceUsed    = 1;

    return;
  }

  if  (b == buffBytes[lastBuffByteUsed])
  {
    if  (buffLens[lastBuffByteUsed] > 126)
    {
      buffBytes[buffSpaceUsed] = b;
      buffLens [buffSpaceUsed] = 1;
      lastBuffByteUsed = buffSpaceUsed;
      buffSpaceUsed++;
    }
    else
    {
      buffLens[lastBuffByteUsed]++;
    }
  }
  else
  {
    buffBytes[buffSpaceUsed] = b;
    buffLens [buffSpaceUsed] = 1;
    lastBuffByteUsed = buffSpaceUsed;
    buffSpaceUsed++;
  }
} /* AddByte */




kkuint32  SimpleCompressor::CalcCompressedBytesNeeded ()
{
  kkuint32  compressedBuffUsed = 0;

  compressedBuffUsed++;  // AddByteToCmpressedBuffer (compressedBuff, compressedBuffUsed, compressedBytesNeeded / 255);
  compressedBuffUsed++;  // AddByteToCmpressedBuffer (compressedBuff, compressedBuffUsed, compressedBytesNeeded % 255);

  kkuint32  idx = 0;

  while  (idx < buffSpaceUsed)
  {
    while  ((idx < buffSpaceUsed)  &&  (buffLens[idx] > 1))
    {
      compressedBuffUsed++;  //AddByteToCmpressedBuffer (compressedBuff, compressedBuffUsed, 128 + bufLens[idx]);
      compressedBuffUsed++;  //AddByteToCmpressedBuffer (compressedBuff, compressedBuffUsed, 128 + bufBytes[idx]);

      idx++;
    }

    while  ((idx < buffSpaceUsed)  &&  (buffLens[idx] <= 1))
    {
      // We have some raw bytes to add.
      // Lets look forward to see how many


      kkuint32  endIdx = idx;
      kkuint32  rawBytesInARow = 0;
      while  ((endIdx < buffSpaceUsed)  &&  (buffLens[endIdx] <= 1)  &&  (rawBytesInARow < 127))
      {
        endIdx++;
        rawBytesInARow++;
      }

      endIdx--;

      
      compressedBuffUsed++;  //AddByteToCmpressedBuffer (compressedBuff, compressedBuffUsed, 0 + rawBytesInARow);
      for  (kkuint32 x = 0;   x < rawBytesInARow;  x++)
      {
        compressedBuffUsed++;  // AddByteToCmpressedBuffer (compressedBuff, compressedBuffUsed, buffBytes[idx]);
        idx++;
      }
    }
  }

  return  compressedBuffUsed;
}  /* CalcCompressedBytesNeeded*/






void    SimpleCompressor::AddByteToCmpressedBuffer (uchar*  compressedBuff, 
                                                    kkuint32& compressedBuffUsed,
                                                    uchar   b
                                                    )
{
  compressedBuff[compressedBuffUsed] = b;
  compressedBuffUsed++;
}



uchar*  SimpleCompressor::CreateCompressedBuffer (kkuint32&  compressedBuffserSize)
{
  kkuint32  compressedBytesNeeded = CalcCompressedBytesNeeded ();

  if  (compressedBytesNeeded > (256 * 256))
  {
    ofstream  f ("c:\\Temp\\SimpleCompressor.txt", ios_base::app);
    f << std::endl
      << osGetLocalDateTime () << "\t"
      << "SimpleCompressor::CreateCompressedBuffer     ***ERROR***     compressedBytesNeeded[" << compressedBytesNeeded << "] to large." << std::endl
      << std::endl;
    f.flush ();
    f.close ();
    compressedBuffserSize = compressedBytesNeeded;
    return NULL;
  }


  uchar*  compressedBuff = new uchar[compressedBytesNeeded];
  kkuint32  compressedBuffUsed = 0;

  AddByteToCmpressedBuffer (compressedBuff, compressedBuffUsed, (uchar)(compressedBytesNeeded / 256));
  AddByteToCmpressedBuffer (compressedBuff, compressedBuffUsed, (uchar)(compressedBytesNeeded % 256));

  kkuint32  idx = 0;

  while  (idx < buffSpaceUsed)
  {
    while  ((idx < buffSpaceUsed)  &&  (buffLens[idx] > 1))
    {
      AddByteToCmpressedBuffer (compressedBuff, compressedBuffUsed, (uchar)(128 + buffLens[idx]));
      AddByteToCmpressedBuffer (compressedBuff, compressedBuffUsed, buffBytes[idx]);
      idx++;
    }

    while  ((idx < buffSpaceUsed)  &&  (buffLens[idx] <= 1))
    {
      // We have some raw bytes to add.
      // Lets look forward to see how many

      kkuint32  endIdx = idx;
      kkuint32  rawBytesInARow = 0;
      while  ((endIdx < buffSpaceUsed)  &&  (buffLens[endIdx] <= 1)  &&  (rawBytesInARow < 127))
      {
        endIdx++;
        rawBytesInARow++;
      }

      endIdx--;
      
      AddByteToCmpressedBuffer (compressedBuff, compressedBuffUsed, (uchar)(0 + rawBytesInARow));
      for  (kkuint32 x = 0;   x < rawBytesInARow;  x++)
      {
        AddByteToCmpressedBuffer (compressedBuff, compressedBuffUsed, buffBytes[idx]);
        idx++;
      }
    }
  }

  if  (compressedBuffUsed != compressedBytesNeeded)
  {
    // This is not a good thing; something has damaged the compressed string.
    ofstream  f ("c:\\Temp\\SimpleCompressor.txt", ios_base::app);
    f << std::endl
      << osGetLocalDateTime () << "\t" << "SimpleCompressor::CreateCompressedBuffer    ***ERROR***"
      << std::endl
      << "               "     << "\t" << "Something has gone terribly wrong with the simpleCompression algorithm." << std::endl
      << std::endl;
    f.flush ();
    f.close ();
  }


  compressedBuffserSize = compressedBuffUsed;

  return  compressedBuff;
}   /* CreateCompressedBuffer*/





uchar*   SimpleCompressor::Decompress (const uchar*  compressedBuff,
                                       kkuint32      compressedBuffLen,
                                       kkuint32&       unCompressedSize
                                      )
{
  if  ((compressedBuff == NULL)  ||  ((compressedBuffLen < 2)))
    return  NULL;

  // First two bytes specifies the total number of compressed bytes including the first two bytes.
  kkuint32  compressedBuffSize = compressedBuff[0] * 256 + compressedBuff[1];

  if  (compressedBuffSize > compressedBuffLen)
  {
    ofstream  f ("c:\\Temp\\SimpleCompressor.txt", ios_base::app);
    f << std::endl
      << osGetLocalDateTime ()  << "\t"
      << "compressedBuffSize[" << compressedBuffSize << "] > compressedBuffLen[" << compressedBuffLen << "]" << "\t" 
      << std::endl;
    f.flush ();
    f.close ();

    compressedBuffSize = compressedBuffLen;
  }

  kkuint32  nextByte = 2;

  // Calculate the number of uncompressed bytes needed.  Which is done by decompressing the data.
  unCompressedSize = 0;
  while  (nextByte < (compressedBuffSize - 1))  // There has to be at least 2 bytes in each group
  {
    uchar  controlByte = compressedBuff[nextByte];
    nextByte += 1;

    if  (controlByte < 128)
    {
      // Raw data that consists of 'controlByte' bytes.
      unCompressedSize = unCompressedSize + controlByte;
      nextByte = nextByte + controlByte;
    }
    else
    {
      // Run Length data;  next byte represents the byte that is to be repeated 'controByte' times.
      unCompressedSize = unCompressedSize + (controlByte % 128);
      nextByte = nextByte + 1;
    }
  }

  if  (nextByte > compressedBuffSize)
  {
    // This is not a good thing; something has damaged the compressed string.
    ofstream  f ("c:\\Temp\\SimpleCompressor.txt", ios_base::app);
    f << std::endl
      << osGetLocalDateTime ()  << "\t"
      << "nextByte[" <<  nextByte << "] > compressedBuffSize[" << compressedBuffSize << "]"
      << std::endl;
    f.flush ();
    f.close ();
  }

  uchar*  unCompressedBuff = new uchar[unCompressedSize];
  memset (unCompressedBuff, 0, unCompressedSize);

  nextByte = 2;

  kkuint32  unCompressedBuffIDX = 0;

  while  (nextByte < (compressedBuffSize - 1))
  {
    uchar  controlByte = compressedBuff[nextByte];
    if  (controlByte < 128)
    {
      // We have raw data that consists of 'controlByte' bytes.
      nextByte++;  // Step past control byte.

      for  (kkint32 x = 0;  (x < controlByte)   &&  (nextByte < compressedBuffSize);  x++)
      {
        unCompressedBuff[unCompressedBuffIDX] = compressedBuff[nextByte];
        unCompressedBuffIDX++;
        nextByte++;
      }
    }
    else
    {
      // Since the control byte is 128 or greater;  then we are dealing with a repeating value.
      controlByte = controlByte % 128;  // Get the lower 7 bits 
      nextByte++;  // Step past control byte so now we are pointing at byte that is to be repeated.
      for  (kkint32 x = 0;  x < controlByte;  x++)
      {
        unCompressedBuff[unCompressedBuffIDX] = compressedBuff[nextByte];
        unCompressedBuffIDX++;
      }
      nextByte++;  // Move us to next control Byte.
    }
  }

  if  (unCompressedBuffIDX != unCompressedSize)
  {
    cerr << std::endl
         << std::endl
         << "SimpleCompressor::Decompress     ***ERROR***" << std::endl
         << std::endl
         << "                       Did not decompress correctly." << std::endl
         << std::endl;
  }
  return  unCompressedBuff;
}  /* Decompress */
