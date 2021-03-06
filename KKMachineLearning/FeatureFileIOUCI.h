#ifndef  _FEATUREFILEIOUCI_
#define  _FEATUREFILEIOUCI

#include  "FeatureFileIO.h"

namespace KKMLL
{


  /**
    @class FeatureFileIOUCI
    @brief  Supports the reading and writing of Feature data from a file format commonly used by many dataset's in the UCI repository.
    @details
    @code
    * ************************************************************************************************
    * *  FeatureFileIOUCI  Sub-classed from FeatureFileIO.  It supports the reading and writing      *
    * *  of many of the files found in the UCI repository.                                           *
    * *                                                                                              *
    * *  1) One example per row.                                                                     *
    * *  2) Each field is separated by comma.                                                        *
    * *  3) Each row has the same number of fields.                                                  *
    * *  4) The last field is the ClassName.                                                         *
    * *  5) Each field is numeric except the last one which is the ClassName                         *
    * ************************************************************************************************
    @endcode
    @see  FeatureFileIO
    */
  class  FeatureFileIOUCI: public FeatureFileIO
  {
  public:
    FeatureFileIOUCI ();

    ~FeatureFileIOUCI ();

    typedef  FeatureFileIOUCI*  FeatureFileIOUCIPtr;

    static  FeatureFileIOUCIPtr  Driver ()  {return &driver;}

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
                              VolConstBool&         _cancelFlag,
                              bool&                 _successful,
                              KKStr&                _errorMessage,
                              RunLog&               _log
                             )  override;

  private:
    static  FeatureFileIOUCI  driver;
  };  /* FeatureFileIOUCI*/

}  /* namespace KKMLL */

#endif
