// GnutellaSocket.h
#pragma once

#include "TEventSocket.h"
#include "GnutellaHeader.h"
#include "GnutellaHost.h"
//#include "GnutellaHostConnectionStatus.h"
#include "ProjectSupplyQuery.h"
#include "GnutellaSocketStatusData.h"
//#include "SupplyEntry.h"
#include "FalseQueryHit.h"

class ConnectionSockets;
class SupplyProject;

class GnutellaSocket : public TEventSocket
{
public:
	GnutellaSocket();
	~GnutellaSocket();
	void InitParent(ConnectionSockets *sockets);

	int Connect(GnutellaHost &host);

	void OnConnect(int error_code);
	void OnReceive(int error_code);
	void OnClose(int error_code);

//	GnutellaHostConnectionStatus ReturnHostConnectionStatus();

	void TimerHasFired();

	unsigned int ReturnHostIP();

	void PerformProjectSupplyQuery(ProjectSupplyQuery &psq);

	void ProcessQueryHit(char *data);

	bool IsConnected();

	void KeywordsUpdated();

	unsigned int UpTime();

	unsigned int ReturnCachedQueryHitCount();

	unsigned int ReturnQueryCount();

	string ReturnRemoteVendor();

protected:
	void SocketDataSent(unsigned int len);
	void SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len);
	void SocketDataReceived(char *data,unsigned int len);

	int Close();

private:
	// Private Data Members
	ConnectionSockets *p_sockets;

	GnutellaHeader m_hdr;
	bool m_received_gnutella_connection_string;
	bool m_received_gnutella_header;

	bool m_qrp_table_sent;

//	GnutellaHostConnectionStatus m_host_connection_status;

	GnutellaSocketStatusData m_status_data;

	bool m_connecting_to_limewire_reflector;

	bool m_on_receive_cached;

	vector<char *> v_cached_query_hits_to_send;

	unsigned int m_query_count;	// to determine whether or not i am a high-traffic socket to send searcher queries through
	vector<unsigned int> v_query_counts;

	// Private Member Functions
	void ExtractHostsFromHttpHeader(char *data);
	void ResetData();
	
	void ReceiveGnutellaHeader();
	void ReceivedGnutellaHeader();
	void ReceivedGnutellaData(char *data);

	void SendPing(unsigned char ttl);
	void SendPong(unsigned char ttl);
	void ProcessPong(char *data);

	void SendQRPTable();

	void ProcessQuery(char *query);

	void ExtractKeywordsFromQuery(vector<string> *keywords,char *query);
	void ClearNonAlphaNumeric(char *buf,int buf_len);

	int CheckQueryHitRequiredKeywords(ProjectSupplyQuery *psq,char *lfilename,unsigned int file_size);

	void RespondToQuery(ProjectKeywords *keywords);

	unsigned int BitScramble(unsigned int val);

	bool IsStringValid(char *ptr);
	CString ExtractFirstWord(const char* query);


	unsigned int m_state;

	CTime m_time_last_ping_sent;
};