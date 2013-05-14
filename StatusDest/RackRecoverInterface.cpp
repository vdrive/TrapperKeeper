#include "stdafx.h"
#include "rackrecoverinterface.h"
#include <afxsock.h>		// MFC socket extensions

RackRecoverInterface::RackRecoverInterface(void)
{
}

RackRecoverInterface::~RackRecoverInterface(void)
{
}

void RackRecoverInterface::RestartRack(const char* ip)
{
	//send 3 pings to insure that our message gets there
	for(int i=0;i<3;i++){
		CAsyncSocket command_socket;
		command_socket.Create(0,SOCK_DGRAM,0,NULL);
		byte buf[4];
		*(UINT*)buf=(UINT)RACKRESTARTCOMMAND;
		command_socket.SendTo(buf,4,RACKRECOVERYPORT,ip,0);
	}
}

void RackRecoverInterface::DeleteFileOnRack(const char* ip, const char* full_file_name)
{
	//send 3 pings to insure that our message gets there
	for(int i=0;i<3;i++){
		CAsyncSocket command_socket;
		command_socket.Create(0,SOCK_DGRAM,0,NULL);
		int len=(int)strlen(full_file_name);
		byte *buf=new byte[4+len+1];
		*(UINT*)buf=(UINT)RACKDELETEFILE;
		strcpy((char*)(buf+4),full_file_name);
		command_socket.SendTo(buf,4+len+1,RACKRECOVERYPORT,ip,0);
	}
}
