#pragma once
#include "..\sampleplugin\dll.h"
#include "..\StatusSource\MyComInterface.h"
#include "..\NameServer\NameServerInterface.h"
#include "DCMasterDlg.h"
#include "ProjectKeywords.h"
#include "ProjectChecksums.h"
#include "DCHeader.h"
#include "DCStatus.h"
#include "DataBaseInfo.h"
#include "ProjectDBInterface.h"
#include "BrowseProjectsDlg.h"

class DCMasterDll :	public Dll
{

public:
	DCMasterDll(void);
	~DCMasterDll(void);
	void DllInitialize();
	void DllUnInitialize();
	void DllStart();
	void DllShowGUI();
	void OnTimer(UINT nIDEvent);

	//override function to receive data from the Interface
	void DataReceived(char *source_name, void *data, int data_length);
	void Reset();
	CTime GetLastResponseTime(CString& ip, CString& network);
	void OnBnClickedBrowseProjectsButton();
	void OnRestart(vector<CString>& selected_racks);
	int AlterGnutellaSinglesLoadBalancingDivider(int value);
	int AlterGnutellaLoadBalancingDivider(int value);
	int AlterPioletLoadBalancingDivider(int value);
	int AlterBearShareLoadBalancingDivider(int value);
	int AlterGiftSwarmerLoadBalancingDivider(int value);
	int AlterGiftSpooferLoadBalancingDivider(int value);
	void ResetSplittedGnutellaProjects();
	void ResetGiftSpooferProjects();
	void ResetGiftSwarmerProjects();
	void ResetWinmxSpooferProjects();
	void ResetWinmxSwarmerProjects();
	void ResetSplittedPioletProjects();
	void ResetBearShareProjects();


private:

	int m_gnutella_load_balancing_divider;
	int m_gnutella_singles_load_balancing_divider;
	int m_piolet_load_balancing_divider;
	int m_bearshare_load_balancing_divider;
	int m_gift_swarmer_load_balancing_divider;
	int m_gift_spoofer_load_balancing_divider;
	bool m_allow_to_send_projects_updates_to_splitted_gnutella_spoofers;
	bool m_allow_to_send_projects_updates_to_gift_spoofers;
	bool m_allow_to_send_projects_updates_to_winmx_spoofers;
	bool m_allow_to_send_projects_updates_to_gift_swarmers;
	bool m_allow_to_send_projects_updates_to_winmx_swarmers;
	bool m_allow_to_send_projects_updates_to_piolet_spoofers;
	bool m_allow_to_send_projects_updates_to_bearshare_spoofers;
	//private data members
	MyComInterface m_com;
	CDCMasterDlg m_dlg;
	NameServerInterface m_nameserver;

	vector<ProjectKeywords> v_project_data;
	vector<ProjectKeywords> v_active_gnutella_protection_projects; //for splitted gnutella protectors
	vector<ProjectKeywords> v_gift_dc_project_data;
	vector<ProjectKeywords> v_gift_spoofers_project_data;
	vector<ProjectKeywords> v_gift_swarmers_project_data;
	vector<ProjectKeywords> v_singles_project_data;
	vector<ProjectKeywords> v_regular_active_gnutella_protection_projects;
	vector<ProjectKeywords> v_winmx_project_data;
	vector<ProjectKeywords> v_winmx_swarmers_project_data;
	vector<ProjectKeywords> v_winmx_spoofers_project_data;
	vector<ProjectKeywords> v_soulseek_project_data;
	vector<ProjectKeywords> v_bittorrent_project_data;
	vector<ProjectKeywords> v_generic_project_data;
	vector<ProjectKeywords> v_overnet_project_data;
	vector<ProjectKeywords> v_piolet_spoofer_project_data;
	vector<ProjectKeywords> v_bearshare_spoofer_project_data;

	ProjectChecksums m_project_checksums;
	ProjectChecksums m_active_gnutella_protection_project_checksums;
	ProjectChecksums m_regular_active_gnutella_protection_project_checksums;
	ProjectChecksums m_gift_dc_project_checksums;
	ProjectChecksums m_gift_spoofers_project_checksums;
	ProjectChecksums m_gift_swarmers_project_checksums;
	ProjectChecksums m_winmx_project_checksums;
	ProjectChecksums m_winmx_swarmers_project_checksums;
	ProjectChecksums m_winmx_spoofers_project_checksums;
	ProjectChecksums m_soulseek_project_checksums;
	ProjectChecksums m_bittorrent_project_checksums;
	ProjectChecksums m_generic_project_checksums;
	ProjectChecksums m_overnet_project_checksums;
	ProjectChecksums m_piolet_spoofer_project_checksums;
	ProjectChecksums m_bearshare_spoofer_project_checksums;

	vector<DCStatus> v_emule_network_status;
	vector<DCStatus> v_gnutella_network_status;
	vector<DCStatus> v_kazaa_network_status;
	vector<DCStatus> v_piolet_network_status;
	vector<DCStatus> v_piolet_spoofer_status;
	vector<DCStatus> v_piolet_spoofer_splitted_status;
	vector<DCStatus> v_gnutella_protectors_status;
	vector<DCStatus> v_direct_connect_status;
	vector<DCStatus> v_direct_connect_spoofer_status;
	vector<DCStatus> v_fasttrack_dc_status;
	vector<DCStatus> v_fasttrack_spoofers_status;
	vector<DCStatus> v_fasttrack_swarmers_status;
	vector<DCStatus> v_fasttrack_decoyers_status;
	vector<DCStatus> v_gnutella_spoofers_splited_status;
	vector<DCStatus> v_winmx_dc_status;
	vector<DCStatus> v_winmx_spoofer_status;
	vector<DCStatus> v_winmx_swarmer_status;
	vector<DCStatus> v_winmx_decoyer_status;
	vector<DCStatus> v_soulseek_spoofer_status;
	vector<DCStatus> v_bittorrent_status;
	vector<DCStatus> v_generic_status;
	vector<DCStatus> v_overnet_dc_status;
	vector<DCStatus> v_overnet_spoofer_status;
	vector<DCStatus> v_bearshare_spoofer_status;


	DataBaseInfo m_db_info;
	DataBaseInfo m_piolet_raw_db_info;
	DataBaseInfo m_gnutella_raw_db_info;
	DataBaseInfo m_gift_raw_db_info;

	//int m_active_project_count;
	CBrowseProjectsDlg m_browse_projects_dlg;

	//private member functions
	bool ReadInKeywordData();
	void SendInitToAllDataCollectors();
	//DCMasterDll::project_data_type GetProjectEntryType(char * entry_string);
	
	void ReceivedRequestProjectChecksums(char* from);
	void ReceivedEmuleInitResponse(char* from);
	void ReceivedGnutellaInitResponse(char* from);
	void ReceivedKazaaInitResponse(char* from);
	void ReceivedPioletInitResponse(char* from);
	void ReceivedPioletSpooferInitResponse(char* from);
	void ReceivedGnutellaProtectorInitResponse(char* from);
	void ReceivedDirectConnectInitResponse(char* from);
	void ReceivedDirectConnectSpooferInitResponse(char* from);
	void ReceivedFastTrackDCInitResponse(char* from);
	void ReceivedFastTrackSpooferInitResponse(char* from);
	void ReceivedFastTrackSwarmerInitResponse(char* from);
	void ReceivedFastTrackDecoyerInitResponse(char* from);
	void ReceivedGnutellaSpoofersSplitedInitResponse(char* from);
	void ReceivedWinmxDCInitResponse(char* from);
	void ReceivedWinmxSpooferInitResponse(char* from);
	void ReceivedWinmxDecoyerInitResponse(char* from);
	void ReceivedWinmxSwarmerInitResponse(char* from);
	void ReceivedSoulSeekSpooferInitResponse(char* from);
	void ReceivedBitTorrentInitResponse(char* from);
	void ReceivedGenericInitResponse(char* from);
	void ReceivedOvernetDCInitResponse(char* from);
	void ReceivedOvernetSpooferInitResponse(char* from);
	void ReceivedPioletSpooferSplittedInitResponse(char* from);
	void ReceivedBearshareSpooferInitResponse(char* from);


	
	void SendRequestedProjects(char* dest, ProjectChecksums requested_projects);
	void SendProjectChecksums(DCStatus& status);
	
	void ReassignProjectsForEmule();
	void ReassignProjectsForGnutella();
	void ReassignProjectsForKazaa();
	void ReassignProjectsForPiolet();
	void ReassignProjectsForPioletSpoofer();
	void ReassignProjectsForGnutellaProtector();
	void ReassignProjectsForDirectConnect();
	void ReassignProjectsForDirectConnectSpoofer();
	void ReassignProjectsForFastTrackDC();
	//void ReassignProjectsForFastTrackSwarmersRacksDown(vector<ProjectKeywords>& v_projects); //reason: some racks are down
	//void ReassignProjectsForFastTrackSpoofersRacksDown(vector<ProjectKeywords>& v_projects); //reason: some racks are down
	//void ReassignProjectsForFastTrackDecoyersRacksDown(vector<ProjectKeywords>& v_projects); //reason: some racks are down
	//void ReassignProjectsForGnutellaSpoofersSplitedRacksDown(vector<ProjectKeywords>& v_projects); //reason: some racks are down
	//void ReassignUpdatedProjectsForFastTrackSpoofers();
	//void ReassignUpdatedProjectsForFastTrackSwarmers();
	//void ReassignUpdatedProjectsForGnutellaSpoofersSplited();
	void ReassignProjectsForWinmxDC();
	//void ReassignProjectsForWinmxSpoofer();
	void ReassignProjectsForWinmxDecoyer();
	void ReassignProjectsForSoulSeekSpoofer();
	void ReassignProjectsForBitTorrent();
	void ReassignProjectsForGeneric();
	void ReassignProjectsForOvernetDC();
	void ReassignProjectsForOvernetSpoofer();

	void LoadBalanceProjectsForGiftSwarmers();
	void LoadBalanceProjectsForGiftSpoofers();
	void LoadBalanceProjectsForGiftDecoyers();
	void LoadBalanceProjectsForGnutellaSplitedSpoofers();
	void LoadBalanceProjectsForWinmxSwarmers();
	void LoadBalanceProjectsForWinmxSpoofers();
	void LoadBalanceProjectsForPioletSpoofers();
	void LoadBalanceProjectsForBearShareSpoofers();
	int GetLeastNumberOfProjects(vector<UINT>& exclude_collectors, vector<DCStatus>& network);
	
	void CheckForCollectorsStatus(bool recalculate_project_distribution=true);

	void ReadInDatabaseInfo();
	void SendDBInfo(char* dest);
	void SendPioletRawDBInfo(char* dest);
	void SendGnutellaRawDBInfo(char* dest);
	UINT GetLeastNumberOfProjectsOfKazaaCollector(vector<UINT>& exclude_collectors);
	/*void WriteOutKeywordData();
	void WriteOutGiftDCKeywordData();
	void WriteOutGiftSpoofersKeywordData();
	void WriteOutGiftSwarmersKeywordData();
	*/
	//void WriteOutActiveGnutellaKeywordData();

	void SendGiftRawDBInfo(char* dest);
	int UsingProjectKeywordsVersion(const char* dest);
	int FindMinimumProjectsGiftSwarmerRack();
	//int FindMinimumProjectsGnutellaSpooferSplited();
	int GetNumRacksAssignedForProject(int project_id, vector<DCStatus>& network, vector<UINT>& assigned_racks);
	void RestartCrashedRacks();

};
