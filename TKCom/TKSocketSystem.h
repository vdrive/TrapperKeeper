#pragma once
#include "..\tkcom\threadedobject.h"
#include "..\tkcom\vector.h"
#include <afxmt.h>  //critical section stuff

#define TKTCP_UNKNOWN 0
#define TKTCP_OK 1
#define TKTCP_INVALID_HANDLE 2
#define TKTCP_TIMED_OUT 3
#define TKTCP_SEND_FAILED 4
#define TKTCP_CONNECTION_CLOSED 5
#define TKTCP_RECV_FAILED 6
#define TKTCP_REQUEST_CLOSE 7
#define TKTCP_SERVER_PORT_NOT_AVAILABLE 8
#define TKTCP_SERVER_INIT_FAILED 9
#define TKTCP_CONNECT_INVALID_ADDRESS 10
#define TKTCP_CONNECT_FAILED 11
#define TKTCP_TOOMANYCONNECTINGSOCKETS 12

class TKSocketSystem
{
private:
	class GenericBuffer : public Object{
		friend class TKTCP;
	public:
		byte *m_data;
		UINT m_data_length;
		GenericBuffer(const byte* data,UINT length){
			m_data=new byte[length];
			m_data_length=length;
			memcpy(m_data,data,length);
		}
		~GenericBuffer(){
			if(m_data)	delete []m_data;
			m_data=NULL;
		}
	};

	class EventOnConnect : public Object{
	public:
		UINT m_id;
		string m_peer;
		EventOnConnect(UINT id, const char* peer){
			m_id=id;
			m_peer=peer;
		}
	};

	class BadCon : public Object{
	public:
		UINT m_ip;
		CTime m_create_time;
		BadCon(const char* peer){
			m_ip=inet_addr(peer);
			m_create_time=CTime::GetCurrentTime();
		}

	};

	class EventOnClose : public Object{
	public:
		UINT m_id;
		UINT m_reason;
		string m_peer;
		EventOnClose(UINT id,UINT reason,const char* peer){
			m_id=id;
			m_reason=reason;
			m_peer=peer;
		}
	};

	class EventOnSend : public Object{
	public:
		UINT m_id;
		string m_peer;
		EventOnSend(UINT id,const char* peer){
			m_peer=peer;
			m_id=id;
		}
	};

	class EventOnAcceptedConnection : public Object{
	public:
		UINT m_id;
		UINT m_server_handle;
		string m_ip;
		unsigned short m_port;
		EventOnAcceptedConnection(UINT id,UINT server_handle,const char* ip,unsigned short port){
			m_id=id;
			m_ip=ip;
			m_port=port;
			m_server_handle=server_handle;
		}
	};

	class EventOnReceive : public Object{
	public:
		UINT m_id;
		GenericBuffer m_data;
		string m_peer;
		EventOnReceive(UINT id,byte* p_data,UINT length,const char* peer)
			:m_data(p_data,length)
		{
			m_peer=peer;
			m_id=id;
		}
	};

	class Server : public Object{
	private:
		friend class TKSocketSystem;
		SOCKET m_server_socket;
		unsigned short m_port;
		UINT m_id;
		Server(unsigned short port,UINT id);
	};

	class TKSocketModule : public ThreadedObject
	{
	private:
		CCriticalSection m_socket_lock;
		void SyncSends();
		void SyncReceiving();

		UINT m_id_counter;
		Vector mv_sendable_sockets;
		Vector mv_receivable_sockets;

		WSAEVENT	m_send_event_array[64];  //have an array of up to 64 events where x is the maximum number of sockets
		WSAEVENT	m_rcv_event_array[64];  //have an array of up to 64 events where x is the maximum number of sockets
		UINT m_event_count;

		bool mb_needs_update;

	public:


		TKSocketSystem *mp_owner;
		Vector mv_connections;
		Vector mv_dead_sockets;
		UINT AddConnection(const char* ip,unsigned short port,byte module_index);
		UINT AddConnection(SOCKET handle,const char* ip,unsigned short port,byte module_index);
		UINT Run(void);
		bool SendData(UINT id,const byte* data,UINT length);

		UINT GetConnectionCount();
		void ShutdownModule();
		int GetConnection(UINT id);
		Vector mv_connecting_sockets;
		TKSocketModule(TKSocketSystem* p_owner){
			mp_owner=p_owner;
			m_id_counter=1;
			mb_needs_update=true;
			m_event_count=0;
			mv_connections.EnsureSize(65);
		}
		void CloseConnection(UINT id);
	};

	class Connection : public ThreadedObject{
	public:
		SOCKET m_socket_handle;  //this is the actual socket handle of course
		WSABUF m_wsa_recv_buffer;
		WSABUF m_wsa_send_buffer;
		WSAOVERLAPPED	m_ol_send;
		WSAOVERLAPPED	m_ol_recv;
		WSAEVENT m_event_recv;
		WSAEVENT m_event_send;
		Vector mv_send_queue;
		bool mb_valid;
		bool mb_connected;
		bool mb_connecting;
		bool mb_is_closed;
		bool mb_cleaned;
		CTime m_create_time;
		//Object* mp_owner_module;
		
		string m_peer;
		unsigned short m_peer_port;
		UINT m_id;

		bool mb_awaiting_send_notification;
		bool mb_needs_receive; //a flag to denote whether WSARecv has been called or not.  We must be careful to call it from the correct thread or it will fuck up.
		bool mb_needs_send;  //ty 6.12.2003
		bool mb_started_connecting;

		UINT m_error;
		
		Connection(SOCKET handle,const char* peer,unsigned short peer_port,UINT id,TKSocketModule* p_owner_module);
		bool SendMore();
		void Clean();
		UINT Run();
		void EnlargeReceiveBuffer();
		TKSocketModule *mp_owner_module;

		~Connection(){
			//TRACE("~Connection() id=%d, peer=%s.\n",m_connection_info.GetHandle(),m_peer.c_str());
			this->StopThread();
			this->Clean();
		}
	};

	friend class TKSocketModule;

	Vector mv_modules;

	fd_set m_server_set;
	string m_local_ip;
	Vector mv_servers; //a vector of listening sockets
	UINT m_server_id_counter;
	CCriticalSection m_socket_lock;
	Vector mv_onsend_events;
	Vector mv_onreceive_events;
	Vector mv_onconnect_events;
	Vector mv_onclose_events;
	Vector mv_onacceptedconnection_events;
	Vector mv_bad_cons[1000];
	void InitWinSock(void);

	CCriticalSection m_bad_con_lock;
	CTime m_last_bad_con_update;

public:
	TKSocketSystem(void);
	~TKSocketSystem(void);

	//override this to be notified when data has come in for one of your connections
	virtual void OnReceive(UINT con_id , byte* data , UINT length,const char* peer){}

	//override this to be notified when data has been successfully sent to the destination
	//if a connection is functioning properly, then you will get 1 OnSend for each send request you made.
	virtual void OnSend(UINT con_id,const char* peer){}

	virtual void OnServerError(UINT server_id,UINT error){}

	//Override this to be notified when a particular connection has been closed for any number of reasons.  This will not be called if you explicitly called CloseConnection().
	virtual void OnClose(UINT con_id,UINT reason, const char* peer){}

	//override this to keep new connections from being automatically closed.
	//handle is how you reference your new connection.
	virtual void OnAcceptedConnection(UINT server_handle,UINT con_id,const char* peer,unsigned short port);

	//notification that a OpenConnection call has completed.
	virtual void OnConnect(UINT con_id, const char* peer){}

	const char* GetLocalIP(){return m_local_ip.c_str();}

	UINT Listen(unsigned short port);  //call to listen for new connections on a certain port
	void StopListening(UINT con_handle);  //call to stop listening for new connections on a certain port

	UINT SendData(UINT id,const byte* data, UINT length);
	
	UINT OpenConnection(const char* peer, unsigned short port);
	void Update(void);
	void CloseConnection(UINT id);
	void CheckForNewConnections(void);
	void StopSystem(void);
	bool IsBadCon(const char* ip);
	void AddBadCon(const char* peer);

	static int  UDPSend(SOCKET hsock,const char* dest, unsigned short port,byte *p_data,UINT len);
	static int UDPReceive(SOCKET hsock,string &addr,unsigned short &port,byte *p_data);
	UINT GetNumberOfModules(void);
};
