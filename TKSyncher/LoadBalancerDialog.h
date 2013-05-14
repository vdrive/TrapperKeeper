#pragma once
#include "resource.h"
#include "afxcmn.h"
#include "afxwin.h"
#include "LoadBalanceSystem.h"

// LoadBalancerDialog dialog

class LoadBalancerDialog : public CDialog
{
	DECLARE_DYNAMIC(LoadBalancerDialog)

public:
	LoadBalancerDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~LoadBalancerDialog();

// Dialog Data
	enum { IDD = IDD_LOADBALANCERDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	CTreeCtrl m_project_info;
	CTreeCtrl m_rack_info;
	CListBox m_unassigned_projects;
public:
	

	afx_msg void OnRefreshData();
	virtual BOOL OnInitDialog();
	// text control that displays the desired number of files on each rack
	CStatic m_desired_rack_load;
	// text control that displays the actual number of files on each rack
	CStatic m_actual_rack_load;
	LoadBalanceSystem *mp_load_balancer;
	void SetLoadBalancePointer(LoadBalanceSystem* p_system);
	
};
