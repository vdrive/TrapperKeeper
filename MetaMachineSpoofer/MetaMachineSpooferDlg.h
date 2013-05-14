#pragma once
#include "resource.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "..\tkcom\vector.h"
#include "..\tkcom\buffer2000.h"

// CMetaMachineSpooferDlg dialog

class CMetaMachineSpooferDlg : public CDialog
{
	DECLARE_DYNAMIC(CMetaMachineSpooferDlg)
	class LogMsg : public Object{
	private:
		CString m_msg;
	public:
		LogMsg(const char *msg){
			m_msg=msg;
		}
	
		inline CString GetMsg(){return m_msg;}
	};
public:
	CMetaMachineSpooferDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMetaMachineSpooferDlg();

// Dialog Data
	enum { IDD = IDD_METASPOOFERDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDestroy();

private:
	int m_connection_count;
public:
	void GotConnection(void);
private:
	CStatic m_control_connections_per_hour;
	CListBox m_connected_servers;
	CListBox m_down_servers;
	Vector mv_needed_logs;
	Vector mv_needed_data_logs;
public:
	void Log(const char* source_ip,const char* data);
	CRichEditCtrl m_log;
	CRichEditCtrl m_data_log;
	void LogData(const char* source_ip, Buffer2000* buffer);
	CStatic m_client_connection_count;
	CStatic m_active_interdiction_connection_count;
	CStatic m_inactive_interdiction_connection_count;
};
