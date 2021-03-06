/* OSservices.h -- O/S related functions, meant to be O/S neutral
 * Copyright (C) 1994-2014 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#ifndef  _OSSERVICES_
#define  _OSSERVICES_

/*
 *@file OSServices.h
 *@author  Kurt Kramer
 *@brief  General module for functions that interact with a specific operating system.
 *@details The idea is that as long as you use the functions in this module we should 
 *         be able to work in both Unix and Windows world.
 */

#include "DateTime.h"
#include "KKStr.h"

namespace KKB 
{
  typedef  long long  osFilePos;

  /**
   *@brief Will add the appropriate Directory separator character
   * to the end of fileSpec if one is not there already.
   */
  void           osAddLastSlash (KKStr&  fileSpec);

  KKB::KKStr     osAddSlash (const KKStr&  fileSpec); // Returns fileSpec with a directory separating character at end.

  void           osChangeDir (const KKStr&  dirName,
                              bool&         successful
                             );

  /**
   *@brief  Get a unique file name based off file spec 'fileName'.
   *@details  Make sure that fileName is unique, if file name already is used will add
   *          a sequence num to the root part of the name, starting with 0 and incrementing
   *          until file name is unique.
   */
  KKStr          osCreateUniqueFileName (KKStr  fileName);


  bool           osCopyFile (const KKStr&  srcFileName,
                             const KKStr&  destFileName,
                             bool          overwriteExisting
                            );

  bool           osCopyFileBetweenDirectories (const KKStr&  _fileName,
                                               const KKStr&  _srcDir,
                                               const KKStr&  _destDir
                                              );

  bool           osCreateDirectory (const KKStr&  dirName);


  /**  
   *@brief  Will create the whole Directory Path not just the final part of the path.
   *@details For example if you need a directory path "C:\\Temp\\Temple\\DataFiles"  created
   *         this function will create a directory in the specified path if they do not
   *         already exists.
   */
  bool           osCreateDirectoryPath (KKStr  _pathName);  

  bool           osDeleteFile (KKStr  _fileName);

  void           osDisplayWarning (KKStr  _message);

  bool           osFileExists (const KKStr&  _fileName);

  std::FILE*     osFOPEN (const char* fileName,
	  	                    const char* mode
                         );

  /**
   *@brief Calls the appropriate 64 bit function for operating system.
   */
  kkint64        osFTELL (std::FILE* f);

  int            osFSEEK (std::FILE*  f,
                          kkint64     offset,
                          int         origin
                         );

  //***************************************************************************
  //*  These next set of functions are meant to extract different information *
  //*  from path strings that might be just a directory path or a complete    *
  //*  file specification.                                                    *
  //***************************************************************************

  KKStr          osGetCurrentDirectory ();

  /**
   *@brief  Extracts the final sub-directory name of the fill directory specification.
   *@code
   **  ex:Input                              Returns
   **     -----------------------            ----------------------
   **     c:\an\Example\FileSpec\Romeo       Romeo
   **     /usr/kkramer/Kinsey/Release_2      Release_2
   **     /usr/kkramer/Thesis/               Thesis
   **     /usr/kkramer/Thesis.dat/           Thesis.dat
   *@endcode
   *@param[in]  dirPath Directory path to get dir name from .
   */
  KKStr          osGetDirNameFromPath (KKStr  dirPath);


  KKStr          osGetErrorNoDesc (kkint32  errorNo);


  /**
   *@brief  osGetFileNamePartOfFile, retrieves the file name part of the file spec.
   *@details
   *@code
   * ex:  Input (fullFileName)                 Returned
   *      --------------------------------_    ---------------
   *      c:\KurtUtilities\KKBase\KKStr.cpp    KKStr.cpp
   *      /bin/usr/KKramer/Dictionary.dat      Dictionary.dat
   *      dataFile.data                       dataFile.data
   *@endcode
   *@param[in]  fullFileName  Sting that contains the full file name specification.
   *@return File Name.
   */
  KKStr          osGetFileNamePartOfFile (const KKStr&  fullFileName);
  

  KKStr          osGetFileExtension (const KKStr&  fullFileName);


  /** @brief  returns the name and path of the current running application. */
  KKStr          osGetFullPathOfApplication ();


  /**
   *@brief  Returns the Parent directory path to 'dirPath'.
   *@details
   *@code
   **  ex:Input                              Returns
   **     -----------------------            ----------------------
   **     c:\an\Example\FileSpec\Romeo       c:\an\Example\FileSpec
   **     /usr/kkramer/Kinsey/Release_2      /usr/kkramer/Kinsey
   **     /usr/kkramer/Thesis/               /usr/kkramer
   **     /usr/kkramer/Thesis.dat/           /usr/kkramer
   *@endcode
   */
  KKStr          osGetParentDirPath (KKStr  dirPath);

  OptionKKStr    osGetHostName ();

  KKStr          osGetUserName ();

  /**
   *@brief  Given a full drive with path and name return the drive letter specified.
   *@details  If not on a Windows system returns x'00'= char(0)
   *@param[in]  pathName  File path to extract driver letter from.
   *@return Drive Letter ('A' - 'Z').
   */
  char           osGetDriveLetter (const KKStr&  pathName);   

  KKStrPtr       osGetEnvVariable (const KKStr&  _varName);

  KKB::DateTime  osGetFileDateTime (const KKStr& fileName);

  kkint64        osGetFileSize (const KKStr&  fileName);

  double         osGetKernalTimeUsed () noexcept;

  KKStrListPtr   osGetListOfFDirectoryEntries (const KKStr&  fileSpec, 
                                               bool          includeSubdirectories,
                                               bool          includeFiles
                                              );


  /**
   *@brief  Returns a list of files that match the provided file specification.
   *@details You can provide any file specification including a directory path. If there is a failure will return NULL.
   *         The caller will be responsible for deleting the returned list. The file names returned will not include
   *         any directory path.
   */
  KKStrListPtr  osGetListOfFiles (const KKStr&  fileSpec);


  void  osGetListOfFilesInDirectoryTree (const KKStr&  rootDir,
                                         KKStr         fileSpec,
                                         VectorKKStr&  fileNames   // The file names include full path.
                                        );

  KKStrListPtr  osGetListOfDirectories (const KKStr&  fileSpec);

  KKStrListPtr  osGetListOfImageFiles (const KKStr&  fileSpec);


  /**
   *@brief Returns mili-secs that system (Windows = has been started, Linux time in epoch).
   */
  kkuint64  osGetSystemTimeInMiliSecs () noexcept;


  /**
   *@brief Returned the current local date and time.
   */
  KKB::DateTime  osGetLocalDateTime ();

  KKStr          osGetParentDirectoryOfDirPath (KKStr  path);


  /**
   *@brief   Get the path part of a full file name specification.
   *@details
   *@code
   *    ex:Input                              Returns
   *       -----------------------            ----------------------
   *       c:\an\Example\FileSpec\data.txt    c:\an\Example\FileSpec
   *       /usr/kkramer/Plankton/Relase_2     /usr/kkramer/Plankton
   *       /usr/kkramer/Thesis/               /usr/kkramer/Thesis
   *@endcode
   *@param[in]  fullFileName    The full file specification including directory path, root-name, and extension.
   *@return  Path part of fullFileName.
   */
  KKStr          osGetPathPartOfFile (const KKStr&  fullFileName);   

  kkint32        osGetProcessId ();

  /** @brief returns the number of CPU's or number of simultaneous threads that you can have. */
  kkint32        osGetNumberOfProcessors ();

  KKStr          osGetProgName ();

  KKStr          osGetRootName (const KKStr&  fullFileName);

  KKStr          osGetRootNameOfDirectory (KKStr  fullDirName);

  KKStr          osGetRootNameWithExtension (const KKStr&  fullFileName);

  /**
   *@brief  Returns the number of CPU seconds used by current process.
   *@details  This consists of both User and Kernel Time.
   * When used to get timing data of some function, Call this method before and after running the function and use the difference.
   */
  double         osGetSystemTimeUsed ();

  kkint32        osGetThreadId ();

  double         osGetUserTimeUsed () noexcept;

  OptionUInt32   osLocateFirstSlashChar (const KKStr&  fileSpec);

  OptionUInt32   osLocateLastSlashChar (const KKStr&  fileSpec);


  /** 
   *@brief  Look for a specified file in a sub-directory structure.
   *@param[in]  fileName  Name of file we are looking for.
   *@param[in]  srcDir    Sub Directory tree we want to search.
   *@return  Full directory path to where first occurrence of fileName is located. If not found will return back an empty string.
   */
  KKStr         osLookForFile (const KKStr&  fileName,
                               const KKStr&  srcDir
                              );

                       
  KKStr         osMakeFullFileName (const KKStr&  _dirName,
                                    const KKStr&  _fileName
                                   );


  bool          osMoveFileBetweenDirectories (const KKStr&  _fileName,
                                              const KKStr&  _srcDir,
                                              const KKStr&  _destDir
                                             );

  void          osParseFileName (KKStr   _fileName, 
                                 KKStr&  _dirPath,
                                 KKStr&  _rootName, 
                                 KKStr&  _extension
                                );
  

  void          osParseFileSpec (KKStr   fullFileName,
                                 KKStr&  driveLetter,
                                 KKStr&  path,
                                 KKStr&  root,
                                 KKStr&  extension
                                );

  KKStr         osRemoveExtension (const KKStr&  _fullFileName);

  void          osSleep (float numOfSecs);

  void          osSleepMiliSecs (kkuint32  numMiliSecs);

  VectorKKStr   osSplitDirectoryPathIntoParts (const KKStr&  path);


  /**
   *@brief    Substitute in the value of the environment variables into 'src'.
   *@details  Will scan the 'src' string for references to environment variables. The
   * format would be $(env-var-name). These sub-strings in 'src' will be replaced by 
   * the the actual value of the environment variable. If no such environment variable
   * exists the substring will be left as is besides parentheses '{}' or '[]' can be
   * used, ex the string  "${KinseyHomeDir}\\Reports" would be replaced by "c:\Kinsey\reports"
   *@param[in]  src  String that you want to scan for environment variables.
   *@return  'src' string with environment variables expanded.
   */
  KKB::KKStr  osSubstituteInEnvironmentVariables (const KKStr&  src) noexcept;


  /**
   *@brief Read the next logical token from a file using characters in 'delimiters' to 
   *      separate tokens.
   *@details  Use this function to read logical tokens from a file. It recognizes the
   *          concept of a line that is terminated by the line feed ('\n') character.
   *          When end of line is reached it will return an empty string setting 'eol'
   *          to true;  the very next call will return the first token on the next
   *          line. The leading whitespace characters ' ' and '\\r' will be skipped
   *          before reading the next token. Tokens will be separated by any one of
   *          the characters specified in 'delimiters'. Two or more delimiters that
   *          occur consecutively will not be treated as one separator. You will need
   *          to call this method for each one.
   *
   *          ex:  the string  "Apple\tPeer\t\tGrape\t\n"  with the delimiters = "\t"
   *               will generate the following tokens when 
   *               "Apple", "Peer", "", "Grape", "", ""(eol = True)
   *@param[in]  in          File handle to read from.
   *@param[in]  delimiters  c string listing delimiter characters.
   *@param[out] eof         End of file flag. When this goes true so will eol, also 
   *                        no token will be returned.
   *@param[out] eol         Will be set to true when the end-of-line is reached ('\\n').
   *                        No token will be returned. The next call to this method 
   *                        will start at the next line.
   *@return  The next token read from the file. You must also monitor the 'eol' and 
   *         'eof' variables. If either one is true the no token will be returned.
   */
   KKB::KKStr   osReadNextToken (FILE*       in, 
                                 const char* delimiters,
                                 bool&       eof,
                                 bool&       eol
                                );
  
  /**
   *@brief Read the next logical token from a file using characters in 'delimiters' to 
   *      separate tokens where '\n'(end of line) is just another whitespace character.
   *@details  Use this function to read logical tokens from a file. Leading whitespace 
   *          characters ' ', '\\n', and '\\r' will be skipped before reading the 
   *          next token. Tokens will be separated by any one of the characters specified 
   *          in 'delimiters'. Two or more delimiters that occur consecutively will not 
   *          be treated as one separator. You will need to call this method for each one.
   *          Note that whitespace characters are not treated as delimiters but characters
   *          that are to be ignored before and after the token.
   *
   *          ex:  the string  "Apple\tPeer\t\tGrape\t\n"  with the delimiters = "\t"
   *               will generate the following tokens when 
   *               "Apple", "Peer", "", "Grape", "", ""(eol = True)
   *@param[in]  in          File handle to read from.
   *@param[in]  delimiters  c string listing delimiter characters.
   *@param[out] eof         End of file flag. When this goes true so will eol, also 
   *                        no token will be returned.
   *@return  The next token read from the file. You must also monitor the 'eol' and 
   *         'eof' variables. If either one is true the no token will be returned.
   */
  KKStr         osReadNextToken (FILE*       in, 
                                 const char* delimiters,
                                 bool&       eof
                                );
  
  /**
   *@brief Read the next logical token from a file using characters in 'delimiters' to 
   *       separate tokens.
   *@details  Use this function to read logical tokens from a input stream. It 
   *          recognizes the concept of a line that is terminated by the line feed 
   *          ('\n') character. When end of line is reached it will return an empty 
   *          string setting 'eol' to true; the very next call will return the first 
   *          token on the next line. The leading whitespace characters ' ' and '\]r' 
   *          will be skipped before reading the next token. Tokens will be separated 
   *          by any one of the characters specified in 'delimiters'. Two or more
   *          delimiters that occur consecutively will not be treated as one separator.
   *          You will need to call this method for each one.
   *
   *          ex:  the string  "Apple\tPeer\t\tGrape\t\n"  with the delimiters = "\t"
   *               will generate the following tokens when 
   *               "Apple", "Peer", "", "Grape", "", ""(eol = True)
   *@param[in]  in          Input Stream to read from.
   *@param[in]  delimiters  c string listing delimiter characters.
   *@param[out] eof         End of file flag. When this goes true so will eol, also 
   *                        no token will be returned.
   *@param[out] eol         Will be set to true when the end-of-line is reached ('\n').
   *                        No token will be returned. The next call to this method 
   *                        will start at the next line.
   *@return  The next token read from the file. You must also monitor the 'eol' and 
   *         'eof' variables. If either one is true the no token will be returned.
   */
  KKB::KKStr    osReadNextToken (std::istream&  in, 
                                 const char*    delimiters,
                                 bool&          eof,
                                 bool&          eol
                                );

  /**
   *@brief Extracts text up to and including the next end-of-line character and returns pointer to dynamical allocated str that caller will own.
   *@details The new-line character that terminated the nor any carriage-return will not be included. If stream [[in]] is already at 
   * end-of-file then will return NULL as well as eof being set to true.
   *@param  in   Stream to read text from.
   *@param  eof  Set to true if end-of-file is reached while reading file.
   */
  KKB::KKStrPtr  osReadRestOfLine (std::istream&  in,
                                   bool&          eof
                                  );

  KKB::KKStr     osReadRestOfLine2 (std::istream&  in,
                                    bool&          eof
                                   );

  KKB::KKStrPtr  osReadRestOfLine (FILE*  in,
                                   bool&  eof
                                  );

  KKB::KKStr     osReadRestOfLine2 (FILE*  in,
                                    bool&  eof
                                   );

  /**
   *@brief Read the next line from the file and return a c-String(NULL terminated).
   *@details  Will read in characters from 'in' until a line feed character is
   *          read. The caller will own the string returned and be responsible
   *          for deleting it.
   *@param[in]  in  File to read from.
   *@return  A dynamically allocated c-string (NULL Terminated) containing the next
   *         line in the file. If end-of-file reached will return a NULL pointer.
   */
  KKStrPtr    osReadNextLine (FILE*  in);
  
  
  
  /**
   *@brief Read the next Quoted String from the file. 
   *@details  If the first non whitespace character is a quote (") character will read 
   *          in rest of string until the next occurrence of a Quote character. Otherwise
   *          it will read in characters until another whitespace character occurs.
   *@param[in]  in  File to read from.
   *@param[in]  whiteSpaceCharacters  c-string (NULL Terminated) list of white space characters.
   *@param[out] eof  Will be set to true if we are at the end of file when you make the call.
   *@return  A String containing the next String in the file.
   */
  KKB::KKStr     osReadNextQuotedStr (FILE*        in,
                                      const char*  whiteSpaceCharacters,
                                      bool&        eof
                                     );

  /**
   *@brief  Skips rest of the characters in the current line in the file.
   *@details  Will skip all characters in 'in' until a '\n' is read or  the end of file 
   *          has been reached. It is meant be be used in conjunction with 
   *          'osReadNextToken'.
   *@param[in]  in    File being read.
   *@param[out] eof   Will be set to true if end of file is encountered.
   */
  void           osSkipRestOfLine (FILE*  in,
                                   bool&  eof
                                  );

  /**
   *@brief  Skips rest of the characters in the current line in the input stream.
   *@details  Will skip all characters in 'in' until a '\n' is read or the end of file 
   *          has been reached. It is meant be be used in conjunction with 
   *          'osReadNextToken'.
   *@param[in]  in    File being read.
   *@param[out] eof   Will be set to true if end of file is encountered.
   */
  void           osSkipRestOfLine (std::istream&  in,
                                   bool&          eof
                                  );

  bool           osRenameFile (const KKStr&  oldName,
                               const KKStr&  newName
                              );

  bool           osValidDirectory (KKStrConstPtr  _name);

  bool           osValidDirectory (const KKStr&  _name);

  bool           osValidFileName (const KKStr&  _name);
  
  /** @brief  Returns list of errors in '_name' with respect to it being a valid file name for the O/S. */
  KKStrListPtr   osValidFileNameErrors (const KKStr&  _name);

  bool           osIsBackGroundProcess ();

  void           osRunAsABackGroundProcess ();
  
  void           osWaitForEnter ();
  
  
  template<class T> 
      inline T* osGrowAllocation (T*       src, 
                                  kkint32  origSize,
                                  kkint32  newSize
                                 )
  {
    kkint32  zed = 0;
    T*  dest = new T[newSize];
    while  (zed < origSize)    {dest[zed] = src[zed];           ++zed;}
    while  (zed < newSize)     {dest[zed] = static_cast<T>(0);  ++zed;}
    delete  src;
    return  dest;
  }  /* GrowAllocation */


  
  #ifdef  WIN32
  #define  DS "\\"
  #define  DSchar  '\\'
  #else
  #define  DS "/"
  #define  DSchar '/'
  #endif
  
}  /* namespace KKB */

#endif

