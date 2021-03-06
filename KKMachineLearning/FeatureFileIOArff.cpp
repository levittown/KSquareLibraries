#include  "FirstIncludes.h"
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <time.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "MemoryDebug.h"
using namespace  std;

#include "KKBaseTypes.h"
#include "DateTime.h"
#include "OSservices.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace  KKB;

#include "FeatureFileIOArff.h"
#include "FileDesc.h"
#include "MLClass.h"
using namespace  KKMLL;



FeatureFileIOArff  FeatureFileIOArff::driver;



FeatureFileIOArff::FeatureFileIOArff ():
   FeatureFileIO ("ARFF", false, true)
{
}



FeatureFileIOArff::~FeatureFileIOArff(void)
{
}



void   FeatureFileIOArff::SaveFile (FeatureVectorList&    _data,
                                    const KKStr&          _fileName,
                                    FeatureNumListConst&  _selFeatures,
                                    ostream&              _out,
                                    kkuint32&             _numExamplesWritten,
                                    VolConstBool&         _cancelFlag,
                                    bool&                 _successful,
                                    KKStr&                _errorMessage,
                                    RunLog&               _log
                                   )
{
  _log.Level (20) << "FeatureFileIOArff::SaveFile    FileName[" << _fileName << "]" << endl;

  _numExamplesWritten = 0;

  FileDescConstPtr  fileDesc = _data.FileDesc ();
  auto  attrTable = fileDesc->CreateAAttributeConstTable ();

  {
    _out << "% ARFF Format Definition: http://www.cs.waikato.ac.nz/~ml/weka/arff.html"  << endl
         << "%"                                                             << endl
         << "% FileName         [" << _fileName                      << "]" << endl
         << "% DateWritten      [" << osGetLocalDateTime ()          << "]" << endl
         << "% SelectedFeatures [" << _selFeatures.ToString ()       << "]" << endl
         << "% TotalRecords     [" << _data.QueueSize ()             << "]" << endl
         << "% NumAttributes    [" << _selFeatures.NumOfFeatures ()  << "]" << endl
         << "%"                                                      << endl
         << "%  ClassName" << "\t" << "Count"                        << endl;
    
    ClassStatisticListPtr  classStatistics = _data.GetClassStatistics ();
    KKStr  classListStr (classStatistics->QueueSize () * 15);
    for  (kkuint32 x = 0;  x < classStatistics->QueueSize ();  x++)
    {
      ClassStatisticPtr  classStatistic = classStatistics->IdxToPtr (x);
      _out << "%  " << classStatistic->Name () << "\t" << classStatistic->Count () << endl;
      if  (x > 0)
        classListStr << ", ";
      classListStr << classStatistic->Name ();
    }

    _out << "%"  << endl
         << "%  Total" << "\t" << _data.QueueSize () << endl
         << "%"                                << endl
         << "%"                                << endl
         << "@relation  image_features"        << endl
         << "%"                                << endl;

    for  (kkint32 fnIDX = 0;  fnIDX < _selFeatures.NumOfFeatures ();  fnIDX++)
    {
      kkint32  featureNum = _selFeatures[fnIDX];
      AttributeConstPtr attr = attrTable[featureNum];
      _out << "@attribute " 
           << attr->Name ()
           << " ";

      if  ((attr->Type () == AttributeType::Nominal)   ||
           (attr->Type () == AttributeType::Symbolic)
          )
      {
        _out << "(";
        for (kkint32 x = 0;  x < attr->Cardinality ();  x++)
        {
          if  (x > 0)
            _out << ",";
          _out << attr->GetNominalValue (x);
        }
        _out << ")" << endl;
      }
      else
      {
        _out << "NUMERIC";
      }

      _out << endl;
    }

    _out << "@attribute ExampleFileName  " << "string" << endl;

    _out << "@attribute Classes? { " << classListStr << " }" << endl;

    _out << endl
         << endl
         << "@data" << endl
         << "%"     << endl
         << "% " << _data.QueueSize () << " Instances" << endl
         << "%"     << endl;
    delete  classStatistics;
  }

  kkint32  numOfDigistsNeededInRowMask = Min (1, kkint32 (log10 (float (_data.QueueSize ()))) + 1);

  KKStr rowMask = "0";
  rowMask.RightPad (numOfDigistsNeededInRowMask, '0');

  FeatureVectorPtr   example = NULL;

  for  (kkuint32 idx = 0; idx < _data.QueueSize (); idx++)
  {
    example = _data.IdxToPtr (idx);

    for  (kkuint32 x = 0;  x < _selFeatures.NumOfFeatures ();  x++)
    {
      kkint32  featureNum = _selFeatures[x];

      if  ((attrTable[featureNum]->Type () == AttributeType::Nominal)  ||
           (attrTable[featureNum]->Type () == AttributeType::Symbolic)
          )
        _out << attrTable[featureNum]->GetNominalValue ((kkint32)(example->FeatureData (featureNum)));
      else
        _out << example->FeatureData (featureNum);
      _out << ",";
    }

    KKStr  imageFileName = osGetRootNameWithExtension (example->ExampleFileName ());
    if  (imageFileName.Empty ())
      imageFileName == "Row_" + StrFormatInt (idx, rowMask.Str ());
    _out << imageFileName << ",";
    _out << example->MLClassName ();
    _out << endl;
    _numExamplesWritten++;
  }

  _successful = true;
  delete  attrTable;

  _log.Level (50) << "FeatureFileIOArff::SaveFile  _cancelFlag: " << _cancelFlag 
    << "  _cancelFlag: " << "  _errorMessage: " << _errorMessage << endl;
  return;
}  /* SaveFile */
