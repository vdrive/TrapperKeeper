#include "resource.h"
#include "afxwin.h"
#include "Timer.h"
#include "afxcmn.h"
#include "MessageDispatcher.h"
#pragma once


// CComDialog dialog
//AUTHOR:  Ty Heath
//Last Modified:  4.7.2003
//Purpose:  This class is the GUI for the Com service.  It records and displays general com statistics

class CComDialog : public CDialog
{
	DECLARE_DYNAMIC(CComDialog)

private:
	//A class that encapsulates data used to derive statitics.  It has a timestamp for age.  Actual statistics are created based on a Vector of AppDatas
	class AppData : public Object{  //Derive from object for memory management and Vector storage
		UINT m_op_code;
		string m_dest;

		//2^64 should be big enough
		UINT64 m_uploaded;  
		UINT64 m_downloaded;
		UINT m_upload_bandwidth;
		UINT m_download_bandwidth;
	public:
		AppData(UINT op_code,const char *dest){
			m_op_code=op_code;
			if(dest)
				m_dest=dest;
			Clear();
		}
		inline UINT GetApp(){return m_op_code;}
		inline const char* GetDest(){return m_dest.c_str();}
		void AddUpload(UINT amount,bool bandwidth){
			m_uploaded+=amount;
			if(bandwidth)
				m_upload_bandwidth+=amount;
		}
		void AddDownload(UINT amount,bool bandwidth){
			m_downloaded+=amount;
			if(bandwidth)
				m_download_bandwidth+=amount;
		}
		void Clear(){
			m_uploaded=0;
			m_downloaded=0;
			m_upload_bandwidth=0;
			m_download_bandwidth=0;
		}
		inline bool IsApp(UINT app){return (m_op_code==app)?true:false;}
		inline bool IsDest(const char* dest){return (stricmp(m_dest.c_str(),dest)==0)?true:false;}
		inline float GetUploaded(){
			double temp=(double)m_uploaded;
			temp/=(double)(1<<20); //conver to MB
			return (float)temp;
		}
		inline float GetDownloaded(){
			double temp=(double)m_downloaded;
			temp/=(double)(1<<20); //conver to MB
			return (float)temp;
		}
		inline float GetUploadBandwidth()
		{
			double temp=(double)m_upload_bandwidth;
			temp/=(double)(1<<10); //conver to KB
			temp/=10.0; //convert to KB/s
			return (float)temp;
		}
		inline float GetDownloadBandwidth()
		{
			double temp=(double)m_download_bandwidth;
			temp/=(double)(1<<10); //conver to KB
			temp/=10.0; //convert to KB/s
			return (float)temp;
		}

		int CompareTo(Object* object)  //for sorting by ip
		{
			AppData *co=(AppData*)object;

			int uPeer[4];
			sscanf ( co->m_dest.c_str(), "%d.%d.%d.%d", &uPeer[0], &uPeer[1], &uPeer[2], &uPeer[3] ) ;
			DWORD uPeerInt=(uPeer[0]<<24)|(uPeer[1]<<16)|(uPeer[2]<<8)|(uPeer[3]);

			int uHost[4];
			sscanf ( m_dest.c_str(), "%d.%d.%d.%d", &uHost[0], &uHost[1], &uHost[2], &uHost[3] ) ;
			DWORD uHostInt=(uHost[0]<<24)|(uHost[1]<<16)|(uHost[2]<<8)|(uHost[3]);

			if(uPeerInt>uHostInt)
				return -1;
			else if(uPeerInt<uHostInt)
				return 1;

			return 0;  //all the fields are equal heh
		}
	};

	//a class that encapsulates a message from com saying that x data was sent by y app to z dest
	class StatData : public Object{
	private:
		friend class CComDialog;
		Timer time;
		UINT m_op_code;
		string m_source;
		UINT m_length;
		StatData(UINT op_code,const char* source,UINT length){
			m_op_code=op_code;
			if(source)
				m_source=source;
			m_length=length;
		}
	};
	
	//vector for stats from com about received data
	Vector mv_received_stats;

	//vector for stats from co mabout sent data
	Vector mv_sent_stats;

	//vector for display of app statistics
	Vector mv_app_data;

	//vector for display of connection statistics
	Vector mv_active_connections;

	//dispatcher which lamely pulses based off of WM_TIMER events to sent notifications using the main GUI thread to other plugins.
	MessageDispatcher *p_dispatcher;


	UINT m_int_total_connections;
public:
	CComDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CComDialog();

// Dialog Data
	enum { IDD = IDD_COM_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	CStatic m_current_download_bandwidth;  //static text control
	CStatic m_current_upload_bandwidth;  //static text control
	CStatic m_total_download;  //static text control
	CStatic m_total_upload;  //static text control

	CStatic m_send_counter;
	CStatic m_receive_counter;
	CStatic m_send_counter_actual;

	UINT m_int_send_counter;
	UINT m_int_send_counter_actual;
	UINT m_int_receive_counter;

	DECLARE_MESSAGE_MAP()
public:
	void SetTotalTCPConnections(UINT num_connections);
	CStatic m_total_tcp_sockets;
	void PostSend(UINT op_code, const char* source, UINT data_length);
	void PostReceive(UINT op_code, const char* source, UINT data_length);
	afx_msg void OnTimer(UINT nIDEvent);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	
	CListCtrl m_application_stats;
	CListCtrl m_active_connections;
	void SetDispatcher(MessageDispatcher* dispatcher);

private:  
	bool mb_enable_logging;
	//called internally to add another stat to the connection vector
	void AddConnectionData(CComDialog::StatData* data,bool b_download);
	//called internally to add another stat to the app vector
	void AddAppData(StatData* data,bool b_download);

	//called internally to update the app display
	void CheckStats(void);

	//called internally to update the connection display
	void CheckActives(void);
public:
	void CheckExpired(void);
	afx_msg void OnEnableStatistics();
	CButton m_enable_statistics;
	void EncodeTrapperOn(void);

	
	void PostSendRequest(UINT op_code, const char* source, UINT data_length);
};

namespace comspace{
	extern CComDialog g_com_dialog;
}