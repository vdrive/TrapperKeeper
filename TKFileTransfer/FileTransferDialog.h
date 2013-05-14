//AUTHOR:  Ty Heath
//Last Modified:  4.8.2003

#pragma once
#include "..\TKCom\Vector.h"
#include "afxwin.h"
#include "resource.h"
#include "Timer.h"
#include "ComLink.h"
#include "FileTransferJob.h"
#include "Dll.h"
#include "FileCacher.h"
#include "P2PInfoDialog.h"

// FileTransferDialog dialog

//For now this is just a plain log of activity.  It posts events at a certain rate to avoid taking up very much cpu
class FileTransferDialog : public CDialog
{
private: 
	//a private utility class that encapsulates a send job
	class SendJob : public Object{
	private:
		ComLink *p_com_link; //a link to the com service
		FileTransferJob *p_job;
	public:
		SendJob(ComLink *link,FileTransferJob *job);  //constructor
		bool Process();  //call to process this job. called within the owners Run() method
		inline bool IsJob(FileTransferJob* job){ if(p_job==job) return true; else return false; }
	};

private:
	Vector mv_send_jobs;  //vector of send jobs
	P2PInfoDialog m_p2p_info_dialog;

	//moved these into the dialog as well.
	FileTransferService m_system;
	ComLink m_link;
	Dll *p_dll;

private:
	//a private stat class for status tracking
	class FileStat : public Object{
	private:
		Timer birthday;
		UINT m_id;
		float m_progress;
		string m_name;
		string m_peer;
		static int sort_mode;
	public:
		FileStat(const char* file_name,const char* peer,UINT id){
			m_peer=peer;
			m_name=file_name;
			m_progress=0;
			m_id=id;
		}

		static SetSortMode(int mode){sort_mode=mode;}

		inline const char* GetFileName(){return m_name.c_str();}
		inline const char* GetPeer(){return m_peer.c_str();}
		inline void SetProgress(float progress){m_progress=progress;}
		inline UINT GetID(){return m_id;}
		inline float GetProgress(){return m_progress;}
		
		inline bool IsReallyOld(){if(birthday.HasTimedOut(6*60*60)){ return true;}else return false;}

		int CompareTo(Object* object)  //for sorting by ip
		{
			FileStat *co=(FileStat*)object;

			if(sort_mode==0){
				int uPeer[4];
				sscanf ( co->m_peer.c_str(), "%d.%d.%d.%d", &uPeer[0], &uPeer[1], &uPeer[2], &uPeer[3] ) ;
				DWORD uPeerInt=(uPeer[0]<<24)|(uPeer[1]<<16)|(uPeer[2]<<8)|(uPeer[3]);

				int uHost[4];
				sscanf ( m_peer.c_str(), "%d.%d.%d.%d", &uHost[0], &uHost[1], &uHost[2], &uHost[3] ) ;
				DWORD uHostInt=(uHost[0]<<24)|(uHost[1]<<16)|(uHost[2]<<8)|(uHost[3]);

				if(uPeerInt>uHostInt)
					return -1;
				else if(uPeerInt<uHostInt)
					return 1;

				return 0;  //all the fields are equal heh
			}
			else if(sort_mode==1){
				//find the last slash
				char* fo1=strchr(m_name.c_str(),'\\');
				char* tmpptr1=fo1;
				while(tmpptr1){
					fo1=tmpptr1;
					tmpptr1=strchr(fo1+1,'\\');
				}

				char* fo2=strchr(co->m_name.c_str(),'\\');
				char* tmpptr2=fo2;
				while(tmpptr2){
					fo2=tmpptr2;
					tmpptr2=strchr(fo2+1,'\\');
				}

				if(fo2==NULL || fo1==NULL)
					return 0;

				return stricmp(fo1,fo2);
			}
			else return 0;
		}
	};
	DECLARE_DYNAMIC(FileTransferDialog)
	
public:
	void SetDll(Dll *dll);

	FileTransferDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~FileTransferDialog();
	//call to post a message 

// Dialog Data
	enum { IDD = IDD_FILETRANSFERDIALOG };

protected:
	//messages to be posted
	//Vector mv_pending_messages;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT nIDEvent);


protected:
	bool mb_valid;
	CScrollBar m_vscroll_bar;
	Vector mv_get_stats;
	Vector mv_get_complete_stats;
	Vector mv_get_failed_stats;
	Vector mv_send_stats;
	Vector mv_send_complete_stats;
	Vector mv_send_failed_stats;
	CCriticalSection m_lock;
	CFont title_font;
	CFont data_font;

	UINT m_cached_file_chunks;
	UINT m_cached_hits;
	float m_cached_hits_per_sec;

	int m_vscroll_pos;

	CBitmap m_back_buffer;
	//CListBox m_log;  //activity log

public: //standard MFC messages
	void WriteToLog(const char* str);
	void AddSendJob(FileTransferJob* job, ComLink* link);
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	void GetFailed(UINT id);
	void GetProgress(UINT id, float progress);
	void NewGet(const char* file_name, const char* peer, UINT id);
	void GetDone(UINT id);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	void NewSend(const char* file_name, const char* peer, UINT id);
	void SendDone(UINT id);
	void SendFailed(UINT id);
	void SendProgress(UINT id, float progress);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	void RepositionScrollBars(void);
	void UpdateScrollBars(void);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void ClearFailures();
	afx_msg void ClearCompleted();
	afx_msg void SortByIP();
	afx_msg void SortByName();
	bool ReceivedAppData(UINT from_app, byte* data);
	void SetCachedFileChunks(UINT chunks);
	void AddCacheHit(void);
	afx_msg void OnShowP2PInfo();
};

//There is and always will be only one FileTransferDialog and thus we make it a global object.  This avoids giving the 4-5 classes that reference it pointers of type FileTransferDialog.
namespace filetransferspace{
	extern FileTransferDialog g_file_transfer_dialog;
	extern FileCacher g_file_cacher;
}
