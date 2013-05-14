#pragma once
#include "..\tkcom\object.h"
#include "..\tkcom\Buffer2000.h"
#include "..\tkcom\HashObject.h"
#include "AresPacket.h"
#include "AresHost.h"
#include "..\tkcom\LinkedList.h"
#include "..\tkcom\Vector.h"
#include "..\tkcom\Timer.h"

class AresConnection : public Object
{

private:
	bool mb_clean;

	Vector mv_files_shared;
	Vector mv_processor_job;
	//Vector mv_shares_to_upload;

	CTime m_last_file_upload;

	unsigned short int m_fss;	// four-shit-short
	unsigned char m_fsb;		// four-shit-byte

	unsigned int m_network_users;
	unsigned int m_network_files;
	unsigned int m_network_MBs;

	UINT m_id;
	Vector mv_hosts;

	bool mb_connecting;
	bool mb_has_connected;
	bool mb_connected;
	bool mb_marked_for_death;
	bool mb_shares_sent;
	bool mb_name_sent;
	bool mb_0x38_client;
	bool mb_first_ping_sent;
	CTime m_death_time;
	CTime m_create_time;
	Timer m_last_receive;
	Timer m_last_ping_sent;
	UINT m_con_handle;
	UINT m_original_con_handle; //test debug
	Timer m_last_purge;
	//PeerSocket *mp_ps_con;

	unsigned int m_remote_ip;
	string m_str_ip;
	unsigned short m_my_port;
	unsigned short m_port;
	string m_status;

	unsigned int m_state;

	Buffer2000 m_receive_buffer;

	bool mb_dead;

	//a vector of events
	LinkedList m_event_list;
	int m_num_shares;

	void ProcessPacket(AresPacket& packet);
	void ExtractHosts(AresPacket& packet);

	char m_user_name[12];

	void Init(const char *host_ip,unsigned short port);	

	void Begin0x38Response(byte* remote_guid);

protected:


public:
	AresConnection(const char* host_ip,int port,unsigned short my_port);
	AresConnection(AresHost* host,unsigned short my_port);
	~AresConnection(void);

	void OnClose();
	void OnSend();
	void OnReceive(byte *data,UINT length);
	void OnConnectFailed();
	void OnConnect();


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
	
	bool IsSharingFiles(void);
private:
	void PurgeEvents(void);
	int SendShares(Vector& v_files);
public:
	void GetSharedFiles(Vector& v);

	bool IsConnection(UINT handle)
	{
		return m_con_handle==handle;
	}
protected:
	void SendData(byte* data, UINT length);
public:
	UINT GetConHandle(void);
};
