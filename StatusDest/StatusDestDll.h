#pragma once
#include "..\sampleplugin\dll.h"
#include "StatusDestDlg.h"
#include "..\StatusSource\StatusData.h"
#include "..\StatusSource\MyComInterface.h"
#include "NetworkBandwidth.h"
#include "SourceHost.h"
#include "..\StatusSource\StatusHeader.h"
#include "..\StatusSource\SystemInfoData.h"
//#include <afxmt.h>	// for CCriticalSection
#include "StatusDestThreadData.h"
#include "..\StatusSource\FileInfos.h"

class StatusDestDll : public Dll
{
public:
	StatusDestDll(void);
	~StatusDestDll(void);

	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);

private:
	int m_interval;
	UINT m_second_count;
	CStatusDestDlg m_dlg;
	MyComInterface m_com;
	vector<SourceHost> v_hosts;
	vector<NetworkBandwidth*> v_network_bandwidths;

public:
	void SendStatusRequest(const char* dest);
	void OnTimer(UINT nIDEvent);
	void OpenNetworkDialog(CString network);
	bool IsComputerUp(string network, string ip);
	void OnStopAndKillKazaa(vector<CString>& selected_racks);
	void OnResumeKazaa(vector<CString>& selected_racks);
	void OnRestartKazaa(vector<CString>& selected_racks);
	void OnChangeMaxKazaa(vector<CString>& selected_racks, UINT max_kazaa);
	void OnRemoveDll(vector<CString>& selected_racks, CString filename);
	void OnRemoveDllNetwork(vector<CString>& selected_networks, CString filename);
	void OnRestartComputer(vector<CString>& selected_racks);
	bool SendGenericMessage(CString dest, StatusHeader::op_code op_code);
//	void OnInitStatusDestThreadData(WPARAM wparam,LPARAM lparam);
	void OnUpdateNetworkSummary(WPARAM wparam,LPARAM lparam);
	void OnRequestKazaaUploads(vector<CString>& selected_racks);
	void OnEmergencyRestart(vector<CString>& selected_racks);
	void OnEmergencyRestartNetwork(vector<CString>& selected_networks);
	void RebootAllDownRacks();


private:
	void UpdateNetworkStatus(string& network);
	void ReceivedStatus(char* from, StatusData* status);
	bool LoadINIFile();	//load the network.ini file
	void ReceivedSystemInfoReply(char* source_name, SystemInfoData* sys_info);
	//CCriticalSection m_critical_section;
	void CalculatePingingInterval(void);
	void UpdateNetworkStatus();
	void ReceivedKazaaUploads(char* from, int uploads);
	void ReceivedSharedFilesReply(char* from, FileInfos& reply);
};
