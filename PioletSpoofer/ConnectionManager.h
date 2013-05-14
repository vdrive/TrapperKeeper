// ConnectionManager.h

#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include "Manager.h"
#include "ConnectionModule.h"
#include "PoisonEntries.h"

class PioletSpooferDll;
class KeywordManager;
class SupplyManager;
class VendorCount;
class ConnectionManager : public Manager
{
public:
	ConnectionManager();
	~ConnectionManager();
	void InitParent(PioletSpooferDll *parent);
	void InitKeywordManager(KeywordManager* parent);
	void InitSupplyManager(SupplyManager* parent);

	void TimerHasFired();

	void ReportStatus(char *status,unsigned int *counts);
	
	void KeywordsUpdated();
	vector<ProjectKeywords> ReturnProjectKeywords();
	vector<SupplyProject>* ReturnSupplyProjectsPointer();
	void SpoofDataReady(ConnectionModuleStatusData& status);
	void ProjectSupplyUpdated(char* project_name);
	void PerformProjectSupplyQuery(char *project);
	void ReportVendorCounts(vector<VendorCount>* vendor_counts);
	void AddDC(CString dc);
	void SendPoisonEntry(char* poisoner, PoisonEntries& pe);
	vector<PoisonerStatus>* GetPoisoners();
	void Log(char *buf, COLORREF color=0xFFFFFFFF, bool bold=false, bool italic=false);
	int GetNumPoisoners();

//	void LockSupplyProject();
//	void ReleaseSupplyProject();

	SupplyManager* p_supply_manager;

private:
	//PioletDCDll *p_parent;
	ConnectionModule m_mod;
	KeywordManager* p_keyword_manager;
public:
	void UpdateHostLimits(UINT max_host, UINT max_host_cache);
};

#endif // CONNECTION_MANAGER_H