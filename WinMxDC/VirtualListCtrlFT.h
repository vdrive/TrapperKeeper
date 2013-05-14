#pragma once

/////////////////////////////////////////////////////////////////////////////
// VirtualListCtrlFT window

#define SUB_REMOTE_IP	0
#define SUB_MODULE		1
#define SUB_SOCKET		2
#define SUB_FILENAME	3
#define SUB_RANGE		4
#define SUB_TOTAL_SIZE	5
#define SUB_SENT		6
#define SUB_ELAPSED		7

#include "VirtualListControlItemFT.h"
#include "NoiseModuleThreadStatusData.h"

class VirtualListCtrlFT : public CListCtrl
{
// Construction
public:
	VirtualListCtrlFT();

	unsigned int GetItemCount();

	void DeleteAllItems();

	vector<VirtualListControlItemFT> v_data;

	void ReportStatus(vector<NoiseModuleThreadStatusData> &status);

	string FormatSize(unsigned int size);
	string FormatElapsed(CTimeSpan &span);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VirtualListCtrlFT)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~VirtualListCtrlFT();

	// Generated message map functions
protected:
	//{{AFX_MSG(VirtualListCtrlFT)
	afx_msg void OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

