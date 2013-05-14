#pragma once
#include "..\sampleplugin\dll.h"
#include "MainDlg.h"
#include "MyComInterface.h"
#include "KazaaControllerHeader.h"
#include "RackSuperNodes.h"
#include "IPAddress.h"
#include "SupernodeCollector.h"

class SupernodeControllerDll :
	public Dll
{
public:
	SupernodeControllerDll(void);
	~SupernodeControllerDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	void OnTimer(UINT nIDEvent);
	void DataReceived(char *source_name, void *data, int data_length);
	void SendInitMsgToAll();
	vector<IPAndPort> GetSupernodesFromRackList(string& rack_name, UINT& kazaa_running);
	UINT GetMaxHashSetSize();
	void ResetAll();
	void SaveSupernodes();
	//bool IsSorted();
	void ReadAndSendAdditionalSharedFolders();



private:
	bool SendGenericMessage(char* dest, KazaaControllerHeader::op_code op_code);
	void ReceivedKazaaRackIPs(vector<string>& ips);
	void ReceivedSupernodeCollectorIPs(vector<string>& ips);
	void TotallyRemoveSuperNode(char* source, int ip, int port);
	void AddNodeToRackList(char* rack_name);
	bool AddNodeToCollector(char* rack_name);
	bool SendSupernodesToRack(char* dest, UINT num_request);
	bool SendRenewSupernode(char* dest, IPAndPort& old_ip, IPAndPort& new_ip);
	bool GetSupernode(char* source_name, IPAndPort& old_ip, IPAndPort& new_ip, bool delete_old);
	bool ReceivedRequestForNewSupernode(char* source_name, IPAndPort& old_ip, IPAndPort& current_ip, bool delete_old = true);
	bool ReceivedCheckNewSupernode(char* source_name, IPAndPort& old_ip_port, IPAndPort& new_ip_port);
	int GetIntIPFromStringIP(string ip);
	string GetIPStringFromIPInt(int ip_int);
	void ReceivedRemoteSupernodeList(char* source_name, vector<IPAndPort>& supernodes, UINT kazaa_running);
	void ReceivedCollectorSupernodes(vector<IPAddress>& supernodes);
	void SetInitMsgOffsetForKazaaRacks();
	int GetNumUpCollectors();
	void GetNumConsumedSupernodes(int& greater_0,int& launcher_supernodes);
	void SaveUserNames(void);
	bool SendSupernodeIsOk(char* dest, IPAndPort& ip);
	bool IsValidRack(const char* rack);

private:
	MyComInterface m_com_interface;
	CMainDlg m_dlg;
	vector<RackSuperNodes> v_rack_list; //storing all the supernodes assigned to each rack
	vector<SupernodeCollector> v_collectors; //all supernode collectors status
	UINT m_second_count;

//	vector<IPAddress> v_main_supernode_list; //storing the main supernode list, sorted by IP
//	vector<IPAddress> v_main_supernode_list_by_up_time; //storing the supernode list, sorted by their up time

	hash_set<IPAddress,IPAddressHash> hs_main_supernode_list; //storing the main supernode list, hashed by ip
	hash_set<IPAddress>::iterator p_available_supernode;

public:
	UINT m_max_same_supernode_assigned;
//	UINT m_supernode_distributed;
};
