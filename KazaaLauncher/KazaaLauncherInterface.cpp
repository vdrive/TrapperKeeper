#include "StdAfx.h"
#include "kazaalauncherinterface.h"
#include "DllInterface.h"
#include "../SupernodeCollector/KazaaControllerHeader.h"
#include "../DllLoader/AppID.h"

KazaaLauncherInterface::KazaaLauncherInterface(void)
{
}

//
//
//
KazaaLauncherInterface::~KazaaLauncherInterface(void)
{
}

//
//
//
bool KazaaLauncherInterface::InterfaceReceivedData(AppID from_app_id,void* input_data,void* output_data)
{
	return false; //pass this data to the main app
}

//
//
//
int KazaaLauncherInterface::RequestKazaaNumber()
{
	int kazaa_number = 0;
	byte buf[sizeof(KazaaControllerHeader)];
	KazaaControllerHeader* header = (KazaaControllerHeader*)buf;
	header->op = KazaaControllerHeader::Request_Kazaa_Number;
	header->size = 0;

	AppID app_id;
	app_id.m_app_id = 12; //kazaa launcher app id
	app_id.m_version = 0x00090002;

	DllInterface::SendData(app_id,buf,&kazaa_number);
	
    return kazaa_number;
}

//
//
//
bool KazaaLauncherInterface::KillAllKazaa()
{
	byte buf[sizeof(KazaaControllerHeader)];
	KazaaControllerHeader* header = (KazaaControllerHeader*)&buf;
	header->op = KazaaControllerHeader::Source_Kill_All_Kazaa;
	header->size = 0;

	AppID app_id;
	app_id.m_app_id = 12; //kazaa launcher app id
	app_id.m_version = 0x00090002;

	return DllInterface::SendData(app_id,buf);	
}

//
//
//
bool KazaaLauncherInterface::ResumeLaunchingKazaa()
{
	byte buf[sizeof(KazaaControllerHeader)];
	KazaaControllerHeader* header = (KazaaControllerHeader*)&buf;
	header->op = KazaaControllerHeader::Source_Launch_All_Kazaa;
	header->size = 0;

	AppID app_id;
	app_id.m_app_id = 12; //kazaa launcher app id
	app_id.m_version = 0x00090002;

	return DllInterface::SendData(app_id,buf);	
}

//
//
//
bool KazaaLauncherInterface::SetMaxKazaaNumber(UINT max_kazaa)		//set the maximum number of kazaa it can run
{
	byte buf[sizeof(KazaaControllerHeader)+sizeof(UINT)];
	KazaaControllerHeader* header = (KazaaControllerHeader*)&buf;
	header->op = KazaaControllerHeader::Source_Number_Of_Kazaa_To_Run;
	header->size = sizeof(UINT);

	UINT *pMax_Kazaa = (UINT*)&buf[sizeof(KazaaControllerHeader)];
	*pMax_Kazaa = max_kazaa;

	AppID app_id;
	app_id.m_app_id = 12; //kazaa launcher app id
	app_id.m_version = 0x00090002;

	return DllInterface::SendData(app_id,buf);	
}

//
//
//
bool KazaaLauncherInterface::SetKazaaLaunchingInterval(int seconds)	//set the launching interval (default is 1 minute)
{
	if(seconds > 0)
	{
		byte buf[sizeof(KazaaControllerHeader)+sizeof(int)];
		KazaaControllerHeader* header = (KazaaControllerHeader*)&buf;
		header->op = KazaaControllerHeader::Kazaa_Launching_Interval;
		header->size = sizeof(int);

		int *pLaunch_Interval = (int*)&buf[sizeof(KazaaControllerHeader)];
		*pLaunch_Interval = seconds;

		AppID app_id;
		app_id.m_app_id = 12; //kazaa launcher app id
		app_id.m_version = 0x00090002;

		return DllInterface::SendData(app_id,buf);	
	}
	else
		return false;
}

//
//
//
bool KazaaLauncherInterface::DisableMinimizingKazaaWindows()
{
	byte buf[sizeof(KazaaControllerHeader)];
	KazaaControllerHeader* header = (KazaaControllerHeader*)&buf;
	header->op = KazaaControllerHeader::Disable_Minimizing_Kazaa_Windows;
	header->size = 0;

	AppID app_id;
	app_id.m_app_id = 12; //kazaa launcher app id
	app_id.m_version = 0x00090002;

	return DllInterface::SendData(app_id,buf);	
}

//
//
//
bool KazaaLauncherInterface::CheckKazaaConnectionStatus(int seconds)
{
	if(seconds > 0)
	{
		byte buf[sizeof(KazaaControllerHeader)+sizeof(int)];
		KazaaControllerHeader* header = (KazaaControllerHeader*)&buf;
		header->op = KazaaControllerHeader::Check_Kazaa_Connection_Status;
		header->size = sizeof(int);

		int *pLaunch_Interval = (int*)&buf[sizeof(KazaaControllerHeader)];
		*pLaunch_Interval = seconds;

		AppID app_id;
		app_id.m_app_id = 12; //kazaa launcher app id
		app_id.m_version = 0x00090002;

		return DllInterface::SendData(app_id,buf);	
	}
	else
		return false;
}