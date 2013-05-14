#pragma once
#include "SocketEventListener.h"
// PeerSocket command target
#include "..\tkcom\object.h"

class PeerSocket : public CAsyncSocket, public Object
{
protected:
	virtual void OnClose(int nErrorCode);
	virtual void OnConnect(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	virtual void OnSend(int nErrorCode);
	SocketEventListener* mp_listener;
	
	string m_ip;
	string m_bind_ip;
	unsigned short m_port;
	CTime m_time_created;
	byte *mp_tmp_receive_buffer;
	UINT m_tmp_receive_buffer_length;
	

public:
	PeerSocket();
	virtual ~PeerSocket();
	void Init(const char* peer, unsigned short port,SocketEventListener* p_listener);
	void SetSocketEventListener(SocketEventListener *p_listener);
	void SetIpPort(const char* peer, unsigned short port);
	inline const char* GetIP()		{return m_ip.c_str();	}
	inline unsigned short GetPort()	{return m_port;			}
	void SetBindIP(const char* bind_ip);
	const char* GetBindIP(void){return m_bind_ip.c_str();}
};


