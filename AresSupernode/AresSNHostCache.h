#pragma once
#include "..\tkcom\threadedobject.h"
#include "..\tkcom\vector.h"
#include "..\AresProtector\AresHost.h"
#include "..\AresDataCollector\SupernodeServerSystemInterface.h"
#include <afxmt.h>

class AresSNHostCache : public ThreadedObject
{



	class SaveHostObject : public Object
	{
	public:
		string m_ip;
		unsigned short m_port;
		SaveHostObject(const char *host_ip,unsigned short port){
			m_ip=host_ip;
			m_port=port;
		}
	};

	class SNInterface : public SupernodeServerSystemInterface
	{
	public:
		Vector mv_saved_hosts;
		void ReceivedSupernodes(vector<string> &v_ips, vector<unsigned short> &v_ports);
	};
protected:
	/*
	class AresSN : public Object
	{
	public:
		string m_ip;
		unsigned short m_port;

		AresSN(const char* ip,unsigned short port){
			m_ip=ip;
			m_port=port;
		}
	};*/

	inline int GetBinFromPeer(const char* peer)
	{
		UINT ip=inet_addr(peer);
		return (int)(ip%NUMCONBINS);
	}


	UINT m_num_hosts;

	Vector mv_hosts_array[NUMCONBINS];
	Vector mv_md_hosts;
	Vector mv_user_names;
	CCriticalSection m_lock;
	CCriticalSection m_md_host_lock;
	UINT m_index1;
	UINT m_index2;
	UINT m_user_index;
	SNInterface m_interface;

	Vector mv_hosts_to_save;

	Vector mv_reconnect_hosts;
public:
	AresSNHostCache(void);
	~AresSNHostCache(void);
protected:
	UINT Run(void);
public:
	bool GetNextHost(Vector &v,bool tcp_ip);
	UINT GetKnownHosts(void);
	bool IsReady(void);
	void SaveHost(const char* host_ip, unsigned short port);
	void GetSomeUserNames(int amount, Vector& v_tmp);
	UINT GetLoadedUserNames(void);
	bool GetRandomMDHost(Vector &v_tmp);
	void StartSystem(void);
	void StopSystem(void);
	void AttemptReconnection(const char* ip, unsigned short port);
};
