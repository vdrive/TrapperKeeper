#pragma once
#include "Dll.h"
#include "MainDlg.h"
#include "HttpSocket.h"
#include "IPAddress.h"
#include "SuperNodeCollector.h"
#include "RackSuperNodes.h"
#include "SDComInterface.h"
#include "KazaaControllerHeader.h"

class SupernodeDistributerDll :	public Dll
{
public:
	SupernodeDistributerDll(void);
	~SupernodeDistributerDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	void ReportStatus(HttpSocket* socket, char* ip,int port,int error,char *username=NULL,char *supernode_ip=NULL,char *supernode_port=NULL);
	void AddSuperNode(char* ip, char* port);
	void TotallyRemoveSuperNode(char* source, int ip, int port);
	UINT GetSuperNodeListSize();
	void ProbeNextItem();
	void LoadSupernodeList(void);
	void SaveSupernodeList(void);
	void ImportSuperNodeList(void);
	void ReconnectKazaa(void);
	void OnMaintenanceTimer(void);
	void KillAllIdleSockets(void);
	void SaveUserNames(void);
	void DataReceived(char *source_name, void *data, int data_length);
	//void OnEditKazaaNumber(char* rack, int num);
	vector<IPAndPort> GetSupernodesFromRackList(string& rack_name, UINT& kazaa_running);
	void OnTimer(int nIDEvent);
	void OnStopAndKillKazaa(vector<CString>& selected_racks);
	void OnResumeKazaa(vector<CString>& selected_racks);
	void OnRestartKazaa(vector<CString>& selected_racks);
	void OnGetRemoteSupernodes(vector<CString>& selected_racks);

private:
	void AddNodeToRackList(char* rack_name);
	bool SendSupernodesToRack(char* dest, UINT num_request);
//	bool SendKazaaNum(char* dest);
	bool SendControllerGenericMessage(char* dest, KazaaControllerHeader::op_code op_code);
	void ReceivedRemoteSupernodeList(char* source_name, vector<IPAndPort>& supernodes, UINT kazaa_running);
	bool ReceivedRequestForNewSupernode(char* source_name, IPAndPort& old_ip, IPAndPort& current_ip);
	bool GetSupernode(char* source_name, IPAndPort& old_ip, IPAndPort& new_ip);
	bool SendRenewSupernode(char* dest, IPAndPort& old_ip, IPAndPort& new_ip);
	bool ReceivedCheckNewSupernode(char* source_name, IPAndPort& old_ip_port, IPAndPort& new_ip_port);
	void RemoveAllSupernodesAssociatedWithRack(char* rack_name);
//	void SaveRacksMaxKazaaNumber();
//	void ReadRacksMaxKazaaNumber();
	void IsKazaaRunning();
	void GetKazaaPathFromRegistry();
	void ReceivedIPs(vector<string>& ips);
	int GetIntIPFromStringIP(string ip);
	string GetIPStringFromIPInt(int ip_int);
	bool SendAllSupernodesToRack(char* dest);
	void CalculatePingOffset();


	CMainDlg* p_dlg;
	HttpSocket m_sockets[100];
	vector<IPAddress> v_supernode_list;
	vector<IPAddress> v_temp_supernode_list;
	vector<RackSuperNodes> v_rack_list;
	SuperNodeCollector m_supernode_collector;
	SDComInterface* p_com_interface;
	string m_kazaa_path;
	UINT m_second_count;

public:
	UINT m_probing_index;
	UINT m_connections;
	
	void SendSharedFoldersToAllRacks(vector<CString>folders);
private:
	void ResetAllSupernodes(void);
};
