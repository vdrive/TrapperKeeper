#pragma once
#include "..\tkcom\tktcp.h"
#include "..\tkcom\Vector.h"



class MetaSpooferTCP :	public TKTCP
{
private:
	Vector mv_donkey_servers;
	MetaSpooferTCP(void);
	~MetaSpooferTCP(void);

	void OnConnect(TKTCPConnection &con);

	//notification that data has been receivinged.
	void OnReceive(TKTCPConnection &con , byte* data , UINT length);

	//notification that a send has completed
	void OnSend(TKTCPConnection &con);

	//notification of an error on a listening socket
	void OnServerError(UINT handle,UINT error);

	//notification that a connection has been closed due to an error or the peer closing the other end
	void OnClose(TKTCPConnection &con,UINT reason);

	//notification that a connection has been accepted
	void OnAcceptedConnection(UINT server_handle,TKTCPConnection &new_con);


public:

	UINT m_servers[MULTI_CONNECTION_COUNT];

	unsigned short m_server_ports[MULTI_CONNECTION_COUNT];



//************ TCP Singleton Paradigm *****************//
private:
	friend class SpooferTCPReference;  //make tcpreference a friend so it can access these private function.
	static void ReleaseTCP(){
		ASSERT(sm_tcp_count>0);
		sm_tcp_count--;
		if(sm_tcp_count==0){
			TRACE("MetaMachine Service:  MetaSpooferTCP Static Object Destroyed.\n");
			sm_tcp->StopTCP();
			delete sm_tcp;
			sm_tcp=NULL;
		}
	}
	static MetaSpooferTCP* GetTCP(){
		if(sm_tcp==NULL){
			TRACE("MetaMachine Service:  New MetaSpooferTCP Static Object Created.\n");
			sm_tcp=new MetaSpooferTCP();
			//sm_tcp->StartTCP();
		}
		sm_tcp_count++;
		return sm_tcp;
	}

	static UINT sm_tcp_count;
	static MetaSpooferTCP *sm_tcp;  //this is set to NULL in the .cpp file
//************ End TCP Singleton Paradigm *************//
};

//standard singleton accessor class.  It's extremely simple so it gets to be in with the MetaSpooferTCP class which is the sole reason it exists anyways
class SpooferTCPReference :
	public Object
{
private:
	MetaSpooferTCP *mp_reference;
public:
	SpooferTCPReference(void){
		mp_reference=MetaSpooferTCP::GetTCP();
	}
	~SpooferTCPReference(void){
		MetaSpooferTCP::ReleaseTCP();
	}
	inline MetaSpooferTCP* TCP(void){
		return mp_reference;
	}
};