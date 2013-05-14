#pragma once
#include "NetInfo.h"
#include "windows.h"
#include "winsock.h"
#include "iprtrmib.h"
#include "tlhelp32.h"
#include "iphlpapi.h"

// Maximum string lengths for ASCII ip address and port names
//
#define HOSTNAMELEN		256
#define PORTNAMELEN		256
#define ADDRESSLEN		HOSTNAMELEN+PORTNAMELEN

//
// Our option flags
//
#define FLAG_ALL_ENDPOINTS	1
#define FLAG_SHOW_NUMBERS	2


//
// Undocumented extended information structures available 
// only on XP and higher
// 


typedef struct {
  DWORD   dwState;        // state of the connection
  DWORD   dwLocalAddr;    // address on local computer
  DWORD   dwLocalPort;    // port number on local computer
  DWORD   dwRemoteAddr;   // address on remote computer
  DWORD   dwRemotePort;   // port number on remote computer
  DWORD	  dwProcessId;
} MIB_TCPEXROW, *PMIB_TCPEXROW;


typedef struct {
	DWORD			dwNumEntries;
	MIB_TCPEXROW	table[ANY_SIZE];
} MIB_TCPEXTABLE, *PMIB_TCPEXTABLE;



typedef struct {
  DWORD   dwLocalAddr;    // address on local computer
  DWORD   dwLocalPort;    // port number on local computer
  DWORD	  dwProcessId;
} MIB_UDPEXROW, *PMIB_UDPEXROW;


typedef struct {
	DWORD			dwNumEntries;
	MIB_UDPEXROW	table[ANY_SIZE];
} MIB_UDPEXTABLE, *PMIB_UDPEXTABLE;

class NetStatP
{
public:
	NetStatP(void);
	~NetStatP(void);
	int StartEnumerating(vector<NetInfo>& net_infos);
private:
	void PrintError(DWORD ErrorCode);
	PCHAR GetPortName(DWORD Flags,UINT port,PCHAR proto,PCHAR name,int namelen);
	PCHAR GetIpHostName(DWORD Flags,BOOL local,UINT ipaddr,PCHAR name,int namelen);
	PCHAR ProcessPidToName(HANDLE hProcessSnap,DWORD ProcessId,PCHAR ProcessName);
	BOOL ExApisArePresent();
};
