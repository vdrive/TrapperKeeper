#pragma once
#include "..\tkcom\threadedobject.h"
#include "AresUDPSupernode.h"
#include "SearchJob.h"
#include "..\tkcom\Timer.h"

class UDPSearcher : public ThreadedObject
{
protected:
	inline int GetBinFromPeer(const char* peer)
	{
		UINT ip=inet_addr(peer);
		return (int)(ip%NUMCONBINS);
	}

	Vector mv_udp_connections[NUMCONBINS];
	CAsyncSocket m_rcv_socket;
	
	Vector mv_search_jobs;


	UINT m_forward_index1;
	UINT m_forward_index2;

	UINT m_counter;

	void RespondQuery(UINT query_id,AresUDPSupernode* host);
	void SendMessage0x20(AresUDPSupernode* host);
	void SendMessage0x21(byte* twenty, AresUDPSupernode* host);
	void SendMessage0x22(byte* twenty,AresUDPSupernode* host);
	void SendMessage0x1E(AresUDPSupernode* host);
	
	void InitHost(AresUDPSupernode* host);
	bool IsConnectedToSupernode(const char* host_ip, Vector& v_tmp);

	Vector mv_queued_jobs;
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

//	byte m_spoof_hashes[NUM_SPOOF_HASHES][20];  //precalculated set of hashes to spoof
//	int m_num_results[NUM_SPOOF_HASHES];  //precalculated number of results for each hash
//	int m_total_results;

public:
	void AddHost(const char* host_ip, unsigned short port, bool saveable);
	UDPSearcher(void);
	~UDPSearcher(void);
	UINT Run(void);
	void RunSearch(SearchJob* job);
	void Update(void);
	void StartSystem(void);
	void StopSystem(void);
	UINT GetAliveHostCount(void);
	void ProcessSearchResult(byte* data, UINT len);
};
