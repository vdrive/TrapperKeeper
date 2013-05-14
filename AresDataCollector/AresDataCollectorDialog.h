#pragma once
#include "resource.h"
#include "AresDCLogEntry.h"
#include "afxcmn.h"
#include "afxwin.h"

// CAresDataCollectorDialog dialog

class CAresDataCollectorDialog : public CDialog
{
	DECLARE_DYNAMIC(CAresDataCollectorDialog)

public:
	CAresDataCollectorDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAresDataCollectorDialog();

// Dialog Data
	enum { IDD = IDD_ARESDCDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	void UpdateGUI(void);
	CRichEditCtrl m_log;
	CStatic m_known_hosts;
	afx_msg void OnRefreshProjectInfo();
	void RefreshProjectTree(void);
	CTreeCtrl m_project_tree;
protected:
	CStatic m_live_connections;
public:
	CStatic m_connecting_sockets;
	CStatic m_interdiction_targets;
	CStatic m_loaded_supernodes;
	CStatic m_md_servers;
};
