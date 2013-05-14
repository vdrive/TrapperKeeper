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

//#include "StdAfx.h"
//#include "bnbt.h"
//#include "atom.h"
#include "bencode.h"
#include <iostream>
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

string DecodeString(unsigned short * buf, int * pos)
{
	char str_len[10+1];
	int leniter = 0;
	int len = 0;
	int cur_pos = *pos;
	string ret_string;
	//cout << "len: ";
	while(buf[cur_pos] >0 && buf[cur_pos] < 255 && buf[cur_pos] != ':') 
	{
		if(buf[cur_pos+1] < 0) {
			unsigned short tmp = buf[cur_pos+1];
			//printf("ERROR: %d\n%s\n", (cur_pos+1, buf));
			return "break";
		}
		memcpy(&str_len[leniter],&buf[cur_pos],1);
		//cout << (char)buf[cur_pos];
		leniter++;
		cur_pos++;
	}
	//cout << endl;
	str_len[leniter] = 0;
	// skip over the :
	cur_pos++;
	len = atoi(str_len);
	char * ret_char = new char[len+1];
	for(int i = 0; i < len; i++ ) {
		ret_char[i] = (char)buf[i+cur_pos];
	}
	//memcpy(ret_char,&buf[cur_pos],len);
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
	cur_pos++;

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
		else if (isdigit(buf[cur_pos]))
		{
			tempstr = DecodeString(buf,&cur_pos);
			
			return_list.v_elements.push_back(tempstr);
		}
	}
	//skip the e at the end
	cur_pos++;
	*pos = cur_pos;
	return (return_list);
}

/*
CAtomDicti *DecodeDicti( const string &x, unsigned long iStart )
{
	unsigned long i = iStart + 1;

	CAtomDicti *pDicti = new CAtomDicti( );

	while( i < x.size( ) && x[i] != 'e' )
	{
		CAtom *pKey = Decode( x, i );

		if( pKey && pKey->isString( ) )
		{
			i += pKey->EncodedLength( );

			if( i < x.size( ) )
			{
				CAtom *pValue = Decode( x, i );

				if( pValue )
				{
					i += pValue->EncodedLength( );

					pDicti->setItem( (CAtomstring )pKey, pValue );
				}
				else
				{
					UTIL_LogPrint( "error decoding dictionary - error decoding value, discarding dictionary\n" );

					delete pDicti;

					pDicti = NULL;

					return NULL;
				}
			}
		}
		else
		{
			UTIL_LogPrint( "error decoding dictionary - error decoding key, discarding dictionary\n" );

			delete pDicti;

			pDicti = NULL;

			return NULL;
		}
	}

	return pDicti;
}
*/
AtomDicti DecodeDicti(char * buf, int pos)
{
	AtomDicti return_dicti;


	return(return_dicti);
}

/*
CAtom *Decode( const string &x, unsigned long iStart )
{
	if( iStart < x.size( ) )
	{
		if( x[iStart] == 'i' )
			return DecodeLong( x, iStart );
		else if( isdigit( x[iStart] ) )
			return DecodeString( x, iStart );
		else if( x[iStart] == 'l' )
			return DecodeList( x, iStart );
		else if( x[iStart] == 'd' )
			return DecodeDicti( x, iStart );

		string temp = x.substr( iStart );

		UTIL_LogPrint( "error decoding - found unexpected character %d, halting decode\n", (unsigned int)x[iStart] );
	}
	else
	{
		UTIL_LogPrint( "error decoding - out of range\n" );

		return NULL;
	}

	return NULL;
}

CAtom *DecodeFile( const char *szFile )
{
	FILE *pFile = NULL;

	if( ( pFile = fopen( szFile, "rb" ) ) == NULL )
	{
		UTIL_LogPrint( "error decoding file - unable to open %s for reading\n", szFile );

		return NULL;
	}

	fseek( pFile, 0, SEEK_END );
	unsigned long ulFileSize = ftell( pFile );
	fseek( pFile, 0, SEEK_SET );

	char *pData = (char *)malloc( sizeof( char ) * ulFileSize );

	memset( pData, 0, sizeof( char ) * ulFileSize );

	fread( (void *)pData, sizeof( char ), ulFileSize, pFile );

	fclose( pFile );

	pFile = NULL;

	// this should run about a million times faster than the old char vector push_back crap

	string strFile( pData, ulFileSize );

	free( pData );

	return Decode( strFile );
}
*/

vector<unsigned short*> DecodePieces(unsigned short * buf, int * pos) {

	vector<unsigned short*> v_elements;

	int cur_pos = *pos;

	int len = 0;
	int leniter = 0;
	char str_len[32+1];

	while(buf[cur_pos] != ':') 
	{
		if(buf[cur_pos+1] < 0) {
			unsigned short tmp = buf[cur_pos+1];
			//printf("ERROR: %d\n%s\n", (cur_pos+1, buf));
		}
		memcpy(&str_len[leniter],&buf[cur_pos],1);
		leniter++;
		cur_pos++;
	}
	str_len[leniter] = 0;
	// skip over the :
	cur_pos++;
	len = atoi(str_len);
	int end_pos = cur_pos + len;
	//cout << "cur_pos: " << cur_pos << " len: " << len << " final_pos: " << end_pos << endl;

	while(cur_pos < end_pos)
	{
		//cout << "cur_pos: " << cur_pos << " ";
		unsigned short * temp = new unsigned short[21];
		memcpy(temp, &buf[cur_pos], 20);
		temp[20] = 0;
		cur_pos += 20;
		v_elements.push_back(temp);
	}
	//cout << endl << cur_pos << endl;
	*pos = cur_pos;
	return v_elements;
}
