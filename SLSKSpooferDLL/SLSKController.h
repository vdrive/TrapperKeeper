

#pragma once
//#include "SLSKaccountInfo.h"
#include "buffer.h"
#include "db.h"
#include "ConnectionManager.h"
//#include "SLSKSpooferDlg.h"
#include "ConnectionModuleStatusData.h"
#include "NoiseManager.h"
#include "NoiseModuleThreadData.h"
//#include "SLSKController.h"
#include "SLSKtask.h"
//#include "SLSKaccountInfo.h"
#include "ProjectKeywordsVector.h"
//#include "TrackInformation.h"

class CSLSKSpooferDlg;
class SLSKController
{
private:
	//SLSKaccountInfo account_info;
	
	bool cp;
	bool m_server_started;
	db DB;
	ConnectionManager m_connection_manager;
	buffer buf;
	NoiseManager noise_manager;
	vector<SLSKtask> server_tasks;
	vector<SLSKtask> peer_tasks;
public:
	SLSKController(void);
	~SLSKController(void);
	void start(void);
	void stop(void);
	void initAccountAll(void);
	void initAccountInfo(void);
	void InitDialog(CSLSKSpooferDlg *pDlg);
	void resetAccountInfo(void);
	void connectServer(void);
	void connectParents(void);
	void connectNewParents(void);
	void OnTimer(UINT nIDEvent);
	void Log(const char* log);
	void setServerStatus(const char* status);
	void setParentStatus(const char* status);
	void setPeerStatus(const char* status);
	void setServerModStatus(const char* status);
	void setParentModStatus(const char* status);
	void setPeerModStatus(const char* status);
	void setParentConnectingStatus(const char* status);
	void setPeerConnectingStatus(const char* status);
	void AddServerConnection(void);
	void UpdateProjects(void);
	void StartSpoofing(void);
	void SendTask(SLSKtask *t);
	ProjectKeywordsVector& GetProjects(void);
	//vector<TrackInformation *> GetTracks(void);
	CSLSKSpooferDlg *m_dlg;
//	void GetProjects(void);
	//void DllInitialize();
	//void DllUnInitialize();
	//void DllStart();
	//void DllShowGUI();
};
