/*
 * Copyright (c) 1999, Jesper Hansen. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither name of the company nor the names of its contributors may
 *    be used to endorse or promote products derived from this software
 *    without specific prior written permission. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

//-----------------------------------------------------------------------------
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
//-------------------------------------
//-------------------------------------
#include "CGIQueryParse.h"
//-----------------------------------------------------------------------------

damn::CGIQueryParse::CGIQueryParse( const char *string )
{
//	printf( "CGIQueryParse: [%s]\n", string );

	fArgs = 0;
	fOrgString = strdup( string );
	fArgList = NULL;
	
	char *cgipos = fOrgString;
	while( cgipos )
	{
		char *begincgi = cgipos;
		cgipos = strchr( begincgi, '&' );
		if( cgipos )
			*cgipos++ = 0;
		AddPair( begincgi );
	}
}

damn::CGIQueryParse::~CGIQueryParse()
{
	free( fOrgString );
	free( fArgList );
}

void damn::CGIQueryParse::AddPair( char *name_value )
{
	assert( name_value != NULL );

	char *value = strchr( name_value, '=' );
	if( value )
		*value++ = 0;
	AddPair( name_value, value );
}

const char *damn::CGIQueryParse::FindName( const char *name, const char *defstring )
{
	for( int i=0; i<fArgs; i++ )
		if( strcmp(fArgList[i*2],name) == 0 )
			return fArgList[i*2+1];
	return defstring;
}

void damn::CGIQueryParse::AddPair( char *name, char *value )
{
	assert( name != NULL );
	if( name[0] )
	{
		CGIDeescapeString( name, name, strlen(name)+1 );
		if( value )
			CGIDeescapeString( value, value, strlen(value)+1 );
		else
			value = "";

		char **newlist = (char**)realloc( fArgList, (fArgs+1)*2*sizeof(char*) );
		assert( newlist != NULL );
		if( newlist )
		{
			fArgList = newlist;
			fArgList[fArgs*2] = name;
			fArgList[fArgs*2+1] = value;
			fArgs++;
		}
	}
}

size_t damn::CGIDeescapeString( char *dst, const char *src, size_t maxlen )
{
	assert( dst != NULL );
	assert( src != NULL );
	assert( maxlen >= 1 ); // we need at least room for a tailing \0

	int srcpos = 0;
	int dstpos = 0;
	
	while( src[srcpos] && dstpos<(maxlen-1) )
	{
		int c = (unsigned char)src[srcpos++];
		if( c == '%' )
		{
			int c1 = (unsigned char)src[srcpos];
			int c2 = c1?(unsigned char)src[srcpos+1]:0;
			if( isxdigit(c1) && isxdigit(c2) )
			{
				dst[dstpos++] = (c1<='9'?c1-'0':toupper(c1)-'A'+10)<<4 | (c2<='9'?c2-'0':toupper(c2)-'A'+10);
				srcpos+=2;
			}
			else
				dst[dstpos++] = c;
		}
		else
			dst[dstpos++] = c;
	}
	dst[dstpos] = 0;
	
	return dstpos;
}

//-----------------------------------------------------------------------------
