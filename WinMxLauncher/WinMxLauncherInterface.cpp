#include "StdAfx.h"
#include "winmxlauncherinterface.h"
#include "DllInterface.h"
#include "WinMxLauncherHeader.h"
#include "../DllLoader/AppID.h"

WinMxLauncherInterface::WinMxLauncherInterface(void)
{
}

//
//
//
WinMxLauncherInterface::~WinMxLauncherInterface(void)
{
}


//
//
//
bool WinMxLauncherInterface::InterfaceReceivedData(AppID from_app_id,void* input_data,void* output_data)
{
	return false; //pass this data to the main app
}

//
//
//
int WinMxLauncherInterface::RequestWinMxNumber()
{
	int number = 0;
	byte buf[sizeof(WinMxLauncherHeader)];
	WinMxLauncherHeader* header = (WinMxLauncherHeader*)buf;
	header->op = WinMxLauncherHeader::Request_WinMx_Number;
	header->size = 0;

	AppID app_id;
	app_id.m_app_id = 13; //winmx launcher app id
	app_id.m_version = 0x00000001;

	DllInterface::SendData(app_id,buf,&number);
	
    return number;
}

//
//
//
bool WinMxLauncherInterface::RestartAllWinMx()
{
	byte buf[sizeof(WinMxLauncherHeader)];
	WinMxLauncherHeader* header = (WinMxLauncherHeader*)&buf;
	header->op = WinMxLauncherHeader::Restart_All_WinMx;
	header->size = 0;

	AppID app_id;
	app_id.m_app_id = 13; //winmx launcher app id
	app_id.m_version = 0x00000001;

	return DllInterface::SendData(app_id,buf);	
}

//
//
//
bool WinMxLauncherInterface::SetMaxWinMxNumber(UINT max)
{
	byte buf[sizeof(WinMxLauncherHeader)+sizeof(UINT)];
	WinMxLauncherHeader* header = (WinMxLauncherHeader*)&buf;
	header->op = WinMxLauncherHeader::Set_Number_Of_WinMx_To_Run;
	header->size = sizeof(UINT);

	UINT *pMax = (UINT*)&buf[sizeof(WinMxLauncherHeader)];
	*pMax = max;

	AppID app_id;
	app_id.m_app_id = 13; //winmx launcher app id
	app_id.m_version = 0x00000001;


	return DllInterface::SendData(app_id,buf);	
}

//
//
//
bool WinMxLauncherInterface::StopRunningWinMx()
{
	byte buf[sizeof(WinMxLauncherHeader)];
	WinMxLauncherHeader* header = (WinMxLauncherHeader*)&buf;
	header->op = WinMxLauncherHeader::Stop_Running_WinMx;
	header->size = 0;

	AppID app_id;
	app_id.m_app_id = 13; //winmx launcher app id
	app_id.m_version = 0x00000001;

	return DllInterface::SendData(app_id,buf);	
}