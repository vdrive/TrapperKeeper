// ConnectionManager.h

#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include "Manager.h"
#include "ConnectionModule.h"
#include "..\DCMaster\ProjectKeywordsVector.h"
#include <afxmt.h>	// for CCriticalSection



class ConnectionManager : public Manager
{
	static CCriticalSection m_critSect;
public:
	ConnectionManager();
	~ConnectionManager();

	void TimerHasFired();
	void ReportStatus(char *status,unsigned int *counts);
	void KeywordsUpdated();
	vector<ProjectKeywords> ReturnProjectKeywords();
	void SetProjectKeywords(ProjectKeywordsVector& projects);
	void StatusReady(ConnectionModuleStatusData& status);
	void Log(const char* log);
	void SupplySynched();


private:
	ConnectionModule m_mod;
	ProjectKeywordsVector m_projects;
	UINT m_num_peers;
	UINT m_seconds;
};

#endif // CONNECTION_MANAGER_H