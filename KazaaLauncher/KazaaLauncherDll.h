#pragma once
#include "..\SamplePlugIn\dll.h"
#include "KazaaLauncherDlg.h"
#include "HttpSocket.h"
#include "NetStatP.h"
#include "NetInfo.h"
#include "KazaaProcess.h"
#include "KLComInterface.h"
#include "../SupernodeCollector/KazaaControllerHeader.h"
#include "../SupernodeCollector/IPAndPort.h"
#include "UpdatedIPAndPort.h"
#include "KazaaStatusInfo.h"
//#include "ProcessesDirectories.h"
#include "KazaaControllerDestSyncher.h"
#include "RegistryMonitorThreadData.h"
#include <afxmt.h>	// for CCriticalSection
#include "GetRealProcessIDThreadData.h"

class KazaaLauncherDll :
	public Dll
{
public:
	KazaaLauncherDll(void);
	~KazaaLauncherDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	void ReportStatus(HttpSocket* socket, char* ip,int port,int error,NetInfo& net_info,char *username=NULL,char *supernode_ip=NULL,char *supernode_port=NULL);
	void GetAllKazaaCurrentSupernodes();
	void EnumAllKazaaProcesses();
	void DataReceived(char *source_name, void *data, int data_length);
	bool SendGenericMessage(char* dest, KazaaControllerHeader::op_code op_code);
	bool SendSupernodesRequest(char* dest, int num_request);
	bool SendAllCurrentSupernodes(char* dest);
	void LaunchKazaa();
	bool KillKazaa(DWORD process_id);
	void KillAllKazaaProcesses();
	void StopLaunchingKazaa();
	void ResumeLaunchingKazaa();
	void NewNameListArrived(const char* source);
	 //overriding the base Dll function to receive data from my other dlls
	bool ReceivedDllData(AppID from_app_id, void* input_data, void* output_data);
	void MinimizeAllKazaaWindows(void);
	void GotThreadDoneMessage();
	void Log(const char* log);
	void RestoreAllKazaaWindows();



private:
	KazaaControllerDestSyncher m_syncher;
	CKazaaLauncherDlg m_dlg;
	
	HttpSocket m_sockets[100];
	NetStatP m_netstatp; //netstatp use to query the tcp table
	vector<NetInfo> v_kazaa_listening_tcp_ports; //stores all listening TCP port its' PID of kazaa from netstatp;
	vector<KazaaProcess> v_kazaa_processes; //current kazaa's PID and supernodes
	//vector<KazaaProcess> v_temp_kazaa_processes; //temporary kazaa's PID and supernodes from netstatp
	KLComInterface* p_com_interface; //Com Interface
	string m_controller_source; //source name
	int m_kazaa_instances; //max kazaas allowed
	string m_kazaa_path;
	bool m_request_supernodes_again;
	bool m_stop_kazaa;
//	int m_desktop_num;
	vector<CString> v_usernames;
	vector<KazaaStatusInfo> v_available_info;
	//ProcessesDirectories m_processes_directories;
	int m_next_port;
	bool m_disable_minimizing_kazaa_windows;
	bool m_getting_real_id;
	GetRealProcessIDThreadData m_get_real_process_id_thread_data;
	CTime m_thread_begins_at,m_thread_ends_at;
	int m_restart_desktop_control_threashold;
	

	void UpdateKazaaListeningPort();
	int GetIntIPFromStringIP(string ip);
	bool SendCheckNewSupernode(vector<UpdatedIPAndPort>& ips);
	bool SendRemoveSupernode(vector<IPAndPort>& ips);
	bool SendRequestNewSupernode(vector<IPAndPort> & ips);
	bool SendCheckNewSupernode(UpdatedIPAndPort& ip);
	bool SendRemoveSupernode(IPAndPort& ip);
	bool SendRequestNewSupernode(IPAndPort & ip);
	void GetKazaaPathFromRegistry();
	void EditRegistry(int ip, int port);
	void RandomlyModifyUsername(HKEY hkcu);
	void ChangeUsername(char * username);
	void ReadInUsernames();
	void CheckNumberOfSupernodes();
//	bool CheckIfSupernodesDiedOrJumped();
	string GetIPStringFromIPInt(int ip_int);
	string GetFreeRunInDir();
	void KillAllIdleSockets(void);
	long GetProcessRunningTime(DWORD process_id);
	void DeleteDatabaseDirOption();
	void ChangeDefaultShareFolder();
	void ResetAllKazaaProcessesAliveFlag();
	void RemoveAllGoneProcesses();
	void GetRealProcessID(KazaaProcess& process);
	void ChangeDataBaseDirOptionInRegistry(string run_in);
	void ChangeApplicationNameInRegistry(string run_in_dir);
	void ChangeExeDirInRegistry(string run_in_dir);
	void ChangeApplicationNameInCurrentUserRegistry(string run_in);


public:

	void OnTimer(int nIDEvent);
	bool m_ending_thread;
	HWND m_dlg_hwnd;
	
private:
	void SaveMaxKazaaNumber(void);
	void ReadMaxKazaaNumber(void);
//	void KillUpgradeMessageBox(void);
	// add a share folder for kazaa sharing
	void AddShareFolder(const char* share_folder);
	void ReceivedSharedFolders(vector<CString>& folders);
	//bool IsShareAlready(CString folder);
	void RemoveOldSharedFoldersFromRegistry();
	//void CopyKazaaFolder(CString from, CString to);
	void StartMonitoringRegistry(CString database_dir);
	void StopMonitoringRegistryKey();


	CCriticalSection m_critical_section;
	RegistryMonitorThreadData m_thread_data;
	
	CWinThread* m_msg_box_killing_thread;
	CWinThread* m_registry_monitoring_thread;
	CWinThread* m_kazaa_real_process_id_thread;

	int GetFreePort(void);
	vector<CString> v_directory_cached; //last used directories for kazaa

	void ChangeKazaaPortInRegistry(int port);
	void DisablePort1214(void);
	void ChangeIgnoreAllMessageOption(void);
	void ChangeMaxUploadOption(void);
	void ChangeSupernodeOption(void);
	void ChangeKppForceSupernodeOption(void);
	void ChangeKppHDDScanOption(void);
	void ChangeNoUploadLimitWhenIdleOption(void);
	void SendLocalDllMessage(AppID app_id, byte* data, UINT data_len, KazaaControllerHeader::op_code op_code, bool& ret);
	int GetLeastKazaaDesktopNum();
	void RestartDesktopControl();
	bool IsFileWritable(CString file);
	void KillProcess(const char* process_name);
	void KillAllMyDocumentaWindows();

};

