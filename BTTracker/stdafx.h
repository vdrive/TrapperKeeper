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
// window messages
//
#define WM_LISTENER_INIT_THREAD_DATA		WM_USER+1
#define WM_LISTENER_NEW_SOCKET				WM_USER+2
#define WM_LISTENER_STATUS					WM_USER+3
#define WM_LISTENER_LOG_MSG					WM_USER+4

#define WM_CMOD_INIT_THREAD_DATA			WM_USER+5
#define WM_CMOD_SOCKET_CONNECT				WM_USER+6
#define WM_CMOD_SOCKET_DISCONNECT			WM_USER+7
#define WM_CMOD_CONNECTIONS_REPAIRED		WM_USER+8
#define WM_CMOD_CLIENT_CONNECT_ERROR		WM_USER+9
#define WM_CMOD_TORRENT_INDEX_REQUEST		WM_USER+10
#define WM_CMOD_LOG_MSG						WM_USER+11

#define WM_TMANAGER_INIT_THREAD_DATA		WM_USER+12
#define WM_TMANAGER_INDEX_RESULT			WM_USER+13
#define WM_TMANAGER_LOG_MSG					WM_USER+14
#define WM_TMANAGER_CHECK_CLIENTS			WM_USER+15
#define WM_TMANAGER_PEER_RESULT				WM_USER+16
#define WM_TMANAGER_LIST_UPDATE				WM_USER+17

//
// extra global information
//
#define TCPSERVERPORT						6969

//
// Converts a set of two hex characters into the integer version
//
BYTE htoi(char left, char right);

//
// Returns a new char* containing a duplicate of pData (NOTE: pData must be NULL terminated)
//
char *dupstring(const CHAR *pData);

//
// Returns a new char* containing nLen bytes copied from pData with a NULL termination
//
char *dupstring(const CHAR *pData, size_t nLen);

//
// Returns true if both strings are equal to each other for the first nLen bytes.  If either pointer
//  is NULL or the two are not equal, return false.
//
inline bool nullSafeStrnicmp(const char *pLeft, const char *pRight, size_t nLen)
{
	return ( pLeft != NULL && pRight != NULL ? strnicmp( pLeft, pRight, nLen ) == 0 : false );
}

//
// Returns true if both strings are equal to each other.  If either pointer is NULL or the two are
//  not equal, return false.
//
inline bool nullSafeStricmp(const char *pLeft, const char *pRight)
{
	return ( pLeft != NULL && pRight != NULL ? stricmp( pLeft, pRight ) == 0 : false );
}

//
// Returns random int
//
inline int randInt(void)
{
	return (int)( ( 0xffffffff * ( rand() / (float)RAND_MAX ) ) + 0.5 );
}

//
// Returns random int from 'low' to 'high', inclusive
//
inline int randInt(int low, int high)
{
	return low + (int)( ( ( high - low ) * ( rand() / (float)RAND_MAX ) ) + 0.5 );
}

//
// Seed the random number generator with the time
//
inline void seedRand(void)
{
	srand( (UINT)time(NULL) );
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
//
//
void inet_itoa(char aBuf[16], DWORD nAddr);
