// NoiseSockets.cpp

#include "stdafx.h"
#include "NoiseSockets.h"
#include "FrameHeader.h"
#include "FileSharingManager.h"
//
//
//
NoiseSockets::NoiseSockets(HWND hwnd,CCriticalSection *connection_data_critical_section,vector<ConnectionData> *connection_data,FileSharingManager* fsm)
{
	m_sharing_enabled = true;
	p_fs_manager = fsm;
	m_hwnd=hwnd;
	p_connection_data_critical_section=connection_data_critical_section;
	p_connection_data=connection_data;

	m_num_bogus_connections=0;
	m_num_good_connections=0;

	int num_reserved_events=ReturnNumberOfReservedEvents();
	int num_socket_events=ReturnNumberOfSocketEvents();

	// Init the parent pointers and message window handles
	for(int i=0;i<num_socket_events;i++)
	{
		m_sockets[i].InitParent(this);
	}
	
	// Create the reserved events
	for(i=0;i<num_reserved_events;i++)
	{
		m_events[i]=WSACreateEvent();
		if(m_events[i]==WSA_INVALID_EVENT)
		{
			char msg[1024];
			strcpy(msg,"Could not create a valid reserved event ");
			_itoa(i,&msg[strlen(msg)],10);
			::MessageBox(NULL,msg,"Error",MB_OK);
		}
	}

	// Fully initialize events array
	for(i=0;i<num_socket_events;i++)
	{
		m_events[num_reserved_events+i]=m_sockets[i].ReturnEventHandle();
	}

	//p_noise_data_buf=NULL;
	
	//InitNoiseData();
}

//
//
//
NoiseSockets::~NoiseSockets()
{
	/*
	if(p_noise_data_buf!=NULL)
	{
		delete [] p_noise_data_buf;
		p_noise_data_buf=NULL;
	}
	*/

	int num_reserved_events=ReturnNumberOfReservedEvents();
	
	// Close the reserved events
	for(int i=0;i<num_reserved_events;i++)
	{
		if(WSACloseEvent(m_events[i])==FALSE)
		{
			char msg[1024];
			strcpy(msg,"Could not close reserved event ");
			_itoa(i,&msg[strlen(msg)],10);
			::MessageBox(NULL,msg,"Error",MB_OK);
		}
	}
}


//
//
//
DWORD NoiseSockets::ReturnNumberOfEvents()
{
	return NumberOfEvents;
}

//
//
//
int NoiseSockets::ReturnNumberOfReservedEvents()
{
	return NumberOfReservedEvents;
}

//
//
//
int NoiseSockets::ReturnNumberOfSocketEvents()
{
	return NumberOfSocketEvents;
}

//
//
//
void NoiseSockets::AcceptConnection(SOCKET hSocket)
{
	int i;

	// Find an idle socket
	for(i=0;i<NumberOfSocketEvents;i++)
	{
		if(m_sockets[i].IsSocket()==false)
		{
			m_sockets[i].Attach(hSocket);
			break;
		}
	}
}

//
// Timer has fired - every second
//
vector<NoiseModuleThreadStatusData> NoiseSockets::ReportStatus()
{
	// Check to see if it a new day...to see if we need to init the noise data again
	/*
	CTime now=CTime::GetCurrentTime();
	if(m_time_last_initialized_noise_data.GetDay()!=now.GetDay())
	{
		InitNoiseData();
	}
	*/
	int i;

	vector<NoiseModuleThreadStatusData> status;

	for(i=0;i<NumberOfSocketEvents;i++)
	{
		NoiseModuleThreadStatusData data=m_sockets[i].ReportStatus();
		data.m_socket=i;
		data.m_num_bogus_connections=0;		// just in case
		data.m_num_good_connections=0;		// just in case
		status.push_back(data);
	}

	status[0].m_num_bogus_connections=m_num_bogus_connections;
	status[0].m_num_good_connections=m_num_good_connections;
	m_num_bogus_connections=0;
	m_num_good_connections=0;

	return status;
}

//
//
//
void NoiseSockets::LogToDB(UploadInfo& info)
{
	p_fs_manager->StoreUploadLog(info);
}