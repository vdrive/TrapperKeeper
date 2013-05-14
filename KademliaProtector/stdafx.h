// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#include "types.h"

#define ARRSIZE(x)	(sizeof(x)/sizeof(x[0]))


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
#include <afxsock.h>		// MFC socket extensions
#endif // _AFX_NO_AFXCMN_SUPPORT

#define NUMBERPORTS 20
#define BASEPORT 4690

#define KADEMLIA

#define NUMCONBINS 2000

//hate this, but these are required for the emule stuff to build
#pragma warning(disable:4200) // nonstandard extension used : zero-sized array in struct/union
#pragma warning(disable:4244) // 'conversion' conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable:4800) // 'type' : forcing value to bool 'true' or 'false' (performance warning)

// whenn using warning level 4
#pragma warning(disable:4100) // unreferenced formal parameter
#pragma warning(disable:4238) // nonstandard extension used : class rvalue used as lvalue
#pragma warning(disable:4389) // signed/unsigned mismatch

#include <vector>
using namespace std;

extern "C" int __cdecl __ascii_stricmp(const char * dst, const char * src);