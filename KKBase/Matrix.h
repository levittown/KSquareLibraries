/* Matrix.h -- A simple two dimensional floating point matrix.
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef  _MATRIX_
#define  _MATRIX_
//****************************************************************************************
//*                                   Matrix Class                                       *
//*                                                                                      *
//*                                                                                      *
//*  Supports two dimensional matrices.                                                  *
//*                                                                                      *
//*  Developed for Machine Learning Project for support of Import Vector Machine.        *
//*  Handles two dimensional matrixes.  Functions supported are matrix addition, sub-    *
//*  traction, multiplication, transpose, determinant, and inversion.  Where appropriate *
//*  arithmetic operators +, -, * were overloaded.  Addition, Subtraction and Multipli-  *
//*  cation can be done against either another matrix or scaler.                         *
//*                                                                                      *
//*======================================================================================*
//*                                                                                      *
//*     Date      Descriptions                                                           *
//*  ===========  ====================================================================== *
//*  Nov-11-2002  Initial Development.                                                   *
//*                                                                                      *
//****************************************************************************************

#include <fstream>

#include "KKBaseTypes.h"

namespace  KKB
{
  template<typename T>  class  Row; 

  /**
   *@class  Matrix
   *@brief  Supports two dimensional matrices.
   *@details  Developed for Machine Learning Project for support of Import Vector Machine.
   *  Handles two dimensional matrices.  Functions supported are matrix addition, subtraction
   *  multiplication, transpose, determinant, and inversion.  Where appropriate arithmetic
   *  arithmetic operators +, -, * were overloaded.  Addition, Subtraction and Multiplication
   *  can be done against either another matrix or scaler.  Also Transpose and Determinant
   *  operations are supported.
   */
  template<typename T>
  class  Matrix
  {
  public:
    typedef  Matrix*  MatrixPtr;
    typedef  Row<T>*  RowPtr;

    Matrix ();

    Matrix (kkuint32  _numOfRows,
            kkuint32  _numOfCols
           );

    Matrix (const Matrix&  _matrix);

    Matrix (Matrix&&  _matrix);

    Matrix (const std::vector<T>&  _v);

    ~Matrix ();

    template<typename U>
    static  MatrixPtr  BuildFromArray (kkuint32 numOfRows,
                                       kkuint32 numOfCols,
                                       U**   data
                                      );

    Matrix<T>&  operator=  (const Matrix<T>&  right);

    template<typename U>
    Matrix<T>&  operator=  (const std::vector<U>&  right);

    template<typename U>
    Matrix<T>&  operator*= (U  right);

    template<typename U>
    Matrix<T>&  operator+= (U  right);
  
    Matrix<T>&  operator+= (const Matrix<T>&  right);

    Matrix<T>   operator+  (const Matrix<T>&  right);

    Matrix<T>   operator+  (T  right);

    Matrix<T>   operator-  (const Matrix<T>&  right);  

    Matrix<T>   operator-  (T right);

    Matrix<T>   operator*  (const Matrix<T>&  right);

    Matrix<T>   operator*  (T right);

    Row<T>&     operator[] (kkuint32  rowIDX) const;

    friend  Matrix<T>  operator- (T left, const Matrix<T>& right);


    Matrix<T>*     CalcCoFactorMatrix ();

    /**
     *@brief  Returns a Covariance matrix.
     *@details  Each column represents a variable and each row represents an instance of each variable.
     *@return  Returns a symmetric matrix that will be (numOfRows x numOfRows) where each element will represent the covariance 
     *         between their respective variables.
     */
    Matrix<T>*      Covariance ()  const;

    T const * const *   Data ()  const  {return  data;}

    T**             DataNotConst ()  {return data;}
     
    T               Determinant ();

    T               DeterminantSlow ();  /**<  @brief Recursive Implementation. */

    void            EigenVectors (Matrix<T>*&       eigenVectors,
                                  std::vector<T>*&  eigenValues
                                 )  const;

    /** @brief  Locates the maximum value in a matrixw1 along with the row and column that is located. */
    void            FindMaxValue (T&         maxVal, 
                                  kkuint32&  row, 
                                  kkuint32&  col
                                 );

    std::vector<T>  GetCol (kkuint32 col)  const;

    Matrix<T>       Inverse ();

    kkuint32        NumOfCols () const  {return numOfCols;}

    kkuint32        NumOfRows () const  {return numOfRows;}

    void            ReSize (kkuint32 _numOfRows,
                            kkuint32 _numOfCols
                           );

    bool            Symmetric ()  const;  /**< Returns true is the matrix is Symmetric */

    Matrix<T>       Transpose ();

    friend  std::ostream&  operator<< (      std::ostream&  os, 
                                       const Matrix<T>&     matrix
                                      );

  private:
    void  Destroy ();

    void  AllocateStorage ();

    T  DeterminantSwap (T**       mat, 
                        kkuint32  offset
                       );

    T  CalcDeterminent (kkuint32*  rowMap,
                        kkuint32*  colMap,
                        kkuint32 size
                       );

    T  Pythag (const T a,
               const T b
              )  const;

    kkint32  Tqli (T*        d, 
                   T*        e,
                   kkuint32  n,
                   T**       z
                  )  const;

    void  Tred2 (T**       a, 
                 kkuint32  n, 
                 T*        d, 
                 T*        e
                )  const;

    T**       data;       /**< A two dimensional array that will index into 'dataArea'.       */
    T*        dataArea;   /**< one dimensional array that will contain all the matrices data. */
    kkuint32  alignment;
    kkuint32  numOfCols;
    kkuint32  numOfRows;
    RowPtr    rows;
    kkuint32  totNumCells; /**<  Total number of cells allocated  = (numOfRows x nmumOfCols). */
  };  /* matrix */


  typedef  Matrix<double> MatrixD;
  typedef  MatrixD*  MatrixDPtr;

  typedef  Matrix<float> MatrixF;

  typedef  Matrix<float>*   MatrixFPtr;
  typedef  Matrix<double>*  MatrixDPtr;

  template<typename T>
  class  Row
  {
  public:
     Row  ();
 
     Row (kkuint32  _numOfCols,
          T*        _cells
         );

     Row (const Row&  _row);

     ~Row ();


     T*  Cols ()  {return cells;}

     T&  operator[] (kkuint32  idx);

     void  Define (kkuint32  _numOfCols,
                   T*        _cells
                  );

  private:
    T*  cells;
    kkuint32  numOfCols;
  };  /* Row */

  template<typename T>
  Matrix<T>  operator- (T left, const Matrix<T>& right);

  template<typename T>
  std::ostream&  operator<< (std::ostream&  os,
                             const Matrix<T>&     matrix
                            );
}  /* KKB */

#endif

