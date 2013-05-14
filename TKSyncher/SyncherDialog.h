#pragma once
#include "resource.h"
#include "afxwin.h"
#include "SyncherService.h"
#include "..\tkcom\vector.h"
#include "..\NameServer\NameServerInterface.h"
#include "afxcmn.h"
#include "LoadBalancerDialog.h"
#include "LoadBalanceSystem.h"
// CSyncherDialog dialog

class CSyncherDialog : public CDialog
{
	class FileGroup : public Object{
	public:
		int m_ihighest_version;
		CString m_name;
		CString m_highest_version;
		bool mb_running;

		FileGroup(){
			m_ihighest_version=0;
			mb_running=false;
		}
	};

private:
	DECLARE_DYNAMIC(CSyncherDialog)

	//hack to dispatch messages on the main gui thread
	class DispatchJob : public Object{
	private:
		UINT m_dest;
		byte *m_data;
	public:
		DispatchJob(UINT dest,byte *data,UINT data_length){
			m_dest=dest;
			m_data=new byte[data_length];
			memcpy(m_data,data,data_length);
		}
		~DispatchJob(){
			if(m_data) delete[] m_data;
		}
		void Handle();
	};

private:
	Vector mv_dispatch_jobs;
	SyncherService *p_service;
	LoadBalancerDialog m_load_balance_dialog;

public:
	CSyncherDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSyncherDialog();

// Dialog Data
	enum { IDD = IDD_SYNCHERDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	
	CStatic m_source_name;
	CListBox m_source_list;
	afx_msg void NewSource();
	afx_msg void RemoveSource();
	afx_msg void SourceListSelectionChanged();
	void SetService(SyncherService* service);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void RefreshSources();
	afx_msg void ResynchAll();
	afx_msg void RescanSources();
	void ApplyChanges();
	CComboBox m_source_destinations;
//	afx_msg void OnCbnEditchangeSourcedestination();
	afx_msg void OnCbnDropdownSourcedestination();
	afx_msg void OnCbnSelchangeSourcedestination();
	afx_msg void OnCbnEditchangeSourcedestination();
	afx_msg void OnTimer(UINT nIDEvent);
	void DispatchMessage(UINT dest, byte* message, UINT message_length);
	void Dispatch(UINT dest, byte* data, UINT data_length);
	CTreeCtrl m_status_tree;
	afx_msg void OnRefreshStatus();
	afx_msg void OnPurgeSourceFromIPRange();
	afx_msg void OnShowLoadBalancer();
	void RunLatestProgramsInExecutables(void);
	void EnumerateFilesInDirectory(const char* directory,vector <string> &v_files);
};

namespace syncherspace{
	extern CSyncherDialog g_syncher_dialog;
}
