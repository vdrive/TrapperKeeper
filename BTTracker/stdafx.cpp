// stdafx.cpp : source file that includes just the standard includes
// BTTracker.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "StdAfx.h"

//
// Converts a set of two hex characters into the integer version
//
BYTE htoi(char left, char right)
{
	// IF left == 0 to 9
	if( left > 0x29 && left < 0x40 )
	{
		// IF right == 0 to 9
		if( right > 0x29 && right < 0x40 )
		{
			return (16 * (left & 0x0f)) + (right & 0x0f);
		}
		else if( ( right >= 'a' && right <= 'f' ) || ( right >= 'A' && right <= 'F' ) )
		{
			return (16 * (left & 0x0f)) + (right & 0x0f) + 9;
		}
	}
	else if( ( left >= 'a' && left <= 'f' ) || ( left >= 'A' && left <= 'F' ) )
	{
		if( right > 0x29 && right < 0x40 )
		{
			return (16 * ( (left & 0x0f) + 9 ) ) + (right & 0x0f);
		}
		else if( ( right >= 'a' && right <= 'f' ) || ( right >= 'A' && right <= 'F' ) )
		{
			return (16 * ( (left & 0x0f) + 9 ) ) + (right & 0x0f) + 9;
		}
	}

	return 0;
}

//
// Returns a new char* containing a duplicate of pData (NOTE: pData must be NULL terminated)
//
char *dupstring(const CHAR *pData)
{
	if( pData == NULL )
		return NULL;

	size_t dataLen = strlen( pData );
	char *pRet = new char[ dataLen + 1 ];
	strcpy( pRet, pData );

	return pRet;
}

//
// Returns a new char* containing nLen bytes copied from pData with a NULL termination
//
char *dupstring(const CHAR *pData, size_t nLen)
{
	if( pData == NULL )
		return NULL;

	char *pRet = new char[ nLen + 1 ];
	strncpy( pRet, pData, nLen );
	pRet[nLen] = '\0';

	return pRet;
}

//
//
//
void inet_itoa(char aBuf[16], DWORD nAddr)
{
	if( aBuf == NULL )
		return;
	else	
		sprintf( aBuf, "%d.%d.%d.%d", nAddr&0xff, (nAddr>>8)&0xff, (nAddr>>16)&0xff, (nAddr>>24)&0xff );
}

//
//
//
HTREEITEM FindSiblingItem(const CTreeCtrl &rTree, const HTREEITEM hStartingItem, const string &rStr)
{
	HTREEITEM hItem = hStartingItem;
	while( hItem != NULL )
	{
		if( rTree.GetItemText( hItem ).Compare( rStr.c_str() ) == 0 )
			break;
		else
            hItem = rTree.GetNextSiblingItem( hItem );
	}

	return hItem;
}
