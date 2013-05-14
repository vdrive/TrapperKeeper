// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#endif

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0400		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0400	// Change this to the appropriate value to target Windows 2000 or later.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0400	// Change this to the appropriate value to target IE 5.0 or later.
#endif

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC Automation classes
#endif // _AFX_NO_OLE_SUPPORT

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>			// MFC ODBC database classes
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#include <afxdao.h>			// MFC DAO database classes
#endif // _AFX_NO_DAO_SUPPORT

#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <vector>
#include <queue>
#include <set>
#include <map>
using namespace std;

//
// Window Messages
//
#define WM_CMOD_INIT_THREAD_DATA		WM_USER+10
#define WM_CMOD_SOCKET_CONNECT			WM_USER+11
#define WM_CMOD_SOCKET_DISCONNECT		WM_USER+12
#define WM_CMOD_CLIENT_CONNECT_ERROR	WM_USER+13
#define WM_CMOD_CONNECTIONS_REPAIRED	WM_USER+14
#define WM_CMOD_TORRENT_SEARCH			WM_USER+15
#define WM_CMOD_LOG_MSG					WM_USER+16
#define WM_CMOD_ONE_SEC_TIMER			WM_USER+17
#define WM_CMOD_AUTO_REPAIR_TIMER		WM_USER+18

//
// Converts a set of two hex characters into the integer version
//
unsigned char htoi(char left, char right);

//
// Returns a new char* containing a duplicate of pData (NOTE: pData must be NULL terminated)
//
char *dupstring(const char *pData);

//
// Returns a new char* containing nLen bytes copied from pData with a NULL termination
//
char *dupstring(const char *pData, size_t nLen);

//
// Returns random byte from 'low' to 'high', inclusive
//
inline BYTE randByte(BYTE low, BYTE high)
{
	return low + (BYTE)( ( ( high - low ) * ( rand() / (float)RAND_MAX ) ) + 0.5 );
}

//
// Returns random int from 'low' to 'high', inclusive
//
inline int randInt(int low, int high)
{
	return low + (int)( ( ( high - low ) * ( rand() / (float)RAND_MAX ) ) + 0.5 );
}

//
// Returns random int from 'low' to 'high', inclusive
//
inline size_t randSize_t(size_t low, size_t high)
{
	return low + (size_t)( ( ( high - low ) * ( rand() / (float)RAND_MAX ) ) + 0.5 );
}

//
//
//
inline void seedRand(void)
{
	srand( (unsigned int)time(NULL) );
}

//
// Swaps byte 0 and byte 1 in the given ushort
//
inline USHORT swapbytes(USHORT data)
{
	return (USHORT)( ( (data>>8) & 0xff ) | ( data<<8 ) );
}

//
// Swaps byte 0 with byte 3, and byte 1 with byte 2 in the given dword
//
inline DWORD swapbytes(DWORD data)
{
	return ((data>>24)&0xff) | (data<<24) | ((data>>8)&0x0000ff00) | ((data<<8)&0x00ff0000);
}

//
// Swaps byte 0 with byte 3, and byte 1 with byte 2 in the given dword
//
inline int swapbytes(int data)
{
	return ((data>>24)&0xff) | (data<<24) | ((data>>8)&0x0000ff00) | ((data<<8)&0x00ff0000);
}

//
//
//
template <class T>
static bool UniqueVectorPushBack(const T &rData, vector<T> &rVec)
{
	// set the flag that the object is unique in the vector
	bool bUnique = true;

	// FOR all of the words in the vector, see if the word to add is unique
	for( size_t i = 0, nLen = rVec.size(); i < nLen && bUnique; i++ )
		bUnique = ( rData != rVec[i] );

	// IF the object is unique to all other objects in the vector, add it to the vector
	if( bUnique )
		rVec.push_back( rData );

	return bUnique;
}

//
// Goes through a vector of unique entries, checks each entry for a match to the given item,
//  if the current item matches the given item it is removes.  When an item is found, the search
//  is over--no other entries will be removed.
//
template <class T>
static bool UniqueVectorRemove(const T &rData, vector<T> &rVec)
{
	// FOR each of the items in the vector
	for( size_t i = 0, nLen = rVec.size(); i < nLen; i++ )
	{
		// IF the given data item matches the item in the given vector
		if( rVec[i] == rData )
		{
			// remove the ip/portObj and stop checking ip/portObjs
			rVec.erase( rVec.begin() + i );
			return true;
		}
	}

	return false;
}

//
//
//
const char *uitoa(unsigned int nNum, char *pBuf);
