#ifndef  _FEATUREFILEIOCOLUMN_
#define  _FEATUREFILEIOCOLUMN_

#include  "FeatureFileIO.h"


namespace KKMLL
{
/**
  @class FeatureFileIOColumn
  @brief  Supports a simple Feature File format where each column represents a example and each row a feature value.
  @details
  @code
  ************************************************************************************************
  *  FeatureFileIOColumn    Sub-classed from FeatureFileIO.  A simple format where each column   *
  *  represents a  example and each row an attribute.  The First line has the Classes Names.     *
  ************************************************************************************************
  @endcode
  @see  FeatureFileIO
  */
class FeatureFileIOColumn:  public FeatureFileIO
{
public:
  FeatureFileIOColumn ();
  ~FeatureFileIOColumn ();

  typedef  FeatureFileIOColumn*  FeatureFileIOColumnPtr;

  static   FeatureFileIOColumnPtr  Driver ()  {return &driver;}

  virtual  FileDescConstPtr  GetFileDesc (const KKStr&    _fileName,
                                          istream&        _in,
                                          MLClassListPtr  _classList,
                                          kkint32&        _estSize,
                                          KKStr&          _errorMessage,
                                          RunLog&         _log
                                         )  override;


  virtual  FeatureVectorListPtr  LoadFile (const KKStr&      _fileName,
                                           FileDescConstPtr  _fileDesc,
                                           MLClassList&      _classes, 
                                           istream&          _in,
                                           OptionUInt32      _maxCount,    // Maximum # images to load.
                                           VolConstBool&     _cancelFlag,
                                           bool&             _changesMade,
                                           KKStr&            _errorMessage,
                                           RunLog&           _log
                                          )  override;

  virtual  void   SaveFile (FeatureVectorList&    _data,
                            const KKStr&          _fileName,
                            FeatureNumListConst&  _selFeatures,
                            ostream&              _out,
                            kkuint32&             _numExamplesWritten,
                            VolConstBool&          _cancelFlag,
                            bool&                 _successful,
                            KKStr&                _errorMessage,
                            RunLog&               _log
                           )  override;

private:
  static  FeatureFileIOColumn  driver;
};

}  /* namespace KKMLL */


#endif

