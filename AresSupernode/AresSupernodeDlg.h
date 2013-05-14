#pragma once
#include "resource.h"
#include "afxcmn.h"
#include "afxwin.h"

// CAresSupernodeDlg dialog

class CAresSupernodeDlg : public CDialog
{
	DECLARE_DYNAMIC(CAresSupernodeDlg)

public:
	CAresSupernodeDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAresSupernodeDlg();

// Dialog Data
	enum { IDD = IDD_ARESSUPERNODEDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	CRichEditCtrl m_connection_log;
	CRichEditCtrl m_query_log;
	CStatic m_total_connections;
	CStatic m_fully_connected;
	CStatic m_known_hosts;
protected:
	void UpdateGUI(void);
	void UpdateLogs(void);
public:
	CStatic m_loaded_decoys;
	CStatic m_udp_hosts;
	CStatic m_loaded_user_names;
	afx_msg void OnTestQuery();
};
