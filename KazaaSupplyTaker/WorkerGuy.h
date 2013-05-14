#if !defined(AFX_WORKERGUY_H__07EFBB70_1169_4169_A949_508D8655EB4F__INCLUDED_)
#define AFX_WORKERGUY_H__07EFBB70_1169_4169_A949_508D8655EB4F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// WorkerGuy.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// WorkerGuy window
#include "SearchResult.h"
#include "..\DCMaster\ProjectKeywords.h"

//#define WM_PROCESS_RAW_DATA				WM_USER+2
#define WM_RAW_DUMP						WM_USER+4
#define WM_FIND_MORE_DUMP				WM_USER+5

class KazaaManager;

class WorkerGuy : public CWnd
{
// Construction
public:
	WorkerGuy();

	void Trigger();
	void InitParent(KazaaManager *parent);
	void ResetWatchdog();
	void Reset();
	void FindWindows();
	void Type(HWND edit,char c);
	void ProcessDatFiles();
	void ExtractSearchItems(HWND search);
	void ExtractSearchDialogItems(HWND left_dlg);
	void SetId(int id);
	void CloseKaZaA();
	void Clear();
	void NewSearch();
	void TypeSearchString(char *search_string);
	void TypeArtistSearchString(char *search_string);
	void SearchMore();
	void SearchNow();
	void WriteOutPreviouslyFoundHosts();
	void ReadInPreviouslyFoundHosts();
	void FindMoreFromSameUser(HTREEITEM item);

	void Click(HWND hwnd);

	void PressAudioRadioButton();
	void PressVideoRadioButton();
	void PressSoftwareRadioButton();

	void PressMoreSearchOptionsStatic();
	void PressMoreSearchOptionsArtistButton();
	bool PressSearchBackStatic();

	void FreeMemory(void *ptr);
	void *AllocateMemory(unsigned int size);

	unsigned int ReadMemory(void *ptr,char *buf,unsigned buf_len);
	unsigned int WriteMemory(void *ptr,char *buf,unsigned buf_len);

	bool IsConnected();
	bool AreWindowsFound();
	bool IsSearchMoreEnabled();
	bool IsSearchNowEnabled();
	bool IsSearching();

	vector<string> ExtractColumns();
	vector<SearchResult> ExtractSearchResults();
	vector<unsigned int> v_find_more_hosts;

	ProjectKeywords m_current_project;

    DWORD m_process_id;
	HANDLE m_process_handle;

	HWND m_search_header_hwnd;
	HWND m_search_tree_hwnd;
	HWND m_search_now_button_hwnd;
	HWND m_search_more_button_hwnd;
	HWND m_search_edit_hwnd;
	HWND m_search_video_radio_button_hwnd;
	HWND m_search_audio_radio_button_hwnd;
	HWND m_search_software_radio_button_hwnd;
	HWND m_search_status_static_hwnd;

	HWND m_more_search_options_static_hwnd;
	HWND m_more_search_options_artist_button_hwnd;
	HWND m_more_search_options_artist_edit_hwnd;
	HWND m_search_back_static_hwnd;

	CWnd *p_kazaa;	// The (oh so important) Kazaa window

	KazaaManager *p_parent;

	HWND m_hwnd;	// for the timers
	HWND m_kza_hwnd;

	unsigned int m_state;
	unsigned int m_search_more_count;
	unsigned int m_no_connect;
	unsigned int m_week_number;

	unsigned int m_lower_ip;
	unsigned int m_upper_ip;

	CTime m_start_date;
	CTime m_last_state_change;

	bool m_first_hit;
	bool m_last_project;

	int m_id;
	CString m_download_dir;

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(WorkerGuy)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~WorkerGuy();

	// Generated message map functions
protected:
	//{{AFX_MSG(WorkerGuy)
//	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WORKERGUY_H__07EFBB70_1169_4169_A949_508D8655EB4F__INCLUDED_)
