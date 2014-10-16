/* Raster.h -- Class that one raster image.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef _RASTER_
#define _RASTER_

#include <map>


#include "KKBaseTypes.h"
#include "KKQueue.h"
#include "KKStr.h"
#include "PixelValue.h"
#include "Point.h"


namespace KKB
{
  /**
   *@file  Raster.h
   *@author  Kurt Kramer
   *@details
   *@code
   ***************************************************************************
   **                                Raster                                  *
   **                                                                        *
   **  Supports image Morphological operations such as Dilation, Opening,   *
   **  Closing and other operations.                                        *
   ***************************************************************************
   *@endcode
   *@sa Blob
   *@sa ContourFollower
   *@sa ConvexHull
   *@sa PixelValue
   *@sa Point
   *@sa Sobel
   */


  #ifndef  _BLOB_
  class  Blob;
  typedef  Blob*  BlobPtr;
  class  BlobList;
  typedef  BlobList*  BlobListPtr;
  #endif

  #ifndef  _MATRIX_
  class Matrix;
  typedef  Matrix*  MatrixPtr;
  #endif

  #if  !defined(_KKU_GOALKEEPER_)
	class  GoalKeeper;
	typedef  GoalKeeper*  GoalKeeperPtr;
  #endif


  typedef  enum  
  {
    CROSS3   = 0,
    CROSS5   = 1,
    SQUARE3  = 2,
    SQUARE5  = 3,
    SQUARE7  = 4,
    SQUARE9  = 5,
    SQUARE11 = 6
  }  MaskTypes;


  typedef  enum  {RedChannel, GreenChannel, BlueChannel}  ColorChannels;


  class  RasterList;
  typedef  RasterList*  RasterListPtr;

  class  BmpImage;
  typedef  BmpImage*  BmpImagePtr;


  #ifndef  _HISTOGRAM_
  class Histogram;
  typedef  Histogram*  HistogramPtr;
  #endif


  /**
   *@class Raster 
   *@brief  A class that is used by to represent a single image in memory.  
   *@details  It supports morphological operations and other tasks.  It can handle either 
   *          Grayscale or Color.  By default it will use Grayscale unless specified 
   *          otherwise.  It will allocate one continuous block of memory for each 
   *          channel (RGB).  If the image is only grayscale the Green Channel (G) will 
   *          be used leaving the Red and Blue channels set to NULL.  You can access
   *          Individual pixels through access methods that will ensure memory integrity
   *          or you can also access the pixel data directly in memory.
   *@see Blob
   *@see ContourFollower
   *@see ConvexHull
   *@see PixelValue
   *@see Point
   *@see PointList
   *@see RasterList()
   *@see Sobel
   */
  class  Raster
  {
  public:
    typedef  Raster*  RasterPtr;

    typedef  Raster const*  RasterConstPtr;


    Raster ();

    Raster (const Raster&  _raster);  /**< @brief Copy Constructor */

    /**
     *@brief Constructs a blank GrayScale image with given dimensions; all pixels will be initialized to 0.  
     *@details  When working with Grayscale images pixel value of '0' = Background and '255'= foreground.
     *          The green channel will be used to represent the grayscale value.  When these raster images
     *          are saved to a image file such as a BMP file the pixel value of 0 will point to the color
     *          value of (255, 255, 255) and  pixel value 255 will point to the color value of (0, 0, 0).
     *          This way when displaying the image background will appear as white.
     */
    Raster (int32  _height,
            int32  _width
           );


    /**
     *@brief  Constructs a blank image with given dimensions.  
     *@details  The third parameter determines whether it will be a color or grayscale image,  If a Color
     *          image then all three color channel will be set to = 255 which stands for white.  If
     *          Grayscale the green channel will be set to 0.
     */
    Raster (int32 _height,
            int32 _width,
            bool  _color
           );

    /**
     *@brief  Constructs a Raster from a BMP image loaded from disk.
     *@details If BMP Image is a grayscale value pixel values will be reversed.  See description of
     *        grayscale constructor.
     */
    Raster (const BmpImage&  _bmpImage);


    /**
     *@brief Constructs a new Raster using a subset of the specified Raster as its source.  The
     *      dimensions of the resultant raster will be '_height', and '_width'
     */
    Raster (const Raster& _raster,  /**<  Source Raster                             */
            int32         _row,     /**<  Starting Row in '_raster' to copy from.             */
            int32         _col,     /**<  Starting Col in '_raster' to copy from.             */
            int32         _height,  /**<  Height of resultant raster. Will start from '_row'  */
            int32         _width    /**<  Width of resultant raster.                          */
           );

    /**
     *@brief Constructs a Raster that will be the same size as the specified '_mask' with the top left specified by '_row' and '_col'.
     *@details The Height and Width of the resultant image will come from the bias of the specified mask.  The Image data will come from
     * the specified raster using '_row' and '_col' to specify the top left column.
     *@param[in]  _raster  Source Raster to extract data from.
     *@param[in]  _mask  Used to derive height and with of resultant image.
     *@param[in]  _row  Starting row where image data is to be extracted from.
     *@param[in]  _col  Starting column where image data is to be extracted from.
     *@see MaskTypes
     */
    Raster (const Raster&   _raster,
            MaskTypes       _mask,
            int32           _row,
            int32           _col
           );

    /**
     *@brief  Constructs a Raster image from by reading an existing image File such as a BMP file.
     *@details  Will read from the specified file (fileName) the existing image.  If the load fails then 
     *          the contents of this object will be undefined.
     *@param[in]  fileName  Name of Image file to read.
     *@param[out] validFile  If image successfully loaded will be set to 'True' otherwise 'False'.
     */
    Raster (const KKStr&  fileName,    /**<  @param  fileName  name of image file to load/                    */
            bool&         validFile    /**<  @param  validFile will return true if image successfully loaded. */
           );


    /**
     *@brief  Construct a raster object that will utilize a image already in memory.
     *@details  This instance will NOT OWN the raster data; It will only point to it.  That means when this instance 
     * is destroyed the raster data will still be left intact.
     *@param[in]  _height  Height of image.
     *@param[in]  _width   Width of image.
     *@param[in]  _grayScaleData   Source grayscale raster data; needs to be continuous and of length (_height * _width) with data 
     *                             stored row major.
     *@param[in]  _grayScaleRows   Two dimensional array where each entry will point into the respective image row data in '_grayScaleData'.
     */
    Raster (int32    _height,
            int32    _width,
            uchar*   _grayScaleData,
            uchar**  _grayScaleRows
           );

    /**
     *@brief  Construct a Grayscale Raster object using provided raw data.
     *@param[in] _height Image Height.
     *@param[in] _width  Image Width.
     *@param[in] _grayScaleData 8 Bit grayscale data, Row Major, that is to be used to populate new instance.
     */
    Raster (int32         _height,
            int32         _width,
            const uchar*  _grayScaleData
           );

    /**
     *@brief  Construct a Color Raster object using provided raw data,
     *@param[in] _height Image Height.
     *@param[in] _width  Image Width.
     *@param[in] _redChannel   8 Bit data, Row Major, that is to be used to populate the red channel.
     *@param[in] _greenChannel 8 Bit data, Row Major, that is to be used to populate the green channel.
     *@param[in] _blueChannel  8 Bit data, Row Major, that is to be used to populate the blue channel.
     */
    Raster (int32         _height,
            int32         _width,
            const uchar*  _redChannel,
            const uchar*  _greenChannel,
            const uchar*  _blueChannel
           );


    virtual
    ~Raster ();



    /**
     *@brief  Lets you resize the raster dimensions; old image data wil be lost.
     */
    void  ReSize (kkint32  _height, 
                  kkint32  _width,
                  bool     _color
                 );



    /** 
     *@brief  Sets an existing instance to specific Raster Data of a grayscale image. 
     *@details  This instance of 'Raster' can take ownership of '_grayScaleData' and '_grayScaleRows' 
     * depending on '_takeOwnership'.
     *@param[in] _height Image Height.
     *@param[in] _width  Image Width.
     *@param[in] _grayScaleData The raster data that is to be used by this instance of 'Raster'; it should 
     *           be continuous data, Row Major, of length (_height * _width).
     *@param[in] _grayScaleRows  Two dimensional accessor to '_grayScaleData'; each entry will point to the
     *           respective row in '_grayScaleData' that contains that row.
     *@param[in] _takeOwnership Indicates whether this instance of 'Raster' will own the memory pointed
     *           to by '_grayScaleData' and '_grayScaleRows'; if set to true will delete them in the
     *           destructor.
     */
    void  Initialize (int32    _height,
                      int32    _width,
                      uchar*   _grayScaleData,
                      uchar**  _grayScaleRows,
                      bool     _takeOwnership
                     );

    /** 
     *@brief  Sets an existing instance to specific Raster Data of a grayscale image. 
     *@details  This instance of 'Raster' can take ownership of '_grayScaleData' and '_grayScaleRows' 
     * depending on '_takeOwnership'.  The parameters '_redArea', '_greenArea', and '_blueArea' will point 
     * to raster data that represents their respective color channels.  This data will be 'Row-Major' and 
     * of length '(_height * _width) bytes.  For each color channel there will be a corresponding 2d accessor
     * matrix '_red', '_green', and '_blue' where each entry in these 2d arrays will point to their 
     * respective rows in the color channel.  The '_takeOwnership' parameters indicates whether this
     * instance of 'Raster' will own these memory locations.
     * '
     *@param[in] _height Image Height.
     *@param[in] _width  Image Width.
     *@param[in] _redArea The raster data representing the red channel.
     *@param[in] _red  Two dimensional accessor to '_redArea'.
     *@param[in] _greenArea  The raster data representing the green channel.
     *@param[in] _green  Two dimensional accessor to '_greenArea'.
     *@param[in] _blueArea  The raster data representing the blue channel.
     *@param[in] _blue  Two dimensional accessor to '_blueArea'.
     *@param[in] _takeOwnership  Indicates whether this instance of 'Raster' will own the supplied raster data.
     */
    void  Initialize (int32    _height,
                      int32    _width,
                      uchar*   _redArea,
                      uchar**  _red,
                      uchar*   _greenArea,
                      uchar**  _green,
                      uchar*   _blueArea,
                      uchar**  _blue,
                      bool     _takeOwnership
                     );

    float    CentroidCol ();  /**< @return  returns  the centroid's column  */
    float    CentroidRow ();  /**< @return  returns  the centroid's row     */

    bool     Color ()        const  {return  color;}

    int32    Divisor ()      const  {return  divisor;}

    const
    KKStr&   FileName ()     const  {return  fileName;}

    int32    ForegroundPixelCount () const {return foregroundPixelCount;}

    int32        Height    ()    const  {return  height;}
    uchar        MaxPixVal ()    const  {return  maxPixVal;} /**< The maximum grayscale pixel value encountered in the image. */
    uchar**      Rows      ()    const  {return  green;}     /**< returns a pointer to a 2D array that allows the caller to access the raster data by row and column. */
    const KKStr& Title     ()    const  {return  title;}
    int32        TotPixels ()    const  {return  totPixels;} /**< The total number of pixels  (Height * Width). */
    int32        Width     ()    const  {return  width;}

    uchar**      Red       ()    const  {return  red;}        /**< returns a pointer to two dimensional array for 'Red' color channel. */
    uchar**      Green     ()    const  {return  green;}      /**< returns a pointer to two dimensional array for 'Green' color channel; note this is the same as 'Rows'.  */
    uchar**      Blue      ()    const  {return  blue;}       /**< returns a pointer to two dimensional array for 'Blue' color channel. */
    uchar*       RedArea   ()    const  {return  redArea;}
    uchar*       GreenArea ()    const  {return  greenArea;}
    uchar*       BlueArea  ()    const  {return  blueArea;}

    float*       FourierMagArea ()  const {return fourierMagArea;}

    uchar        BackgroundPixelTH    () const {return backgroundPixelTH;}
    uchar        BackgroundPixelValue () const {return backgroundPixelValue;}
    uchar        ForegroundPixelValue () const {return foregroundPixelValue;}

    int32    TotalBackgroundPixels () const;

    void     BackgroundPixelTH    (uchar          _backgroundPixelTH)    {backgroundPixelTH    = _backgroundPixelTH;}
    void     BackgroundPixelValue (uchar          _backgroundPixelValue) {backgroundPixelValue = _backgroundPixelValue;}
    void     Divisor              (int32          _divisor)              {divisor              = _divisor;}
    void     ForegroundPixelCount (int32          _foregroundPixelCount) {foregroundPixelCount = _foregroundPixelCount;}
    void     ForegroundPixelValue (uchar          _foregroundPixelValue) {foregroundPixelValue = _foregroundPixelValue;}
    void     FileName             (const KKStr&   _fileName)             {fileName             = _fileName;}
    void     MaxPixVal            (uchar          _maxPixVal)            {maxPixVal            = _maxPixVal;}
    void     Title                (const KKStr&   _title)                {title                = _title;}
    void     WeOwnRasterData      (bool           _weOwnRasterData)      {weOwnRasterData      = _weOwnRasterData;}

    bool     BackgroundPixel (int32  row,
                              int32  col
                             )  const;


    uchar         GetPixelValue (int32 row,  int32 col)  const;


    void          GetPixelValue (int32   row,
                                 int32   col,
                                 uchar&  r,
                                 uchar&  g,
                                 uchar&  b
                                )  const;


    void          GetPixelValue (int32       row,
                                 int32       col,
                                 PixelValue& p
                                )  const;


    uchar         GetPixelValue (ColorChannels  channel,
                                 int32          row,
                                 int32          col
                                )  const;


    void          SetPixelValue (const Point&       point,
                                 const PixelValue&  pixVal
                                );


    void          SetPixelValue (int32  row,
                                 int32  col,
                                 uchar  pixVal
                                );


    void          SetPixelValue (int32              row,
                                 int32              col,
                                 const PixelValue&  pixVal
                                );


    void          SetPixelValue (int32  row,
                                 int32  col,
                                 uchar  r,
                                 uchar  g,
                                 uchar  b
                                );


    void          SetPixelValue (ColorChannels  channel,
                                 int32          row,
                                 int32          col,
                                 uchar          pixVal
                                );


    /**
     *@brief Returns a image that is the result of a BandPass using Fourier Transforms.
     *@details A 2D Fourier transform is performed.  The range specified is from 0.0 to 1.0 where range is
     * determined from the center of the image to the farthest corner where the center is 0.0 and the farthest
     * corner is 1.0.  Pixels in the resultant 2D Transform that are "NOT" in the specified range are set to
     * 0.0.  A reverse transform is then performed and the resultant image is returned.
     *@param[in] lowerFreqBound  Lower range of frequencies to retain; between 0.0 and 1.0.
     *@param[in] upperFreqBound  Upper range of frequencies to retain; between 0.0 and 1.0.
     *@return The result image.
     */
    RasterPtr     BandPass (float  lowerFreqBound,    /**< Number's between 0.0 and 1.0  */
                            float  upperFreqBound,    /**< Represent fraction.           */
                            bool   retainBackground
                           );


    RasterPtr     BinarizeByThreshold (uchar  min,
                                       uchar  max
                                      );

    /**
     *@brief  Return the ID of the blob that the specified pixel location belongs to.
     *@details If a connected component (ExtractBlobs) was performed on this image then the pixels that belong
     *         to blobs were assigned a blob ID. These ID's are retained with the original image in 'blobIds'.
     *@param[in] row Row in image.
     *@param[in] col Column in image.
     *@returns BlobID of pixel location or -1 of does not belong to a blob.
     *@see ExtractBlobs
     */
    int32         BlobId (int32  row,  int32  col)  const;


    /**
     *@brief  Builds a 2d Gaussian kernel
     *@details Determines the size of the gaussian kernel based off the specified sigma parameter. returns a
     *         2D matrix representing the kernel which will have 'len' x 'len' dimensions.  The caller will be
     *         responsible for deleting the kernel.
     *@param[in]  sigma   parameter used to control the width of the gaussian kernel
     *@returns A 2-dimensional matrix representing the gaussian kernel.
     */
    static
    MatrixPtr     BuildGaussian2dKernel (float  sigma);  // Used by the Gaussian Smoothing algorithm.


    int32         CalcArea ();

    
    /**
     *@brief Calculates the occurrence of different intensity levels.
     *@details The pixel values 0-255 are split into 8 ranges.  (0-31), (32-63),  (64-95), (96-127), (128-159),
     *         (160-191), (192-223), (224-255).  The background range (0-31) are not counted.
     *@param[out]  area Total number of foreground pixels in the image.
     *@param[out]  intensityHistBuckets  An array of 8 buckets where each bucket represents an intensity range.
     */
    void          CalcAreaAndIntensityHistogram (int32&  area,
                                                 uint32  intensityHistBuckets[8]
                                                );

    /**
     *@brief  Calculates a Intensity Histogram including Background pixels in the image.
     *@details All background pixels that are inside the image will also be included in the counts. This is done
     *         by building a mask on the original image then performing a FillHole operation. This mask is then
     *         used to select pixels for inclusion in the histogram.
     */
    void          CalcAreaAndIntensityHistogramWhite (int32&  area,
                                                      uint32  intensityHistBuckets[8]
                                                     );
    


    /**
     *@brief  Calculates both Intensity Histograms, one not including internal background pixels and one with
     *        plus size and weighted size.
     *@details
     *        This method incorporates the functionality of several methods at once.  The idea being that while
     *        we are iterating through the raster image we might as well get all the data we can so as to save
     *        total overall processing time.
     *@code
     * Histogram Ranges:
     *   0:   0  - 31    4: 128 - 159
     *   1:  31  - 63    5: 192 - 223
     *   2:  64  - 95    6: 192 - 223
     *   3:  96 - 127    7: 224 - 255
     *@endcode
     *
     *@param[out]  area          Number of foreground pixels.
     *@param[out]  weightedSize  Area that takes intensity into account.  The largest pixel will have a value of 1.0.
     *@param[out]  intensityHistBuckets A 8 element array containing a histogram by intensity range.
     *@param[out]  areaWithWhiteSpace  Area including any whitespace enclosed inside the image.
     *@param[out]  intensityHistBucketsWhiteSpace  A 8 element array containing a histogram by intensity range,
     *             with enclosed whitespace pixels included.
     */
    void          CalcAreaAndIntensityFeatures (int32&   area,
                                                float&   weightedSize,
                                                uint32   intensityHistBuckets[8],
                                                int32&   areaWithWhiteSpace,
                                                uint32   intensityHistBucketsWhiteSpace[8]
                                               );


    /**
     *@brief  Calculates both Intensity Histograms, one not including internal background pixels and one with
     *         plus size and weighted size.
     *@details 
     *        This method incorporates the functionality of several methods at once.  The idea being that while
     *        we are iterating through the raster image we might as well get all the data we can so as to save
     *        total overall processing time.
     *@code
     * Histogram Ranges:
     *   0:   0  - 31    4: 128 - 159
     *   1:  32  - 63    5: 192 - 223
     *   2:  64  - 95    6: 192 - 223
     *   3:  96 - 127    7: 224 - 255
     *@endcode
     *
     *@param[out]  area          Number of foreground pixels.
     *@param[out]  weightedSize  Area that takes intensity into account.  The largest pixel will have a value of 1.0.
     *@param[out]  intensityHistBuckets  A 8 element array containing a histogram by intensity range where each bucket 
     * represents a range of 32.
     */
    void          CalcAreaAndIntensityFeatures (int32&  area,
                                                float&  weightedSize,
                                                uint32  intensityHistBuckets[8]
                                               );


    void          CalcCentroid (int32&  size,
                                int32&  weight,
                                float&  rowCenter,  
                                float&  colCenter,
                                float&  rowCenterWeighted,
                                float&  colCenterWeighted
                               );


    void          CalcOrientationAndEigerRatio (float&  eigenRatio,
                                                float&  orientationAngle
                                               );

    float         CalcWeightedArea ()  const;


    double        CenMoment (int32   colMoment,
                             int32   rowMoment,
                             double  centerCol,
                             double  centerRow 
                            )  const;

    void          CentralMoments (float  features[9])  const;

    float         CenMomentWeighted (int32   p, 
                                     int32   q, 
                                     float   ew, 
                                     float   eh
                                    ) const;

    void          CentralMomentsWeighted (float  features[9])  const;

    void          Closing ();

    void          Closing (MaskTypes  mask);

    void          ConnectedComponent (uchar connectedComponentDist);

    RasterPtr     CreateColor ()  const;

    RasterPtr     CreateDialatedRaster ()  const;

    void          Dialation ();

    void          Dialation (RasterPtr  dest);

    void          Dialation (RasterPtr  dest,
                             MaskTypes  mask
                            );


    RasterPtr     CreateDialatedRaster (MaskTypes  mask)  const;
    void          Dialation (MaskTypes  mask);

    RasterPtr     CreateErodedImage (MaskTypes  mask)  const;


    RasterPtr     CreateGrayScale ()  const;

    /**
     *@brief Creates a grayscale image using a KLT Transform with the goal of weighting in favor the color
     * channels with greatest amount of variance.
     *@details The idea is to weight each color channel by the amount of variance.  This is accomplished by
     *  producing a covariance matrix of the three color channels and then taking the Eigen vector with the
     *  largest eigen value and using its components to derive weights for each channel for the conversion 
     *  from RGB to grayscale.
     */
    RasterPtr     CreateGrayScaleKLT ()  const;

    /**
     *@brief  Same as 'CreateGrayScaleKLT' except it will only take into account 
     *        pixels specified by the 'mask' image.
     *@param[in]  mask  Raster object where pixels that are greater than 'backgroundPixelTH' are to be considered.
     */
    RasterPtr     CreateGrayScaleKLTOnMaskedArea (const Raster&  mask)  const;

    static
    RasterPtr     CreatePaddedRaster (BmpImage&  image,
                                      int32      padding
                                     );

    RasterPtr     CreateSmoothImage (int32  maskSize = 3)  const;
    
    RasterPtr     CreateSmoothedMediumImage (int32 maskSize)  const;

    RasterPtr     CreateGaussianSmoothedImage (float sigma)  const;

    /**
     *@brief Produces a color image using the 'greenArea' channel, assuming that each unique value will be
     *       assigned a unique color.
     *@details
     *        Assuming that each value in the grayscale channel(GreenArea) will be assigned  a different color
     *        useful for image created by  "SegmentorOTSU::SegmentImage".
     *@see Raster::CreateGrayScaleKLT
     */
    RasterPtr     CreateColorImageFromLabels ();


    /**
     *@brief Returns image where each blob is labeled with a different color.
     *@details
     *        Only useful if 'ExtractBlobs' was performed on this instance. Eight different colors are used and
     *        they are selected by the modules of the blobId(blobId % 8).  Assignments are 0:Red, 1:Green,
     *        2:Blue, 3:Yellow, 4:Orange, 5:Magenta, 6:Purple, 7:Teal.
     */
    RasterPtr     CreateColorWithBlobsLabeldByColor (BlobListPtr  blobs);  /**< Only useful if 'ExtractBlobs' was performed on this instance, the returned image will be color with each blob labeled a different color. */


    /**
     *@brief Returns a copy of 'origImage' where only the blobs specified in 'blobs' are copied over.
     *@details 
     *@code
     *  Example:
     *      origImage = image that we want to segment and get list of discrete blobs from.
     *      RasterPtr  segmentedImage = origImage->SegmentImage ();
     *      BlobListPtr  blobs = segmentedImage->ExtractBlobs (1);
     *      RasterPtr  imageWithBlobOnly = segmentedImage->CreateFromOrginalImageWithSpecifidBlobsOnly (blobs);
     *@endcode
     *@param[in]  origImage  Image that this instance was derived for, must have same dimensions.
     *@param[in]  blobs      Blobs that you want copied over from.
     *@returns Image consisting of specified blobs only.
     */
    RasterPtr     CreateFromOrginalImageWithSpecifidBlobsOnly (RasterPtr    origImage,
                                                               BlobListPtr  blobs
                                                              );

    /** 
     *@brief Draw a circle who's center is at 'point' and radius in pixels is 'radius'using color 'color'. 
     *@param[in]  point   Location in image where the center of circle is to be located.
     *@param[in]  radius  The radius in pixels of the circle that is to be drawn.
     *@param[in]  color   Color thta is to be used to draw the circle with.
     */
    void          DrawCircle (const Point&       point,
                              int32              radius,
                              const PixelValue&  color
                             );


    void          DrawCircle (float              centerRow, 
                              float              centerCol, 
                              float              radius,
                              const PixelValue&  pixelValue
                             );


    void          DrawCircle (float              centerRow,    /**< Row that will contain the center of the circle.    */
                              float              centerCol,    /**< Column that will contain the center of the circle. */
                              float              radius,       /**< The radius of the circle in pixels.                */
                              float              startAngle,   /**< Start and End angles should be given in radians    */
                              float              endAngle,     /**< Where the angles are with respect to the compass   */
                              const PixelValue&  pixelValue    /**< Pixel value that is to be assigned to locations in the image that ar epart of the circle. */
                             );


    void          DrawDot (const Point&       point, 
                           const PixelValue&  color,
                           int32              size
                          );


    void          DrawFatLine (Point       startPoint,
                               Point       endPoint, 
                               PixelValue  pv,
                               float       alpha
                              );


    void          DrawGrid (float              pixelsPerMinor,
                            uint32             minorsPerMajor,
                            const PixelValue&  hashColor,
                            const PixelValue&  gridColor
                           );


    void          DrawLine (int32 bpRow,  int32 bpCol,
                            int32 epRow,  int32 epCol
                           );


    void          DrawLine (int32 bpRow,    int32 bpCol,
                            int32 epRow,    int32 epCol,
                            uchar pixelVal
                           );


    void          DrawLine (const Point&  beginPoint,
                            const Point&  endPoint,
                            uchar         pixelVal
                           );

    void          DrawLine (const Point&       beginPoint,
                            const Point&       endPoint,
                            const PixelValue&  pixelVal
                           );


    void          DrawLine (int32 bpRow,    int32 bpCol,
                            int32 epRow,    int32 epCol,
                            uchar  r,
                            uchar  g,
                            uchar  b
                           );


    void          DrawLine (int32 bpRow,    int32 bpCol,
                            int32 epRow,    int32 epCol,
                            uchar  r,
                            uchar  g,
                            uchar  b,
                            float  alpha
                           );


    void          DrawLine (int32  bpRow,    int32 bpCol,
                            int32  epRow,    int32 epCol,
                            PixelValue  pixelVal
                           );

    void          DrawLine (int32  bpRow,    int32 bpCol,
                            int32  epRow,    int32 epCol,
                            PixelValue  pixelVal,
                            float       alpha
                           );


    void          DrawConnectedPointList (Point              offset,
                                          const PointList&   borderPixs,
                                          const PixelValue&  pixelValue,
                                          const PixelValue&  linePixelValue
                                         );

    void          DrawPointList (const PointList&   borderPixs,
                                 const PixelValue&  pixelValue
                                );

    void          DrawPointList (Point              offset,
                                 const PointList&   borderPixs,
                                 const PixelValue&  pixelValue
                                );


    void          DrawPointList (const PointList&  borderPixs,
                                 uchar             redVal,
                                 uchar             greenVal,
                                 uchar             blueVal
                                );


    void          DrawPointList (Point             offset,
                                 const PointList&  borderPixs,
                                 uchar             redVal,
                                 uchar             greenVal,
                                 uchar             blueVal
                                );

    PointListPtr  DeriveImageLength () const;


    /** @brief reduces image to edge pixels only.  */
    void          Edge ();

    void          Edge (RasterPtr  dest);

    /** @brief removes spurs from image. */
    void          ErodeSpurs ();

    void          Erosion ();

    void          Erosion (MaskTypes  mask);


    /**
     *@brief  Place into destination a eroded version of this instances image.
     */
    void          Erosion (RasterPtr  dest);

    void          Erosion (RasterPtr  dest,
                           MaskTypes  mask
                          );


    /**
     *@brief  Extracts a specified blob from this image;  useful to extract individual detected blobs.
     *@details  The 'ExtractBlobs' method needs to have been performed on this instance first.  You
     * would use this method after calling 'ExtractBlobs'. The extracted image will be of the same 
     * dimensions as the original image except it will extract the pixels that belong to the specified
     * blob only.
     *@code
     *   // Example of processing extracted blobs
     *   void  ProcessIndividulConectedComponents (RasterPtr  image)
     *   {
     *     BlobListPtr blobs = image->ExtractBlobs (3);
     *     BlobList::iterator  idx;
     *     for  (idx = blobs->begin ();  idx != end ();  ++idx)
     *     {
     *       RasterPtr  individuleBlob = image->ExtractABlob (*idx);
     *       DoSomethingWithIndividuleBlob (individuleBlob);
     *       delete  individuleBlob;
     *       individuleBlob = NULL;
     *     )
     *     delete  blobs;
     *     blobs = NULL;
     *   }
     *@endcode
     */
    RasterPtr     ExtractABlob (const BlobPtr  blob)  const;


    /**
     *@brief  Extracts a specified blob from this image into a tightly bounded image.
     *@details
     *        Similar to 'ExtractABlob' except that the returned image will have the dimension necessary
     *        to contain the specified blob with the specified number of padded row and columns.
     */
    RasterPtr     ExtractABlobTightly (const BlobPtr  blob,
                                       int32          padding
                                       ) const;


    /**
     *@brief Will extract a list of connected components from this instance.
     *@details
     *        Will perform a connected component analysis and label each individual blob.  A list of blob
     *        descriptors will be returned.  These blob descriptors can then be used to access individual
     *        blobs.  See 'ExtractABlob' for an example on how to use this method.  The 'ForegroundPixel'
     *        method is used to determine if a given pixel is foreground or background.
     *
     *@param[in]  dist  The distance in pixels that two different pixel locations have to be for them to
     *            be considered connected.  "dist = 1" would indicate that two pixels have to be directly
     *            connected.
     *@returns A list of Blob descriptor instances.
     *@see  ExtractABlob, ExtractABlobTightly, Blob
     */
    BlobListPtr   ExtractBlobs (int32  dist);

    
    /**
     *@brief Will return a grayscale image consisting of the specified color channel only.
     */
    RasterPtr     ExtractChannel (ColorChannels  channel);

    /**
     *@brief  Extracts the pixel locations where the 'mask' images pixel location is a forground pixel. 
     */
    RasterPtr     ExtractUsingMask (RasterPtr  mask);

    RasterPtr     FastFourier ()  const;

    void          FillHole ();


    /**
     *@brief  Fills holes in the image using the 'mask' raster as a work area.
     *@details  Any pixel that is not a forground pixels that has not path by a cross structure to the 
     *  edge of the image wil be painted with the foreground pixel value.  The 'rask' raster instance provided
     *  will be used as a temporary work area.  If its dimensioansd are not the same as this instance it will
     *  e resized.
     */
    void          FillHole (RasterPtr  mask);


    /**
     *@brief  Will paint the specified blob with the specified color
     *@details
     *@code
     * Example Use:
     *    BlobListPtr  blobs = srcImage->ExctractBlobs (1);
     *    RasterPtr  labeledColorImage = new Raster (srcImage->Height (), srcImage->Width (), true);
     *    BlobList::iterator  idx;
     *    for  (idx = blobs->begin ();  idx != blobs->end ();  ++idx)
     *    {
     *      BlobPtr  blob = *idx;
     *      labeledColorImage->FillBlob (srcImage, blob, PixelValue::Red);
     *    }
     *@endcode
     *@param[in] origImage  The image where the blob was extracted from.
     *@param[in] blob  The specific blob that you want to fill in/ paint.
     *@param[in] color that is to be filled in.
     */
    void          FillBlob (RasterPtr   origImage,
                            BlobPtr     blob,
                            PixelValue  color
                           );

    void          FillRectangle (int32              tlRow,
                                 int32              tlCol,
                                 int32              brRow,
                                 int32              brCol,
                                 const PixelValue&  fillColor
                                );

    void          FindBoundingBox (int32&  tlRow,
                                   int32&  tlCol,
                                   int32&  brRow,
                                   int32&  brCol
                                  )  const;

    /**
     *@brief Returns an image that reflects the differences between this image and the image supplied in the parameter.
     *@details  Each pixel will represent the magnitude of the difference between the two raster instances for that 
     * pixel location.  If there are no differences than a raster of all 0's will be returned. If dimensions are different
     * then the largest dimensions will be sued.
     *@param[in]  r  Raster to compare with.
     *@returns A raster that wil reflect the differences between the two instances where each pixel wil represent 
     * the magnitude of the differences.
     */
    RasterPtr     FindMagnitudeDifferences (const Raster&  r);


    void          FollowContour (float  countourFreq[5])  const;


    void          FourierExtractFeatures (float  fourierFeatures[5])  const;

    bool          ForegroundPixel (int32  row,  
                                   int32  col
                                  )  const;


    /**
     * @brief Creates a raster from a compressedBuff created by 'SimpleCompression'.
     */
    static
    RasterPtr     FromSimpleCompression (const uchar*  compressedBuff,
                                         uint32        compressedBuffLen
                                        ); 

    /**
     *@brief  Creates a new instance of Raster object from zLib compressed data.
     *@details Performs the inverse operation of Raster::ToCompressor.
     *@param[in] compressedBuff  Pointer to buffer area containing compressed data originally created by 'ToCompressor'.
     *@param[in] compressedBuffLen  Length in bytes of 'compressedBuff'.
     *@returns If successful a pointer to a new instance of 'Raster'; if there is an error will return NULL.
     *@see  ToCompressor
     */
    static
    RasterPtr     FromCompressor (const uchar*  compressedBuff,
                                  uint32        compressedBuffLen
                                 ); 

    uchar**       GetSubSet (uchar** _src,
                             int32   _row,
                             int32   _col,
                             int32   _height,
                             int32   _width
                            )  const;

    RasterPtr     HalfSize ();

    HistogramPtr  Histogram (ColorChannels  channel)  const;

    RasterPtr     HistogramEqualizedImage ()  const;

    RasterPtr     HistogramEqualizedImage (HistogramPtr  equalizedHistogram) const;

    HistogramPtr  HistogramGrayscale ()  const;

    RasterPtr     HistogramImage (ColorChannels  channel)  const;

    RasterPtr     HistogramGrayscaleImage ()  const;

    int32         MemoryConsumedEstimated ()  const;

    RasterPtr     Padded (int32 padding);  // Creates a Padded raster object.

    RasterPtr     ReversedImage ();

    RasterPtr     StreatchImage (float  rowFactor,
                                 float  colFactor
                                );

    void          ReverseImage ();   // Reverse the image Foreground and Background.

    void          Opening ();

    void          Opening (MaskTypes mask);

    void          PaintPoint (int32              row,
                              int32              col,
                              const PixelValue&  pv,
                              float              alpha
                             );

    void          PaintFatPoint (int32             row,
                                 int32             col,
                                 const PixelValue  pv,
                                 float             alpha
                                );


    RasterPtr     ReduceByEvenMultiple (int32  multiple)  const;

    RasterPtr     ReduceByFactor (float factor)  const;  //  0 < factor <= 1.0  ex: 0.5 = Make raster half size

    RasterPtr     Rotate (float  turnAngle);

    RasterPtr     SegmentImage (bool  save = false);

    /**
     *@brief  Compresses the image in Raster using a simple Run length algorithm and returns a pointer to compressed data.
     *@details Using a simple run length compression algorithm compress the data in Raster and return a 
     *        pointer to the resultant buffer.  The caller will take ownership of the compressed data and be 
     *        responsible for deleting it.  The function 'FromCompressor' can take the compressed data with its
     *        length and recreate the original Raster object.
     *@param[out]  buffLen Length of the compressed buffer returned.
     *@return pointer to compressed data.
     */
     uchar*       SimpleCompression (uint32&  buffLen)  const;
    

    RasterPtr     SobelEdgeDetector ();

    RasterListPtr SplitImageIntoEqualParts (int32 numColSplits,
                                            int32 numRowSplits
                                           )  const;

    RasterPtr     SwapQuadrants ()  const;

    /**
     *@brief Thresholds image in HSI space.
     *@details
     *        Returns an image with only the pixels that are within a specified distance in HSI space to the
     *        supplied HSI parameters.  All pixels that are not within the specified distance will be set to
     *        'flagValue'.
     *@param[in] thresholdH  Hue in radians(0.0 <-> 2Pie).
     *@param[in] thresholdS  Saturation (0.0 <-> 1.0).
     *@param[in] thresholdI  Intensity (0.0 <-> 1.0).
     *@param[in] distance    Threshold Distance (0.0 <-> 1.0) that a pixel must be within in HSI space to be included.
     *@param[in] flagValue   PixelValue to set for pixels that are NOT within 'distance' of threshold.
     *@returns A image where pixels that are within the threshold will retain their original pixel values and 
     * the ones that are not will be set to 'flagValue'.
     */
    RasterPtr     ThresholdInHSI (float              thresholdH,
                                  float              thresholdS, 
                                  float              thresholdI, 
                                  float              distance,
                                  const PixelValue&  flagValue
                                 );

    RasterPtr     ThinContour ();

    RasterPtr     TightlyBounded (uint32 borderPixels)  const;

    RasterPtr     Transpose ()  const;

    RasterPtr     ToColor ()  const;

    /**
     *@brief Compresses the image in Raster using zlib library and returns a pointer to compressed data.
     *@details Will first write Rater data to a buffer that will be compressed by the Compressor class using the zlib library.
     *@code
     *        Buffer Contents:
     *           0 - 3: Height:  high order to low order
     *           4 - 7:    Width:   high order to low order
     *           8 - 8:    Color    0 = Grayscale,  1 = Color
     *           9 -  :   Green Channel  (Height * Width bytes)
     *           xxxxx:   Red  Channel, if color image.
     *           xxxxx:   Blue Channel, if color image.
     *@endcode
     *@param[out]  compressedBuffLen Length of the compressed buffer returned.
     *@return pointer to compressed data.
     */
    uchar*        ToCompressor (uint32&  compressedBuffLen)  const;


    virtual
      RasterPtr  AllocateARasterInstance (int32  height,
                                          int32  width,
                                          bool   color
                                         )  const;

    virtual
      RasterPtr  AllocateARasterInstance (const Raster& r)  const;

    virtual
      RasterPtr  AllocateARasterInstance (const Raster& _raster,  /**<  Source Raster                                       */
                                          int32         _row,     /**<  Starting Row in '_raster' to copy from.             */
                                          int32         _col,     /**<  Starting Col in '_raster' to copy from.             */
                                          int32         _height,  /**<  Height of resultant raster. Will start from '_row'  */
                                          int32         _width    /**<  Width of resultant raster.                          */
                                         )  const;

  private:
    void   AllocateBlobIds ();

    void   AllocateImageArea ();

    void   AllocateFourierMagnitudeTable ();

    void   CleanUpMemory ();

    inline
    bool   BackgroundPixel (uchar  pixel)  const;


    // Used by the Gaussian Smoothing algorithm.
    void   BuildGaussian2dKernel (float     sigma,
                                  int32&    len,
                                  float**&  kernel
                                 )  const;


    inline
    void   CalcDialatedValue (int32   row,
                              int32   col,
                              int32&  totVal,
                              uchar&  numNeighbors
                             )  const;


    inline
    bool   CompletlyFilled3By3 (int32  row, 
                                int32  col
                               )  const;


    void  DeleteExistingBlobIds ();


    uchar  DeltaMagnitude (uchar c1, uchar c2);


    void   FillHoleGrow (int32  _row, 
                         int32  _col
                        );

    bool   Fit (MaskTypes  mask,
                int32      row, 
                int32      col
               )  const;

    
    bool   ForegroundPixel (uchar  pixel)  const;


    uchar    Hit (MaskTypes  mask,
                  int32      row, 
                  int32      col
                 )  const;


    bool     IsThereANeighbor (MaskTypes  mask,
                               int32      row, 
                               int32      col
                              )  const;




    bool     ThinningSearchNeighbors  (int32    x, 
                                       int32    y,
                                       uchar**  g, 
                                       uchar    m_Matrix22[][3]
                                      );


    void  Moment (kkint32& m00,
                  kkint32& m10,
                  kkint32& m01
                 )  const;


    void   MomentWeighted (float& m00,
                           float& m10,
                           float& m01
                          )  const;


    inline
    int32    NearestNeighborUpperLeft (int32  centRow,
                                       int32  centCol,
                                       int32  dist
                                      );

    inline
    int32  NearestNeighborUpperRight (int32  centRow,
                                      int32  centCol,
                                      int32  dist
                                     );


    void     SmoothImageChannel (uchar**  src,
                                 uchar**  dest,
                                 int32    maskSize
                                )  const;

    void     SmoothUsingKernel (Matrix&  kernel,
                                uchar**  src,
                                uchar**  dest
                               )  const;


  protected:
    uchar    backgroundPixelValue;
    uchar    backgroundPixelTH;     /**< Threshold used to split Background and foreground pixel/ */
    int32**  blobIds;               /**< Used when searching for connected components  */
    mutable  float  centroidCol;
    mutable  float  centroidRow;
    bool     color;
    int32    divisor;
    KKStr    fileName;
    mutable  int32    foregroundPixelCount;
    uchar    foregroundPixelValue;
    float**  fourierMag;           /**< Only used if image is result of a Fourier Transform   */
    float*   fourierMagArea;       /**< Only used if image is result of a Fourier Transform   */
    int32    height;
    uchar    maxPixVal;
    KKStr    title;                /**< Title such as 'Class" that can be assigned to an image. */
    int32    totPixels;
    bool     weOwnRasterData;
    int32    width;

    uchar*   redArea;              // Each color channel is allocated as a single block
    uchar*   greenArea;            // for 2 dimensional access use corresponding 2d variables
    uchar*   blueArea;             // red for redAreas, green for grenArea.  If GrayScale
                                   // image then only green channel is used.

    // The next three variables provide row indexing into there respective color channels.  For performance
    // and simplicity purposes I allocate each channel in a continuous block of memory but to allow for
    // simple accessing by 'row' and 'col' I create the following 3 variables.  Depending on what you
    // are trying to do you could use the appropriate variable.
    uchar**  red;                  // Provides row indexes into 'redArea'.
    uchar**  green;                // Provides row indexes into 'greenArea'.
    uchar**  blue;                 // Provides row indexes into 'blueArea'.


    //  The following code is being added to support the tracking down of memory leaks in Raster.
    static std::map<RasterPtr, RasterPtr>  allocatedRasterInstances;
    static volatile GoalKeeperPtr  goalKeeper;
    static volatile bool           rasterInitialized;
    static void  Initialize ();
    static void  FinaleCleanUp ();
    static void  AddRasterInstance (const RasterPtr  r);
    static void  RemoveRasterInstance (const RasterPtr  r);
  public:
    static void  PrintOutListOfAllocatedrasterInstances ();

  };  /* Raster */


  typedef  Raster::RasterPtr       RasterPtr;

  typedef  Raster::RasterConstPtr  RasterConstPtr;

#define  _Raster_Defined_


  typedef  struct  
  {
    int32  row;
    int32  col;
  }  MovDir;




  class  RasterList:  public  KKQueue<Raster>
  {
  public:
    RasterList (bool  owner):
        KKQueue<Raster> (owner)
        {}

  private:
    RasterList (const RasterList&  rasterList):
        KKQueue<Raster> (rasterList)
        {}

  public:
    RasterList (const RasterList&  rasterList,
                bool               _owner
               ):
        KKQueue<Raster> (rasterList, _owner)
        {}
        
        
   RasterPtr  CreateSmoothedFrame ();
  };


  typedef  RasterList*  RasterListPtr;

#define  _RasterList_Defined_


}  /* namespace KKB; */
#endif