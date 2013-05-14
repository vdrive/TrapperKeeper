#pragma once
// VirtualProjectStatusListCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// VirtualProjectStatusListCtrl window

#include "VirtualProjectStatusListCtrlItem.h"
#include "ProjectStatus.h"

#define SUB_PS_PROJECT				0
#define SUB_PS_SPOOFS_DISTS_POISONS	1
#define SUB_PS_QUERY				2
#define SUB_PS_TRACK_QUERY			3
#define SUB_PS_QUERY_HIT			4
#define SUB_PS_TRACK_QUERY_HIT		5

class VirtualProjectStatusListCtrl : public CListCtrl
{
// Construction
public:
	VirtualProjectStatusListCtrl();

	vector<VirtualProjectStatusListCtrlItem> v_data;

	int GetItemCount();
	void DeleteAllItems();

	void IncrementCounters(ProjectStatus &status);
	void ClearProjectStatus(string &project);

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(VirtualProjectStatusListCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~VirtualProjectStatusListCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(VirtualProjectStatusListCtrl)
	afx_msg void OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};