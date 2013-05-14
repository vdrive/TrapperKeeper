#pragma once
#include "..\tkcom\object.h"
#include "..\tkcom\vector.h"
#include "..\tkcom\Timer.h"
#include "..\tkcom\ThreadedObject.h"
#include "Buffer2000.h"
#include "SearchJob.h"
#include <afxmt.h>

//#define LOGIPS
//This class encapsulates a connection to a server.
//This class also encapsulates an state based automaton.
class DonkeyServer :	public ThreadedObject
{
private:
	//state definitions
	//S_INIT = In the process of initing
	//S_INIT_RESPONSE = Waiting for a response from the server
	//S_READY = Ready to do a search
	//S_SEARCH = We are in the process of doing a search
	//S_SEARCH_RESPONSE = We are in the process of waiting for a search response
	//S_HASH_STROBE = We are in the process of probing hash sources from the server
	//S_HASH_STROBE_RESPONSE = We are in the process of waiting for a result from the server
	//S_PAUSE = basically a pause state for debugging
	enum SERVERSTATE {S_INIT=1,S_INIT_RESPONSE,S_READY_PAUSE,S_READY,S_SEARCH,S_SEARCH_RESPONSE,S_HASH_STROBE,S_HASH_STROBE_RESPONSE,S_HASH_STROBE_PAUSE,S_PAUSE};

	string m_ip;
	unsigned short m_port;
	UINT m_tcp_handle;  //a connection reference handle for use with MetaTCP
	UINT m_tcp_firewall_check; //a connection reference to the connection we will receive to check if we have a firewall going.
	Buffer2000 m_firewall_buffer; //data from the firewall connection
	Buffer2000 m_tcp_buffer_pre; //data from the normal connection
	Buffer2000 m_tcp_buffer_post; //data from the normal connection
	byte m_my_hash[16];  //a hash representing this client, so we report the same one each time

	Vector mv_search_tasks;
	Vector mv_current_search_results;

	int m_search_count;  //number of times it has asked for more search results;
	int m_hash_probe;  //which hash we are currently probing for sources
	int m_retry;  //times we have tried to do whatever it is we are trying to do.
	

	CTime m_connected;
	SERVERSTATE m_state;  //current state of the machine
	Timer m_state_time;  //time we have been in the current state
	SearchJob *mp_current_search_job;

	CCriticalSection m_firewall_buffer_lock;
	CCriticalSection m_tcp_buffer_lock;
	Timer m_infinite_loop_timer;  //a check
	int m_connect_retry;
	bool b_new_normal_data;
	bool b_new_firewall_data;

public:
	DonkeyServer(const char* ip,unsigned short port);
	~DonkeyServer(void);
	bool IsServer(const char* ip);
	
	void NewConnection(UINT handle);
	void NewData(UINT handle, Buffer2000 &data);

	void Reset(bool b_change_state=true);
	void Search(SearchJob *new_search);
	void LostConnection(UINT handle);
	void Update(void);
	UINT Run();

private:
	void ProcessMainBuffer(void);
	void ProcessFirewallBuffer(void);
	void ProcessSearchResults(void);
	void QueryHash(void);
	void Init(void);
	void SubmitSearch(void);
	void ChangeState(SERVERSTATE new_state);
	bool HasFullResponse(byte code);
public:
	
	inline const char* GetIP() { return m_ip.c_str(); }
	inline unsigned short GetPort() { return m_port; }
	inline UINT GetSearchTaskCount() { return mv_search_tasks.Size(); }

	void ProcessHashQueryResponse(void);
	void ClearUnwantedPackets(void);
private:
	void AdvanceSearch(void);
public:
	CTime ConnectionTime(void);
	bool IsThreadLockedUp(void);
	
};
