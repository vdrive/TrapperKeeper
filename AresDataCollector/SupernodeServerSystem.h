#pragma once
#include "..\tkcom\ThreadedObject.h"
#include "..\tkcom\Vector.h"
#include "..\tkcom\Timer.h"

class SupernodeServerSystem : public ThreadedObject
{
private:


	class MDServer : public ThreadedObject
	{
	public:
		UINT m_ip;
		Timer m_discovery_time;
		MDServer(UINT ip){
			m_ip=ip;
		}
	};

	class AresSupernode : public Object
	{
	public:
		UINT m_ip;
		unsigned short m_port;
		Timer m_discovery_time;
		bool mb_farmed;
		bool mb_bad;
		AresSupernode(UINT ip,unsigned short port){
			mb_bad=false;
			m_ip=ip;
			m_port=port;
			mb_farmed=false;
		}
		AresSupernode(const char* ip,unsigned short port){
			mb_bad=false;
			m_ip=inet_addr(ip);
			m_port=port;
			mb_farmed=false;
		}
	};

	class SupernodeSaver : public ThreadedObject
	{
		
	public:
		Vector *mpv_supernodes;

		SupernodeSaver(Vector* v){
			mpv_supernodes=v;
		}
		
		~SupernodeSaver(){
			mpv_supernodes->Clear();
			delete mpv_supernodes;
		}

		UINT Run(void);
	};

	CAsyncSocket m_rcv_socket;
	CAsyncSocket m_send_socket;

	inline int GetBinFromPeer(const char* peer)
	{
		UINT ip=inet_addr(peer);
		return (int)(ip%NUMCONBINS);
	}


	inline int GetBinFromPeer(UINT ip)
	{
		return (int)(ip%NUMCONBINS);
	}

	Vector mv_supernodes[NUMCONBINS];
	Vector mv_md_servers[NUMCONBINS];


	UINT m_forward_index1,m_forward_index2;

	UINT m_md_server_count;
	UINT m_loaded_supernode_count;
	UINT m_bad_supernode_count;
	Vector mv_supernode_savers;
	//SupernodeSaver* mp_supernode_saver;

	void AddSupernodes(vector<UINT>&v_ips,vector<unsigned short>v_ports,const char* src_ip);
	void SaveSupernodes(void);
	void ReportNewSupernodes(vector<UINT> &v_ips,vector<unsigned short> &v_ports);
	void RespondToGetSupernodes(const char* src_ip);

public:
	
	SupernodeServerSystem(void);
	~SupernodeServerSystem(void);
	void StartSystem(void);
	void StopSystem(void);
	UINT Run(void);

	UINT GetMDServerCount(void);
	UINT GetLoadedSupernodeCount(void);
	
};
