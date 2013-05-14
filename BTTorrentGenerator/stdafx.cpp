// stdafx.cpp : source file that includes just the standard includes
// BTTorrentGenerator.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "StdAfx.h"

//
//
//
const char *uitoa(unsigned int nNum, char *pBuf)
{
	if( pBuf == NULL )
		return NULL;
	
	int i = 0;
	char digit = 0;

	digit = ( ( nNum / 1000000000 ) % 10 ) + 0x30;
	if( digit != '0' || ( digit == '0' && i != 0 ) )
	{
		pBuf[i++] = digit;
	}
    
	digit = ( ( nNum / 100000000 ) % 10 ) + 0x30;
	if( digit != '0' || ( digit == '0' && i != 0 ) )
	{
		pBuf[i++] = digit;
	}
    
	digit = ( ( nNum / 10000000 ) % 10 ) + 0x30;
	if( digit != '0' || ( digit == '0' && i != 0 ) )
	{
		pBuf[i++] = digit;
	}
    
	digit = ( ( nNum / 1000000 ) % 10 ) + 0x30;
	if( digit != '0' || ( digit == '0' && i != 0 ) )
	{
		pBuf[i++] = digit;
	}
    
	digit = ( ( nNum / 100000 ) % 10 ) + 0x30;
	if( digit != '0' || ( digit == '0' && i != 0 ) )
	{
		pBuf[i++] = digit;
	}
    
	digit = ( ( nNum / 10000 ) % 10 ) + 0x30;
	if( digit != '0' || ( digit == '0' && i != 0 ) )
	{
		pBuf[i++] = digit;
	}
    
	digit = ( ( nNum / 1000 ) % 10 ) + 0x30;
	if( digit != '0' || ( digit == '0' && i != 0 ) )
	{
		pBuf[i++] = digit;
	}
    
	digit = ( ( nNum / 100 ) % 10 ) + 0x30;
	if( digit != '0' || ( digit == '0' && i != 0 ) )
	{
		pBuf[i++] = digit;
	}
    
	digit = ( ( nNum / 10 ) % 10 ) + 0x30;
	if( digit != '0' || ( digit == '0' && i != 0 ) )
	{
		pBuf[i++] = digit;
	}
    
	digit = ( ( nNum / 1 ) % 10 ) + 0x30;
	if( digit != '0' || ( digit == '0' && i != 0 ) )
	{
		pBuf[i++] = digit;
	}

	pBuf[i++] = '\0';

	return pBuf;
}
