#pragma once
#include "..\tkcom\threadedobject.h"
#include "..\tkcom\Vector.h"
#include "AresSNUDPHost.h"
#include <afxmt.h>
#include "..\tkcom\Timer.h"
#include "..\tkcom\Buffer2000.h"

class SupernodeUDPSystem : public ThreadedObject
{
	class WaitingQuery : public Object
	{
	public:
		AresSNUDPHost *mp_host;
		UINT m_fake_query_id;
		UINT m_real_query_id;
		CTime m_create_time;
		WaitingQuery(AresSNUDPHost *host,UINT fake_query_id,UINT real_query_id){
			m_fake_query_id=fake_query_id;
			m_real_query_id=real_query_id;
			mp_host=host;
			mp_host->ref();
			m_create_time=CTime::GetCurrentTime();
		}
		~WaitingQuery(){
			mp_host->deref();
		}
	};
private:
	Vector mv_udp_connections[NUMCONBINS];
	CAsyncSocket m_rcv_socket;
	
	UINT m_forward_index1;
	UINT m_forward_index2;

	UINT m_counter;

	void RespondQuery(UINT query_id,AresSNUDPHost* host);
	void SendMessage0x20(AresSNUDPHost* host);
	Vector mv_queued_jobs;
	Vector mv_waiting_queries;
	CCriticalSection m_query_lock;

	//byte m_captured_eighty[0x80];
	//byte m_captured_0x22[28];
	byte m_captured_0x1E[10];
	//bool mb_captured_eighty;
	//bool mb_captured_0x22;
	bool mb_captured_0x1e;
	CCriticalSection m_lock;
	CCriticalSection m_processor_job_lock;

	UINT m_alive_host_count;
	UINT m_alive_host_count_outbound;

	byte m_spoof_hashes[NUM_SPOOF_HASHES][20];  //precalculated set of hashes to spoof
	int m_num_results[NUM_SPOOF_HASHES];  //precalculated number of results for each hash
	int m_total_results;


public:
	SupernodeUDPSystem(void);
	~SupernodeUDPSystem(void);

	UINT Run(void);
	void Update(void);

	void StartSystem(void);
	void StopSystem(void);
	bool IsConnectedToSupernode(const char* host_ip, Vector& v_tmp);
	void AddHost(const char* host_ip, unsigned short port, bool saveable);
	void InitHost(AresSNUDPHost* host);
	void ReceivedQuery(byte* data, UINT length,AresSNUDPHost* host);
	void SendMessage0x21(byte* twenty, AresSNUDPHost* host);
	UINT GetAliveHostCount(void);
	//void SendMessage0x20(AresSNUDPHost* host);
	void SendMessage0x22(byte* twenty,AresSNUDPHost* host);
	void DoQuery(void);
	void ForwardQuery(AresSNUDPHost* host,byte* data, UINT length);
	void SendQueryHit(AresSNUDPHost* host,UINT query_id, const char* query_str);
	void SendMessage0x1E(AresSNUDPHost* host);
	void ForwardQueryHit(AresSNUDPHost* host, UINT real_query_id, byte* data, UINT len);
	UINT GetAliveHostCountOutbound(void);
};
