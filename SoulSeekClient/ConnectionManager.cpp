// ConnectionManager.cpp

#include "stdafx.h"
#include "ConnectionManager.h"
#include "SoulSeekClientDll.h"
#include "ClientModule.h"

//
//
//
ConnectionManager::ConnectionManager()
{
	p_parent=NULL;
	m_socketNumberCounter = 0;
}

//
//
//
ConnectionManager::~ConnectionManager()
{
	CloseAllModules();
}

//
//
//
void ConnectionManager::InitParent(SoulSeekClientDll *parent)
{
	p_parent=parent;
}


void ConnectionManager::LogMsg(char *pMsg)
{
	if( pMsg != NULL )
	{
		p_parent->m_dlg.WriteToLog( 0,PARENT_NUM,pMsg );
		delete[] pMsg;
	}
}

void ConnectionManager::CloseClientConnection()
{
	p_parent->m_dlg.SetParentSocketData(0,0,0,-1,0);
	if(v_clientMods.size() <= 1) return;  //only one mod should ever be empty
	if(v_clientMods.size() > 16)
	{
		for( size_t i = 16; i <v_clientMods.size(); i++ )
		{
			if( v_clientMods[i]->GetIdleSocketCount() >= 60 )
			{
				//delete v_clientMods[i];
				TRACE("Closing Client Mod %i\n", i);
				//delete v_clientMods[i];
				v_clientMods[i]->Shutdown();
				v_clientMods.erase(v_clientMods.begin() + i);
				p_parent->m_dlg.SetParentSocketData(0,0,0,0,-1);
				i--;
			}
		}
	}
}
void ConnectionManager::OpenAnotherClientConnection()
{
	p_parent->m_dlg.SetParentSocketData(0,0,0,1,0);
}

void ConnectionManager::RoutePacket(PacketWriter* packet)
{
	if( packet == NULL )
	{
		p_parent->m_dlg.WriteToLog(1,0,"Error: packet is null");
		return;
	}
	if( v_clientMods.size() > 1){
		for( size_t i = 0; i < v_clientMods.size()-1; i++ )
		{
			if( v_clientMods[i]->GetIdleSocketCount() > 0 )
			{
				if(v_clientMods[i]->SendPacket( packet )== true)
				{
					//TRACE("Routing Packet\n");
					return;
				}
				else
				{
					p_parent->m_dlg.WriteToLog(1,0,"Could not add socket");
				}
			}
		}
	}
	if(v_clientMods.size() > 0 && v_clientMods[v_clientMods.size()-1]->SendPacket( packet )== true)
	{
		//TRACE("Routing Packet\n");
		AddClientModule(m_socketNumberCounter);
		p_parent->m_dlg.SetParentSocketData(0,0,0,0,1);
		TRACE("Created new socket and added socket to module %d\r\n", v_clientMods.size()-1);
		m_socketNumberCounter++;
	}
	else
	{
		AddClientModule(m_socketNumberCounter);
		p_parent->m_dlg.SetParentSocketData(0,0,0,0,1);
		v_clientMods[v_clientMods.size()-1]->SendPacket( packet );
		TRACE("Created new socket and added socket to module %d\r\n", v_clientMods.size()-1);
		m_socketNumberCounter++;
	}
}

//
//
//
void ConnectionManager::CloseAllModules()
{
	for(size_t i=0;i<v_clientMods.size();i++)
	{
		p_parent->m_dlg.SetParentSocketData(0,0,0,0,-1);
		delete v_clientMods[i];
	}
	v_clientMods.clear();
}


//
//
//
void ConnectionManager::AddClientModule(int id)
{
	ClientModule *mod = new ClientModule(id);
	mod->InitParent(this);
	v_clientMods.push_back(mod);
}

void ConnectionManager::DeleteClient(ClientModule* cmod)
{
	p_parent->m_dlg.DeleteClient(cmod);
}

void ConnectionManager::WriteToFile(string s)
{
	p_parent->WriteToTextFile(s);
}