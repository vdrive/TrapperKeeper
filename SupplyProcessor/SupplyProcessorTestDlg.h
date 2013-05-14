#pragma once
#include "resource.h"

// SupplyProcessorTestDlg dialog
class SupplyProcessorDLL ;

class SupplyProcessorTestDlg : public CDialog
{
	DECLARE_DYNAMIC(SupplyProcessorTestDlg)

public:
	SupplyProcessorTestDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~SupplyProcessorTestDlg();

// Dialog Data
	enum { IDD = IDD_SUPPLYPROCESSORTESTDLG };

	void InitParent(SupplyProcessorDLL *parent);
	void Log(const char *message);
	void Queue(const char *message);
	void ClearQueue();
	void OnTimer(UINT nIDEvent);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	SupplyProcessorDLL * p_dll;
	virtual BOOL OnInitDialog();
	CListCtrl  m_list_ctrl;
	CListCtrl  m_queue_list_ctrl;

	DECLARE_MESSAGE_MAP()


};
