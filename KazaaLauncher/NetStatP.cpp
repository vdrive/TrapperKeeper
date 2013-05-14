#include "StdAfx.h"
#include "netstatp.h"

// Possible TCP endpoint states
//
static char TcpState[][32] = {
	"???",
	"CLOSED",
	"LISTENING",
	"SYN_SENT",
	"SYN_RCVD",
	"ESTABLISHED",
	"FIN_WAIT1",
	"FIN_WAIT2",
	"CLOSE_WAIT",
	"CLOSING",
	"LAST_ACK",
	"TIME_WAIT",
	"DELETE_TCB"
};

// APIs that we link against dynamically in case they aren't 
// present on the system we're running on.
//
DWORD (WINAPI *pAllocateAndGetTcpExTableFromStack)(
  PMIB_TCPEXTABLE *pTcpTable,  // buffer for the connection table
  BOOL bOrder,               // sort the table?
  HANDLE heap,
  DWORD zero,
  DWORD flags
);

DWORD (WINAPI *pAllocateAndGetUdpExTableFromStack)(
  PMIB_UDPEXTABLE *pTcpTable,  // buffer for the connection table
  BOOL bOrder,               // sort the table?
  HANDLE heap,
  DWORD zero,
  DWORD flags
);

HANDLE (WINAPI *pCreateToolhelp32Snapshot)(
  DWORD dwFlags,       
  DWORD th32ProcessID  
);

BOOL (WINAPI *pProcess32First)(
  HANDLE hSnapshot,      
  LPPROCESSENTRY32 lppe  
);

BOOL (WINAPI *pProcess32Next)(
  HANDLE hSnapshot,      
  LPPROCESSENTRY32 lppe  
);

//
//
//
NetStatP::NetStatP(void)
{
}

//
//
//
NetStatP::~NetStatP(void)
{
}

//--------------------------------------------------------------------
//
// PrintError
// 
// Translates a Win32 error into a text equivalent
//
//--------------------------------------------------------------------
void NetStatP::PrintError(DWORD ErrorCode)
{
	LPVOID lpMsgBuf;
 
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
					NULL, ErrorCode, 
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
					(LPTSTR) &lpMsgBuf, 0, NULL );
	TRACE("%s\n", lpMsgBuf );
	LocalFree( lpMsgBuf );
}

//------------------------------------------------------------
//
// GetPortName
//
// Translate port numbers into their text equivalent if 
// there is one
//
//------------------------------------------------------------
PCHAR NetStatP::GetPortName(DWORD Flags,UINT port,PCHAR proto,PCHAR name,int namelen) 
{
	struct servent *psrvent;

	if( Flags & FLAG_SHOW_NUMBERS ) {

		sprintf( name, "%d", htons( (WORD) port));
		return name;
	} 

	//
	// Try to translate to a name
	//
	if( psrvent = getservbyport( port, proto )) {

		strcpy( name, psrvent->s_name );

	} else {
	
		sprintf( name, "%d", htons( (WORD) port));
	}		
	return name;
}


//------------------------------------------------------------
//
// GetIpHostName
//
// Translate IP addresses into their name-resolved form
// if possible.
//
//------------------------------------------------------------
PCHAR NetStatP::GetIpHostName(DWORD Flags,BOOL local,UINT ipaddr,PCHAR name,int namelen) 
{
	struct hostent			*phostent;
	UINT					nipaddr;

	//
	// Does the user want raw numbers?
	//
	nipaddr = htonl( ipaddr );
	if( Flags & FLAG_SHOW_NUMBERS ) {

		sprintf( name, "%d.%d.%d.%d", 
			(nipaddr >> 24) & 0xFF,
			(nipaddr >> 16) & 0xFF,
			(nipaddr >> 8) & 0xFF,
			(nipaddr) & 0xFF);
		return name;
	}

	//
	// Try to translate to a name
	//
	if( !ipaddr  ) {

		if( !local ) {

			sprintf( name, "%d.%d.%d.%d", 
				(nipaddr >> 24) & 0xFF,
				(nipaddr >> 16) & 0xFF,
				(nipaddr >> 8) & 0xFF,
				(nipaddr) & 0xFF);

		} else {

			gethostname(name, namelen);
		}

	} else if( ipaddr == 0x0100007f ) {

		if( local ) {

			gethostname(name, namelen);
		} else {

			strcpy( name, "localhost" );
		}

	} else if( phostent = gethostbyaddr( (char *) &ipaddr,
		sizeof( nipaddr ), PF_INET )) {

		strcpy( name, phostent->h_name );

	} else {

		sprintf( name, "%d.%d.%d.%d", 
			(nipaddr >> 24) & 0xFF,
			(nipaddr >> 16) & 0xFF,
			(nipaddr >> 8) & 0xFF,
			(nipaddr) & 0xFF);
	}
	return name;
}


//------------------------------------------------------------
//
// ProcessPidToName
//
// Translates a PID to a name.
//
//------------------------------------------------------------
PCHAR NetStatP::ProcessPidToName(HANDLE hProcessSnap,DWORD ProcessId,PCHAR ProcessName)
{
	PROCESSENTRY32 processEntry;

	strcpy( ProcessName, "???" );
	if( !pProcess32First( hProcessSnap, &processEntry )) {

		return ProcessName;
	}
	do {

		if( processEntry.th32ProcessID == ProcessId ) {

			strcpy( ProcessName, processEntry.szExeFile );
			return ProcessName;
		}

	} while( pProcess32Next( hProcessSnap, &processEntry ));

	return ProcessName;
}


//------------------------------------------------------------
//
// ExApisArePresent
//
// Determines if Ex APIs (the XP version) are present, and
// if so, gets the function entry points.
//
//------------------------------------------------------------
BOOL NetStatP::ExApisArePresent()
{
	pAllocateAndGetTcpExTableFromStack = reinterpret_cast<DWORD(__stdcall *)(PMIB_TCPEXTABLE*,BOOL,HANDLE,DWORD,DWORD)>
		(GetProcAddress( LoadLibrary( "iphlpapi.dll"),"AllocateAndGetTcpExTableFromStack" ));
	if( !pAllocateAndGetTcpExTableFromStack )
	{
		MessageBox(NULL,"Couldn't load the function AllocateAndGetTcpExTableFromStack from iphlpapi.dll","Error",MB_OK|MB_ICONERROR);
		return FALSE;
	}

	pAllocateAndGetUdpExTableFromStack = reinterpret_cast<DWORD(__stdcall *)(PMIB_UDPEXTABLE*,BOOL,HANDLE,DWORD,DWORD)>
		(GetProcAddress( LoadLibrary( "iphlpapi.dll"),"AllocateAndGetUdpExTableFromStack" ));
	if( !pAllocateAndGetUdpExTableFromStack )
	{
		MessageBox(NULL,"Couldn't load the function AllocateAndGetUdpExTableFromStack from iphlpapi.dll","Error",MB_OK|MB_ICONERROR);
		return FALSE;
	}

	pCreateToolhelp32Snapshot = reinterpret_cast<HANDLE(__stdcall *)(DWORD,DWORD)>
		(GetProcAddress( GetModuleHandle( "kernel32.dll" ),"CreateToolhelp32Snapshot" ));
	if( !pCreateToolhelp32Snapshot ) 
	{
		MessageBox(NULL,"Couldn't load the function CreateToolhelp32Snapshot from kernel32.dll","Error",MB_OK|MB_ICONERROR);
		return FALSE;
	}

	pProcess32First = reinterpret_cast<BOOL(__stdcall *)(HANDLE,LPPROCESSENTRY32)>
		(GetProcAddress( GetModuleHandle( "kernel32.dll" ),"Process32First" ));
	if( !pProcess32First ) 
	{
		MessageBox(NULL,"Couldn't load the function Process32First from kernel32.dll","Error",MB_OK|MB_ICONERROR);
		return FALSE;
	}

	pProcess32Next = reinterpret_cast<BOOL(__stdcall *)(HANDLE,LPPROCESSENTRY32)>
		(GetProcAddress( GetModuleHandle( "kernel32.dll" ),"Process32Next" ));
	if( !pProcess32Next ) 
	{
		MessageBox(NULL,"Couldn't load the function Process32Next from kernel32.dll","Error",MB_OK|MB_ICONERROR);
		return FALSE;
	}
	return TRUE;
}

//
//
//
int NetStatP::StartEnumerating(vector<NetInfo>& net_infos)
{
	DWORD		error/*,dwSize*/;
//	WORD		wVersionRequested;
//	WSADATA		wsaData;
	HANDLE		hProcessSnap;
	PMIB_TCPEXTABLE tcpExTable;
//	PMIB_TCPTABLE tcpTable;
//	PMIB_UDPEXTABLE udpExTable;
//	PMIB_UDPTABLE udpTable;
	BOOLEAN		exPresent;
	DWORD		i, flags;
	CHAR		processName[MAX_PATH];
//	CHAR		localname[HOSTNAMELEN], remotename[HOSTNAMELEN];
	CHAR		/*remoteport[PORTNAMELEN],*/ localport[PORTNAMELEN];
//	CHAR		localaddr[ADDRESSLEN], remoteaddr[ADDRESSLEN];

	// 
	// Check for NT
	//
	if( GetVersion() >= 0x80000000 )
	{
		TRACE("It requres Windows NT/2K/XP.\n");
		return -1;
	}

	//
	// Initialize winsock
	//
	/*
	wVersionRequested = MAKEWORD( 1, 1 );
	if( WSAStartup(  wVersionRequested, &wsaData ))
	{
		TRACE("Could not initialize Winsock.\n");
		return -1;
	}
	*/
	exPresent = ExApisArePresent();
	flags = FLAG_ALL_ENDPOINTS|FLAG_SHOW_NUMBERS;

	//
	// Determine if extended query is available (it's only present
	// on XP and higher).
	//
	if( exPresent )
	{

		//
		// Get the tables of TCP and UDP endpoints with process IDs
		//
		error = pAllocateAndGetTcpExTableFromStack( &tcpExTable, TRUE, GetProcessHeap(), 2, 2 );
		if( error )
		{
			TRACE("Failed to snapshot TCP endpoints.\n");
			PrintError( error );
			return -1;
		}
/*
		error = pAllocateAndGetUdpExTableFromStack( &udpExTable, TRUE, GetProcessHeap(), 2, 2 );
		if( error ) {

			TRACE("Failed to snapshot UDP endpoints.\n");
			PrintError( error );
			return -1;
		}
*/
		//
		// Get a process snapshot. Note that we won't be guaranteed to 
		// exactly match a PID against a process name because a process could have exited 
		// and the PID gotten reused between our endpoint and process snapshots.
		//
		hProcessSnap = pCreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
		if( hProcessSnap == INVALID_HANDLE_VALUE )
		{
			TRACE("Failed to take process snapshot. Process names will not be shown.\n\n");
		}

		//Create NetInfos vector
		for( i = 0; i < tcpExTable->dwNumEntries; i++ )
		{
			ProcessPidToName( hProcessSnap, tcpExTable->table[i].dwProcessId, processName );
#ifdef KAZAA_KPP
			if(stricmp(processName, "Kazaa.kpp")==0) //we are looking for kazaa listening TCP port only, K++ version
#else
			if(stricmp(processName, "kazaa.exe")==0) //we are looking for kazaa listening TCP port only
#endif
			{
				string state = TcpState[tcpExTable->table[i].dwState];
				if(strcmp(state.c_str(),"LISTENING")==0) //listening port only
				{
					NetInfo netinfo;
					netinfo.m_process_id = tcpExTable->table[i].dwProcessId;
					netinfo.m_port = atoi(GetPortName(flags, tcpExTable->table[i].dwLocalPort, "tcp", localport, PORTNAMELEN ));
					net_infos.push_back(netinfo);
				}

			}
		}
		CloseHandle(hProcessSnap);
		//
		// Dump the TCP table
		//
		/*
		for( i = 0; i < tcpExTable->dwNumEntries; i++ )
		{
			if( flags & FLAG_ALL_ENDPOINTS || tcpExTable->table[i].dwState == MIB_TCP_STATE_ESTAB )
			{
				sprintf( localaddr, "%s:%s", 
					GetIpHostName( flags, TRUE, tcpExTable->table[i].dwLocalAddr, localname, HOSTNAMELEN), 
					GetPortName( flags, tcpExTable->table[i].dwLocalPort, "tcp", localport, PORTNAMELEN ));

				sprintf( remoteaddr, "%s:%s",
					GetIpHostName( flags, FALSE, tcpExTable->table[i].dwRemoteAddr, remotename, HOSTNAMELEN), 
					tcpExTable->table[i].dwRemoteAddr ? 
						GetPortName( flags, tcpExTable->table[i].dwRemotePort, "tcp", remoteport, PORTNAMELEN ):
						"0" );

				TRACE("%-5s %s:%d\n      State:   %s\n", "[TCP]", 
					ProcessPidToName( hProcessSnap, tcpExTable->table[i].dwProcessId, processName ),
					tcpExTable->table[i].dwProcessId,
					TcpState[ tcpExTable->table[i].dwState ] );
				TRACE("      Local:   %s\n      Remote:  %s\n",
					localaddr, remoteaddr );
			}
		}
		*/
		/*
		//
		// Dump the UDP table
		//
		if( flags & FLAG_ALL_ENDPOINTS ) {

			for( i = 0; i < udpExTable->dwNumEntries; i++ ) {

				sprintf( localaddr, "%s:%s", 
					GetIpHostName( flags, TRUE, udpExTable->table[i].dwLocalAddr, localname, HOSTNAMELEN), 
					GetPortName( flags, udpExTable->table[i].dwLocalPort, "tcp", localport, PORTNAMELEN ));

				TRACE("%-5s %s:%d\n", "[UDP]", 
					ProcessPidToName( hProcessSnap, udpExTable->table[i].dwProcessId, processName ),
					udpExTable->table[i].dwProcessId );
				TRACE("      Local:   %s\n      Remote:  %s\n",
					localaddr, "*.*.*.*:*" );
			}
		}
		*/
	}
	else
	{
		TRACE("Extended query is not available");
		return -1;
	}
	TRACE("\n");
	return 0;
}