#pragma once
#include "resource.h"
#include "afxcmn.h"
#include "afxwin.h"
#include <pdh.h>
#include "..\tkcom\vector.h"

// CAresDialog dialog

class CAresDialog : public CDialog
{
	DECLARE_DYNAMIC(CAresDialog)
	/*
	class ByteSentEvent : public Object{
		CTime m_time;
		UINT m_amount;
		ByteSentEvent(UINT amount){
			m_time=CTime::GetCurrentTime();
			m_amount=amount;
		}
	}*/

private:
	void OnUltrapeerSelectionChange(void);

public:
	CAresDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAresDialog();

// Dialog Data
	enum { IDD = IDD_ARESDIALOG };

protected:
	Vector mv_selected_peer;
	Vector mv_selected_packet;
	HQUERY m_pdh;
	string m_keynames[5];
    HCOUNTER m_pdh_counters[5];
	bool m_performance_counters_initialized;

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
protected:
	void UpdateGUI(void);
public:
	CListCtrl m_ultra_peers;
	CStatic m_known_ultra_peers;
	CStatic m_connected_ultra_peers;
	bool CheckPerformanceCounters(void);
	CStatic m_system_status;
	CStatic m_total_cpu;
	CStatic m_ares_cpu;
	CStatic m_receive_bandwidth;
	CStatic m_send_bandwidth;
	CStatic m_system_memory;
	CStatic m_tk_memory;

	bool InitPerformanceCounters(void);

	void ClosePerformanceCounters(void);
	CString FormatNumberForDisplay(UINT number);
	CListCtrl m_peers;
	CSliderCtrl m_event_time_slider;
	CStatic m_event_time;
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	CButton m_record_events;
	afx_msg void OnBnClickedRecordEvents();
	afx_msg void OnClickUltrapeers(NMHDR *pNMHDR, LRESULT *pResult);
	CListCtrl m_event_list;
	
	afx_msg void OnLvnKeydownUltrapeers(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedPurgedeadconnectionsnow();
	CStatic m_ultra_peers_with_files;
	CStatic m_spoofs_loaded;
	CRichEditCtrl m_hex_view;
	CStatic m_file_transfer_peer_count;
	CStatic m_send_queue_size;
	CStatic m_interdiction_target_count;
	CStatic m_interdiction_transfer_amount;
	CStatic m_interdiction_connection_count;
	CStatic m_interdiction_connection_transfer_count;
	CStatic m_interdiction_banned_client_ratio;
	CStatic m_interdiction_queue_size;
	void UpdateLogs(void);
	CStatic m_hash_cache_efficiency;
};
