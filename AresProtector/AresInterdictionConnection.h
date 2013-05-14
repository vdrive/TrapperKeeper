#pragma once
#include "..\tkcom\Timer.h"
#include "..\tkcom\Buffer2000.h"
#include "..\tkcom\Vector.h"
#include "InterdictionTarget.h"

class AresInterdictionConnection : public Object
{
private:
	bool mb_connected;
	bool mb_dead;
	UINT m_con_handle;
	//PeerSocket *mp_ps_con;
	unsigned int m_state;
	string m_str_ip;
	unsigned short m_port;

	unsigned short m_decryption_code;
	string m_status;

	Vector m_host;

	UINT m_ip;

	CTime m_death_time;
	CTime m_create_time;
	Timer m_last_receive;

	bool mb_marked_for_death;

	Buffer2000 m_receive_buffer;
	//Buffer2000 m_send_buffer;
	//Timer m_last_send_purge;
	//int m_send_offset;

	UINT m_transferred;
	
	byte m_hash[20];
	UINT m_size;

public:

	inline bool IsConnected(void){
		return mb_connected;
	}

	inline bool IsDead(void){
		return mb_dead;
	}

	inline const char* GetStatus(){return m_status.c_str();}

	inline const char* GetIP(){return m_str_ip.c_str();}
	inline unsigned int GetIntIP(){return m_ip;}
	inline unsigned short GetPort(){return m_port;}

	CString GetFormattedAgeString(void);
	inline CTimeSpan GetAliveTime(){return CTime::GetCurrentTime()-m_create_time;}
	void OnReceive(byte *data,UINT length);
	AresInterdictionConnection(InterdictionTarget* target);
	~AresInterdictionConnection(void);
	void OnClose();
	void CleanUp(void);
	void Update(void);
	void Kill(void);
	void ProcessPacket(void);
	void SendData(const byte* data,UINT length);
	//void GetCache(Buffer2000& data);
	//void AppendToCache(Buffer2000& data);
	//void SetCache(Buffer2000& data);
	void RespondHashSet(byte* p_hash, UINT start, UINT end, unsigned short code);
	void OnConnect();
	void MakeFileRequest(void);
	UINT GetTransferred(void);
	UINT GetConHandle(void);
};
