#pragma once
#include "..\tkcom\object.h"
#include "..\tkcom\Buffer2000.h"
#include "..\tkcom\HashObject.h"
#include "..\AresProtector\AresPacket.h"
#include "..\AresProtector\AresHost.h"
#include "..\tkcom\LinkedList.h"
#include "..\tkcom\Vector.h"
#include "..\tkcom\Timer.h"
#include "..\tkcom\PeerSocket.h"
#include "..\tkcom\SocketEventListener.h"

class AresSNClientConnection : public SocketEventListener, public Object
{

private:
	bool mb_clean;
	//Vector mv_shares_to_upload;

	unsigned short int m_fss;	// four-shit-short
	unsigned char m_fsb;		// four-shit-byte

	UINT m_id;
	//Vector mv_hosts;

	bool mb_marked_for_death;

	CTime m_death_time;
	CTime m_create_time;
	Timer m_last_receive;
	Timer m_last_ping_sent;

	UINT m_con_handle;  //only used if we are using event based sockets
#ifndef TKSOCKETSYSTEM
	PeerSocket *mp_ps_con;
#endif


	Timer m_last_purge;
	//PeerSocket *mp_ps_con;

	unsigned int m_remote_ip;
	string m_str_ip;
	unsigned short m_my_port;
	unsigned short m_port;
	string m_status;

	unsigned int m_state;

	Buffer2000 m_receive_buffer;

	bool mb_dead;

	//a vector of events
	LinkedList m_event_list;

	void ProcessPacket(AresPacket& packet);

	void Init(const char *host_ip,unsigned short port);	

protected:



public:
	AresSNClientConnection(PeerSocket *ps);
	~AresSNClientConnection(void);

//#ifdef TKSOCKETSYSTEM
	/*
	void OnClose();
	void OnSend();
	void OnReceive(byte *data,UINT length);
	void OnConnectFailed();
	void OnConnect();
	*/
//#else
	//**************
	void OnClose(CAsyncSocket* src);
	void OnSend(CAsyncSocket* src);
	void OnReceive(CAsyncSocket* src,byte *data,UINT length);
	//**************
//#endif

	inline const char* GetStatus(){return m_status.c_str();}
	inline const char* GetIP(){return m_str_ip.c_str();}
	inline unsigned short GetPort(){return m_port;}
	inline CTimeSpan GetAliveTime(){return CTime::GetCurrentTime()-m_create_time;}

	inline bool IsClean(){return mb_clean;}

	void Update(void);
	void Kill(void);
	CString GetFormattedAgeString(void);
	
	LinkedList* GetEventList(void);

	inline bool IsDead(void){
		return mb_dead;
	}

	inline bool IsDying(void){
		return mb_marked_for_death || mb_dead;
	}

	inline UINT GetID(void)
	{
		return m_id;
	}

	void CleanUp(void);
	void RecordReceiveEvent(AresPacket* packet);
	void RecordSendEvent(AresPacket* packet);
	
private:
	void PurgeEvents(void);

protected:
	void SendData(byte* data, UINT length);
public:
	UINT GetConHandle(void);
	void SendMessage0x38(void);
};
