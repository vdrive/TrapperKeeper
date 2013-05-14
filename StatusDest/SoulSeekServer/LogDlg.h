#pragma once
#include "Resource.h"
#include "afxcmn.h"
#include <string>
#include <vector>
#include "afxwin.h"
using namespace std;

#define MAX_LOG_SIZE	2000

struct LogEntry
{
	string time;
	int level;
	int object;
	string msg;
	LogEntry(string thetime,int thelevel,int theobject,string themsg)
	{
		time = thetime;
		level = thelevel;
		object = theobject;
		msg = themsg;
	}
};

// LogDlg dialog

class CLogDlg : public CDialog
{
	DECLARE_DYNAMIC(CLogDlg)

public:
	CLogDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CLogDlg();

	void WriteToLog(int warningLevel, int object, LPCTSTR strToWrite);

// Dialog Data
	enum { IDD = IDD_LOG_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	void AppendString(LPCTSTR tempString, CRichEditCtrl *rEBox, int red, int green, int blue, bool bold);

	CRichEditCtrl m_richLog;

	DECLARE_MESSAGE_MAP()
private:
	void WriteEntry(int i);
	bool isPaused;

	string objectNames[5];
	vector<LogEntry> logEntry;
public:
	CComboBox m_filter_object;
	CComboBox m_filter_message_level;
	CButton m_refresh;
	CButton m_pause;
	afx_msg void OnBnClickedPause();
	void RefreshLog();
	afx_msg void OnCbnSelchangeObject();
	afx_msg void OnCbnSelchangeWarningLevel();
	afx_msg void OnBnClickedRefresh();
};
