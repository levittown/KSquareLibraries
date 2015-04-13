#include  "FirstIncludes.h"

/**
 *@file SvmWrapper.cpp
 *@brief  Provides an interface to the svm.cpp functions.
 */

#include <algorithm>
#include <ctype.h>
#ifdef  WIN32
#include  "float.h"
#endif
#include <fstream>
#include <iostream>
#include <math.h>
#include <map>
#include <numeric>
#ifdef  WIN32
#include <ostream>
#endif
#include <set>
#include <sstream>
#include <stdio.h>
#include <string>
#include <time.h>
#include <vector>
#include "MemoryDebug.h"
using namespace std;


#include "KKBaseTypes.h"
#include "OSservices.h"
using namespace KKB;


#include "SvmWrapper.h"


#include "KKMLLTypes.h"
#include "svm.h"
using namespace KKMLL;



#ifdef  WIN32
#include "float.h"
#endif





#define Malloc (type,n) (type *)malloc((n)*sizeof(type))




void saveData (svm_problem  ds, 
               kkint32      begin, 
               kkint32      end, 
               std::string  name
              );



template<class T>
kkint32  GetMaxIndex (vector<T>&   vote, 
                    kkint32      voteLength,
                    kkint32&     maxIndex2  // second highest indx
                   )
{
  T max=vote[0];
  kkint32 maxIndex=0;

  T  max2 = 0;
  maxIndex2 = -1;

  for  (kkint32 i = 1;  i < voteLength;  i++)
  {
    if  (vote[i]> max)
    {
      max2      = max;
      maxIndex2 = maxIndex;
      max       = vote[i];
      maxIndex  = i;
    }

    else if  (maxIndex2 < 0)
    {
      max2      = vote[i];
      maxIndex2 = i;
    }

    else if  (vote[i] > max2)
    {
      max2 = vote[i];
      maxIndex2 = i;
    }
  }

  return maxIndex;
}  /* GetMaxIndex */




template<class T>
kkint32  GetMaxIndex (T*      vote, 
                    kkint32 voteLength,
                    kkint32&  maxIndex2  // second highest indx
                   )
{
  T max=vote[0];
  kkint32 maxIndex=0;

  T  max2 = 0;
  maxIndex2 = -1;

  for  (kkint32 i = 1;  i < voteLength;  i++)
  {
    if  (vote[i]> max)
    {
      max2      = max;
      maxIndex2 = maxIndex;
      max       = vote[i];
      maxIndex  = i;
    }

    else if  (maxIndex2 < 0)
    {
      max2      = vote[i];
      maxIndex2 = i;
    }

    else if  (vote[i] > max2)
    {
      max2 = vote[i];
      maxIndex2 = i;
    }
  }

  return maxIndex;
}  /* GetMaxIndex */







bool  GreaterThan (kkint32 leftVotes,
                   double  leftProb,
                   kkint32 rightVotes,
                   double  rightProb
                  )
{
  if  (leftVotes < rightVotes)
    return false;

  else if  (leftVotes > rightVotes)
    return true;

  else if  (leftProb < rightProb)
    return false;
  
  else if  (leftProb > rightProb)
    return  true;
  
  else
    return  false;
}  /* GreaterThan */



void  GreaterVotes (bool     useProbability,
                    kkint32  numClasses,
                    kkint32*   votes,
                    double*  probabilities,
                    kkint32& pred1Idx,
                    kkint32& pred2Idx
                   )
{
  if  (useProbability)
  {
    pred1Idx = GetMaxIndex (probabilities, numClasses, pred2Idx);
    return;
  }

  kkint32 max1Votes = votes[0];
  double  max1Prob  = probabilities[0];
  pred1Idx = 0;

  kkint32 max2Votes = -1;
  double  max2Prob  = -1.0f;
  pred2Idx = -1;

  for  (kkint32 x = 1;  x < numClasses;  x++)
  {
    if  (GreaterThan (votes[x], probabilities[x], max1Votes, max1Prob))
    {
      max2Votes = max1Votes;
      max2Prob  = max1Prob;
      pred2Idx  = pred1Idx;
      max1Votes = votes[x];
      max1Prob  = probabilities[x];
      pred1Idx = x;
    }
    else if  ((pred2Idx < 0)  ||  GreaterThan (votes[x], probabilities[x], max2Votes, max2Prob))
    {
      max2Votes = votes[x];
      max2Prob  = probabilities[x];
      pred2Idx = x;
    }
  }
} /* GreaterVotes*/







void saveData (svm_problem  ds, 
               kkint32      begin, 
               kkint32      end, 
               string       name
              )
{
  ofstream out(name.c_str());
  if(!out.good())
  {
    cout << " cannot open " << name << endl;
    exit(-1);
  }
  for(kkint32 i=begin; i<end; i++)
  {
    svm_node* temp=ds.x[i];
    while(temp->index!=-1)
    {
      out << temp->value << ",";
      temp++;
    }
    out << ds.y[i] << endl;
  }
  out.close();
}


/**
 **@brief Will normailize probabilites such that the sum of all equal 1.0 and no one probability will be less than 'minProbability'.
 *@param[in]  numClasses     Number of classes represented in array 'probabilities'.
 *@param[in,out]  probabilities  Probabilites that are to be adjusted.
 *@param[in]      minProbability  Smallest probablity that any one class can have assigned to it.
 */
void  NormalizeProbabilitiesWithAMinumum (kkint32  numClasses,
                                          double*  probabilities,
                                          double   minProbability
                                         )
{
  double  sumGreaterOrEqualMin = 0.0;
  kkint32 numLessThanMin = 0;

  kkint32 x = 0;
  for  (x = 0;  x < numClasses;  ++x)
  {
    if  (probabilities[x] < minProbability)
      ++numLessThanMin;
    else
      sumGreaterOrEqualMin += probabilities[x];
  }

  double probLessMinTotal = numLessThanMin * minProbability;
  double probLeftToAllocate  = 1.0 - probLessMinTotal;  

  for  (x = 0;  x < numClasses;  ++x)
  {
    if  (probabilities[x] < minProbability)
      probabilities[x] = minProbability;
    else
      probabilities[x] = (probabilities[x] / sumGreaterOrEqualMin) * probLeftToAllocate;
  }
}  /* NormalizeProbabilitiesWithAMinumum */




void  ComputeProbForVoting  (kkint32          numClasses,             /**< Number of Classes. */
                             float            A,                      /**< probability parameter */
                             vector<double>&  dist,                   /**< Distances for each binary classifier from decision boundary. */
                             double**         crossClassProbTable,    /**< Two dimensional array that is 'numClass' by 'numClass';  will receive the probabilities between classes. */
                             kkint32*           votes,                  /**< Array 'numClasses' in length that will receive the number of Votes each class won. */
                             double*          probabilities,          /**< Array 'numClasses' in length that will receive the computed Probability for Each Class */
                             kkint32          knownClassNum,          /**< The Class that we knoe th eexampl eto be. */
                             double           confidence,             /**< Used for calculating 'compact'  probability must exceed this.  */
                             double&          compact                 /**< 'knownClassNum'  and  'confidence'  need to be provided. */
                            )
{
  compact = 0.0;

  kkint32 i;
  for  (i = 0;  i < numClasses;  i++)
    votes[i] = 0;

   kkint32 distIdx = 0;
   for  (i = 0;  i < (numClasses - 1); i++)
   {
     for  (kkint32 j = i + 1;  j < numClasses;  j++)
     {
       if  (dist[distIdx] > 0)
         votes[i]++;
       else
         votes[j]++;

       crossClassProbTable[i][j] = 0.0;
       crossClassProbTable[j][i] = 0.0;

       distIdx++;
     }
   }

   int  win1 = -1;
   int  win2 = -1;
   int  win3 = -1;
   int  win4 = -1;

   int  win1Idx = 0;
   int  win2Idx = 0;
   int  win3Idx = 0;
   int  win4Idx = 0;

   for  (i = 0;  i < numClasses;  ++i)
   {
     int  zed = votes[i];
     if  (zed >= win4)
     {
       win4 = zed;
       win4Idx = i;

       if  (win4 >= win3)
       {
         win4 = win3;
         win4Idx = win3Idx;
         win3 = zed;
         win3Idx = i;

         if  (win3 >= win2)
         {
           win3 = win2;
           win3Idx = win2Idx;
           win2 = zed;
           win2Idx = i;

           if  (win2 >= win1)
           {
             win2 = win1;
             win2Idx = win1Idx;
             win1 = zed;
             win1Idx = i;
           }
         }
       }
     }
   }

   if  (win1 == win2)
   {
     if  (win1 == win3)
     {
       if  (win1 == win4)
       {
         probabilities[win1Idx] = 0.25;
         probabilities[win2Idx] = 0.25;
         probabilities[win3Idx] = 0.25;
         probabilities[win4Idx] = 0.25;
       }
       else
       {
         probabilities[win1Idx] = 0.95 / 3.0;
         probabilities[win2Idx] = 0.95 / 3.0;
         probabilities[win3Idx] = 0.95 / 3.0;
         probabilities[win4Idx] = 0.05;
       }
     }
     else
     {
       probabilities[win1Idx] = 0.85 / 2.0;
       probabilities[win2Idx] = 0.85 / 2.0;
       probabilities[win3Idx] = 0.10;
       probabilities[win4Idx] = 0.05;
     }
   }

   else 
   {
     probabilities[win1Idx] = 0.65;
     if  (win2 == win3)
     {
       if  (win2 == win4)
       {
         probabilities[win2Idx] = 0.30 / 3.0;
         probabilities[win3Idx] = 0.30 / 3.0;
         probabilities[win4Idx] = 0.30 / 3.0;
       }
       else
       {
         probabilities[win2Idx] = 0.30 / 2.0;
         probabilities[win3Idx] = 0.30 / 2.0;
         probabilities[win4Idx] = 0.05;
       }
     }

     else
     {
       probabilities[win2Idx] = 0.20;
       if  (win3 == win4)
       {
         probabilities[win3Idx] = 0.15 / 2.0;
         probabilities[win4Idx] = 0.15 / 2.0;
       }
       else
       {
         probabilities[win3Idx] = 0.10;
         probabilities[win4Idx] = 0.05;
       }
     }
   }

   //NormalizeProbabilitiesWithAMinumum (numClasses, probabilities, 0.001);

   for  (i = 0;  i < numClasses;  ++i)
   {
     crossClassProbTable [win1Idx][i] = probabilities[win1Idx];
     crossClassProbTable [i][win1Idx] = 1.0 - probabilities[win1Idx];
     if  (i != win1Idx)
     {
       crossClassProbTable [win2Idx][i] = probabilities[win2Idx];
       crossClassProbTable [i][win2Idx] = 1.0 - probabilities[win2Idx];
       if  (i != win2Idx)
       {
         crossClassProbTable [win3Idx][i] = probabilities[win3Idx];
         crossClassProbTable [i][win3Idx] = 1.0 - probabilities[win3Idx];
         if  (i != win3Idx)
         {
           crossClassProbTable [win4Idx][i] = probabilities[win4Idx];
           crossClassProbTable [i][win4Idx] = 1.0 - probabilities[win4Idx];
         }
       }
     }
   }

   if  ((knownClassNum >= 0) &&  (knownClassNum < numClasses))
   {
     kkint32 maxIndex1 = -1;
     kkint32 maxIndex2 = -1;
     maxIndex1 = GetMaxIndex (probabilities, numClasses, maxIndex2);

     if  (probabilities[maxIndex1] >= confidence)
     {
       if  ((probabilities[knownClassNum] < 1.0f)  &&  (probabilities[knownClassNum] > 0.0f))
       {
         compact = -log ((double)probabilities[knownClassNum]);
       }
     }
   }
}  /* ComputeProbForVoting */






void  ComputeProb  (kkint32            numClasses,               // Number of Classes
                    const VectorFloat& probClassPairs,         // probability parameter
                    vector<double>&    dist,                   // Distances for each binary classifier from decision boundary.
                    double**           crossClassProbTable,    // A 'numClass' x 'numClass' matrix;  will get the probabilities between classes.
                    kkint32*           votes,                  // votes by class
                    double*            probabilities,          // Probabilities for Each Class
                    kkint32            knownClassNum           // -1 = Don't know the class otherwise the Number of the Class.
                   )
{
  kkint32  i;
  for  (i = 0;  i < numClasses;  i++)
    votes[i] = 0;

   kkint32 distIdx = 0;
   for  (i = 0;  i < (numClasses - 1); i++)
   {
     for  (kkint32 j = i + 1;  j < numClasses;  j++)
     {
       if  (dist[distIdx] > 0)
         votes[i]++;
       else
         votes[j]++;

       double tempProb = (double)(1.0 / (1.0 + exp (-1.0 * probClassPairs[distIdx] * dist[distIdx])));
       crossClassProbTable[i][j] = tempProb;
       crossClassProbTable[j][i] = (1.0 - tempProb);
       distIdx++;
     }
   }

   double  totalProb = 0.0;
   for  (i = 0;  i < numClasses;  i++)
   {
     double  probThisClass = 1.0;
     for  (kkint32 j = 0;  j < numClasses;  j++)
     {
       if  (i != j)
         probThisClass *= crossClassProbTable [i][j];
     }

     probabilities[i] = probThisClass;
     totalProb += probThisClass;
   }

   if  (totalProb == 0.0)
   {
     // I think this happens because we are using float pfor probTable and double for dist[]
     // For now we will give each class an equal probability.
     for  (i = 0; i < numClasses; i++)
       probabilities[i] = (1.0 / double(numClasses));
   }
   else
   {
     for  (i = 0;  i < numClasses;  i++)
       probabilities[i] = probabilities[i] / totalProb;
   }

   if  ((knownClassNum >= 0) &&  (knownClassNum < numClasses))
   {
     kkint32 maxIndex1 = -1;
     kkint32 maxIndex2 = -1;
     maxIndex1 = GetMaxIndex (probabilities, numClasses, maxIndex2);
   }
   //NormalizeProbabilitiesWithAMinumum (numClasses, probabilities, 0.002);
}  /* ComputeProb */






struct svm_model**  KKMLL::SvmTrainModel (const struct svm_parameter&  param,
                                                      struct       svm_problem&    subprob
                                                     )
{ 
  struct svm_model **submodel;

  kkint32 numSVM = param.numSVM;
  kkint32 sample = (kkint32) (param.sample);

  //kkint32 numClass=param.numClass;
  kkint32  sampleSV   = param.sampleSV;
  kkint32  boosting   = param.boosting;
  kkint32  dimSelect  = param.dimSelect;

  Learn_Type learnType;

  if  ((numSVM == 1)  &&  (sample == 100))
    learnType= NORMAL;

  else if  (dimSelect > 0)
    learnType = SUBSPACE;

  else if (boosting != 0)
    learnType=BOOSTING;

  else if(sampleSV!=0)
    learnType=SAMPLESV;

  else
    learnType=BAGGING;

  submodel = new svm_model* [numSVM];
  submodel[0] = svm_train (&subprob,  &param);

  return  submodel;
}  /* SvmTrainModel */






void   KKMLL::SvmPredictClass (SVMparam&               svmParam,
                               struct svm_model**      subModel,
                               const struct svm_node*  unknownClassFeatureData, 
                               kkint32*                votes,
                               double*                 probabilities,
                               kkint32                 knownClass,
                               kkint32&                predClass1,
                               kkint32&                predClass2,
                               kkint32&                predClass1Votes,
                               kkint32&                predClass2Votes,
                               double&                 predClass1Prob,
                               double&                 predClass2Prob,
                               double&                 probOfKnownClass,
                               Ivector&                winners,
                               double**                crossClassProbTable,
                               double&                 breakTie
                              )
{
  const struct svm_parameter&  param = svmParam.Param ();

  kkint32  NUMCLASS = subModel[0][0].nr_class;

  kkint32 numBinary = (NUMCLASS * (NUMCLASS - 1)) / 2;
  Dvector dist (numBinary, 0);


  svm_predict (subModel[0], unknownClassFeatureData, dist, winners, -1);


  ComputeProb  (NUMCLASS,
                svmParam.ProbClassPairs (),
                dist,                   // Distances for each binary classifier from decision boundary.
                crossClassProbTable,    // Will get Probabilities between classes.
                votes,
                probabilities,          // Probabilities for Each Class
                knownClass              // -1 = Don't know the class otherwise the Number of the Class.
               );

  GreaterVotes ((svmParam.SelectionMethod () == SelectByProbability),
                NUMCLASS,
                votes,
                probabilities,
                predClass1,
                predClass2
               );
  if  (predClass1 >= 0)
  {
    predClass1Votes    = votes[predClass1];
    predClass1Prob   = probabilities[predClass1];
  }

  if  (predClass2 >= 0)
  {
    predClass2Votes    = votes[predClass2];
    predClass2Prob   = probabilities[predClass2];
  }

  if  (knownClass >= 0)
    probOfKnownClass   = probabilities[knownClass];

  breakTie = (predClass1Prob - predClass2Prob);
}  /* SvmPredictClass */






kkint32  KKMLL::SvmPredictTwoClass (const svm_parameter&  param,
                                    svm_model**           submodel, 
                                    const svm_node*       unKnownData, 
                                    kkint32               desired, 
                                    double&               dist,
                                    double&               probability,
                                    kkint32               excludeSupportVectorIDX
                                   )
{
  if  (submodel[0]->nr_class != 2)
  {
    cerr << endl
         << endl
         << "SvmPredictTwoClass    *** ERROR ***" << endl
         << endl
         << "Number of classes should be equal to two." << endl
         << endl;
    osWaitForEnter ();
    exit (-1);
  }

  kkint32  v = kkint32 (svm_predictTwoClasses (submodel[0], unKnownData, dist, excludeSupportVectorIDX));

  probability = (1.0 / (1.0 + exp (-1.0 * param.A * dist)));

  return  v;
}  /* SvmPredictTwoClass */




void  KKMLL::SvmPredictRaw (svm_model**      submodel, 
                            const svm_node*  unKnownData,
                            double&          label,
                            double&          dist
                           )
{
  if  (submodel[0]->nr_class != 2)
  {
    cerr << endl
         << endl
         << "SvmPredictTwoClass    *** ERROR ***" << endl
         << endl
         << "Number of classes should be equal to two." << endl
         << endl;
    osWaitForEnter ();
    exit (-1);
  }

  dist = 0.0;

  label = svm_predictTwoClasses (submodel[0], unKnownData, dist, -1);

  return;
}  /* SvmPredictRaw */




void  KKMLL::SvmSaveModel (struct svm_model**  subModel,
                                       const char*         fileName,
                                       bool&               successfull
                                      )
{
  successfull = true;

  kkint32 x = svm_save_model (fileName, subModel[0]);
  successfull = (x == 0);
}



void  KKMLL::SvmSaveModel (ostream&             o,
                         struct  svm_model**  model
                        )
{
   Svm_Save_Model (o, model[0]);
}





struct svm_model**   KKMLL::SvmLoadModel (const char* fileName)
{
  svm_model**  models = new svm_model*[1];
  models[0] = svm_load_model(fileName);

  if  (models[0] == NULL)
  {
    delete  models;
    models = NULL;
  }
  return models;
}



struct svm_model**   KKMLL::SvmLoadModel (istream&  f,
                                        RunLog&   log
                                       )
{
  svm_model**  models = new svm_model*[1];
  models[0] = Svm_Load_Model (f, log);

  if  (models[0] == NULL)
  {
    delete  models;
    models = NULL;
  }

  return models;
}





void   KKMLL::SvmDestroyModel (struct svm_model**  subModel)
{
  svm_destroy_model (subModel[0]);
}
