#pragma once
#include "..\tkcom\object.h"
#include "..\tkcom\Buffer2000.h"
#include "..\tkcom\HashObject.h"
#include "..\AresProtector\AresPacket.h"
#include "..\AresProtector\AresHost.h"
#include "..\tkcom\LinkedList.h"
#include "..\tkcom\PeerSocket.h"
#include "..\tkcom\Vector.h"
#include "..\tkcom\Timer.h"
#include "AresDCProjectInfo.h"

class AresDCConnection : public SocketEventListener, public Object
{

private:

	Vector mv_host;
	bool mb_clean;

	Vector mv_processor_job;
	//CTime m_last_file_upload;

	unsigned short int m_fss;	// four-shit-short
	unsigned char m_fsb;		// four-shit-byte

	unsigned int m_network_users;
	unsigned int m_network_files;
	unsigned int m_network_MBs;

	UINT m_int_ip;

	UINT m_id;
	Vector mv_hosts;

	bool mb_searched;
	bool mb_connecting;
	bool mb_has_connected;
	bool mb_connected;
	bool mb_marked_for_death;
	bool mb_shares_sent;
	bool mb_name_sent;
	bool mb_first_ping_sent;

	CTime m_death_time;
	CTime m_create_time;
	Timer m_last_receive;
	Timer m_last_ping_sent;

	Timer m_last_search;
	PeerSocket *mp_ps_con;

	unsigned int m_remote_ip;
	string m_str_ip;
	unsigned short m_my_port;
	unsigned short m_port;
	string m_status;

	unsigned int m_state;

	Buffer2000 m_receive_buffer;

	Vector mv_search_project;


	CCriticalSection m_search_lock;

	bool mb_dead;

	//a vector of events
	LinkedList m_event_list;
	int m_num_shares;

	void ProcessPacket(AresPacket& packet);
	void ExtractHosts(AresPacket& packet);

	char m_user_name[12];

	void Init(const char *host_ip,unsigned short port);	

	bool mb_0x38_client;

protected:


public:
	//AresDCConnection(const char* host_ip,int port,unsigned short my_port);
	AresDCConnection(AresHost* host,unsigned short my_port);
	~AresDCConnection(void);

	void OnClose(CAsyncSocket* src);
	void OnAccept(CAsyncSocket *server_socket,CAsyncSocket* new_connection);
	void OnSend(CAsyncSocket* src);
	void OnReceive(CAsyncSocket* src,byte *data,UINT length);
	void OnConnectFailed(CAsyncSocket* src);
	void OnConnect(CAsyncSocket* src);


	inline const char* GetStatus(){return m_status.c_str();}
	inline const char* GetIP(){return m_str_ip.c_str();}
	inline unsigned short GetPort(){return m_port;}
	inline CTimeSpan GetAliveTime(){return CTime::GetCurrentTime()-m_create_time;}

	inline bool IsClean(){return mb_clean;}

	void Update(void);
	void Connect(void);
	bool IsConnected(void);
	void Kill(void);
	CString GetFormattedAgeString(void);
	
	LinkedList* GetEventList(void);
	void SendMessage0x00(byte* remote_guid);

	inline bool IsDead(void){
		return mb_dead;
	}

	inline bool IsDying(void){
		return mb_marked_for_death;
	}

	inline unsigned int GetNetworkUsers(){
		return m_network_users;
	}
	inline unsigned int GetNetworkFiles(){
		return m_network_files;
	}
	inline unsigned int GetNetworkMBs(){
		return m_network_MBs;
	}

	inline UINT GetID(void)
	{
		return m_id;
	}

	void CleanUp(void);
	void SendShares(void);
	void SendPing(void);
	void SendName(void);
	void RecordReceiveEvent(AresPacket* packet);
	void RecordSendEvent(AresPacket* packet);
	void PurgeEvents(void);
	bool IsSharingFiles(void);
private:
	int SendShares(Vector& v_files);
public:
	void Search(const char* project,vector <string> &keywords);
	void ProcessSearchResults(AresPacket* packet);
	void SearchProject(AresDCProjectInfo* project);
	void Begin0x38Response(byte* remote_guid);
	bool HaveSharesBeenSent(void);
};
