
//////////////////////////////////////////////////////////////////////
// -- MXSock.dll --
// WinMX WPNP Support Dynamic Link Library Header File
// Copyright 2004 [Nushi]@[2SN]
//////////////////////////////////////////////////////////////////////
#pragma once


//////////////////////////////////////////////////////////////////////
// includes
//////////////////////////////////////////////////////////////////////
#include <Windows.h>


//////////////////////////////////////////////////////////////////////
// Export or Import Switching
//////////////////////////////////////////////////////////////////////
#ifdef _MXSOCK_DEV_ 
	#define MXSOCKAPI extern "C" __declspec(dllexport)
#else
	#define MXSOCKAPI extern "C" __declspec(dllimport)
#endif


//////////////////////////////////////////////////////////////////////
// Export Functions
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Decrypt Message From FrontCode Peer Cache Server
//
//	Parameters
//		pSrc	: (IN)  Message From Peer Cache Server. Buffer must be 132 bytes.
//		pDst	: (OUT) Primary Node Addresses. Buffer must be 120 bytes.
//
//	Remarks
//		Node Address Structure
//
//		typedef struct TAG_NODEINFO {
//			DWORD dwNodeIP;		// Primary Node IP Addresses
//			WORD wUDPPort;		// Primary Node UDP Port
//			WORD wTCPPort;		// Primary Node TCP Port
//			BYTE bFreePri;		// Free Primary Connections
//			BYTE bFreeSec;		// Free Secondary Connections
//			WORD wDummy;		// Padding
//			} NODEINFO, *PNODEINFO;
//
//	Example
//		NODEINFO NodeInfo[10];
//		DecryptFrontCode(pBuff, (BYTE *)NodeInfo);
//		
//////////////////////////////////////////////////////////////////////

MXSOCKAPI void DecryptFrontCode(const BYTE *pSrc, BYTE *pDst);


//////////////////////////////////////////////////////////////////////
// Create Crypt Key Block for send
//
//	Parameters
//		wID		: (IN)  Crypt Key ID.
//		pBlock	: (OUT) Key Block . Buffer must be 16 bytes.
//
//	Remarks
//		WPNP Key IDs
//		0x0050	: Primary Client
//		0x0051	: Primary Server
//		0x0052	: Secondary Client
//		0x0053	: Secondary Server
//		0x0057	: Chat Client
//		0x0058	: Chat Server
//
//	Example
//		BYTE KeyBlock[16];
//		CreateCryptKeyID(0x0057, KeyBlock);
//		
//////////////////////////////////////////////////////////////////////

MXSOCKAPI void CreateCryptKeyID(const WORD wID, BYTE *pBlock);


//////////////////////////////////////////////////////////////////////
// Get Key ID from Key Block
//
//	Parameters
//		pBlock	: (IN)  Key Block. Buffer must be 16 bytes.
//
//	Return Value
//		WPNP Key ID
//
//////////////////////////////////////////////////////////////////////

MXSOCKAPI const WORD GetCryptKeyID(const BYTE *pBlock);


//////////////////////////////////////////////////////////////////////
// Get Crypt Key from Key Block
//
//	Parameters
//		pBlock	: (IN)  Key Block. Buffer must be 16 bytes.
//		pUpKey	: (OUT)	Pointer to Crypt Key for send
//		pDwKey	: (OUT)	Pointer to Crypt Key for recv
//
//	Return Value
//		WPNP Key ID
//
//////////////////////////////////////////////////////////////////////

MXSOCKAPI const WORD GetCryptKey(const BYTE *pBlock, DWORD *pUpKey, DWORD *pDwKey);


//////////////////////////////////////////////////////////////////////
// Encrypt or Decrypt WPNP TCP Message
//
//	Parameters
//		pBuf	: (IN)  Message.
//		iLen	: (IN)	Message Size.
//		dwKey	: (IN)	Crypt Key
//	Remarks
//		WPNP TCP Message Format
//
//		typedef struct TAG_MXTCPMESSAGE {
//			WORD wType;		// Message Type
//			WORD wLen;		// Data Length
//			BYTE Data[0];	// Data
//			} MXTCPMESSAGE, *PMXTCPMESSAGE;
//		
//	Return Value
//		New Crypt Key
//
//////////////////////////////////////////////////////////////////////

MXSOCKAPI const DWORD DecryptMXTCP(BYTE *pBuf, const int iLen, const DWORD dwKey);
MXSOCKAPI const DWORD EncryptMXTCP(BYTE *pBuf, const int iLen, const DWORD dwKey);


//////////////////////////////////////////////////////////////////////
// Decrypt WPNP UDP Message
//
//	Parameters
//		pBuf	: (IN)  Message.
//		iLen	: (IN)	Message Size.
//
//////////////////////////////////////////////////////////////////////

MXSOCKAPI void DecryptMXUDP(BYTE *pBuf, const int iLen);


//////////////////////////////////////////////////////////////////////
// Get File Hash from file
//
//	Parameters
//		lpszFileName	: (IN)  File Path.
//		pHash			: (OUT)	File Hash.
//		pFileLen		: (OUT) File Length.
//
//	Example
//		DWORD dwHash;
//		DWORD dwLen;
//		GetFileHash("c:\\Example.avi", &dwHash, &dwLen);
//
//	Return Value
//		If no error occurs, GetFileHash returns TRUE
//
//////////////////////////////////////////////////////////////////////

MXSOCKAPI const BOOL GetFileHash(LPCTSTR lpszFileName, DWORD *pHash, DWORD *pFileLen);

