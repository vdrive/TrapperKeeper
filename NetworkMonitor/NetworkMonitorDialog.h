#pragma once
#include "afxwin.h"
#include "resource.h"
#include "MainWindow.h"
#include "afxcmn.h"


// CNetworkMonitorDialog dialog

class CNetworkMonitorDialog : public CDialog
{
	DECLARE_DYNAMIC(CNetworkMonitorDialog)

public:
	CNetworkMonitorDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNetworkMonitorDialog();

// Dialog Data
	enum { IDD = IDD_NETWORKMONITORDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	NetworkSystem *mp_network_system;
	CStatic m_alive;
	CStatic m_dead;
	CStatic m_crashed;
	CStatic m_alive_percent;
	CStatic m_not_crashed_percent;

public:
	CStatic m_main_window_frame;
	CMainWindow m_main_window;
	virtual BOOL OnInitDialog();
	void SetNetworkSystem(NetworkSystem* ns);
	CTreeCtrl m_process_tree;
	CTreeCtrl m_dll_tree;
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	void AdjustWindowPositions(void);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	CListBox m_selected_rack_dlls;

	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnAcceleratorDownKey();
	afx_msg void OnUpdateAcceleratorKeyDown(CCmdUI *pCmdUI);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnGlobalRestartDead();
	afx_msg void OnSelectedRestartCrashed();
	afx_msg void OnSelectedRestartDead();
	afx_msg void OnSelectedRestartAll();
	afx_msg void OnSelectedRemoteCrashed();
	afx_msg void OnSelectedRemoteDead();
};
