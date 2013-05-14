#pragma once
// VirtualProjectStatusListCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// VirtualProjectStatusListCtrl window

#include "VirtualProjectStatusListCtrlItem.h"
#include "ProjectStatus.h"
//#include "DBInterface.h"

#define SUB_PS_PROJECT				0
//#define SUB_PS_WRITTEN				1
#define SUB_PS_QUERY_HIT			1
#define SUB_PS_DECOY				2
#define SUB_PS_EFFICIENCY			3
#define SUB_PS_TRACK_QUERY_HIT		4

class VirtualProjectStatusListCtrl : public CListCtrl
{
// Construction
public:
	VirtualProjectStatusListCtrl();

	vector<VirtualProjectStatusListCtrlItem> v_data;

	int GetItemCount();
	void DeleteAllItems();

	bool IncrementCounters(ProjectStatus &status);
	void ClearProjectStatus(string &project);
	void SaveReport();
	//void WriteFakeSuppliesToDB();

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

private:
	//DBInterface m_db_interface;
};