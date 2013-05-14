#pragma once
#include "object.h"
#include "buffer2000.h"
#include "vector.h"
#include "Timer.h"
#include "DataBuffer.h"

#define TKRELIABLEPORT 17239
#define TKUNRELIABLEPORT 17238

class ComConnection : public Object
{
private:
	UINT m_con_handle;
	string m_peer;
	//TKTCPConnection m_con_handle;
	Timer m_last_used;
	Buffer2000 m_received_data;
	//string m_peer_ip;
	Vector mv_queued_data;
	bool mb_dirty;
	bool mb_closed;
	bool mb_error;
	bool mb_ready;
	bool mb_sending;
	
public:

	ComConnection(UINT con_handle,const char* peer);
	~ComConnection(void);
	bool NewData(UINT con_handle, byte* data, UINT length);
	bool CheckData(Vector &v_received_objects);

	inline bool IsConnection(UINT con_handle) {	return m_con_handle==con_handle; }
	inline UINT GetHandle(void)	{ return m_con_handle; }
	inline const char* GetPeer(void)	{ return m_peer.c_str(); }
	inline bool HasError(void)	{ return mb_error; }
	inline bool HasQueuedData(void)	{ if(mv_queued_data.Size()>0) return true; else return false; }
	inline bool IsTooBusy(void)	{ if(mv_queued_data.Size()>2) return true; else return false; }
	inline bool IsReady(void) { return mb_ready; }
	inline bool IsSending(void) { return mb_sending; }
	inline bool IsClosed(void) { return mb_closed; }

	bool IsPeer(UINT con_id);
	bool IsPeer(const char* peer);
	void OnSend(void);
	void Closed(void);

	bool IsExpired(void);
	void QueueData(DataBuffer* db);


	DataBuffer* GetQueuedData(void);
	void PopQueuedData(void);
	void Ready(void);
	void StartSending(void);
};
