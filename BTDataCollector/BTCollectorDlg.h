#pragma once

#include "resource.h"
#include "ClientData.h"

// BTCollectorDlg dialog
class BTDataDll;
class BTCollectorDlg : public CDialog
{
	DECLARE_DYNAMIC(BTCollectorDlg)

public:
	BTCollectorDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~BTCollectorDlg();

// Dialog Data
	enum { IDD = IDD_COLLECTOR_DLG };
	
	void OnTimer(UINT nIDEvent);

	void InitParent(BTDataDll * parent);

	CListCtrl m_list_control;
	CListCtrl m_sent_list_control;

	void DisplayData(vector<ClientData> v_data, vector<double> v_times);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	BTDataDll * p_parent;
public:
	afx_msg void OnLvnItemchangedSentList(NMHDR *pNMHDR, LRESULT *pResult);
};
