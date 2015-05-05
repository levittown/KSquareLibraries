#include "FirstIncludes.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "MemoryDebug.h"
using namespace  std;


#include "GlobalGoalKeeper.h"
#include "KKBaseTypes.h"
#include "RunLog.h"
#include "KKStr.h"
using namespace  KKB;

#include "BinaryClassParms.h"
#include "FileDesc.h"
#include "MLClass.h"
using namespace KKMLL;

#include "svm.h"
using namespace  SVM233;


BinaryClassParms::BinaryClassParms (MLClassPtr               _class1,
                                    MLClassPtr               _class2,
                                    const svm_parameter&     _param,
                                    FeatureNumListConstPtr   _selectedFeatures,
                                    float                    _weight
                                   ):
    class1           (_class1),
    class2           (_class2),
    param            (_param),
    selectedFeatures (NULL),
    weight           (_weight)
{
  if  (_selectedFeatures)
    selectedFeatures = new FeatureNumList (*_selectedFeatures);
}
 



BinaryClassParms::BinaryClassParms (const BinaryClassParms&  binaryClassParms):
    class1           (binaryClassParms.class1),
    class2           (binaryClassParms.class2),
    param            (binaryClassParms.param),
    selectedFeatures (NULL),
    weight           (binaryClassParms.weight)
{
  if  (binaryClassParms.selectedFeatures)
    selectedFeatures = new FeatureNumList (*binaryClassParms.selectedFeatures);
}


  
BinaryClassParms::~BinaryClassParms ()
{
  delete  selectedFeatures;
  selectedFeatures = NULL;
}


kkuint16  BinaryClassParms::NumOfFeatures (FileDescPtr fileDesc) const
{
  return  SelectedFeaturesFD (fileDesc)->NumOfFeatures ();
}


void  BinaryClassParms::SelectedFeatures (const FeatureNumList&  _selectedFeatures)
{
  delete  selectedFeatures;
  selectedFeatures  = new FeatureNumList (_selectedFeatures);
}


void  BinaryClassParms::SelectedFeatures (FeatureNumListConstPtr  _selectedFeatures)
{
   delete selectedFeatures;
   selectedFeatures = new FeatureNumList (*_selectedFeatures);
}





KKStr   BinaryClassParms::Class1Name ()  const
{
  if  (class1)
    return  class1->Name ();
  else
    return "";
}  /* Class1Name */



KKStr   BinaryClassParms::Class2Name ()  const
{
  if  (class2)
    return class2->Name ();
  else
    return "";
}  /* Class2Name */


FeatureNumListConstPtr  BinaryClassParms::SelectedFeaturesFD (FileDescPtr fileDesc) const
{
  if  (!selectedFeatures)
    selectedFeatures = new FeatureNumList (fileDesc);
  return  selectedFeatures;
}



KKStr  BinaryClassParms::ToTabDelString ()  const
{
  KKStr  result;
  result << "Class1"            << "\t" << Class1Name ()                << "\t"
         << "Class2"            << "\t" << Class2Name ()                << "\t"
         << "Svm_Parameter"     << "\t" << param.ToCmdLineStr ()        << "\t";

  if  (selectedFeatures)
      result << "SelectedFeatures"  << "\t" << selectedFeatures->ToString () << "\t";

  result << "Weight"            << "\t" << weight;

  return  result;
}  /* ToTabDelString */



BinaryClassParmsPtr  BinaryClassParms::CreateFromTabDelStr (const KKStr&  _str,
                                                            RunLog&       _log
                                                           )
{
  MLClassPtr         class1    = NULL;
  MLClassPtr         class2    = NULL;
  svm_parameter*     svm_param = NULL;
  FeatureNumListPtr  selectedFeatures = NULL;
  float              weight    = 0.0f;

  KKStr  str (_str);
  while  (!str.Empty())
  {
    KKStr  field = str.ExtractQuotedStr ("\n\r\t", true);
    field.Upper ();
    KKStr  value = str.ExtractQuotedStr ("\n\r\t", true);

    if  (field == "CLASS1")
    {
      class1 = MLClass::CreateNewMLClass (value);
    }

    else if  (field == "CLASS2")
    {
      class2 = MLClass::CreateNewMLClass (value);
    }

    else if  (field == "SVM_PARAMETER")
    {
      svm_param = new svm_parameter (value);
    }

    else if  (field == "SELECTEDFEATURES")
    {
      selectedFeatures = FeatureNumList::ExtractFeatureNumsFromStr (value);
    }

    else if  (field == "WEIGHT")
      weight = float (atof (value.Str ()));
  }

  BinaryClassParmsPtr  binaryClassParms 
        = new  BinaryClassParms (class1, class2, *svm_param, selectedFeatures, weight);

  delete  selectedFeatures;  selectedFeatures = NULL;
  delete  svm_param;         svm_param        = NULL;

  return  binaryClassParms;
}  /* CreateFromTabDelStr */




kkint32  BinaryClassParms::MemoryConsumedEstimated ()  const
{
  kkint32  memoryConsumedEstimated = sizeof (*this)  + param.MemoryConsumedEstimated ();

  if  (selectedFeatures)
    memoryConsumedEstimated += selectedFeatures->MemoryConsumedEstimated ();

  return  memoryConsumedEstimated;
}



BinaryClassParmsList::BinaryClassParmsList (bool  _owner):
        KKQueue<BinaryClassParms> (_owner)
{
}



BinaryClassParmsList::BinaryClassParmsList (const BinaryClassParmsList&  binaryClassList):
        KKQueue<BinaryClassParms> (binaryClassList)
{
}



BinaryClassParmsList::BinaryClassParmsList (const  BinaryClassParmsList&  binaryClassList,
                                            bool                          _owner
                                           ):
        KKQueue<BinaryClassParms> (binaryClassList, _owner)
{
}


  
BinaryClassParmsList::~BinaryClassParmsList ()
{
}



kkint32  BinaryClassParmsList::MemoryConsumedEstimated ()  const
{
  kkint32  memoryConsumedEstimated = sizeof (BinaryClassParmsList);
  BinaryClassParmsList::const_iterator  idx;
  for  (idx = begin ();  idx != end ();  ++idx)
    memoryConsumedEstimated  += (*idx)->MemoryConsumedEstimated ();
  return  memoryConsumedEstimated;
}



BinaryClassParmsListPtr  BinaryClassParmsList::CreateFromXML (FILE*        i, 
                                                              FileDescPtr  fileDesc,
                                                              RunLog&      log
                                                             )
{
  BinaryClassParmsListPtr binaryClassParmsList = new BinaryClassParmsList (true);
  binaryClassParmsList->ReadXML (i, fileDesc, log);
  return  binaryClassParmsList;
}



BinaryClassParmsListPtr  BinaryClassParmsList::CreateFromXML (istream&     i, 
                                                              FileDescPtr  fileDesc,
                                                              RunLog&      log
                                                             )
{
  BinaryClassParmsListPtr binaryClassParmsList = new BinaryClassParmsList (true);
  binaryClassParmsList->ReadXML (i, fileDesc, log);
  return  binaryClassParmsList;
}




/**
 @brief  Returns the Average number of selected features.
 */
float  BinaryClassParmsList::FeatureCountNet (FileDescPtr fileDesc)  const
{
  if  (size () < 1)
    return 0.0f;

  const_iterator  idx;

  kkuint32  featureCountTotal = 0;

  for  (idx = begin ();  idx != end ();  idx++)
  {
    const BinaryClassParmsPtr bcp = *idx;
    featureCountTotal += bcp->NumOfFeatures (fileDesc);
  }

  return  (float)featureCountTotal / (float)size ();
}




BinaryClassParmsPtr  BinaryClassParmsList::LookUp (MLClassPtr  _class1,
                                                   MLClassPtr  _class2
                                                  )  const
{
  KeyField kf (_class1, _class2);
  ClassIndexType::const_iterator  idx;
  idx = classIndex.find (kf);
  if  (idx == classIndex.end ())
  {
    kf.class1 = _class2;
    kf.class2 = _class1;
    idx = classIndex.find (kf);
  }

  if  (idx == classIndex.end ())
    return NULL;
  else
    return idx->second;
}  /* LookUp */




BinaryClassParmsListPtr  BinaryClassParmsList::DuplicateListAndContents ()  const
{
  BinaryClassParmsListPtr  duplicatedQueue = new BinaryClassParmsList (true);

  for  (const_iterator idx = begin ();  idx != end ();  idx++)
  {
    BinaryClassParmsPtr e = *idx;
    duplicatedQueue->PushOnBack (new BinaryClassParms (*e));
  }
  
  return  duplicatedQueue;
}  /* DuplicateListAndContents */




void  BinaryClassParmsList::WriteXML (ostream&  o)  const
{
  o << "<BinaryClassParmsList>" << endl;
  
  const_iterator  idx;

  for  (idx = begin ();  idx != end ();  idx++)
  {
    o << "<BinaryClassParms>"                   << "\t"
      << (*idx)->ToTabDelString ().QuotedStr () << "\t"
      << "</BinaryClassParms>" 
      << endl;
  }
  
  o << "</BinaryClassParmsList>" << endl;
}  /* WriteXML */



void  BinaryClassParmsList::ReadXML (FILE*        i,
                                     FileDescPtr  fileDesc,
                                     RunLog&      log
                                    )
{
  DeleteContents ();

  char  buff  [10240];

  while  (fgets (buff, sizeof (buff), i))
  {
    KKStr  ln (buff);

    KKStr  field = ln.ExtractQuotedStr ("\n\r\t", true);
    field.Upper ();

    if  (field == "</BINARYCLASSPARMSLIST>")
    {
      break;
    }

    else if  (field == "<BINARYCLASSPARMS>")
    {
      KKStr  binaryClassParmsStr = ln.ExtractQuotedStr ("\n\r\t", true);
      PushOnBack (BinaryClassParms::CreateFromTabDelStr (binaryClassParmsStr, log));
    }
  }
}  /* ReadXML */




void  BinaryClassParmsList::ReadXML (istream&     i,
                                     FileDescPtr  fileDesc,
                                     RunLog&      log
                                    )
{
  DeleteContents ();

  char  buff  [10240];

  while  (i.getline (buff, sizeof (buff)))
  {
    KKStr  ln (buff);

    KKStr  field = ln.ExtractQuotedStr ("\n\r\t", true);
    field.Upper ();

    if  (field == "</BINARYCLASSPARMSLIST>")
    {
      break;
    }

    else if  (field == "<BINARYCLASSPARMS>")
    {
      KKStr  binaryClassParmsStr = ln.ExtractQuotedStr ("\n\r\t", true);
      PushOnBack (BinaryClassParms::CreateFromTabDelStr (binaryClassParmsStr, log));
    }
  }
}  /* ReadXML */





void  BinaryClassParmsList::PushOnBack  (BinaryClassParmsPtr  binaryParms)
{
  BinaryClassParmsPtr  existingEntry = LookUp (binaryParms->Class1 (), binaryParms->Class2 ());
  if  (existingEntry)
  {
    // We have a duplicate entry
    KKStr  errMsg (128);
    errMsg << "BinaryClassParmsList::PushOnBack   ***ERROR***  Duplicate Entry   " << binaryParms->Class1Name () << "\t" << binaryParms->Class2Name () << endl;
    cerr << errMsg << endl;
    throw  KKException (errMsg);
  }

  KKQueue<BinaryClassParms>::PushOnBack (binaryParms);
  KeyField kf (binaryParms->Class1 (), binaryParms->Class2 ());
  classIndex.insert(ClassIndexPair (kf, binaryParms));
}  /* PushOnBack */




void  BinaryClassParmsList::PushOnFront (BinaryClassParmsPtr  binaryParms)
{
  BinaryClassParmsPtr  existingEntry = LookUp (binaryParms->Class1 (), binaryParms->Class2 ());
  if  (existingEntry)
  {
    // We have a duplicate entry
    KKStr  errMsg (128);
    errMsg << "BinaryClassParmsList::PushOnFront   ***ERROR***  Duplicate Entry   " << binaryParms->Class1Name () << "\t" << binaryParms->Class2Name () << endl;
    cerr << errMsg << endl;
    throw  KKException (errMsg);
  }

  KKQueue<BinaryClassParms>::PushOnFront (binaryParms);
  KeyField kf (binaryParms->Class1 (), binaryParms->Class2 ());
  classIndex.insert(ClassIndexPair (kf, binaryParms));
}  /* PushOnFront */





BinaryClassParmsList::KeyField::KeyField (MLClassPtr  _class1,  
                                          MLClassPtr  _class2
                                         ):
    class1 (_class1), 
    class2 (_class2)
{
}




bool  BinaryClassParmsList::KeyField::operator< (const KeyField& p2) const
{
  kkint32 x = class1->Name ().Compare(p2.class1->Name ());
  if  (x < 0)
    return true;

  else if  (x > 0)
    return false;

  else 
    return class2->Name () < p2.class2->Name ();
}










void  BinaryClassParmsList::WriteXML (const KKStr&  varName,
                                      ostream&      o
                                     )  const
{
  XmlTag  startTag ("BinaryClassParmsList",  XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.WriteXML (o);
  o << endl;

  for  (auto  idx : *this)
  {
    XmlContent::WriteXml (idx->ToTabDelString, o);
    o << endl;
  }


  XmlTag  endTag ("BinaryClassParmsList", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
  o << endl;
}  /* WriteXML */





void  BinaryClassParmsList::ReadXML (XmlStream&      s,
                                     XmlTagConstPtr  tag,
                                     RunLog&         log
                                    )
{
  DeleteContents ();
  classIndex.clear ();

  bool  errorsFound = false;
  XmlTokenPtr  t = s.GetNextToken (log);
  while  (t  &&  !errorsFound)
  {
    if  (t->TokenType () == XmlToken::TokenTypes::tokContent)
    {
      XmlContentPtr content = dynamic_cast<XmlContentPtr> (t);
      if  (!content)
        continue;

      BinaryClassParmsPtr  bcp = BinaryClassParms::CreateFromTabDelStr (*(content->Content ()), log);
      if  (bcp)
        PushOnBack (bcp);
    }
    delete  t;
    t = s.GetNextToken (log);
  }
}  /* ReadXML */


XmlFactoryMacro(BinaryClassParmsList)



