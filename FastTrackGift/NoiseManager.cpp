// NoiseManager.cpp

#include "stdafx.h"
#include "NoiseManager.h"
#include "NoiseModule.h"
#include "FastTrackGiftDll.h"

//
//
//
NoiseManager::NoiseManager()
{
	m_listening_socket.InitParent(this);
	
#ifdef _DEBUG
	m_listening_port=21688;
#else
	m_listening_port=3368;
#endif
	m_listening_port_retry_count=0;
	m_total_num_received_connections=0;
}

//
//
//
NoiseManager::~NoiseManager()
{
	UINT i;
	for(i=0;i<v_mods.size();i++)
	{
		delete v_mods[i];
	}
	v_mods.clear();
}

//
// Called when the dll is going to be removed...so that the threads (hopefully) aren't still looking for the GUID cache when everything's freed
//
void NoiseManager::KillModules()
{
	// Free memory
	for(int i=0;i<(int)v_mods.size();i++)
	{
		delete v_mods[i];
	}
	v_mods.clear();
}

//
//
//
void NoiseManager::StartServer()
{
	// Create listening socket
	m_listening_port_retry_count++;
	if(m_listening_socket.Create(m_listening_port)==SOCKET_ERROR)
	{
		char msg[1024];
		sprintf(msg,"Error binding listening socket to port %u - Trying again in 3 seconds - Attempt # %u",m_listening_port,m_listening_port_retry_count);
		p_parent->Log(msg);
		p_parent->m_dlg.SetTimer(4,3*1000,0);
		return;
	}

	m_listening_socket.Listen();

	char status[1024];
	sprintf(status,"Listening on port %u",m_listening_port);
	p_parent->Log(status);
}

//
//
//
void NoiseManager::AcceptedConnection(SOCKET hSocket)
{
	UINT i;

	m_total_num_received_connections++;
/*
	CTime time=CTime::GetCurrentTime();
	string formatted_time=time.Format("%H:%M:%S");
	string formatted_date=time.Format("%Y-%m-%d");
	char status[1024];
	sprintf(status,"Received incoming connection at %s on %s",formatted_time.c_str(),formatted_date.c_str());
	p_parent->Log(status);
*/
	// Need to give this connection to a module with an idle socket
	for(i=0;i<v_mods.size();i++)
	{
		if(v_mods[i]->IdleSocketCount()>0)
		{	
			v_mods[i]->AcceptConnection(hSocket);
			return;
		}
	}

	// If we didn't find a module to accept the connection, then create a new one
	NoiseModule *mod=new NoiseModule(&m_connection_data_critical_section,/*&v_connection_data,*/this,p_file_sharing_manager);
	v_mods.push_back(mod);

	//mod->InitParent(this,p_file_sharing_manager);
	mod->InitIndex((UINT)v_mods.size()-1);
	mod->AcceptConnection(hSocket);
}

//
//
//
void NoiseManager::ReportStatus(vector<NoiseModuleThreadStatusData> &status)
{
	p_parent->ReportNoiseManagerStatus(status);
}

//
//
//
unsigned int NoiseManager::GetModCount()
{
	return (UINT)v_mods.size();
}

//
//
//
void NoiseManager::ClearIdleModules()
{
	vector<NoiseModule *>::iterator iter = v_mods.begin();
	while(iter != v_mods.end())
	{
		if((*iter)->IdleSocketCount()==60)
		{
			delete (*iter);
			v_mods.erase(iter);
		}
		else
			iter++;
	}
}

//
//
//
void NoiseManager::OnCloseListeningPort(int error_code)
{
	char msg[256];
	sprintf(msg,"Listening socket attempted to close on port %d - Error code: %d",m_listening_port,error_code);
	p_parent->Log(msg);
	//StartServer();
}

//
//
//
void NoiseManager::SetFileSharingManager(FileSharingManager* manager)
{
	p_file_sharing_manager = manager;
}