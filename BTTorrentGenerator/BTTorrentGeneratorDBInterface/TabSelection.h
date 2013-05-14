#pragma once


// CTabSelection

#define CTABSELECT_TAB_COUNT	5

//
//
//
class CTabSelection : public CTabCtrl
{
	DECLARE_DYNAMIC(CTabSelection)

public:
	CTabSelection();
	virtual ~CTabSelection();

public:
	//Function to Create the dialog boxes during startup
	void InitDialogs();

	//Function to activate the tab dialog boxes 
	void ActivateTabDialogs();

protected:
	afx_msg void OnSelchange(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

protected:
	int m_anDialogID[CTABSELECT_TAB_COUNT];		// Array to hold the list of dialog boxes/tab pages for CTabCtrl
	CDialog *m_apDialog[CTABSELECT_TAB_COUNT];	// CDialog Array Variable to hold the dialogs 
};


