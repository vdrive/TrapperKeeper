#pragma once
#include "Resource.h"
#include "VirtualListCtrlFT.h"

// CGnutellaFileTransferDlg dialog

class CGnutellaDecoyerDlg;
class CGnutellaFileTransferDlg : public CDialog
{
	DECLARE_DYNAMIC(CGnutellaFileTransferDlg)

public:
	CGnutellaFileTransferDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGnutellaFileTransferDlg();
	void InitParent(CGnutellaDecoyerDlg* parent);
	void ReportStatus(vector<NoiseModuleThreadStatusData> &status);
	string FormatWithCommas(unsigned int num);

	unsigned int m_num_good_connections;
	unsigned int m_num_bogus_connections;
	unsigned int m_total_num_received_connections;

// Dialog Data
	enum { IDD = IDD_GNUTELLA_FT_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	CGnutellaDecoyerDlg* p_parent;
	VirtualListCtrlFT m_list;

public:
	afx_msg void OnBnClickedDisableList();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
};
