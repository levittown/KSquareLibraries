#include  "FirstIncludes.h"

#include <stdio.h>
#include <math.h>


#include <ctype.h>
#include <time.h>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <string.h>
#include "MemoryDebug.h"
using namespace std;


#include "KKBaseTypes.h"
#include "DateTime.h"
#include "GlobalGoalKeeper.h"
#include "ImageIO.h"
#include "OSservices.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace KKB;


#include "FeatureFileIO.h"
#include "FeatureFileIOArff.h"
#include "FeatureFileIOC45.h"
#include "FeatureFileIOColumn.h"
#include "FeatureFileIODstWeb.h"
#include "FeatureFileIORoberts.h"
#include "FeatureFileIOSparse.h"
#include "FeatureFileIOUCI.h"

#include "FactoryFVProducer.h"
#include "FeatureVectorProducer.h"
#include "FileDesc.h"
#include "MLClass.h"
using namespace KKMachineLearning;






void  ReportError (RunLog&        log,
                   const KKStr&  fileName,
                   const KKStr&  funcName,
                   kkint32          lineCount,
                   const KKStr&  errorDesc
                  )
{
  log.Level (-1) << endl
                 << funcName << "     *** ERROR ***" << endl
                 << "           File     [" << fileName      << "]" << endl
                 << "           LineCount[" << lineCount     << "]" << endl
                 << "           Error    [" << errorDesc     << "]" << endl
                 << endl;
}  /* ReportError */




bool  FeatureFileIO::atExitDefined = false;


vector<FeatureFileIOPtr>*  FeatureFileIO::registeredDrivers = NULL;


std::vector<FeatureFileIOPtr>*  FeatureFileIO::RegisteredDrivers  ()
{
  if  (registeredDrivers == NULL)
  {
    RegisterAllDrivers ();
  }

  return  registeredDrivers;
}


void  FeatureFileIO::RegisterFeatureFileIODriver (FeatureFileIOPtr  _driver)
{
  GlobalGoalKeeper::StartBlock ();

  if  (!atExitDefined)
  {
    atexit (FeatureFileIO::FinalCleanUp);
    atExitDefined = true;
  }

  if  (!registeredDrivers)
    registeredDrivers = new std::vector<FeatureFileIOPtr> ();

  registeredDrivers->push_back (_driver);

  GlobalGoalKeeper::EndBlock ();
} /* RegisterFeatureFileIODriver */




void  FeatureFileIO::RegisterAllDrivers ()
{
  GlobalGoalKeeper::StartBlock ();

  if  (!atExitDefined)
  {
    atexit (FeatureFileIO::FinalCleanUp);
    atExitDefined = true;
  }

  if  (registeredDrivers == NULL)
  {
    registeredDrivers = new std::vector<FeatureFileIOPtr> ();
    registeredDrivers->push_back (new FeatureFileIOArff    ());
    registeredDrivers->push_back (new FeatureFileIOC45     ());
    registeredDrivers->push_back (new FeatureFileIOColumn  ());
    registeredDrivers->push_back (new FeatureFileIODstWeb  ());
    //registeredDrivers->push_back (new FeatureFileIOKK      ());
    registeredDrivers->push_back (new FeatureFileIORoberts ());
    registeredDrivers->push_back (new FeatureFileIOSparse  ());
    registeredDrivers->push_back (new FeatureFileIOUCI     ());
  }

  GlobalGoalKeeper::EndBlock ();
}  /* RegisterAllDrivers */




FeatureFileIOPtr  FeatureFileIO::LookUpDriver (const KKStr&  _driverName)
{
  GlobalGoalKeeper::StartBlock ();

  FeatureFileIOPtr  result = NULL;

  KKStr  driverNameLower = _driverName.ToLower ();
  vector<FeatureFileIOPtr>::const_iterator  idx;
  for  (idx = registeredDrivers->begin ();  idx != registeredDrivers->end ();  idx++)
  {
    if  ((*idx)->driverNameLower == driverNameLower)
    {
      result =  *idx;
      break;
    }
  }

  GlobalGoalKeeper::EndBlock ();

  return  result;
}  /* LookUpDriver */






void  FeatureFileIO::RegisterDriver (FeatureFileIOPtr  _driver)
{
  GlobalGoalKeeper::StartBlock ();

  FeatureFileIOPtr  existingDriver = LookUpDriver (_driver->driverName);
  if  (existingDriver)
  {
    // We are trying to register two drivers with the same name;  we can not do that.
    cerr << endl << endl 
         << "FeatureFileIO::RegisterDriver     ***ERROR***" << endl 
         << endl
         << "             trying to register more than one FeatureFileIO driver with " << endl
         << "             the same name[" << _driver->driverName << "]." << endl
         << endl;
  }
  else
  {
    registeredDrivers->push_back (_driver);
  }

  GlobalGoalKeeper::EndBlock ();
}  /* RegisterDriver */







/**
 * @brief  Before you terminate your application and after all FeatureFileIO activity is done
 *         call this method to destroy statically dynamically allocated data structures.
 * @details This method undoes everything that 'RegisterAllDrivers' does.  Its primary
 *          usefulness is aid in tracking down memory leaks.  This way the '_CrtSetDbgFlag'
 *          debugging functionality will not report the memory allocated by the different
 *          feature FileIO drivers.
 */
void FeatureFileIO::FinalCleanUp ()
{
  GlobalGoalKeeper::StartBlock ();

  if  (registeredDrivers)
  {
    while  (registeredDrivers->size () > 0)
    {
      FeatureFileIOPtr  f = registeredDrivers->back ();
      registeredDrivers->pop_back ();
      delete  f;
    }

    delete  registeredDrivers;
    registeredDrivers = NULL;
  }

  GlobalGoalKeeper::EndBlock ();
}  /* CleanUpFeatureFileIO */





FeatureFileIOPtr  FeatureFileIO::FileFormatFromStr  (const KKStr&  _fileFormatStr)
{
  return  LookUpDriver (_fileFormatStr);
}




FeatureFileIOPtr   FeatureFileIO::FileFormatFromStr (const KKStr&  _fileFormatStr,
                                                     bool          _canRead,
                                                     bool          _canWrite
                                                    )
{
  FeatureFileIOPtr   driver = LookUpDriver (_fileFormatStr);
  if  (!driver)
    return NULL;

  if  (_canRead  &&  (!driver->CanRead ()))
    return NULL;

  if  (_canWrite  &&  (!driver->CanWrite ()))
    return NULL;

  return  driver;
}  /* FileFormatFromStr */





KKStr  FeatureFileIO::FileFormatsReadOptionsStr ()
{
  vector<FeatureFileIOPtr>*  drivers = RegisteredDrivers ();

  KKStr  driversThatCanRead (128);

  vector<FeatureFileIOPtr>::const_iterator  idx;
  for  (idx = drivers->begin ();  idx != drivers->end ();  idx++)
  {
    FeatureFileIOPtr  driver = *idx;
    if  (driver->CanRead ())
    {
      if  (!driversThatCanRead.Empty ())
        driversThatCanRead << ", ";
      driversThatCanRead << driver->DriverName ();
    }
  }
  return driversThatCanRead;
}  /* FileFormatsReadOptionsStr */



KKStr  FeatureFileIO::FileFormatsWrittenOptionsStr ()
{
  vector<FeatureFileIOPtr>*  drivers = RegisteredDrivers ();

  KKStr  driversThatCanWrite (128);

  vector<FeatureFileIOPtr>::const_iterator  idx;
  for  (idx = drivers->begin ();  idx != drivers->end ();  idx++)
  {
    FeatureFileIOPtr  driver = *idx;
    if  (driver->CanWrite ())
    {
      if  (!driversThatCanWrite.Empty ())
        driversThatCanWrite << ", ";
      driversThatCanWrite << driver->DriverName ();
    }
  }
  return driversThatCanWrite;
}  /* FileFormatsWrittenOptionsStr */




KKStr  FeatureFileIO::FileFormatsReadAndWriteOptionsStr ()
{
  KKStr  driversThatCanReadAndWrite (128);

  vector<FeatureFileIOPtr>*  drivers = RegisteredDrivers ();

  vector<FeatureFileIOPtr>::const_iterator  idx;
  for  (idx = drivers->begin ();  idx != drivers->end ();  idx++)
  {
    FeatureFileIOPtr  driver = *idx;
    if  (driver->CanWrite ()  &&  driver->CanWrite ())
    {
      if  (!driversThatCanReadAndWrite.Empty ())
        driversThatCanReadAndWrite << ", ";
      driversThatCanReadAndWrite << driver->DriverName ();
    }
  }
  return driversThatCanReadAndWrite;
}  /* FileFormatsReadAndWriteOptionsStr */




VectorKKStr  FeatureFileIO::RegisteredDriverNames ()
{
  vector<FeatureFileIOPtr>*  drivers = RegisteredDrivers ();
  VectorKKStr  names;
  vector<FeatureFileIOPtr>::iterator  idx;

  for  (idx = drivers->begin ();  idx != drivers->end ();  idx++)
    names.push_back ((*idx)->DriverName ());

  return  names;
}  /* RegisteredDriverNames */






FeatureFileIO::FeatureFileIO (const KKStr&  _driverName,
                              bool          _canRead,
                              bool          _canWrite
                             ):
   canRead         (_canRead),
   canWrite        (_canWrite),
   driverName      (_driverName),
   driverNameLower (_driverName.ToLower ())
{
}


   
FeatureFileIO::~FeatureFileIO ()
{
}



void  FeatureFileIO::GetLine (istream&  _in,
                              KKStr&    _line,
                              bool&     _eof
                             )
{
  _line = "";
  if  (_in.eof ())
  {
    _eof = true;
    return;
  }

  kkint32  ch = _in.peek ();
  while  ((ch != '\n')  &&  (ch != '\r')  &&  (!_in.eof ()))
  {
    ch = _in.get ();
    _line.Append (ch);
    ch = _in.peek ();
  }

  if  (!_in.eof ())
  {
    _in.get ();  // Skip over eond of line character.
    if  (ch == '\n')
    {
      ch = _in.peek ();
      if  (ch == '\r')
        _in.get ();  // line is terminated by LineFeed + CarrageReturn;  we need to skip over both.
    }
    else if  (ch  == '\r')
    {
      ch = _in.peek ();
      if  (ch == '\n')
        _in.get ();  // line is terminated by CarrageReturn + LineFeed;  we need to skip over both.
    }
  }

  _eof = false;

  return;
}  /* GetLine */



void  FeatureFileIO::GetToken (istream&     _in,
                               const char*  _delimiters,
                               KKStr&       _token,
                               bool&        _eof, 
                               bool&        _eol
                              )
{
  _token = "";
  _eof = false;
  _eol = false;

  if  (_in.eof ())
  {
    _eof = true;
    _eol = true;
    return;
  }

  kkint32  ch;

  // Skip past any leading white space.
  ch = _in.peek ();
  while  ((ch == ' ')  &&  (!_in.eof ()))
  {
    _in.get ();
    ch = _in.peek ();
  }

  if  (_in.eof ())
  {
    _eof = true;
    _eol = true;
    return;
  }

  if  (ch == '\n')
  {
    _eol = true;
    _in.get ();
    if  (_in.peek () == '\r')
      _in.get ();
    return;
  }

  if  (ch == '\r')
  {
    _eol = true;
    _in.get ();
    if  (_in.peek () == '\n')
      _in.get ();
    return;
  }


  while  ((!_in.eof ())  &&  (ch != '\n')  &&  (ch != '\r')  &&  (strchr (_delimiters, ch) == NULL))
  {
    _in.get ();
    _token.Append ((char)ch);
    ch = _in.peek ();
  }

  if  (strchr (_delimiters, ch) != NULL)
  {
    // the next character was a delimiter;  in this case we want to remove from stream.
    _in.get ();
  }


  return;
}  /* GetToken */







FeatureVectorListPtr  FeatureFileIO::LoadFeatureFile 
                                      (const KKStr&   _fileName,
                                       MLClassList&   _mlClasses,
                                       kkint32        _maxCount,
                                       VolConstBool&  _cancelFlag,    // will be monitored,  if set to True  Load will terminate.
                                       bool&          _successful,
                                       bool&          _changesMade,
                                       RunLog&        _log
                                      )
{
  _log.Level (10) << "LoadFeatureFile  File[" << _fileName << "]  FileFormat[" << driverName << "]" << endl;
  
  _changesMade = false;

  kkint32  estimatedNumOfDataItems = -1;
  
  _successful = true;

  ifstream  in (_fileName.Str (), ios_base::in);
  if  (!in.is_open ())
  {
    _log.Level (-1) << "LoadFeatureFile   ***ERROR***      Error Opening File[" << _fileName << "]." << endl;
    _successful = false;
    return  NULL;
  }

  KKStr  errorMessage;
  
  FileDescPtr  fileDescFromFile = GetFileDesc (_fileName, in, &_mlClasses, estimatedNumOfDataItems, errorMessage, _log);
  if  (fileDescFromFile == NULL)
  {
    _log.Level (-1) << endl << endl 
                   << "FeatureFileIO::LoadFeatureFile    ***ERROR***     Loading Feature File[" << _fileName << "]" << endl
                   << endl;
    _successful = false;
    return NULL;
  }

  FileDescPtr fileDesc = FileDesc::GetExistingFileDesc (fileDescFromFile);

  in.clear ();
  in.seekg (0, ios::beg);

  FeatureVectorListPtr  examples = LoadFile (_fileName, fileDesc, _mlClasses, in, _maxCount, _cancelFlag, _changesMade, errorMessage, _log);
  if  (examples == NULL)
  {
    _successful = false;
  }
  else
  {
    examples->Compress ();
    _successful = true;
  }

  in.close ();

  return  examples;
}  /* LoadFeatureFile */




void   FeatureFileIO::AppendToFile (const KKStr&           _fileName,
                                    const FeatureNumList&  _selFeatures,
                                    FeatureVectorList&     _examples,
                                    kkuint32&              _numExamplesWritten,
                                    VolConstBool&          _cancelFlag,
                                    bool&                  _successful,
                                    RunLog&                _log
                                   )
{
  _log.Level (10) << "FeatureFileIO::AppendToFile - File[" << _fileName << "]." << endl;

  _successful = true;

  FileDescPtr  fileDesc = _examples.FileDesc ();

  ofstream out (_fileName.Str (), ios::app);

  if  (!out.is_open())
  {
    KKStr  err;
    err << "AppendToFile  Error Opening File[" << _fileName << "]";
    _log.Level (-1)  << endl
                     << "FeatureFileIO::AppendToFile    ***ERROR***"  << endl
                     << endl
                     << "                   " << err  << endl
                     << endl;
    osDisplayWarning (err);
    _successful = false;
    return;
  }

  KKStr  errorMessage;
  SaveFile (_examples, _fileName,  _selFeatures, out, _numExamplesWritten, _cancelFlag, _successful, errorMessage, _log);

  out.close ();

  return;
}  /* AppendToFile */




void  FeatureFileIO::SaveFeatureFile (const KKStr&           _fileName, 
                                      const FeatureNumList&  _selFeatures,
                                      FeatureVectorList&     _examples,
                                      kkuint32&              _numExamplesWritten,
                                      VolConstBool&          _cancelFlag,
                                      bool&                  _successful,
                                      RunLog&                _log
                                     )
{
  _log.Level (10) << "FeatureFileIO::SaveFeatureFile - File[" << _fileName << "]." << endl;

  ofstream  out (_fileName.Str ());
  if  (!out.is_open())
  {
    _log.Level (-1) << "***ERROR***, SaveFeatureFile, Opening File[" << _fileName << "]" << endl;
    _successful = false;
  }

  out.precision (9);

  FileDescPtr  fileDesc = _examples.FileDesc ();

  KKStr  errorMessage;
  SaveFile (_examples, _fileName, _selFeatures, out, _numExamplesWritten, _cancelFlag, _successful, errorMessage, _log);

  out.close ();
}  /* SaveFeatureFile */





void  FeatureFileIO::SaveFeatureFileMultipleParts (const KKStr&           _fileName, 
                                                   const FeatureNumList&  _selFeatures,
                                                   FeatureVectorList&     _examples,
                                                   VolConstBool&          _cancelFlag,
                                                   bool&                  _successful,
                                                   RunLog&                _log
                                                  )
{
  kkuint32  numExamplesWritten = 0;
  SaveFeatureFile (_fileName, _selFeatures, _examples, numExamplesWritten, _cancelFlag, _successful, _log);

  if  (_cancelFlag  ||  (!_successful))
    return;

  if  (_examples.QueueSize () > 64000)
  {
    kkint32  numPartsNeeded = (_examples.QueueSize () / 64000);
    if  ((_examples.QueueSize () % 64000) > 0)
      numPartsNeeded++;

    kkint32  maxPartSize = (_examples.QueueSize () / numPartsNeeded) + 1;

    kkint32  partNum = 0;
    FeatureVectorList::const_iterator idx = _examples.begin ();

    while  ((idx != _examples.end ())  &&  (_successful)  &&  (!_cancelFlag))
    {
      FeatureVectorListPtr  part = new FeatureVectorList (_examples.FileDesc (), false, _log);

      while  ((idx != _examples.end ())  &&  (part->QueueSize () < maxPartSize))
      {
        part->PushOnBack (*idx);
        idx++;
      }

      KKStr  partFileName = osRemoveExtension (_fileName) + "-" + 
                            StrFormatInt (partNum, "00") + "." +
                            osGetFileExtension (_fileName);

      SaveFeatureFile (partFileName, _selFeatures, *part, numExamplesWritten, _cancelFlag, _successful, _log);

      partNum++;
      delete  part; part = NULL;
    }
  }
}  /* SaveFeatureFileMultipleParts */













FeatureVectorListPtr  FeatureFileIO::LoadInSubDirectoryTree 
                         (FactoryFVProducerPtr  _fvProducerFactory,
                          KKStr                 _rootDir,
                          MLClassList&          _mlClasses,
                          bool                  _useDirectoryNameForClassName,
                          VolConstBool&         _cancelFlag, 
                          bool                  _rewiteRootFeatureFile,
                          RunLog&               _log
                         )
{
  _log.Level (10) << "FeatureFileIO::LoadInSubDirectoryTree    rootDir[" << _rootDir << "]." << endl;

  osAddLastSlash (_rootDir);

  KKStr  featureFileName ("");
  KKStr  fullFeatureFileName ("");

  if  (!_rootDir.Empty ())
  {
    featureFileName = osGetRootNameOfDirectory (_rootDir) + ".data";
    fullFeatureFileName = _rootDir + featureFileName;
  }
  else
  {
    featureFileName     = "Root.data";
    fullFeatureFileName = "Root.data";
  }

  MLClassPtr  unKnownClass = _mlClasses.GetUnKnownClass ();
  if  (_useDirectoryNameForClassName)
  {
    KKStr className = MLClass::GetClassNameFromDirName (_rootDir);
    unKnownClass    = _mlClasses.GetMLClassPtr (className);
  }

  bool  changesMade = false;

  FeatureVectorListPtr  dirImages = NULL;

  FileDescPtr  fileDesc = _fvProducerFactory->FileDesc ();

  if  (_rewiteRootFeatureFile)
  {
    DateTime  timeStamp;
    dirImages = FeatureDataReSink (_fvProducerFactory,
                                   _rootDir,
                                   featureFileName,
                                   unKnownClass,
                                   _useDirectoryNameForClassName,
                                   _mlClasses,
                                   _cancelFlag,
                                   changesMade,
                                   timeStamp,
                                   _log
                                  );
    if  (_useDirectoryNameForClassName)
    {
      FeatureVectorList::iterator  idx;
      for  (idx = dirImages->begin ();  idx != dirImages->end ();  idx++)
      {
        if  ((*idx)->MLClass () != unKnownClass)
        {
          (*idx)->MLClass (unKnownClass);
          changesMade = true;
        }
      }

      if  (changesMade)
      {
        KKStr  fullFileName = osAddSlash (_rootDir) + featureFileName;
        kkuint32  numExamplesWritten = 0;
        bool  cancel     = false;
        bool  successful = false;
        SaveFeatureFile (fullFileName, 
                         dirImages->AllFeatures (), 
                         *dirImages, 
                         numExamplesWritten,
                         cancel,
                         successful,
                         _log
                        );
      }
    }
  }
  else
  {
    dirImages =  _fvProducerFactory->ManufacturFeatureVectorList (true, _log);
  }

  // Now that we have processed all image files in "rootDir",
  // lets process any sub-directories.

  KKStr  dirSearchPath = osAddSlash (_rootDir) + "*.*";

  KKStrListPtr  subDirectories = osGetListOfDirectories (dirSearchPath);
  if  (subDirectories)
  {
    KKStrList::iterator  idx;

    for  (idx = subDirectories->begin ();  (idx != subDirectories->end ()  &&  (!_cancelFlag));   idx++)
    {
      KKStr  subDirName (**idx);
      if  (subDirName == "BorderImages")
      {
        // We ignore this director
        continue;
      }

      KKStr  newDirPath = osAddSlash (_rootDir) + subDirName;

      FeatureVectorListPtr  subDirImages = LoadInSubDirectoryTree (_fvProducerFactory,
                                                                   newDirPath, 
                                                                   _mlClasses, 
                                                                   _useDirectoryNameForClassName, 
                                                                   _cancelFlag,
                                                                   true,     // true = ReWriteRootFeatureFile
                                                                   _log
                                                                  );
      FeatureVectorPtr  fv = NULL;

      osAddLastSlash (subDirName);

      // We want to add the directory path to the ImageFileName so that we can later locate the source image.
      FeatureVectorList::iterator  idx = subDirImages->begin ();
      for  (idx = subDirImages->begin ();  idx != subDirImages->end ();  idx++)
      {
        fv = *idx;
        KKStr  newImageFileName = subDirName + fv->ImageFileName ();
        fv->ImageFileName (newImageFileName);
      }

      dirImages->AddQueue (*subDirImages);
      subDirImages->Owner (false);
      delete  subDirImages;
    }

    delete  subDirectories;  subDirectories = NULL;
  }

  _log.Level (10) << "LoadInSubDirectoryTree - Done" << endl;

  return  dirImages;
}  /* LoadInSubDirectoryTree */








FeatureVectorListPtr  FeatureFileIO::FeatureDataReSink (FactoryFVProducerPtr  _fvProducerFactory,
                                                        const KKStr&          _dirName,
                                                        const KKStr&          _fileName, 
                                                        MLClassPtr            _unknownClass,
                                                        bool                  _useDirectoryNameForClassName,
                                                        MLClassList&          _mlClasses,
                                                        VolConstBool&         _cancelFlag,
                                                        bool&                 _changesMade,
                                                        KKB::DateTime&        _timeStamp,
                                                        RunLog&               _log
                                                      )
{
  _changesMade = false;
  _timeStamp = DateTime ();

  FileDescPtr  fileDesc = _fvProducerFactory->FileDesc ();
  if  (_unknownClass == NULL)
    _unknownClass = MLClass::GetUnKnownClassStatic ();

  KKStr  className = _unknownClass->Name ();

  _log.Level (10) << "FeatureFileIO::FeatureDataReSink  dirName: " << _dirName << endl
                  << "               fileName: " << _fileName << "  UnKnownClass: " << className << endl;

  KKStr  fullFeatureFileName = osAddSlash (_dirName) +  _fileName;

  bool  successful;

  KKStr fileNameToOpen;
  if  (_dirName.Empty ())
    fileNameToOpen = _fileName;
  else
    fileNameToOpen = osAddSlash (_dirName) + _fileName;

  bool  versionsAreSame = false;

  FeatureVectorListPtr  origFeatureVectorData 
        = LoadFeatureFile (fileNameToOpen, _mlClasses, -1, _cancelFlag, successful, _changesMade, _log);

  if  (origFeatureVectorData == NULL)
  {
    successful = false;
    origFeatureVectorData = _fvProducerFactory->ManufacturFeatureVectorList (true, _log);
  }

  if  (_cancelFlag)
  {
    delete  origFeatureVectorData;  origFeatureVectorData = NULL;
    return  _fvProducerFactory->ManufacturFeatureVectorList (true, _log);
  }

  FeatureVectorListPtr  origFeatureData = NULL;

  if  ((&typeid (*origFeatureVectorData) == _fvProducerFactory->FeatureVectorListTypeId ())  &&
       ((*(origFeatureVectorData->FileDesc ())) ==  (*(_fvProducerFactory->FileDesc ())))
      )
  {
     origFeatureData = origFeatureVectorData;
  }
  else
  {
    origFeatureData = _fvProducerFactory->ManufacturFeatureVectorList (true, _log);
    delete  origFeatureVectorData;
    origFeatureVectorData = NULL;
  }

  KKStr  fileSpec = osAddSlash (_dirName) + "*.*";
  KKStrListPtr   fileNameList = osGetListOfFiles (fileSpec);

  if  (!fileNameList)
  {
    // There are no Image Files,  so we need to return a Empty List of Image Features.

    if  (origFeatureData->QueueSize () > 0)
      _changesMade = true;

    delete  origFeatureData;  origFeatureData = NULL;

    return  _fvProducerFactory->ManufacturFeatureVectorList (true, _log);
  }

  FeatureVectorProducerPtr  fvProducer = _fvProducerFactory->ManufactureInstance (_log);

  if  (successful)
  {
    if  (origFeatureData->Version () == fvProducer->Version ())
    {
      versionsAreSame = true;
      _timeStamp = osGetFileDateTime (fileNameToOpen);
    }

    else
    {
      _changesMade = true;
    }
  }
  else
  {
    delete  origFeatureData;
    origFeatureData = _fvProducerFactory->ManufacturFeatureVectorList (true, _log);
  }

  origFeatureData->SortByRootName (false);


  FeatureVectorListPtr  extractedFeatures = _fvProducerFactory->ManufacturFeatureVectorList (true, _log);
  extractedFeatures->Version (fvProducer->Version ());

  fileNameList->Sort (false);

  KKStrList::iterator  fnIDX;
  fnIDX = fileNameList->begin ();   // fileNameList

  KKStrPtr  imageFileName;

  kkint32  numImagesFoundInOrigFeatureData = 0;
  kkint32  numOfNewFeatureExtractions = 0;

  for  (fnIDX = fileNameList->begin ();  (fnIDX != fileNameList->end ())  &&  (!_cancelFlag);  ++fnIDX)
  {
    imageFileName = *fnIDX;
    bool validImageFileFormat = SupportedImageFileFormat (*imageFileName);
    
    if  (!validImageFileFormat)
      continue;

    FeatureVectorPtr  origFV = origFeatureData->BinarySearchByName (*imageFileName);
    if  (origFV)
      numImagesFoundInOrigFeatureData++;

    if  (origFV  &&  versionsAreSame)
    {
      if  (_useDirectoryNameForClassName)
      {
        if  (origFV->MLClass () != _unknownClass)
        {
          _changesMade = true;
          origFV->MLClass (_unknownClass);
        }
      }

      else if  ((origFV->MLClass ()->UnDefined ())  &&  (origFV->MLClass () != _unknownClass))
      {
        _changesMade = true;
        origFV->MLClass (_unknownClass);
      }

      extractedFeatures->PushOnBack (origFV);
      origFeatureData->DeleteEntry (origFV);
    }
    else
    {
      // We either  DON'T have an original image    or    versions are not the same.

      KKStr  fullFileName = osAddSlash (_dirName) + (*imageFileName);
      FeatureVectorPtr fv = NULL;
      try
      {
        RasterPtr image = ReadImage (fullFileName);
        if  (image)
          fv = fvProducer->ComputeFeatureVector (*image, _unknownClass, NULL, _log);
        delete image;
        image = NULL;

      }
      catch  (...)
      {
        _log.Level (-1) << endl << endl
          << "FeatureDataReSink   ***ERROR***"  << endl
          << "       Exception occured calling constructor 'PostLarvaeFV'  trying to compute FeatureVector." << endl
          << endl;
        successful = false;
        fv = NULL;
      }

      if  (!successful)
      {
        _log.Level (-1) << " FeatureFileIOKK::FeatureDataReSink  *** ERROR ***, Processing Image File["
                       << imageFileName << "]."
                       << endl;
        delete  fv;
        fv = NULL;
      }

      else
      {
        _changesMade = true;
        fv->ImageFileName (*imageFileName);

        _log.Level (30) << fv->ImageFileName () << "  " << fv->OrigSize () << endl;
        extractedFeatures->PushOnBack (fv);
        numOfNewFeatureExtractions++;

        if  ((numOfNewFeatureExtractions % 100) == 0)
          cout << numOfNewFeatureExtractions << " Images Extracted." << endl;
      }
    }
  }

  if  (numImagesFoundInOrigFeatureData != extractedFeatures->QueueSize ())
    _changesMade = true;
  
  extractedFeatures->Version (fvProducer->Version ());

  if  ((_changesMade)  &&  (!_cancelFlag))
  {
    //extractedFeatures->WriteImageFeaturesToFile (fullFeatureFileName, RawFormat, FeatureNumList::AllFeatures (extractedFeatures->FileDesc ()));

    kkuint32  numExamplesWritten = 0;

    SaveFeatureFile (fullFeatureFileName,  
                     FeatureNumList::AllFeatures (extractedFeatures->FileDesc ()),
                     *extractedFeatures,
                     numExamplesWritten,
                     _cancelFlag,
                     successful,
                     _log
                    );

    _timeStamp = osGetLocalDateTime ();
  }

  delete  fvProducer;        fvProducer      = NULL;
  delete  fileNameList;      fileNameList    = NULL;
  delete  origFeatureData;   origFeatureData = NULL;

  _log.Level (10) << "FeatureDataReSink  Exiting  Dir: "  << _dirName << endl;

  return  extractedFeatures;
}  /* FeatureDataReSink */