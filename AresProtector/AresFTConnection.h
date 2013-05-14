#pragma once
#include "..\tkcom\object.h"
#include "..\tkcom\Timer.h"
#include "..\tkcom\Buffer2000.h"

class AresFTConnection : public Object
{
private:

	UINT m_id;
	bool mb_connected;
	bool mb_dead;
	UINT m_con_handle;
	//PeerSocket *mp_ps_con;
	unsigned int m_state;
	string m_str_ip;
	unsigned short m_port;
	string m_status;

	CTime m_death_time;
	CTime m_create_time;
	Timer m_last_receive;

	bool mb_marked_for_death;

	Buffer2000 m_receive_buffer;
	//Buffer2000 m_send_buffer;
	//int m_send_offset;
	//Timer m_last_send_purge;
	
	
	UINT m_transferred;

public:
	static UINT sm_queued;

	inline bool IsDead(void){
		return mb_dead;
	}

	inline UINT GetID(void)
	{
		return m_id;
	}

	inline const char* GetStatus(){return m_status.c_str();}

	inline const char* GetIP(){return m_str_ip.c_str();}
	inline unsigned short GetPort(){return m_port;}

	CString GetFormattedAgeString(void);
	inline CTimeSpan GetAliveTime(){return CTime::GetCurrentTime()-m_create_time;}
	void OnReceive(byte *data,UINT length);
	void OnSend();
	AresFTConnection(UINT con_handle,const char* peer,unsigned short port);
	~AresFTConnection(void);
	void OnClose();
	void CleanUp(void);
	void Update(void);
	void Kill(void);
	void ProcessPacket(void);
	void SendData(const byte *data, UINT length);
	//void GetCache(Buffer2000& data);
	//void AppendToCache(Buffer2000& data);
	//void SetCache(Buffer2000& data);
	void RespondHashSet(byte* p_hash, UINT start, UINT end, unsigned short code);
	UINT GetConHandle(void);
};
