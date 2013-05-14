#include "StdAfx.h"
#include "ComService.h"
#include "TKComInterface.h"
#include "ComSubscriber.h"
#include "MessageDispatcher.h"
#include "ComDialog.h"
#include "ComConnection.h"
//#include "TKComSocket.h"

using namespace comspace;

ComService::ComService(void)
{
	TRACE("ComService::ComService()  BEGIN\n");
	TRACE("ComService::ComService()  END\n");
}

ComService::~ComService(void)
{
}

//This is a thread that checks to see if any anyone is trying to open a connection to this trapper keeper.
//It also maintains the vector of connections, controlling when Send threads start up and deleting expired/malfunctioning connections
UINT ComService::Run(){
	
	int retval=0;

	while(!this->b_killThread){ //lets go into an infinite loop checking for new connections until the program closes.

		m_plugin_manager.Process(&m_tcp);

		m_tcp.CheckData();

		m_tcp.Update();

		Sleep(5); //Basically it'll go through this loop at most 20 times a second.  
	}

	return 0;	

}

//Called by the dll to tell the trapper keeper that an app wants to either send data, or register one of its com interfaces.
bool ComService::ReceivedAppData(UINT from_app_id , byte* data )
{
	//The first byte is a com service key, the second is a command, and the next 4 is an op_code

	//THIS KEY BETTER MATCH, OR DATA WAS IMPROPERLY ROUTED HERE
	if(data[0]!=TKCOMBUFFERKEY){
		TRACE("The plugin identified as %d falsely sent data to the Com service.",from_app_id);
		return false; //-1 is an error
	}

	//DOES AN APP WANT TO ADD ONE OF ITS COM INTERFACE OBJECTS TO THE SUBSCRIBER LIST?
	if(data[1]==TKCOMREGISTERCODE){
		UINT op_code=*((UINT*)(data+2));
		TRACE("COM SERVICE:  App %d registering for op_code %d.\n",from_app_id,op_code);
		for(UINT i=0;i<m_message_dispatcher.mv_subscribers.Size();i++){
			ComSubscriber *subscriber = (ComSubscriber*) m_message_dispatcher.mv_subscribers.Get(i);
			if(subscriber->m_from_app==from_app_id && subscriber->m_op_code==op_code){
				subscriber->AddRef();  //wierd but we will allow it.  multiple com interface objects exist within the plugin with the same op_code.
				return true;
			}
		}
		//if it gets here then we need to create a new subscriber object
		m_message_dispatcher.mv_subscribers.Add(new ComSubscriber(op_code,from_app_id));
		return true;
	}

	//DOES AN APP WANT TO REMOVE ONE OF ITS COM INTERFACE OBJECTS?
	if(data[1]==TKCOMDEREGISTERCODE){
		UINT op_code=*((UINT*)(data+2));
		TRACE("COM SERVICE:  App %d deregistering for op_code %d.\n",from_app_id,op_code);
		CSingleLock lock(&m_message_dispatcher.m_subscriber_vector_lock,TRUE);
		TRACE("COM SERVICE:  Final Notice App %d deregistering for op_code %d.\n",from_app_id,op_code);
		
		for(UINT i=0;i<m_message_dispatcher.mv_subscribers.Size();i++){
			ComSubscriber *subscriber = (ComSubscriber*) m_message_dispatcher.mv_subscribers.Get(i);
			if(subscriber->m_from_app==from_app_id && subscriber->m_op_code==op_code){
				subscriber->DeRef();
				if(subscriber->IsEmpty()){ //this should be empty unless an app was doing something wierd and creating multiple com objects with the same op_code
					m_message_dispatcher.mv_subscribers.Remove(i);
					break;
				}
			}
		}
		return true;
	}

	//DOES AN APP WANT TO SEND DATA?
	if(( data[1] & TKCOMMESSAGECODE ) > 0){
		UINT op_code=*((UINT*)(data+2));
		UINT data_length=*((UINT*)(data+6));

		string dest=(char*)(data+10);

		if(dest.size()<1)
			return false;

		UINT data_start=10+(int)dest.size()+1;
		UINT message_length=data_length;

		CString tmp=CString((char*)dest.c_str());  //wierd for unicode

		vector <string> v_ips;

		int dot_count=0;
		for(UINT i=0;i<dest.size();i++){
			if(dest[i]=='.')
				dot_count++;
		}

		if(dot_count!=3){  //don't ask the name server about an ip, but ask it about anything else
			p_name_server.RequestIP(tmp,v_ips);
		}
		
		if(v_ips.size()<1) //if it didn't return anything, lets just push the passed in dest onto the vector
			v_ips.push_back(dest);

		//increment our counters for statistics
		for(int send_index=0;send_index<(int)v_ips.size();send_index++){
			g_com_dialog.PostSendRequest(op_code,v_ips[send_index].c_str(),data_length);
		}

		//see if this plugin has too much queued up already, if it does return false, else return true.
		return m_plugin_manager.QueueSend(new DataBuffer(data+data_start,message_length,op_code),v_ips,from_app_id);
	}

	TRACE("COM SERVICE:  There was a crazy Message sent at the Com Service from %d",from_app_id);
	return true;
}

void ComService::StopComService(void)
{
	TRACE("COM SERVICE:  Beginning Shutdown of Trapper Keeper COM service.\n");
	this->StopThread();  //shutdown the server thread.
	//m_tcp.StopTCP();
	m_tcp.StopSystem();
	//m_socket_pool.Shutdown();
	Sleep(10);
	TRACE("COM SERVICE:  Shutdown of Trapper Keeper COM service complete.\n");
}

void ComService::StartComService(void)
{
	TRACE("ComService::StartComService()\n");
	this->StartThread();
	m_tcp.Listen(TKRELIABLEPORT);
	m_tcp.SetDispatcher(&m_message_dispatcher);
	g_com_dialog.SetDispatcher(&m_message_dispatcher);
}