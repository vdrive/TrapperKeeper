#pragma once
#include "resource.h"
#include "afxwin.h"

// KademliaProtectorDialog dialog

class KademliaProtectorDialog : public CDialog
{
	DECLARE_DYNAMIC(KademliaProtectorDialog)

public:
	KademliaProtectorDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~KademliaProtectorDialog();

// Dialog Data
	enum { IDD = IDD_KADEMLIAPROTECTORDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
protected:
	CStatic m_number_contacts;
public:
	CStatic m_number_new_contacts;
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	void UpdateGUI(void);
	CStatic m_number_ft_clients;
	CStatic m_number_swarms;
	CStatic m_contact_load_status;
};
