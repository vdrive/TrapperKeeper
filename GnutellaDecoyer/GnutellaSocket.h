// GnutellaSocket.h
#pragma once
#include <afxmt.h>	// for CCriticalSection
#include "TEventSocket.h"
#include "GnutellaHeader.h"
#include "GnutellaHost.h"
//#include "GnutellaHostConnectionStatus.h"
#include "GnutellaSocketStatusData.h"
#include "zlib\zlib.h"
#define PACKET_BUFF	 32768
//#define ZSTREAM_BUFF 16384
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
	bool IsConnected();
	void KeywordsUpdated();
	unsigned int UpTime();
	unsigned int ReturnCachedQueryHitCount();

	//unsigned int ReturnQueryCount();

	string ReturnRemoteVendor();
	int ReturnCompressionOnCounts();


protected:
	void SocketDataSent(unsigned int len);
	void SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len);
	void SocketDataReceived(char *data,unsigned int len);

	int Close();

private:
	// Private Data Members
	ConnectionSockets *p_sockets;
	CCriticalSection m_critical_section;

	GnutellaHeader m_hdr;
	bool m_received_gnutella_connection_string;
	bool m_received_gnutella_header;

	bool m_qrp_table_sent;

//	GnutellaHostConnectionStatus m_host_connection_status;

	GnutellaSocketStatusData m_status_data;

	//bool m_connecting_to_limewire_reflector;

	bool m_on_receive_cached;

	vector<char *> v_cached_query_hits_to_send;

	//unsigned int m_query_count;	// to determine whether or not i am a high-traffic socket to send searcher queries through
	//vector<unsigned int> v_query_counts;

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
	void ExtractKeywordsFromQuery(vector<string> *keywords,const char *query);
	void ClearNonAlphaNumeric(char *buf,int buf_len);
	unsigned int BitScramble(unsigned int val);
	bool IsStringValid(char *ptr);
	//int Spoof(ProjectKeywords *keywords,SupplyProject supply_project,ProjectStatus *status, int track, Query& qu, bool swarming_enabled);
	//void Spoof15Files(FalseQueryHit &hit,bool cache);
	//void FindFirstAndLastSpoofEntryIndex(vector<SupplyEntry>& spoof_entries, int track , UINT& first_spoof_entry_index,UINT&  last_spoof_entry_index);
	int ReceivedDeflatedGnutellaHeader();
	CString ExtractFirstWord(const char* query);
	void RemoveArtistNames(vector<string>& keywords, string& artist_name);
	void SendSpoofs(FalseQueryHit &hit);

	unsigned int m_state;
	CTime m_time_last_ping_sent;
	CString m_handshake;
	UINT m_my_ip_int;
	
	// Compression
	//bool m_dnapressionOn;
	//bool m_InflateRecv;
	//bool m_DeflateSend;

	//z_stream InflateStream;
	//z_stream DeflateStream;

	CString m_my_ip;
};