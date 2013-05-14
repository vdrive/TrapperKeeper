#pragma once
#include "..\sampleplugin\dll.h"
#include "SupernodeCollectorDlg.h"
#include "MyComInterface.h"
#include "HttpSocket.h"
#include "KazaaSuperNodeCollector.h"
#include "KazaaControllerHeader.h"

class SupernodeCollectorDll :
	public Dll
{
public:
	SupernodeCollectorDll(void);
	~SupernodeCollectorDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	void ReportStatus(HttpSocket* socket, char* ip,int port,int error,char *username=NULL,char *supernode_ip=NULL,char *supernode_port=NULL);
	void OnTimer(UINT nIDEvent);
	void ProbeNextItem();
	void DataReceived(char *source_name, void *data, int data_length);
	void ResetSupernodes(void);

private:
	void IsKazaaRunning();
	void GetKazaaPathFromRegistry();
	void ImportSuperNodeList();
	void ReconnectKazaa();
	void KillAllIdleSockets();
	void AddSuperNode(char* supernode_ip, char* supernode_port);
	void SaveSupernodeList();
	void ReadSupernodeList();
	bool SendControllerGenericMessage(char* dest, KazaaControllerHeader::op_code op_code);
	bool SendWholeSupernodesList(char* dest);
	bool SendNewSupernodesList(char* dest);



private:
	CSupernodeCollectorDlg m_dlg;
	MyComInterface m_com_interface;
	HttpSocket m_sockets[100];
	string m_kazaa_path;
	vector<IPAddress> v_supernode_list;
	vector<IPAddress> v_temp_supernode_list;
	vector<IPAddress> v_new_supernode_list;
	KazaaSuperNodeCollector m_supernode_collector;

public:
	//UINT m_probing_index;
//	UINT m_connections;


};
