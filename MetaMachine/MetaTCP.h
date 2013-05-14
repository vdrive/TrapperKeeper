#pragma once
#include "..\tkcom\tktcp.h"
#include "..\tkcom\Vector.h"

class MetaTCP :	public TKTCP
{
private:
	Vector mv_donkey_servers;
	MetaTCP(void);
	~MetaTCP(void);

	void OnConnect(UINT handle);

	//notification that data has been receivinged.
	void OnReceive(UINT handle , byte* data , UINT length);

	//notification that a send has completed
	void OnSend(UINT handle);

	//notification of an error on a listening socket
	void OnServerError(UINT handle,UINT error);

	//notification that a connection has been closed due to an error or the peer closing the other end
	void OnClose(UINT handle,UINT reason);

	//notification that a connection has been accepted
	void OnAcceptedConnection(UINT server_handle,UINT connection_handle, const char* source_address);


public:

//************ TCP Singleton Paradigm *****************//
private:
	friend class TCPReference;  //make tcpreference a friend so it can access these private function.
	static void ReleaseTCP(){
		ASSERT(sm_tcp_count>0);
		sm_tcp_count--;
		if(sm_tcp_count==0){
			TRACE("MetaMachine Service:  MetaTCP Static Object Destroyed.\n");
			sm_tcp->StopTCP();
			delete sm_tcp;
			sm_tcp=NULL;
		}
	}
	static MetaTCP* GetTCP(){
		if(sm_tcp==NULL){
			TRACE("MetaMachine Service:  New MetaTCP Static Object Created.\n");
			sm_tcp=new MetaTCP();
			//sm_tcp->StartTCP();
		}
		sm_tcp_count++;
		return sm_tcp;
	}

	static UINT sm_tcp_count;
	static MetaTCP *sm_tcp;  //this is set to NULL in the .cpp file
//************ End TCP Singleton Paradigm *************//
};

//standard singleton accessor class.  It's extremely simple so it gets to be in with the MetaTCP class which is the sole reason it exists anyways
class TCPReference :
	public Object
{
private:
	MetaTCP *mp_reference;
public:
	TCPReference(void){
		mp_reference=MetaTCP::GetTCP();
	}
	~TCPReference(void){
		MetaTCP::ReleaseTCP();
	}
	inline MetaTCP* TCP(void){
		return mp_reference;
	}
};