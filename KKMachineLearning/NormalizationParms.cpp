#include "FirstIncludes.h"

#include <stdio.h>
#include <math.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include "MemoryDebug.h"

using namespace  std;

#include "GlobalGoalKeeper.h"
#include "KKBaseTypes.h"
#include "OSservices.h"
#include "RunLog.h"
#include "XmlStream.h"
using namespace  KKB;

#include "NormalizationParms.h"
#include "FeatureNumList.h"
#include "FeatureVector.h"
#include "MLClass.h"
#include "ModelParam.h"
#include "TrainingConfiguration2.h"
using namespace  KKMLL;


NormalizationParms::NormalizationParms ():
  fileDesc                 (NULL),
  fileName                 (),
  mean                     (NULL),
  normalizeFeature         (NULL),
  normalizeNominalFeatures (false),
  numOfFeatures            (0),
  numOfExamples            (0),
  sigma                    (NULL)
{
}  /*  NormalizationParms */



NormalizationParms::NormalizationParms (bool                _normalizeNominalFeatures,
                                        FeatureVectorList&  _examples,
                                        RunLog&             _log
                                       ):

  fileDesc                 (NULL),
  fileName                 (),
  mean                     (NULL),
  normalizeFeature         (NULL),
  normalizeNominalFeatures (_normalizeNominalFeatures),
  numOfFeatures            (0),
  numOfExamples            (0),
  sigma                    (NULL)

{
  _log.Level (20) << "FeatureNormalization - Creating instance from[" << _examples.FileName () << "]." << endl;

  fileDesc      = _examples.FileDesc ();
  attriuteTypes = fileDesc->CreateAttributeTypeTable ();
  numOfFeatures = _examples.NumOfFeatures ();
  DeriveNormalizationParameters (_examples);
}  /*  NormalizationParms */



NormalizationParms::NormalizationParms (const ModelParam&   _param,
                                        FeatureVectorList&  _examples,
                                        RunLog&             _log
                                       ):
  fileDesc                 (NULL),
  fileName                 (),
  mean                     (NULL),
  normalizeFeature         (NULL),
  normalizeNominalFeatures (false),
  numOfFeatures            (0),
  numOfExamples            (0),
  sigma                    (NULL)
{
  _log.Level (20) << "FeatureNormalization - Creating instance from[" << _examples.FileName () << "]." << endl;

  fileDesc                 = _examples.FileDesc ();
  numOfFeatures = _examples.NumOfFeatures ();
  attriuteTypes            = fileDesc->CreateAttributeTypeTable ();
  normalizeNominalFeatures = _param.NormalizeNominalFeatures ();

  DeriveNormalizationParameters (_examples);
}



NormalizationParms::NormalizationParms (TrainingConfiguration2Ptr _config,
                                        FeatureVectorList&        _examples,
                                        RunLog&                   _log
                                       ):

  fileDesc                 (NULL),
  fileName                 (),
  mean                     (NULL),
  normalizeFeature         (NULL),
  normalizeNominalFeatures (false),
  numOfFeatures            (0),
  numOfExamples            (0),
  sigma                    (NULL)

{
  _log.Level (20) << "FeatureNormalization - Creating instance from[" << _examples.FileName () << "]." << endl;

  fileDesc                 = _config->FileDesc ();
  attriuteTypes            = fileDesc->CreateAttributeTypeTable ();
  normalizeNominalFeatures = _config->NormalizeNominalFeatures ();

  numOfFeatures = _examples.NumOfFeatures ();

  DeriveNormalizationParameters (_examples);
}  /*  NormalizationParms */






NormalizationParms::NormalizationParms (FileDescPtr  _fileDesc,
                                        KKStr        _fileName,
                                        bool&        _successfull,
                                        RunLog&      _log
                                       ):
  fileDesc                  (_fileDesc),
  fileName                  (_fileName),
  mean                      (NULL),
  normalizeFeature          (NULL),
  normalizeNominalFeatures  (false),
  numOfFeatures             (0),
  numOfExamples             (0),
  sigma                     (NULL)

{
  _log.Level (10) << "NormalizationParms::NormalizationParms - Loading instance from File[" << fileName << "]." << endl;

  attriuteTypes = fileDesc->CreateAttributeTypeTable ();

  KKStr  field;

  _successfull = true;

  FILE*  inputFile = osFOPEN (fileName.Str (), "r");
  if  (!inputFile)
  {
    _log.Level (-1) << "NormalizationParms::NormalizationParms   *** ERROR ***" << endl
                   << "                    Input File[" << fileName << "] is not Valid." << endl;
    _successfull = false;
    return;
  }

  Read (inputFile, _successfull, _log);

  fclose (inputFile);

  ConstructNormalizeFeatureVector ();

  return;
}


NormalizationParms::NormalizationParms (FileDescPtr  _fileDesc,
                                        FILE*        _in,
                                        bool&        _successfull,
                                        RunLog&      _log
                                       ):

  fileDesc                  (_fileDesc),
  fileName                  (),
  mean                      (NULL),
  normalizeFeature          (NULL),
  normalizeNominalFeatures  (false),
  numOfFeatures             (0),
  numOfExamples             (0),
  sigma                     (NULL)

{
  attriuteTypes  = fileDesc->CreateAttributeTypeTable ();

  Read (_in, _successfull, _log);

  ConstructNormalizeFeatureVector ();
}


NormalizationParms::NormalizationParms (FileDescPtr  _fileDesc,
                                        istream&     _in,
                                        bool&        _successfull,
                                        RunLog&      _log
                                       ):

  fileDesc                  (_fileDesc),
  fileName                  (),
  mean                      (NULL),
  normalizeFeature          (NULL),
  normalizeNominalFeatures  (false),
  numOfFeatures             (0),
  numOfExamples             (0),
  sigma                     (NULL)

{
  attriuteTypes  = fileDesc->CreateAttributeTypeTable ();

  Read (_in, _successfull, _log);

  ConstructNormalizeFeatureVector ();
}




NormalizationParms::~NormalizationParms ()
{
  delete [] mean;               mean             = NULL;
  delete [] sigma;              sigma            = NULL;
  delete [] normalizeFeature;   normalizeFeature = NULL;
}


kkint32  NormalizationParms::MemoryConsumedEstimated ()  const
{
  kkint32  memoryConsumedEstimated = sizeof (NormalizationParms)
    + attriuteTypes.size () * sizeof (AttributeType)
    + fileName.MemoryConsumedEstimated ()
    + numOfFeatures * (sizeof (bool) + sizeof (double) + sizeof (double));  //  mean + sigma

  return  memoryConsumedEstimated;
}



void  NormalizationParms::DeriveNormalizationParameters (FeatureVectorList&  _examples)
{
  numOfExamples   = 0;
  kkint32 numOfNoise    = 0;

  mean  = new double[numOfFeatures];
  sigma = new double[numOfFeatures];

  double*  total    = new double [numOfFeatures];
  double*  sigmaTot = new double [numOfFeatures];
 
  kkint32 i;

  for  (i = 0; i < numOfFeatures; i++)
  {
    mean[i]     = 0.0;
    sigma[i]    = 0.0;
    total[i]    = 0.0;
    sigmaTot[i] = 0.0;
  }

  double  featureValue;

  FeatureVectorPtr  image;

  FeatureVectorList::iterator imageIDX;

  for  (imageIDX = _examples.begin ();  imageIDX != _examples.end ();  imageIDX++)
  {
    image = *imageIDX;
    if  ((image->MLClass ()->UnDefined ())  ||  
         (image->MissingData ())               ||  
         (!image->FeatureDataValid ())
        )
    {
      // We have a noise image and do not want this as partof our Normalization 
      // procedure.
      numOfNoise++;
    }
    else
    {
      // Since this image is defined then we can use it in our normalization calculations.
      for  (i = 0; i < numOfFeatures; i++)
      {
        featureValue = double (image->FeatureData (i));
        total[i]    += featureValue;
      }

      numOfExamples++;
    }
  }

  for  (i = 0; i < numOfFeatures; i++)
  {
    double meanDouble  = total[i] / double (numOfExamples);
    mean[i]  = meanDouble;
  }


  for  (imageIDX = _examples.begin ();  imageIDX != _examples.end ();  imageIDX++)
  {
    image = *imageIDX;
    if  ((image->MLClass ()->UnDefined ())  ||  
         (image->MissingData ())               ||  
         (!image->FeatureDataValid ())
        )
    {
      // We have a noise image and do not want this as part of our Normalization 
      // procedure.
    }
    else
    {
      // Since this image is defined then we can use it in our normalization calculations.
      for  (i = 0; i < numOfFeatures; i++)
      {
        featureValue = double (image->FeatureData (i));
        double  delta = featureValue - mean[i];
        sigmaTot[i] += delta * delta;
      }
    }
  }

  for  (i = 0; i < numOfFeatures; i++)
  {
    sigma[i] = sqrt (sigmaTot[i] / numOfExamples);
  }

  delete[]  sigmaTot;
  delete[]  total;

  ConstructNormalizeFeatureVector ();

} /* DeriveNormalizationParameters */



void  NormalizationParms::Save (const KKStr&  _fileName,
                                bool&          _successfull,
                                RunLog&        _log
                               )
{
  _log.Level (20) << "NormalizationParms::Save  FileName[" << _fileName << "]." << endl;

  fileName = _fileName;

  _successfull = true;

  ofstream outFile (fileName.Str ());
  if  (!outFile.is_open ())
  {
    _log.Level (-1) << endl << "NormalizationParms::Save  ***EROR***  writing to file["<< _fileName << "]." << endl << endl;
    _successfull = false;
    return;
  }

  Write (outFile);

  outFile.close ();

  return;
} /* Save */



void  NormalizationParms::Write (ostream&  o)
{
  kkint32  i = 0;

  kkint32  origPrecision = (kkint32)o.precision();
  o.precision (12);

  o << "<NormalizationParms>"      << endl;
  o << "NumOfFeatures"             << "\t" << numOfFeatures                             << endl;
  o << "NumOfExamples"             << "\t" << numOfExamples                             << endl;
  o << "NormalizeNominalFeatures"  << "\t" << (normalizeNominalFeatures ? "Yes" : "No") << endl;

  o << "Means";
  for  (i = 0;  i < numOfFeatures;  i++)
    o << "\t" << mean[i];
  o << endl;


  o << "Sigmas";
  for  (i = 0;  i < numOfFeatures;  i++)
     o << "\t" << sigma[i];
  o << endl;

  o << "</NormalizationParms>" << endl;
  o.precision (origPrecision);
}  /* Write */




void  NormalizationParms::Read (FILE*    i,
                                bool&    sucessful,
                                RunLog&  log
                               )
{
  char  buff[40960];

  sucessful = true;

  numOfFeatures = -1;
  numOfExamples   = -1;
  delete[]  mean;   mean = NULL;
  delete[]  sigma;  sigma = NULL;

  while  (fgets (buff, sizeof (buff), i))
  {
    KKStr  ln (buff);
    KKStr  field = ln.ExtractQuotedStr ("\n\r\t", true);
    field.Upper ();

    if  (field == "</NORMALIZATIONPARMS>")
      break;

    if  (field == "NUMOFFEATURES")
    {
      numOfFeatures = ln.ExtractTokenInt ("\t");
      if  (numOfFeatures < 1)
      {
        log.Level (-1) << endl << endl << endl
                       << "NormalizationParms::Read      NumOfFeatures[" << numOfFeatures << "]" << endl
                       << endl
                       << "                              Must be greater than 0." << endl
                       << endl;
        sucessful = false;
        continue;
      }

      if  (numOfFeatures != fileDesc->NumOfFields ())
      {
        log.Level (-1) << endl << endl << endl
                       << "NormalizationParms::Read      NumOfFeatures dose not agree with FileDesc provided." << endl
                       << endl
                       << "           NumOfFeatures[" << numOfFeatures            << "]" << endl
                       << "           FileDesc     [" << fileDesc->NumOfFields () << "]" << endl
                       << endl;
        sucessful = false;
        continue;
      }

      mean  = new double[numOfFeatures];
      sigma = new double[numOfFeatures];

      kkint32 x;
      for  (x = 0;  x < numOfFeatures;  x++)
      {
        mean [x] = 0.0;
        sigma[x] = 0.0;
      }
    }

    else if  (field == "NUMOFEXAMPLES")
      numOfExamples = float (ln.ExtractTokenDouble ("\t"));

    
    else if  (field == "NORMALIZENOMINALFEATURES")
      normalizeNominalFeatures  = ln.ExtractTokenBool ("\t");


    else if  (field == "MEANS")
    {
      if  (!mean)
      {
        log.Level (-1) << endl 
                       << "NormalizationParms::Read  ***ERROR***    NumOfFeatures was not specified before 'Means'." << endl
                       << endl;
        sucessful = false;
        continue;
      }

      kkint32  x = 0;
      while  ((!ln.Empty ())  &&  (x < numOfFeatures))
      {
        mean[x] = ln.ExtractTokenDouble ("\t");
        x++;
      }
    }


    else if  (field == "SIGMAS")
    {
      if  (!sigma)
      {
        log.Level (-1) << endl 
                       << "NormalizationParms::Read   ***ERROR***  NumOfFeatures was not specified before 'Sigmas'." << endl
                       << endl;
        sucessful = false;
        continue;
      }

      kkint32  x = 0;
      while  ((!ln.Empty ())  &&  (x < numOfFeatures))
      {
        sigma[x] = ln.ExtractTokenDouble ("\t\n\r");
        x++;
      }
    }
  }
}  /* Read */





void  NormalizationParms::Read (istream&  i,
                                bool&     sucessful,
                                RunLog&   log
                               )
{
  char  buff[40960];

  sucessful = true;

  numOfFeatures = -1;
  numOfExamples   = -1;
  delete[]  mean;   mean = NULL;
  delete[]  sigma;  sigma = NULL;

  while  (i.getline( buff, sizeof (buff)))
  {
    KKStr  ln (buff);
    KKStr  field = ln.ExtractQuotedStr ("\n\r\t", true);
    field.Upper ();

    if  (field == "</NORMALIZATIONPARMS>")
      break;

    if  (field == "NUMOFFEATURES")
    {
      numOfFeatures = ln.ExtractTokenInt ("\t");
      if  (numOfFeatures < 1)
      {
        log.Level (-1) << endl
                       << "NormalizationParms::Read      NumOfFeatures[" << numOfFeatures << "]" << endl
                       << endl
                       << "                              Must be greater than 0." << endl
                       << endl;
        sucessful = false;
        continue;
      }

      if  (numOfFeatures != fileDesc->NumOfFields ())
      {
        log.Level (-1) << endl
                       << "NormalizationParms::Read      NumOfFeatures dose not agree with FileDesc provided." << endl
                       << endl
                       << "           NumOfFeatures[" << numOfFeatures            << "]" << endl
                       << "           FileDesc     [" << fileDesc->NumOfFields () << "]" << endl
                       << endl;
        sucessful = false;
        continue;
      }

      mean  = new double[numOfFeatures];
      sigma = new double[numOfFeatures];

      kkint32 x;
      for  (x = 0;  x < numOfFeatures;  x++)
      {
        mean [x] = 0.0;
        sigma[x] = 0.0;
      }
    }

    else if  (field == "NUMOFEXAMPLES")
      numOfExamples = (float)ln.ExtractTokenDouble ("\t");

    
    else if  (field == "NORMALIZENOMINALFEATURES")
      normalizeNominalFeatures  = ln.ExtractTokenBool ("\t");


    else if  (field == "MEANS")
    {
      if  (!mean)
      {
        log.Level (-1) << endl
                       << "NormalizationParms::Read      NumOfFeatures was not specified before 'Means'." << endl
                       << endl;
        sucessful = false;
        continue;
      }

      kkint32  x = 0;
      while  ((!ln.Empty ())  &&  (x < numOfFeatures))
      {
        mean[x] = ln.ExtractTokenDouble ("\t");
        x++;
      }
    }


    else if  (field == "SIGMAS")
    {
      if  (!sigma)
      {
        log.Level (-1) << endl
                       << "NormalizationParms::Read      NumOfFeatures was not specified before 'Sigmas'." << endl
                       << endl;
        sucessful = false;
        continue;
      }

      kkint32  x = 0;
      while  ((!ln.Empty ())  &&  (x < numOfFeatures))
      {
        sigma[x] = ln.ExtractTokenDouble ("\t\n\r");
        x++;
      }
    }
  }
}  /* Read */



void  NormalizationParms::WriteXML (const KKStr&  varName,
                                    ostream&      o
                                   )  const
{
  XmlTag  startTag ("NormalizationParms", XmlTag::TagTypes::tagStart);
  if  (!varName.Empty ())
    startTag.AddAtribute ("VarName", varName);
  startTag.WriteXML (o);
  o << endl;

  XmlElementInt32::WriteXML  (numOfFeatures,             "NumOfFeatures",            o);
  XmlElementDouble::WriteXML (numOfExamples,             "NumOfExamples",            o);
  XmlElementBool::WriteXML   (normalizeNominalFeatures,  "NormalizeNominalFeatures", o);

  if  (fileDesc)   XmlElementFileDesc::WriteXML    (*fileDesc,             "FileDesc", o);
  if  (mean)       XmlElementArrayDouble::WriteXML (numOfFeatures, mean,   "Mean",     o);
  if  (sigma)      XmlElementArrayDouble::WriteXML (numOfFeatures, sigma,  "sigma",    o);

  XmlTag  endTag ("NormalizationParms", XmlTag::TagTypes::tagEnd);
  endTag.WriteXML (o);
}



void  NormalizationParms::ReadXML (XmlStream&  s,
                                   RunLog&     log
                                  )
{
  XmlTokenPtr  t = s.GetNextToken (log);

  while  (t)
  {
    if  (t->TokenType () == XmlToken::TokenTypes::tokElement)
    {
      XmlElementPtr  e = dynamic_cast<XmlElementPtr> (t);
      const KKStr&  className = e->Name ();
      const KKStr&  varName = e->VarName ();
      if  (varName.EqualIgnoreCase ("NumOfFeatures"))
      {
        XmlElementInt32Ptr  nf = dynamic_cast<XmlElementInt32Ptr>(e);
        numOfFeatures = nf->Value ();
      }

      else if  (varName.EqualIgnoreCase ("NumOfExamples"))
      {
        XmlElementFloatPtr  noe = dynamic_cast<XmlElementFloatPtr>(e);
        numOfExamples = noe->Value ();
      }

      else if  (varName.EqualIgnoreCase ("NumOfExamples"))
      {
        XmlElementFloatPtr  noe = dynamic_cast<XmlElementFloatPtr>(e);
        numOfExamples = noe->Value ();
      }

      else if  (varName.EqualIgnoreCase ("NormalizeNominalFeatures"))
      {
        XmlElementBoolPtr  b = dynamic_cast<XmlElementBoolPtr>(e);
        normalizeNominalFeatures = b->Value ();
      }

      else if  (varName.EqualIgnoreCase ("FileDesc"))
      {
        XmlElementFileDescPtr  fd = dynamic_cast<XmlElementFileDescPtr>(e);
        fileDesc = fd->Value ();
      }

      else if  (varName.EqualIgnoreCase ("Mean"))
      {
        XmlElementArrayDoublePtr  m = dynamic_cast<XmlElementArrayDoublePtr>(e);
        if  (m->Count () == numOfFeatures)
        {
          delete  mean;
          mean = m->TakeOwnership ();
        }
        else
        {
          log.Level (-1) << endl
            << "XmlElementNormalizationParms   ***ERROR***    mean->Count[" << m->Count ()  << "] does not agree with NumOfFeatures[" << numOfFeatures << "]." <<endl
            << endl;
        }
      }

      else if  (varName.EqualIgnoreCase ("Sigma"))
      {
        XmlElementArrayDoublePtr  s = dynamic_cast<XmlElementArrayDoublePtr>(e);
        if  (s->Count () == numOfFeatures)
        {
          delete  sigma;
          sigma = s->TakeOwnership ();
        }
        else
        {
          log.Level (-1) << endl
            << "XmlElementNormalizationParms   ***ERROR***    sigma->Count[" << s->Count ()  << "] does not agree with NumOfFeatures[" << numOfFeatures << "]." <<endl
            << endl;
        }
      }
    }

    delete t;
    t = s.GetNextToken (log);
  }

  if  (fileDesc)
  {
    attriuteTypes = fileDesc->CreateAttributeTypeTable ();
    ConstructNormalizeFeatureVector ();
  }
}  /* ReadXML */






double  NormalizationParms::Mean (kkint32  i,
                                  RunLog&  log
                                 )
{
  if  ((i < 0)  ||  (i > numOfFeatures))
  {
    log.Level (-1) << "NormalizationParms::Mean   ***ERROR***   Feature Number[" << i << "]  out of bounds." << endl;
    return  -99999.99;
  }
  else
  {
    return  mean[i];
  }
}  /* Mean */





double  NormalizationParms::Sigma (kkint32  i,
                                   RunLog&  log
                                  )
{
  if  ((i < 0)  ||  (i > numOfFeatures))
  {
    log.Level (-1) << "NormalizationParms::Mean   ***ERROR***   Feature Number[" << i << "]  out of bounds." << endl;
    return  (float)-99999.99;
  }
  else
  {
    return  sigma[i];
  }
}  /* Sigma */




void  NormalizationParms::ConstructNormalizeFeatureVector ()
{
  delete  normalizeFeature;
  normalizeFeature = new bool [numOfFeatures];

  kkint32  i;
  for  (i = 0;  i < numOfFeatures;  i++)
  {
    if  (normalizeNominalFeatures)
    {
      normalizeFeature[i] = true;
    }

    else 
    {
      if  ((attriuteTypes[i] == AttributeType::NominalAttribute)  ||
           (attriuteTypes[i] == AttributeType::SymbolicAttribute)
          )
      {
        normalizeFeature[i] = false;
      }
      else
      {
        normalizeFeature[i] = true;
      }
    }
  }
}  /* ConstructNormalizeFeatureVector */




void  NormalizationParms::NormalizeAExample (FeatureVectorPtr  example)
{
  float*  featureData = example->FeatureDataAlter ();

  for  (kkint32 i = 0; i < numOfFeatures; i++)
  {
    if  (normalizeFeature[i])
    {
      double  normValue = 0.0;
      if  (sigma[i] != 0.0)
        normValue = ((double)featureData[i] - mean[i]) / sigma[i];
      featureData[i] = (float)normValue;
    }
  }
}  /* NormalizeAExample */






void  NormalizationParms::NormalizeExamples (FeatureVectorListPtr  examples,
                                             RunLog&               log
                                            )
{
  if  (numOfFeatures != examples->NumOfFeatures ())
  {
    log.Level (-1) << "NormalizationParms::NoralizeImage  **** ERROR ****     Mismatched Feature Count." << endl
                   << "            NormalizationParms [" << numOfFeatures            << "]" << endl
                   << "            ImageFeatiresList  [" << examples->NumOfFeatures () << "]."  << endl
                   << endl;

    osWaitForEnter ();
    exit (-1);
    return;
  }

  FeatureVectorList::iterator idx;

  for  (idx = examples->begin ();  idx != examples->end ();  ++idx)
    NormalizeAExample (*idx);

  return;
}  /* NoralizeImage */



FeatureVectorPtr  NormalizationParms::ToNormalized (FeatureVectorPtr  example)  const
{
  FeatureVectorPtr  result = new FeatureVector (*example);
  float*  featureData = result->FeatureDataAlter ();
  for  (kkint32 i = 0;  i < numOfFeatures;  ++i)
  {
    if  (normalizeFeature[i])
    {
      double  normValue = 0.0;
      if  (sigma[i] != 0.0)
        normValue = ((double)featureData[i] - mean[i]) / sigma[i];
      featureData[i] = (float)normValue;
    }
  }

  return result;
}  /* ToNormalized */





XmlElementNormalizationParms::XmlElementNormalizationParms (XmlTagPtr   tag,
                                                            XmlStream&  s,
                                                            RunLog&     log
                                                           ):
  XmlElement (tag, s, log),
  value (NULL)
{
  value = new NormalizationParms ();
  value->ReadXML (s, log);
}


                
XmlElementNormalizationParms::~XmlElementNormalizationParms ()
{
  delete  value;
  value = NULL;
}



NormalizationParmsPtr  XmlElementNormalizationParms::Value ()  const
{
  return  value;
}



NormalizationParmsPtr  XmlElementNormalizationParms::TakeOwnership ()
{
  NormalizationParmsPtr   v = value;
  value = NULL;
  return  v;
}
    


void  XmlElementNormalizationParms::WriteXML (const NormalizationParms&  normPatms,
                                              const KKStr&               varName,
                                              ostream&                   o
                                             )
{
  normPatms.WriteXML (varName, o);
}


XmlFactoryMacro(NormalizationParms)
