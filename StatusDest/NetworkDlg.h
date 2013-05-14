#pragma once
#include "afxcmn.h"
#include "Resource.h"
#include "..\StatusSource\StatusData.h"
#include "EditDlg.h"
#include "..\StatusSource\SystemInfoData.h"
#include "SharedFilesDlg.h"
#include "ExportRackInfoDlg.h"
#include "RemoveDllEditDlg.h"

#define SUB_RACK_IP						0
#define SUB_COMPUTER_NAME				1
#define SUB_MAC_ADDRESS					2
#define SUB_RACK_VERSION				3
#define SUB_RACK_PROC					4
#define SUB_RACK_BANDWIDTH				5
#define SUB_RACK_AVG_BANDWIDTH			6
#define SUB_RACK_UPLOAD_BANDWIDTH		7
#define SUB_RACK_DOWNLOAD_BANDWIDTH		8
#define SUB_RACK_KAZAA_COUNT			9
#define SUB_RACK_FILES_SHARED			10
#define SUB_RACK_MOVIES_SHARED			11
#define SUB_RACK_MUSIC_SHARED			12
#define SUB_RACK_SWARMER_SHARED			13
#define SUB_RACK_TK_MEMORY				14
#define SUB_RACK_COMMITTED_MEMORY		15
#define SUB_RACK_TOTAL_MEMORY			16
#define SUB_RACK_KAZAA_UPLOADS			17
#define SUB_RACK_KAZAA_MEMS				18
#define SUB_RACK_OS						19
#define SUB_RACK_CPU_INFO				20
#define SUB_RACK_HARDDRIVE_SPACE_LEFT	21
#define SUB_RACK_TOTAL_HARDDRIVE_SPACE	22


#define WM_STOP_KAZAA				WM_USER_MENU+1
#define WM_RESUME_KAZAA				WM_USER_MENU+2
#define WM_RESTART_KAZAA			WM_USER_MENU+3
#define WM_CHANGE_MAX_KAZAA			WM_USER_MENU+4
#define WM_RESTART_COMPUTER			WM_USER_MENU+5
#define WM_REMOTE_CONTROL			WM_USER_MENU+6
#define WM_REQUEST_KAZAA_UPLOADS	WM_USER_MENU+7
#define WM_VIEW_SHARED_FILES		WM_USER_MENU+8
#define WM_EMERGENCY_RESTART		WM_USER_MENU+9
#define WM_EXPORT_INFO				WM_USER_MENU+10
#define WM_REMOVE_DLL				WM_USER_MENU+11


// CNetworkDlg dialog
class StatusDestDll;
class CNetworkDlg : public CDialog
{
	DECLARE_DYNAMIC(CNetworkDlg)

public:
	CNetworkDlg(CWnd* pParent = NULL);   // standard constructor
	void InitParent(StatusDestDll* parent);
	virtual ~CNetworkDlg();

// Dialog Data
	enum { IDD = IDD_NETWORK_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_ip_list;
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
private:

	StatusDestDll* p_parent;
	CEditDlg m_edit_dlg;
	CRemoveDllEditDlg m_remove_dll_edit_dlg;
	vector<CSharedFilesDlg*> v_shared_files_dlgs;
	CExportRackInfoDlg* p_export_dlg;


	void CheckComputerStatus();
	void OnStopAndKillKazaa();
	void OnResumeKazaa();
	void OnRestartKazaa();
	void OnChangeMaxKazaa();
	void OnRestartComputer();
	void OnRemoteControl(void);
	void OnRequestKazaaUploads();
	void OnViewSharedFiles(void);
	void OnEmergencyRestart();
	void OnShowExportRackInfoDlg();
	void OnRemoveDll();
	LRESULT OnDoneChangingMaxKazaa(WPARAM wparam,LPARAM lparam);
	LRESULT OnQuitSharedFilesDlg(WPARAM wparam, LPARAM lparam);
	LRESULT OnDoneRemoveDllEdit(WPARAM wparam,LPARAM lparam);



public:
	void ReceivedSharedFilesReply(char* from, FileInfos& reply);
	void UpdateKazaaUploadsStatus(char* from, int uploads);
	void UpdateSystemInfoStatus(char* from, SystemInfoData* sys_info);
	void UpdateIPList(vector<string>& ips);
	void UpdateStatus(CString ip, StatusData* status, double avg_bandwidth);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnNMRclickIpList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnColumnclickIpList(NMHDR *pNMHDR, LRESULT *pResult);
	void KillExportDlg(void);
	void OnExportRackInfo();
	afx_msg void OnNMDblclkIpList(NMHDR *pNMHDR, LRESULT *pResult);
};
