#pragma once
#include "afxcmn.h"
#include "Resource.h"
#include "FTConnectionStatus.h"
#define WM_NUM_FT_MODULE			WM_USER+3
#define WM_FT_SOCKETS_STATUS		WM_USER+4
#define WM_REMOVE_FT_MODULE			WM_USER+5
#define WM_FILE_REQUEST				WM_USER+6
#define WM_LISTENING_SOCKETS_STATUS	WM_USER+7

// CPioletPoisonerDlg dialog

class PioletPoisoner;
class CPioletPoisonerDlg : public CDialog
{
	DECLARE_DYNAMIC(CPioletPoisonerDlg)

public:
	CPioletPoisonerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPioletPoisonerDlg();
	void InitParent(PioletPoisoner* parent);
	void Log(const char *text,COLORREF color,bool bold,bool italic);


// Dialog Data
	enum { IDD = IDD_PIOLET_POISONER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	LRESULT OnNumFTModule(WPARAM wparam, LPARAM lparam);
	LRESULT OnFTSocketsStatus(WPARAM wparam, LPARAM lparam);
	LRESULT OnRemoveFTModule(WPARAM wparam, LPARAM lparam);
	LRESULT OnReceivedFileRequest(WPARAM wparam, LPARAM lparam);
	LRESULT OnListeningSocketsStatus(WPARAM wparam, LPARAM lparam);

	DECLARE_MESSAGE_MAP()
private:
	CRichEditCtrl m_log_edit;
	PioletPoisoner* p_parent;
	bool m_newline;
	vector<FTConnectionStatus> v_ft_connection_status;

private:
	void InitRunningSinceWindowText();

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
};
