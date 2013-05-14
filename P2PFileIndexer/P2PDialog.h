#pragma once
#include "resource.h"
#include "dll.h"
#include "P2PCom.h"

// P2PDialog dialog

class P2PDialog : public CDialog
{
	DECLARE_DYNAMIC(P2PDialog)

public:
	P2PDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~P2PDialog();

// Dialog Data
	enum { IDD = IDD_P2PDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();

private:
	vector <string> mv_client_ips;
	int m_ip_index;
	Dll *mp_dll;
	P2PCom m_com;

public:
	void SetDLL(Dll* p_dll);
	void UpdateIPs(void);
};
