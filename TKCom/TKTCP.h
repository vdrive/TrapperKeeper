#pragma once
#include "threadedobject.h"
#ifndef FDSETSIZE
//#define FD_SETSIZE 1000  //have our set size be a lot larger than 64
#endif
#include <Winsock2.h>
#include "Vector.h" 
#include <afxmt.h>  //critical section stuff


//Any plugin that wants generic tcp should use this class.
//Override this class to create a powerful and flexible server and client system
//It is threadsafe.
//For each TKTCP instance, you can have up to 10000 open connections (combined listening and connected).  Though the OS limit is probably much lower.
//This may be the best class ever.

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

//#define MODULES 200

class TKTCPConnection : public Object
{
protected:
	string m_peer;
	unsigned short m_port;
	UINT m_handle;
public:
	TKTCPConnection(const char* peer, unsigned short port){
		m_peer=peer;
		m_port=port;
		m_handle=0;
	}

	inline TKTCPConnection& operator=(const TKTCPConnection &copy){
		m_peer=copy.m_peer.c_str();
		m_port=copy.m_port;
		m_handle=copy.m_handle;
		return *this;
	}

	TKTCPConnection(const TKTCPConnection &copy){
		*this=copy;
	}

	inline bool operator==(TKTCPConnection &copy){
		return this->m_handle==copy.GetHandle();
	}
	inline bool operator!=(TKTCPConnection &copy){
		return !(this->m_handle==copy.GetHandle());
	}

	inline UINT GetHandle(){return m_handle;}
	inline const char* GetPeer(){return m_peer.c_str();}
	inline unsigned short GetPort(){return m_port;}
	inline void SetHandle(UINT handle){m_handle=handle;}
};

class TKTCP :  private Object
{
private:
	//A nested class because we don't want class polution in VS and also this class is used only with TKTCP
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

	class Server : public Object{
	private:
		friend class TKTCP;
		SOCKET m_server_socket;
		unsigned short m_port;
		UINT m_id;
		Server(unsigned short port);
	};
	
	//A nested class because we don't want class polution in VS and also this class is used only with TKTCP
	class Connection : public ThreadedObject{
		friend class TKTCP;
		
	public:

		TKTCPConnection m_connection_info;
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
		Object* mp_owner_module;
		
		string m_peer;
		unsigned short m_peer_port;
		bool mb_awaiting_send_notification;
		bool mb_needs_receive; //a flag to denote whether WSARecv has been called or not.  We must be careful to call it from the correct thread or it will fuck up.
		bool mb_needs_send;  //ty 6.12.2003

		UINT m_error;
		Connection(SOCKET handle,const char* peer,unsigned short peer_port);
		bool SendMore();
		void Clean();
		UINT Run();

		~Connection(){
			//TRACE("~Connection() id=%d, peer=%s.\n",m_connection_info.GetHandle(),m_peer.c_str());
			this->StopThread();
			this->Clean();
		}
		
	};

	class Module : public Object{
		friend class TKTCP;
	public:
		TKTCP *mp_owner;
		bool mb_valid;
		
		Vector mv_sockets; //a vector of connected sockets
		WSAEVENT	m_send_event_array[64];  //have an array of up to 64 events where x is the maximum number of sockets
		WSAEVENT	m_rcv_event_array[64];  //have an array of up to 64 events where x is the maximum number of sockets
		UINT m_event_count;


		CCriticalSection m_socket_vector_lock;  //guard our data with our life

		Module(TKTCP* p_owner){
			mb_valid=false;
			mp_owner=p_owner;
			m_event_count=0;
		}
		~Module(){}

		Object* mp_owner_bin;
		void Update(void);
		bool IsFull(void);
		//UINT AddConnection(const char* address, unsigned short port);
		void AddConnection(Connection* con, bool b_new);
		bool CloseConnection(UINT handle);
		Vector mv_dead_sockets;  //temp dead vector
		Connection* GetConnection(UINT handle);
	};

	//A nested class because we don't want class polution in VS and also this class is used only with TKTCP
	class TKTCPJob : public Object
	{
	public:
		virtual bool IsNewConnectionJob(){return false;}
		virtual bool IsDataSendJob(){return false;}
		virtual bool IsCloseConnectionJob(){return false;}
	};

	//A nested class because we don't want class polution in VS and also this class is used only with TKTCP
	class TKTCPNewConnectionJob :  public TKTCPJob
	{
	public:
		virtual bool IsNewConnectionJob(){return true;}
		Connection *mp_new_connection;
		TKTCPNewConnectionJob(Connection *con) 
		{
			mp_new_connection=con;
			mp_new_connection->ref();
		}

		~TKTCPNewConnectionJob() 
		{
			//TRACE("~TKTCPNewConnectionJob()\n");
			mp_new_connection->deref();
		}
	};

	//A nested class because we don't want class polution in VS and also this class is used only with TKTCP
	class TKTCPCloseConnectionJob :  public TKTCPJob
	{
	public:
		TKTCPConnection m_con_info;
		virtual bool IsCloseConnectionJob(){return true;}
		TKTCPCloseConnectionJob(TKTCPConnection &con) 
			:m_con_info(con)
		{}
	};

	//A nested class because we don't want class polution in VS and also this class is used only with TKTCP
	class TKTCPDataSendJob : public TKTCPJob{
	public:
		virtual bool IsDataSendJob(){return true;}
		GenericBuffer *m_data;
		string m_dest;
		UINT m_handle;
		TKTCPDataSendJob(const byte* data, UINT length, UINT handle){
			m_data=new GenericBuffer(data,length);
			m_data->ref();
			m_handle=handle;
		}
		~TKTCPDataSendJob(){
			m_data->deref();
		}
	};

	class ModuleBin : public Object
	{
	public:
		Vector mv_modules;
		UINT m_connection_count;

		void AddConnection(TKTCP::Connection* con,TKTCP* tktcp,bool b_new);
		Connection* SendData(UINT handle,GenericBuffer *data);
		TKTCP::Connection* GetConnection(UINT handle);
		bool CloseConnection(UINT handle);
		void StopTCP(void);
		void Update(void);

		ModuleBin(){m_connection_count=0;}
		~ModuleBin(){}
		//UINT GetQueuedDataSize(void);
	};

private:
	//WSAEVENT	m_event_array[FD_SETSIZE*2/64][64];  //have an array of up to x*2 events where x is the maximum number of sockets
	//UINT		m_indexing_array[MODULES][64];  //an array for indexing which sockets have events in the event array
	//WSAEVENT	m_event_array[MODULES][64];  //have an array of up to x*2 events where x is the maximum number of sockets
	//UINT		m_event_index[MODULES];  //counts of how many each module has in it
	//UINT		m_modules_used;

	//a signal that signifies time to clean out the event_array because a socket needs removing.
	//bool mb_valid;
	//bool mb_valid_array[MODULES];
	//bool mb_valid;
	bool mb_update_mode;
	Vector mv_invalid_module_bins;

	Vector mv_connecting_sockets;
	Vector mv_sendable_sockets;
	Vector mv_receivable_sockets;

	//CCriticalSection m_sendable_vector_lock;  //guard our data with our life
	//CCriticalSection m_receivable_vector_lock;  //guard our data with our life

	//CCriticalSection m_data_lock;  //guard our data with our life

	Vector mv_job_queue;
	
	//UINT m_queued_data_size;

	//CCriticalSection module_bin_lock;
	Vector mv_servers; //a vector of listening sockets

	//Vector mv_modules;  //clusters of connections, binned only for efficiency.
	ModuleBin m_module_bins[128];  //collections of modules for rapid lookup of a handle, its a power of 2 for a fast mod operation by ANDing the index with 0x7f or 127

	//Vector mv_new_sockets;

	//** internal processing
	void SyncReceiving();
	void SyncSends();
	void CheckForNewConnections(void);
	//**

	//** internal winsock management
	static void InitWinSock(void);  //call to do the WSAStartup stuff.  Only call once in your plugin.
	static void CleanUpWinSock(void);  //call to do the WSACleanup stuff.  Only call once when shutting down.
	ModuleBin* GetModuleBin(UINT handle);
	fd_set m_server_set;
	static string m_local_ip;
	//**

public:
	TKTCP(void);
	~TKTCP(void);

	//This creates a connection to the specified peer.  It is a blocking call.
	//It returns a number of values to denote its success or failure, TKTCP_OK is successful
	//handle will be the way you reference your new connection.
	UINT OpenConnection(TKTCPConnection& con_ref);

	//If OnClose has already been called with this handle, there is no need to call CloseConnection()
	//Also, there is no need to call this before everything is shutdown.  It is all cleaned up.
	void CloseConnection(TKTCPConnection &con);

	//Call to send data over a specific connection
	UINT SendData(TKTCPConnection &con, const byte* data, UINT length);

	//call to start listening for new connections on a certain port.
	//returns the handle to the server now representing that port
	UINT Listen(unsigned short port);

	//You MUST call this function when you are done using TKTCP to allow it to clean up.
	void StopTCP(void);

//***************************OVERRIDABLES - OVERRIDE TO RECEIVE NOTIFICATIONS**************************//
protected:
	//override to be notified of errors that have caused your server to not be able to accept any connections
	virtual void OnServerError(UINT handle,UINT error){}

	//override this to be notified when data has come in for one of your connections
	virtual void OnReceive(TKTCPConnection &con , byte* data , UINT length){}

	//override this to be notified when data has been successfully sent to the destination
	//if a connection is functioning properly, then you will get 1 OnSend for each send request you made.
	virtual void OnSend(TKTCPConnection &con){}

	//Override this to be notified when a particular connection has been closed for any number of reasons.  This will not be called if you explicitly called CloseConnection().
	virtual void OnClose(TKTCPConnection &con,UINT reason){}

	//override this to keep new connections from being automatically closed.
	//handle is how you reference your new connection.
	virtual void OnAcceptedConnection(UINT server_handle,TKTCPConnection &new_con);

	//notification that a OpenConnection call has completed.
	virtual void OnConnect(TKTCPConnection &con){}

	TKTCP::Connection* GetConnection(UINT id);
public:

	//retreives the number of connected sockets that are alive
	UINT GetConnectionCount(void);
	
	//retreives information, if any is available, about a particular handle you may have been given
	//bool GetPeerInfo(UINT handle,string &peer_name,unsigned short &port);

	const char* GetLocalIP(){return m_local_ip.c_str();}

	//TKTCP will not update unless you call this function.  It is it's heartbeat.  Information will only flow if you say so.
	//This allows you to control which thread that you want TKTCP to do notifications in.
	//The faster you call this function, the more responsive your tcp system will be.
	//There is no real need to call it faster than every 10 ms.  15-30 ms is ideal for a responsive socket system.
	void Update(void);
	UINT GetQueuedDataSize(void);
	
	
};


