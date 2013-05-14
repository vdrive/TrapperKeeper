#pragma once
#include "Resource.h"

// CExportRackInfoDlg dialog

class CNetworkDlg;
class CExportRackInfoDlg : public CDialog
{
	DECLARE_DYNAMIC(CExportRackInfoDlg)

public:
	CExportRackInfoDlg(CWnd* pParent = NULL);   // standard constructor
	void InitParent(CNetworkDlg* parent);
	virtual ~CExportRackInfoDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:

	BOOL m_mac_address_checked;
	BOOL m_ip_checked;
	BOOL m_cpu_checked;
	BOOL m_memory_checked;
	BOOL m_hdd_checked;
	BOOL m_os_checked;
	CNetworkDlg* p_parent;

	afx_msg void OnBnClickedExportButton();
	afx_msg void OnBnClickedCancelButton();
	virtual BOOL OnInitDialog();
	BOOL m_computer_name;
	BOOL m_network;
};
