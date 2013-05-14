/***
*
* BNBT Beta 7.4 - A C++ BitTorrent Tracker
* Copyright (C) 2003 Trevor Hogan
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*
***/

#include "StdAfx.h"
//#include "bnbt.h"
//#include "atom.h"
#include "bencode.h"
//#include "util.h"

string EncodeInt( const int x )
{
	char pBuf[128];

	memset( pBuf, 0, sizeof( char ) * 128 );

	sprintf( pBuf, "%d", x);

	string strDest;

	strDest += "i";
	strDest += pBuf;
	strDest += "e";

	return strDest;
}
/*
string EncodeLong( const Long &x )
{
	char pBuf[128];

	memset( pBuf, 0, sizeof( char ) * 128 );

#ifdef WIN32
	sprintf( pBuf, "%I64d", x.getValue( ) );
#else
	sprintf( pBuf, "%lld", x.getValue( ) );
#endif

	string strDest;

	strDest += "i";
	strDest += pBuf;
	strDest += "e";

	return strDest;
}
*/
string EncodeString( const string &x )
{
	char pBuf[128];

	memset( pBuf, 0, sizeof( char ) * 128 );

	sprintf( pBuf, "%d", (unsigned int)x.length() );

	string strDest;

	strDest += pBuf;
	strDest += ":";
	strDest += x;

	return strDest;
}
/*
string EncodeList( const CAtomList &x )
{
	vector<CAtom *> v = x.getValue( );

	string strDest;

	strDest += "l";

	for( unsigned long i = 0; i < v.size( ); i++ )
	{
		if( v[i]->isInt( ) )
			strDest += EncodeInt( *(CAtomInt *)v[i] );
		else if( v[i]->isLong( ) )
			strDest += EncodeLong( *(CAtomLong *)v[i] );
		else if( v[i]->isString( ) )
			strDest += EncodeString( *(CAtomstring )v[i] );
		else if( v[i]->isList( ) )
			strDest += EncodeList( *(CAtomList *)v[i] );
		else if( v[i]->isDicti( ) )
			strDest += EncodeDicti( *(CAtomDicti *)v[i] );
	}

	strDest += "e";

	return strDest;
}
*/
/*
string EncodeDicti( const AtomDicti &x )
{
	vector<string > vecKeys = x.v_Keys;
	vector<int *> vecValues = x.v_Values;

	string strDest;

	strDest += "d";

	for( unsigned long i = 0; i < vecKeys.size( ); i++ )
	{
		strDest += EncodeString( *vecKeys[i] );

		if( vecValues[i]->isInt( ) )
			strDest += EncodeInt( *(int *)vecValues[i] );
		else if( vecValues[i]->isLong( ) )
			strDest += EncodeLong( *(Long *)vecValues[i] );
		else if( vecValues[i]->isString( ) )
			strDest += EncodeString( *(string )vecValues[i] );
		else if( vecValues[i]->isList( ) )
			strDest += EncodeList( *(CAtomList *)vecValues[i] );
		else if( vecValues[i]->isDicti( ) )
			strDest += EncodeDicti( *(AtomDicti *)vecValues[i] );
	}

	strDest += "e";

	return strDest;
}
*/
/*
string Encode( CAtom *pAtom )
{
	if( pAtom->isInt( ) )
		return EncodeInt( *(CAtomInt *)pAtom );
	else if( pAtom->isLong( ) )
		return EncodeLong( *(CAtomLong *)pAtom );
	else if( pAtom->isString( ) )
		return EncodeString( *(CAtomstring )pAtom );
	else if( pAtom->isList( ) )
		return EncodeList( *(CAtomList *)pAtom );
	else if( pAtom->isDicti( ) )
		return EncodeDicti( *(CAtomDicti *)pAtom );

	return string( );
}

CAtomInt *DecodeInt( const string &x, unsigned long iStart )
{
	string :: intype iEnd = x.find( "e" );

	if( iEnd == string :: npos )
	{
		UTIL_LogPrint( "error decoding int - couldn't find \"e\", halting decode\n" );

		return NULL;
	}

	return new CAtomInt( atoi( x.substr( iStart + 1, iEnd - iStart - 1 ).c_str( ) ) );
}
*/

int DecodeInt(unsigned short  * buf, int * pos)
{
	char str_len[32+1];
	int ret_int;
	int leniter = 0;
	int cur_pos = *pos;
	while((buf[cur_pos]) != 'e')
	{
		if (buf[cur_pos] != 'i')
		{
			memcpy(&str_len[leniter],&buf[cur_pos],1);
			leniter++;
			//pos++;
		}
		cur_pos++;
	}
	// increment over the e
	cur_pos++;
	ret_int = atoi(str_len);

	*pos = cur_pos;

	return ret_int;
}

/*

CAtomLong *DecodeLong( const string &x, unsigned long iStart )
{
	string :: intype iEnd = x.find( "e", iStart );

	if( iEnd == string :: npos )
	{
		UTIL_LogPrint( "error decoding long - couldn't find \"e\", halting decode\n" );

		return NULL;
	}

	int64 i;

#ifdef WIN32
	sscanf( x.substr( iStart + 1, iEnd - iStart - 1 ).c_str( ), "%I64d", &i );
#else
	sscanf( x.substr( iStart + 1, iEnd - iStart - 1 ).c_str( ), "%lld", &i );
#endif

	return new CAtomLong( i );
}

CAtomstring DecodeString( const string &x, unsigned long iStart )
{
	string :: intype iSplit = x.find_first_not_of( "1234567890", iStart );

	if( iSplit == string :: npos )
	{
		UTIL_LogPrint( "error decoding string - couldn't find \":\", halting decode\n" );

		return NULL;
	}

	return new CAtomString( x.substr( iSplit + 1, atoi( x.substr( iStart, iSplit - iStart ).c_str( ) ) ) );
}

*/


int GetLength(unsigned short * buf, int * pos)
{
	char str_len[10+1];
	int leniter = 0;
	int len = 0;
	int cur_pos = *pos;
	
	while(buf[cur_pos] >0 && buf[cur_pos] < 255 && buf[cur_pos] != ':') 
	{
		if(buf[cur_pos+1] < 0) {
			unsigned short tmp = buf[cur_pos+1];
			return -1;
		}
		memcpy(&str_len[leniter],&buf[cur_pos],1);
		leniter++;
		cur_pos++;
	}
	str_len[leniter] = 0;
	// skip over the :
	cur_pos++;
	len = atoi(str_len);
	*pos = cur_pos;
	return len;
}

string DecodeString(unsigned short * buf, int * pos)
{
	//char str_len[10+1];
	//int leniter = 0;
	//int len = 0;
	int cur_pos = *pos;
	string ret_string;

	//while(buf[cur_pos] >0 && buf[cur_pos] < 255 && buf[cur_pos] != ':') 
	//{
	//	if(buf[cur_pos+1] < 0) {
	//		unsigned short tmp = buf[cur_pos+1];
	//		return "break";
	//	}
	//	memcpy(&str_len[leniter],&buf[cur_pos],1);
	//	leniter++;
	//	cur_pos++;
	//}
	//str_len[leniter] = 0;
	//// skip over the :
	//cur_pos++;
	//len = atoi(str_len);
	int len = GetLength(buf, &cur_pos);
	if(len == -1) return "break";

	char * ret_char = new char[len+1];
	for(int i = 0; i < len; i++ ) {
		ret_char[i] = (char)buf[i+cur_pos];
	}
	ret_char[len] = '\0';
	ret_string = ret_char;
	delete [] ret_char;
	cur_pos += len;
	*pos = cur_pos;
	return (ret_string);

	
}

/*
CAtomList *DecodeList( const string &x, unsigned long iStart )
{
	unsigned long i = iStart + 1;

	CAtomList *pList = new CAtomList( );

	while( i < x.size( ) && x[i] != 'e' )
	{
		CAtom *pAtom = Decode( x, i );

		if( pAtom )
		{
			i += pAtom->EncodedLength( );

			pList->addItem( pAtom );
		}
		else
		{
			UTIL_LogPrint( "error decoding list - error decoding list item, discarding list\n" );

			delete pList;

			pList = NULL;

			return NULL;
		}
	}

	return pList;
}

*/

AtomList DecodeList(unsigned short * buf, int * pos)
{
	AtomList return_list;
	string tempstr;

	int cur_pos = *pos;
	cur_pos++; // past the l

	while(buf[cur_pos] != 'e')
	{
		// case: list of lists...
		if(buf[cur_pos] == 'l') {
			AtomList l = DecodeList(buf, &cur_pos);
			
			vector <string>::iterator v1_Iter;
			for(v1_Iter = l.v_elements.begin(); v1_Iter != l.v_elements.end( ) ;v1_Iter++) {
				string temp = *v1_Iter;
				
				return_list.v_elements.push_back(*v1_Iter);	
			}
			
		}
		else if(buf[cur_pos] == 'd') {
			//AtomDicti ad = DecodeDicti(buf, &cur_pos);
			TRACE("ERROR, need to implement decode dicti\n");
		}
		else if (isdigit(buf[cur_pos]))
		{
			tempstr = DecodeString(buf,&cur_pos);
			
			return_list.v_elements.push_back(tempstr);
		}
		else {
			TRACE("This file sucks!\n");
		}
	}
	//skip the e at the end
	cur_pos++;
	*pos = cur_pos;
	return (return_list);
}


