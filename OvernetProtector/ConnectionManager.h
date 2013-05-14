// ConnectionManager.h

#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include "Manager.h"
#include "ConnectionModule.h"
#include "..\DCMaster\ProjectKeywordsVector.h"
#include <afxmt.h>	// for CCriticalSection
#include "DonkeyMD4.h"



class OvernetProtectorDll;
class ConnectionManager : public Manager
{
	static CCriticalSection m_critSect;
public:
	ConnectionManager();
	~ConnectionManager();
	void InitParent(OvernetProtectorDll *parent);

	void TimerHasFired();
	void ReportStatus(char *status,unsigned int *counts);
	void KeywordsUpdated();
	vector<ProjectKeywords> ReturnProjectKeywords();
	void PerformProjectSupplyQuery();
	void SetProjectKeywords(ProjectKeywordsVector& projects);
	void StatusReady(ConnectionModuleStatusData& status);
	void Log(const char* log);


private:
	DonkeyFile m_df;
	ConnectionModule m_mod;
	ProjectKeywordsVector m_projects;
	UINT m_num_peers;
	UINT m_seconds;
	UINT m_searching_project;
	//int m_searching_track;
};

#endif // CONNECTION_MANAGER_H