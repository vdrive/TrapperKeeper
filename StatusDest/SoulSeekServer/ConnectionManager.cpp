// ConnectionManager.cpp

#include "stdafx.h"
#include "ConnectionManager.h"
#include "SoulSeekServerDll.h"
#include "ConnectionModule.h"

//
//
//
ConnectionManager::ConnectionManager()
{
	p_parent=NULL;
}

//
//
//
ConnectionManager::~ConnectionManager()
{
	unsigned int i;
	for(i=0;i<v_mods.size();i++)
	{
		delete v_mods[i];
	}
	v_mods.clear();
}

//
//
//
void ConnectionManager::InitParent(SoulSeekServerDll *parent)
{
	p_parent=parent;
}

//
//
//
void ConnectionManager::AddParentConnection(SOCKET socket, char* ip)
{
	for( size_t i = 0; i < v_mods.size(); i++ )
	{
		if( v_mods[i]->GetIdleSocketCount() > 0 )
		{
			p_parent->m_dlg.SetParentSocketData(1,0,0,0,0);
			v_mods[i]->AddParentConnection( socket, ip );
			TRACE("Added socket to module %d\r\n", i);
			return;
		}
	}

	AddModule();
	p_parent->m_dlg.SetParentSocketData(1,0,1,0,0);
	v_mods[v_mods.size()-1]->AddParentConnection( socket, ip );
	TRACE("Created new socket and added socket to module %d\r\n", v_mods.size()-1);
}

void ConnectionManager::LogMsg(char *pMsg)
{
	if( pMsg != NULL )
	{
		p_parent->m_dlg.WriteToLog( 0,PARENT_NUM,pMsg );
		delete pMsg;
	}
}

void ConnectionManager::ReceivedConnection(unsigned int token)
{
	if( token != NULL )
	{
		p_parent->parents.ReceivedConnection(token);
	}
}

void ConnectionManager::ProcessSearchRequest(SearchRequest* sr)
{
	if( sr != NULL )
	{
		p_parent->m_SM.ProcessSearchRequest(sr);
	}
}

//
//
//
void ConnectionManager::CloseAllModules()
{
	unsigned int i;
	for(i=0;i<v_mods.size();i++)
	{
		p_parent->m_dlg.SetParentSocketData(0,0,-1,0,0);
		delete v_mods[i];
	}
	v_mods.clear();
}

//
//
//
void ConnectionManager::AddModule()
{
	ConnectionModule *mod=new ConnectionModule;
	mod->InitParent(this);
	v_mods.push_back(mod);
}

void ConnectionManager::UpdateParentSockets(int iCParents,int iTParents,int iPMods)
{
	//p_parent->m_dlg.SetParentSocketData(iCParents,iTParents,iPMods,0,0);
}