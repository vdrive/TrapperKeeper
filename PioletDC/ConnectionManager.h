// ConnectionManager.h

#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include "Manager.h"
#include "ConnectionModule.h"

class PioletDCDll;
class KeywordManager;
class SupplyManager;
class VendorCount;
class ConnectionManager : public Manager
{
public:
	ConnectionManager();
	~ConnectionManager();
	void InitParent(PioletDCDll *parent);
	void InitKeywordManager(KeywordManager* parent);
	void InitSupplyManager(SupplyManager* parent);

	void TimerHasFired();

	void ReportStatus(char *status,unsigned int *counts);
	
	void KeywordsUpdated();
	vector<ProjectKeywords> ReturnProjectKeywords();
	vector<SupplyProject> ReturnSupplyProjects();
	void DemandSupplyDataReady(ConnectionModuleStatusData& status);
	void ProjectSupplyUpdated(char* project_name);
	void PerformProjectSupplyQuery(char *project);
	void ReportVendorCounts(vector<VendorCount>* vendor_counts);


private:
	//PioletDCDll *p_parent;
	ConnectionModule m_mod;
	KeywordManager* p_keyword_manager;
	SupplyManager* p_supply_manager;
public:
	void UpdateHostLimits(UINT max_host, UINT max_host_cache);
};

#endif // CONNECTION_MANAGER_H