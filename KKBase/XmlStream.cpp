/* XmlStream.cpp -- Class to XML Objects;  still in development.
 * Copyright (C) 1994-2011 Kurt Kramer
 * For conditions of distribution and use, see copyright notice in KKB.h
 */
#include "FirstIncludes.h"

#include <stdio.h>
#include <fstream>
#include <string.h>
#include <string>
#include <iostream>
#include <vector>

#include "MemoryDebug.h"

using namespace std;


#include "BitString.h"
#include "KKBaseTypes.h"
#include "KKException.h"
#include "Tokenizer.h"
using namespace KKB;


#include "XmlStream.h"



XmlAttributeList::XmlAttributeList ()
{
}


XmlAttributeList::XmlAttributeList (const XmlAttributeList&  attributes)
{
  XmlAttributeList::const_iterator  idx;
  for  (idx = attributes.begin ();  idx != attributes.end ();  ++idx)
    insert (*idx);
}



KKStrConstPtr  XmlAttributeList::LookUp (const KKStr&  name)  const
{
  XmlAttributeList::const_iterator  idx;
  idx = find (name);
  if  (idx == end ())
    return NULL;
  else
    return &(idx->second);
}

 


XmlStream::XmlStream (TokenizerPtr _tokenStream):
  tokenStream (_tokenStream)
{
}



XmlStream::~XmlStream ()
{
}



void  ReadWholeTag (istream&  i,
                    KKStr&    tagStr
                   )
{
  tagStr = "";
  while  (!i.eof ())
  {
    char nextCh = i.get ();
    if  (nextCh != 0)
      tagStr.Append (nextCh);
    if  (nextCh == '>')
      break;
  }

  return;
}  /* ReadWholeTag */



void  ExtractAttribute (KKStr&  tagStr, 
                        KKStr&  attributeName,
                        KKStr&  attributeValue
                       )
{
  kkint32  startIdx = 0;
  kkint32  len = tagStr.Len ();
  attributeName  = "";
  attributeValue = "";

  // Skip over lading spaces
  while  (startIdx < len)
  {
    if  (strchr ("\n\t\r ", tagStr[startIdx]) == NULL)
      break;
    startIdx++;
  }

  if  (startIdx >= len)
  {
    tagStr = "";
    return;
  }

  kkint32 idx = startIdx;

  // Skip until we find the '=' character.
  while  (idx < len)
  {
    if  (tagStr[idx] == '=')
      break;
    ++idx;
  }

  if  (idx >= len)
  {
    tagStr = "";
    return;
  }

  attributeName = tagStr.SubStrPart (startIdx, idx - 1);

  ++idx;  // Skip past '=' character.
  
  // Skip over leading spaces
  while  (idx < len)
  {
    if  (strchr ("\n\t\r ", tagStr[idx]) == NULL)
      break;
    ++idx;
  }

  if  (idx >= len)
  {
    tagStr = "";
    return;
  }

  int  valueStartIdx = idx;

  while  (idx < len)
  {
    if  (strchr ("\n\t\r ", tagStr[idx]) != NULL)
      break;
    ++idx;
  }

  attributeValue = tagStr.SubStrPart (valueStartIdx, idx - 1);

  tagStr = tagStr.SubStrPart (idx + 1);

  return;
}  /* ExtractAttribute */



XmlTag::XmlTag (istream&  i)
{
  tagType = tagNULL;

  if  (i.peek () == '<')
    i.get ();

  KKStr tagStr (100);
  ReadWholeTag (i, tagStr);

  if  (tagStr.FirstChar () == '/')
  {
    tagStr.ChopFirstChar ();
    tagType = tagEnd;
  }

  if  (tagStr.EndsWith ("/>"))
  {
    tagType = tagEmpty;
    tagStr.ChopLastChar ();
    tagStr.ChopLastChar ();
  }

  else if  (tagStr.LastChar () != '>')
  {
    tagType = tagStart;
  }

  else
  {
    if  (tagType == tagNULL)
      tagType = tagStart;
    tagStr.ChopLastChar ();
  }

  name.TrimLeft ();
  name.TrimRight ();

  name = tagStr.ExtractToken2 (" \n\r\t");
  KKStr attributeName (20);
  KKStr attributeValue (20);

  while  (!tagStr.Empty ())
  {
    ExtractAttribute (tagStr, attributeName, attributeValue);
    if  (!attributeName.Empty ())
      attributes.insert (pair<KKStr, KKStr> (attributeName, attributeValue));
  }
}




XmlTag::XmlTag (TokenizerPtr  tokenStream):
     tagType (tagNULL)
{
  // We are assumed to be at the beginning of a new tag.
  KKStrListPtr  tokens = tokenStream->GetNextTokens (">");
  if  (!tokens)
    return;

  KKStrPtr t = NULL;


  if  (tokens->QueueSize () < 1)
  {
    delete  tokens;
    return;
  }

  if  ((*tokens)[0] == "<")
  {
    t = tokens->PopFromFront ();
    delete  t;
    t = NULL;
  }

  if  (tokens->QueueSize () < 1)
  {
    // We have a empty tag with no name.
    delete  tokens;
    tokens = NULL;
    return;
  }


  if  (*(tokens->BackOfQueue ()) == ">")
  {
    t = tokens->PopFromBack ();
    delete  t;
    t = NULL;
  }

  if  (tokens->QueueSize () < 1)
  {
    // We have a empty tag with no name.
    delete  tokens;
    tokens = NULL;
    return;
  }

  if  (tokens->QueueSize () > 1)
  {
    if  (*(tokens->BackOfQueue ()) == "/")
    {
      tagType = tagEmpty;
      t = tokens->PopFromBack ();
      delete  t;
      t = NULL;
    }
  }

  t = tokens->PopFromFront ();
  if  (*t == "/")
  {
    tagType = tagEnd;
    delete  t;
    t = tokens->PopFromFront ();
  }

  if  (t)
  {
    // At this point "t" should be the name of the tag.
    name = *t;
    delete  t;
    t = NULL;


    // Everything else should be attribute pairs  (Name = value).

    t = tokens->PopFromBack ();
    while  (t != NULL)
    {
      KKStrPtr t2 = tokens->PopFromBack ();
      KKStrPtr t3 = tokens->PopFromBack ();
      while  ((*t2 != "=")  &&  (t3 != NULL))
      {
        delete t;
        t = t2;
        t2 = t3;
        t3 = tokens->PopFromBack ();
      }

      if  (t3 != NULL)
      {
        if  (attributes.LookUp (*t) == NULL)
          attributes.insert (pair<KKStr, KKStr>(*t1, *t3));
      }

      delete  t;   t  = NULL;
      delete  t2;  t2 = NULL;
      delete  t3;  t3 = NULL; 

      t = tokens->PopFromBack ();
    }
  }

  delete  tokens;  tokens = NULL;
}  /* XmlTag::XmlTag (TokenizerPtr  tokenStream) */




KKStrConstPtr  XmlTag::AttributeValue (const KKStr& attributeName)  const
{
  return  attributes.LookUp (attributeName);
} /* AttributeValue */




KKStrConstPtr  XmlTag::AttributeValue (const char* attributeName)  const
{
  return  attributes.LookUp (attributeName);
} /* AttributeValue */






/**  
 *@brief  Will return either a XmlElement or a XmlContent
 */
XmlTokenPtr  XmlStream::GetNextToken ()  /*!< Will return either a XmlElement or a XmlContent */
{
  if  (tokenStream->EndOfFile ())
    return NULL;

  KKStrConstPtr  tokenStr = tokenStream->Peek (0);
  if  (tokenStr == NULL)
    return NULL;

  if  ((*tokenStr) == "<")
    return  ProcessElement ();
  else
    return new XmlContent (tokenStream);
}  /* GetNextToken */




XmlElementPtr  XmlStream::ProcessElement ()
{
  // We are assuming that we are at the very beginning of a new element.  In this case
  // the very next thing we get should be a tag field.

  XmlTagPtr tag = new XmlTag (tokenStream);
  if  (!tag)
    return  NULL;

  XmlElementCreator creator = LookUpXmlElementCreator (tag->Name ());
  if  (creator)
    return  creator (tag, *this);
  else
    return NULL;
}  /* ProcessElement */






XmlToken::XmlToken (TokenTypes  _tokenType):
  tokenType (_tokenType)
{
}





XmlElement::XmlElement ():
   XmlToken (tokElement)
{
}


XmlElement::XmlElement (TokenizerPtr  _tokenStream):
   XmlToken (tokElement)
{
}



XmlContent::XmlContent (TokenizerPtr  _tokenStream):
   XmlToken (tokContent)
{
}






map<KKStr, XmlStream::XmlElementCreator>  XmlStream::xmlElementCreators;



/**
 @brief Register a 'XmlElementCreator' function with its associated name.
 @details  if you try to register the same function with the same name will generate a warning to 
           cerr.  If you try and register two different functions with the same name will throw 
           an exception.
 */
void   XmlStream::RegisterXmlElementCreator  (const KKStr&       elementName,
                                              XmlElementCreator  creator
                                             )
{
  map<KKStr, XmlElementCreator>::iterator  idx;
  idx = xmlElementCreators.find (elementName);
  if  (idx != xmlElementCreators.end ())
  {
    // A 'XmlElementCreator'  creator already exists with name 'elementName'.
    if  (idx->second == creator)
    {
      // Trying to register the same function,  No harm done.
      cerr << std::endl
           << "XmlStream::RegisterXmlElementCreator   ***WARNING***   trying to register[" << elementName << "] Creator more than once." << std::endl
           << std::endl;
      return;
    }
    else
    {
      // Trying to register two different 'XmlElementCreator' functions.  This is VERY VERY bad.
      KKStr  errMsg = "XmlStream::RegisterXmlElementCreator   ***WARNING***   Trying to register[" + elementName + "] as two different Creator functions.";
      cerr << std::endl << errMsg << std::endl;
      throw KKException (errMsg);
    }
  }

  xmlElementCreators.insert (pair<KKStr,XmlElementCreator>(elementName, creator));
}  /* RegisterXmlElementCreator */



XmlStream::XmlElementCreator  XmlStream::LookUpXmlElementCreator (const KKStr&  elementName)
{
  map<KKStr, XmlElementCreator>::iterator  idx;
  idx = xmlElementCreators.find (elementName);
  if  (idx == xmlElementCreators.end ())
    return NULL;
  else
    return idx->second;
}  /* LookUpXmlElementCreator */
