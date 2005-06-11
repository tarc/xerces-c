/*
 * Copyright 1999-2000,2004 The Apache Software Foundation.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * $Id$
 */


// ---------------------------------------------------------------------------
//  Includes
// ---------------------------------------------------------------------------

#if HAVE_CONFIG_H
#	include <config.h>
#endif

#if HAVE_WCHAR_H
#	include <wchar.h>
#endif
#if HAVE_WCTYPE_H
#	include <wctype.h>
#endif

// Fill in for broken or missing wctype functions on some platforms
#if !HAVE_ISWSPACE
#	include <lib/iswspace.h>
#endif
#if !HAVE_TOWUPPER
#	include <lib/towupper.h>
#endif
#if !HAVE_TOWLOWER
#	include <lib/towlower.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "IconvTransService.hpp"
#include <xercesc/util/XMLUniDefs.hpp>
#include <xercesc/util/XMLUni.hpp>
#include <xercesc/framework/MemoryManager.hpp>


XERCES_CPP_NAMESPACE_BEGIN

// ---------------------------------------------------------------------------
//  Local, const data
// ---------------------------------------------------------------------------
static const int    gTempBuffArraySize = 1024;
static const XMLCh  gMyServiceId[] =
{
    chLatin_I, chLatin_C, chLatin_o, chLatin_n, chLatin_v, chNull
};


// ---------------------------------------------------------------------------
//  Local methods
// ---------------------------------------------------------------------------
static unsigned int getWideCharLength(const XMLCh* const src)
{
    if (!src)
        return 0;

    unsigned int len = 0;
    const XMLCh* pTmp = src;
    while (*pTmp++)
        len++;
    return len;
}



// ---------------------------------------------------------------------------
//  IconvTransService: Constructors and Destructor
// ---------------------------------------------------------------------------
IconvTransService::IconvTransService()
{
}

IconvTransService::~IconvTransService()
{
}


// ---------------------------------------------------------------------------
//  IconvTransService: The virtual transcoding service API
// ---------------------------------------------------------------------------
int IconvTransService::compareIString(  const   XMLCh* const    comp1
                                        , const XMLCh* const    comp2)
{
    const XMLCh* cptr1 = comp1;
    const XMLCh* cptr2 = comp2;

    while ( (*cptr1 != 0) && (*cptr2 != 0) )
    {
        wint_t wch1 = towupper(*cptr1);
        wint_t wch2 = towupper(*cptr2);
        if (wch1 != wch2)
            break;

        cptr1++;
        cptr2++;
    }
    return (int) ( towupper(*cptr1) - towupper(*cptr2) );
}


int IconvTransService::compareNIString( const   XMLCh* const    comp1
                                        , const XMLCh* const    comp2
                                        , const unsigned int    maxChars)
{
    unsigned int  n = 0;
    const XMLCh* cptr1 = comp1;
    const XMLCh* cptr2 = comp2;

    while (true && maxChars)
    {
        wint_t wch1 = towupper(*cptr1);
        wint_t wch2 = towupper(*cptr2);

        if (wch1 != wch2)
            return (int) (wch1 - wch2);

        // If either ended, then both ended, so equal
        if (!*cptr1 || !*cptr2)
            break;

        cptr1++;
        cptr2++;

        //  Bump the count of chars done. If it equals the count then we
        //  are equal for the requested count, so break out and return
        //  equal.
        n++;
        if (n == maxChars)
            break;
    }

    return 0;
}


const XMLCh* IconvTransService::getId() const
{
    return gMyServiceId;
}


bool IconvTransService::isSpace(const XMLCh toCheck) const
{
    return (iswspace(toCheck) != 0);
}


XMLLCPTranscoder* IconvTransService::makeNewLCPTranscoder()
{
    // Just allocate a new transcoder of our type
    return new IconvLCPTranscoder;
}

bool IconvTransService::supportsSrcOfs() const
{
    return true;
}


// ---------------------------------------------------------------------------
//  IconvTransService: The protected virtual transcoding service API
// ---------------------------------------------------------------------------
XMLTranscoder*
IconvTransService::makeNewXMLTranscoder(const   XMLCh* const
                                        ,       XMLTransService::Codes& resValue
                                        , const unsigned int            
                                        ,       MemoryManager* const)
{
    //
    //  NOTE: We don't use the block size here
    //
    //  This is a minimalist transcoding service, that only supports a local
    //  default transcoder. All named encodings return zero as a failure,
    //  which means that only the intrinsic encodings supported by the parser
    //  itself will work for XML data.
    //
    resValue = XMLTransService::UnsupportedEncoding;
    return 0;
}


void IconvTransService::upperCase(XMLCh* const toUpperCase) const
{
    XMLCh* outPtr = toUpperCase;
    while (*outPtr)
    {
        *outPtr = towupper(*outPtr);
        outPtr++;
    }
}


void IconvTransService::lowerCase(XMLCh* const toLowerCase) const
{
    XMLCh* outPtr = toLowerCase;
    while (*outPtr)
    {
        *outPtr = towlower(*outPtr);
        outPtr++;
    }
}


// ---------------------------------------------------------------------------
//  IconvLCPTranscoder: The virtual transcoder API
// ---------------------------------------------------------------------------
unsigned int IconvLCPTranscoder::calcRequiredSize(const char* const srcText
                                                  , MemoryManager* const)
{
    if (!srcText)
        return 0;

    unsigned int len=0;
    unsigned int size=strlen(srcText);
    for( unsigned int i = 0; i < size; ++len )
    {
        unsigned int retVal=::mblen( &srcText[i], MB_CUR_MAX );
        if( -1 == retVal ) 
            return 0;
        i += retVal;
    }
    return len;
}


unsigned int IconvLCPTranscoder::calcRequiredSize(const XMLCh* const srcText
                                                  , MemoryManager* const manager)
{
    if (!srcText)
        return 0;

    unsigned int  wLent = getWideCharLength(srcText);
    wchar_t       tmpWideCharArr[gTempBuffArraySize];
    wchar_t*      allocatedArray = 0;
    wchar_t*      wideCharBuf = 0;

    if (wLent >= gTempBuffArraySize)
        wideCharBuf = allocatedArray = (wchar_t*)
            manager->allocate
            (
                (wLent + 1) * sizeof(wchar_t)
            );//new wchar_t[wLent + 1];
    else
        wideCharBuf = tmpWideCharArr;

    for (unsigned int i = 0; i < wLent; i++)
    {
        wideCharBuf[i] = srcText[i];
    }
    wideCharBuf[wLent] = 0x00;

    const unsigned int retVal = ::wcstombs(NULL, wideCharBuf, 0);
    manager->deallocate(allocatedArray);//delete [] allocatedArray;

    if (retVal == ~0)
        return 0;
    return retVal;
}


bool IconvLCPTranscoder::transcode( const   XMLCh* const    toTranscode
                                    ,       char* const     toFill
                                    , const unsigned int    maxBytes
                                    , MemoryManager* const  manager)
{
    // Watch for a couple of pyscho corner cases
    if (!toTranscode || !maxBytes)
    {
        toFill[0] = 0;
        return true;
    }

    if (!*toTranscode)
    {
        toFill[0] = 0;
        return true;
    }

    unsigned int  wLent = getWideCharLength(toTranscode);
    wchar_t       tmpWideCharArr[gTempBuffArraySize];
    wchar_t*      allocatedArray = 0;
    wchar_t*      wideCharBuf = 0;

    if (wLent > maxBytes) {
        wLent = maxBytes;
    }

    if (maxBytes >= gTempBuffArraySize) {
        wideCharBuf = allocatedArray = (wchar_t*)
            manager->allocate
            (
                (maxBytes + 1) * sizeof(wchar_t)
            );//new wchar_t[maxBytes + 1];
    }
    else
        wideCharBuf = tmpWideCharArr;

    for (unsigned int i = 0; i < wLent; i++)
    {
        wideCharBuf[i] = toTranscode[i];
    }
    wideCharBuf[wLent] = 0x00;

    // Ok, go ahead and try the transcoding. If it fails, then ...
    size_t mblen = ::wcstombs(toFill, wideCharBuf, maxBytes);
    if (mblen == -1)
    {
        manager->deallocate(allocatedArray);//delete [] allocatedArray;
        return false;
    }

    // Cap it off just in case
    toFill[mblen] = 0;
    manager->deallocate(allocatedArray);//delete [] allocatedArray;
    return true;
}


bool IconvLCPTranscoder::transcode( const   char* const     toTranscode
                                    ,       XMLCh* const    toFill
                                    , const unsigned int    maxChars
                                    , MemoryManager* const  manager)
{
    // Check for a couple of psycho corner cases
    if (!toTranscode || !maxChars)
    {
        toFill[0] = 0;
        return true;
    }

    if (!*toTranscode)
    {
        toFill[0] = 0;
        return true;
    }

    unsigned int len = calcRequiredSize(toTranscode);
    wchar_t       tmpWideCharArr[gTempBuffArraySize];
    wchar_t*      allocatedArray = 0;
    wchar_t*      wideCharBuf = 0;

    if (len > maxChars) {
        len = maxChars;
    }

    if (maxChars >= gTempBuffArraySize)
        wideCharBuf = allocatedArray = (wchar_t*) manager->allocate
        (
            (maxChars + 1) * sizeof(wchar_t)
        );//new wchar_t[maxChars + 1];
    else
        wideCharBuf = tmpWideCharArr;

    if (::mbstowcs(wideCharBuf, toTranscode, maxChars) == -1)
    {
        manager->deallocate(allocatedArray);//delete [] allocatedArray;
        return false;
    }

    for (unsigned int i = 0; i < len; i++)
    {
        toFill[i] = (XMLCh) wideCharBuf[i];
    }
    toFill[len] = 0x00;
    manager->deallocate(allocatedArray);//delete [] allocatedArray;
    return true;
}


static void reallocString(char *&ref, size_t &size, MemoryManager* const manager, bool releaseOld)
{
	char *tmp = (char*)manager->allocate(2 * size * sizeof(char));
	memcpy(tmp, ref, size * sizeof(char));
	if (releaseOld) manager->deallocate(ref);
	ref = tmp;
	size *= 2;
}


char* IconvLCPTranscoder::transcode(const XMLCh* const toTranscode,
                                    MemoryManager* const manager)
{
    if (!toTranscode)
        return 0;

	size_t resultSize = gTempBuffArraySize;
	size_t srcCursor = 0, dstCursor = 0;
	char localBuffer[gTempBuffArraySize];
	char* resultString = localBuffer;

	while (toTranscode[srcCursor])
	{
		char mbBuf[MB_CUR_MAX];
		int len = wctomb(mbBuf, toTranscode[srcCursor++]), j;
		if (len < 0)
		{
			dstCursor = 0;
			break;
		}
		if (dstCursor + len >= resultSize - 1)
			reallocString(resultString, resultSize, manager, resultString != localBuffer);
		for (j=0; j<len; ++j)
			resultString[dstCursor++] = mbBuf[j];
	}

	if (resultString == localBuffer)
	{
		resultString = (char*)manager->allocate((dstCursor + 1) * sizeof(char));
		memcpy(resultString, localBuffer, dstCursor * sizeof(char));
	}

	resultString[dstCursor] = '\0';
	return resultString;
}



static void reallocXMLString(XMLCh *&ref, size_t &size, MemoryManager* const manager, bool releaseOld)
{
	XMLCh *tmp = (XMLCh*)manager->allocate(2 * size * sizeof(XMLCh));
	memcpy(tmp, ref, size * sizeof(XMLCh));
	if (releaseOld) manager->deallocate(ref);
	ref = tmp;
	size *= 2;
}

XMLCh* IconvLCPTranscoder::transcode(const char* const toTranscode,
                                     MemoryManager* const manager)
{
    if (!toTranscode)
        return 0;
	XMLCh localBuffer[gTempBuffArraySize];
	XMLCh* resultString = localBuffer;
	size_t resultSize = gTempBuffArraySize;
	size_t srcCursor = 0, dstCursor = 0, srcLen = strlen(toTranscode);

	for ( ;; )
	{
		wchar_t wcBuf[1];
		int len = mbtowc(wcBuf, toTranscode + srcCursor, srcLen - srcCursor);
		if (len <= 0)
		{
			if (len < 0)
				dstCursor = 0;
			break;
		}
		srcCursor += len;
		if (dstCursor + 1 >= resultSize - 1)
			reallocXMLString(resultString, resultSize, manager, resultString != localBuffer);
		resultString[dstCursor++] = wcBuf[0];
	}

	if (resultString == localBuffer)
	{
		resultString = (XMLCh*)manager->allocate((dstCursor + 1) * sizeof(XMLCh));
		memcpy(resultString, localBuffer, dstCursor * sizeof(XMLCh));
	}

	resultString[dstCursor] = L'\0';
	return resultString;
}


// ---------------------------------------------------------------------------
//  IconvLCPTranscoder: Constructors and Destructor
// ---------------------------------------------------------------------------
IconvLCPTranscoder::IconvLCPTranscoder()
{
}

IconvLCPTranscoder::~IconvLCPTranscoder()
{
}

XERCES_CPP_NAMESPACE_END