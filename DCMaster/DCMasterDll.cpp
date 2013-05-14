#include "StdAfx.h"
#include "dcmasterdll.h"
#include "DllInterface.h"
#include "imagehlp.h"
#include "ProjectKeywordsVector.h"
#include "RackRecoverInterface.h"
#include <algorithm>
#include "RegistryKeys.h"

DCMasterDll::DCMasterDll(void)
{
	srand( (unsigned)time( NULL ) );
}

//
//
//
DCMasterDll::~DCMasterDll(void)
{
}

//
//
//
void DCMasterDll::DllInitialize()
{

	
}

//
//
//
void DCMasterDll::DllUnInitialize()
{
	m_dlg.DestroyWindow();
	m_browse_projects_dlg.DestroyWindow();
}

//
//
//
void DCMasterDll::DllStart()
{
	m_allow_to_send_projects_updates_to_splitted_gnutella_spoofers=true;
	m_allow_to_send_projects_updates_to_gift_spoofers=true;
	m_allow_to_send_projects_updates_to_winmx_spoofers=true;
	m_allow_to_send_projects_updates_to_gift_swarmers=true;
	m_allow_to_send_projects_updates_to_winmx_swarmers=true;
	m_allow_to_send_projects_updates_to_piolet_spoofers=true;
	m_allow_to_send_projects_updates_to_bearshare_spoofers=true;

	RegistryKeys::GetDividers(m_gnutella_load_balancing_divider,m_gnutella_singles_load_balancing_divider);
	RegistryKeys::GetPioletDivider(m_piolet_load_balancing_divider);
	RegistryKeys::GetBearShareDivider(m_bearshare_load_balancing_divider);
	RegistryKeys::GetGiftSwarmerDivider(m_gift_swarmer_load_balancing_divider);
	RegistryKeys::GetGiftSpooferDivider(m_gift_spoofer_load_balancing_divider);
	//m_active_project_count = 0;
	m_dlg.Create(IDD_DC_MASTER_DIALOG,CWnd::GetDesktopWindow());
	m_browse_projects_dlg.Create(IDD_BROWSE_PROJECTS_DIALOG,CWnd::GetDesktopWindow());
	m_browse_projects_dlg.ShowWindow(SW_HIDE);
	m_dlg.InitParent(this);
	m_dlg.SetDividers(m_gnutella_load_balancing_divider,m_gnutella_singles_load_balancing_divider);
	m_dlg.SetPioletDivider(m_piolet_load_balancing_divider);
	m_dlg.SetBearShareDivider(m_bearshare_load_balancing_divider);
	m_dlg.SetGiftSwarmerDivider(m_gift_swarmer_load_balancing_divider);
	m_dlg.SetGiftSpooferDivider(m_gift_spoofer_load_balancing_divider);
	m_piolet_raw_db_info.m_db_password = "ebertsux37";
	m_piolet_raw_db_info.m_db_user = "onsystems";
#ifdef DCMASTER2
	m_gnutella_raw_db_info.m_db_host="63.216.246.43";
#else
	m_gnutella_raw_db_info.m_db_host="63.221.232.35";
#endif
	m_gnutella_raw_db_info.m_db_password = "ebertsux37";
	m_gnutella_raw_db_info.m_db_user = "onsystems";
	m_gift_raw_db_info.m_db_host="63.221.232.34";
	m_gift_raw_db_info.m_db_password = "ebertsux37";
	m_gift_raw_db_info.m_db_user = "onsystems";
	ReadInKeywordData();
	//WriteOutActiveGnutellaKeywordData();
	//WriteOutKeywordData();
	//WriteOutGiftDCKeywordData();
	//WriteOutGiftSpoofersKeywordData();
	//WriteOutGiftSwarmersKeywordData();
	ReadInDatabaseInfo();

	//register com service
	m_com.Register(this,DllInterface::m_app_id.m_app_id);

#ifdef _DEBUG
	m_dlg.SetTimer(1, 1000*10, NULL);  //wait at startup
#else
	m_dlg.SetTimer(1, 1000*60*5, NULL);  //wait at startup
#endif

	m_dlg.SetTimer(2, 60*1000, NULL); //send init message


#ifdef _DEBUG
	m_dlg.SetTimer(4, 5*60*1000, NULL); //query projects from SQL database
#else
	m_dlg.SetTimer(4, 30*60*1000, NULL); //query projects from SQL database
#endif

	m_dlg.SetTimer(5, 1000, NULL);  //wait at startup
	m_dlg.SetTimer(6, 15*60*1000, NULL);
	m_dlg.SetTimer(7, 60*60*1000,NULL);


	SendInitToAllDataCollectors();

/*
#ifdef _DEBUG
	DCStatus dc;
	strcpy(dc.m_ip,"1");
	v_fasttrack_swarmers_status.push_back(dc);
	strcpy(dc.m_ip,"2");
	v_fasttrack_swarmers_status.push_back(dc);
	strcpy(dc.m_ip,"3");
	v_fasttrack_swarmers_status.push_back(dc);
	strcpy(dc.m_ip,"4");
	v_fasttrack_spoofers_status.push_back(dc);
	strcpy(dc.m_ip,"5");
	v_fasttrack_spoofers_status.push_back(dc);
	strcpy(dc.m_ip,"6");
	v_fasttrack_spoofers_status.push_back(dc);
	for(int i=0; i<v_fasttrack_swarmers_status.size(); i++)
	{
		m_dlg.UpdateCollectorStatus(v_fasttrack_swarmers_status[i],"FastTrack Swarmer");
	}
	for(i=0; i<v_fasttrack_spoofers_status.size(); i++)
	{
		m_dlg.UpdateCollectorStatus(v_fasttrack_spoofers_status[i],"FastTrack Spoofer");
	}
	m_dlg.SetTimer(6,30*1000,0);
#endif
	*/

}

//
//
//
void DCMasterDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
bool DCMasterDll::ReadInKeywordData()
{
	ProjectDBInterface project_interface;
	if(!project_interface.OpenDBConnection())
		return false;

	
	CStdioFile poisoners_file;
	BOOL open = poisoners_file.Open("poisoners.ini",CFile::typeText|CFile::modeRead|CFile::shareDenyNone);

	vector<NetAddress> global_poisoners;
	if(open==TRUE)
	{
		char poisoner_data[256];

		LPTSTR more_data = poisoners_file.ReadString(poisoner_data, 256);
		
		while(more_data!=NULL)
		{
			NetAddress address;
			char * port = strchr(poisoner_data, ':');
			if(port!=NULL)
			{
				port[0]='\0';
				port++;

				address.SetPort(atoi(port));
			}
			else
			{
				address.SetPort(6346);
			}

			address.SetIP(poisoner_data);

			global_poisoners.push_back(address);

			more_data = poisoners_file.ReadString(poisoner_data, 256);
		}

		poisoners_file.Close();
	}
	

	int active_project_count = 0;
	v_project_data.clear();
	v_gift_dc_project_data.clear();
	v_regular_active_gnutella_protection_projects.clear();
	v_winmx_project_data.clear();
	v_soulseek_project_data.clear();
	v_bittorrent_project_data.clear();
	v_generic_project_data.clear();
	v_overnet_project_data.clear();
	
	if(m_allow_to_send_projects_updates_to_splitted_gnutella_spoofers)
	{
		v_active_gnutella_protection_projects.clear();
		v_singles_project_data.clear();
		m_active_gnutella_protection_project_checksums.Clear();
		/*
		v_gift_spoofers_project_data.clear();
		v_gift_swarmers_project_data.clear();
		m_gift_spoofers_project_checksums.Clear();
		m_gift_swarmers_project_checksums.Clear();
		v_winmx_swarmers_project_data.clear();
		m_winmx_swarmers_project_checksums.Clear();
		v_winmx_spoofers_project_data.clear();
		m_winmx_spoofers_project_checksums.Clear();
		*/
	}
	if(m_allow_to_send_projects_updates_to_gift_spoofers)
	{
		v_gift_spoofers_project_data.clear();
		m_gift_spoofers_project_checksums.Clear();
	}
	if(m_allow_to_send_projects_updates_to_gift_swarmers)
	{
		v_gift_swarmers_project_data.clear();
		m_gift_swarmers_project_checksums.Clear();
	}
	if(m_allow_to_send_projects_updates_to_winmx_spoofers)
	{
		v_winmx_spoofers_project_data.clear();
		m_winmx_spoofers_project_checksums.Clear();
	}
	if(m_allow_to_send_projects_updates_to_winmx_swarmers)
	{
		v_winmx_swarmers_project_data.clear();
		m_winmx_swarmers_project_checksums.Clear();
	}
	if(m_allow_to_send_projects_updates_to_piolet_spoofers)
	{
		v_piolet_spoofer_project_data.clear();
		m_piolet_spoofer_project_checksums.Clear();
	}
	if(m_allow_to_send_projects_updates_to_bearshare_spoofers)
	{
		v_bearshare_spoofer_project_data.clear();
		m_bearshare_spoofer_project_checksums.Clear();
	}
	m_project_checksums.Clear();
	m_gift_dc_project_checksums.Clear();
	m_regular_active_gnutella_protection_project_checksums.Clear();
	m_winmx_project_checksums.Clear();
	m_soulseek_project_checksums.Clear();
	m_bittorrent_project_checksums.Clear();
	m_generic_project_checksums.Clear();
	m_overnet_project_checksums.Clear();

	project_interface.ReadProjectKeywords(v_project_data);
	project_interface.CloseDBConnection();

	for(UINT i=0; i<v_project_data.size();i++)
	{
		if(v_project_data[i].m_project_name[0]!='\0')
		{
			/*
			if(v_project_data[i].v_poisoners.size()==0)
			{
				v_project_data[i].v_poisoners = global_poisoners;
			}
			*/
			if(v_project_data[i].m_interval==0)
			{
				v_project_data[i].m_interval=60;
			}
			//KLUDGE
			if(v_project_data[i].m_owner.size()==0)
			{
				v_project_data[i].m_owner = "Unknown";
			}
			if(v_project_data[i].m_project_active == true)
			{
				active_project_count++;
			}
		}
		ProjectChecksum  checksum;
		checksum = v_project_data[i].CalculateChecksum();
		checksum.m_project_name = v_project_data[i].m_project_name;
		m_project_checksums.v_checksums.push_back(checksum);

		//copy old project keywords to new project keywords
		//insert active and gift dc project only
		if( v_project_data[i].m_project_active && v_project_data[i].m_gift_supply_enabled)
		{
			v_gift_dc_project_data.push_back(v_project_data[i]);
			v_gift_dc_project_data[v_gift_dc_project_data.size()-1].m_version = 0x00000004;
		}

		//for regular gnutella protector
		if(v_project_data[i].m_project_active && (v_project_data[i].m_gnutella_spoofing_enabled ||
			v_project_data[i].m_gnutella_swarming_enabled || v_project_data[i].m_gnutella_decoys_enabled))
		{
			//if( (stricmp(v_project_data[i].m_owner.c_str(),"Warner Bros Music.Test Jan 2005")==0))
			//{
			v_regular_active_gnutella_protection_projects.push_back(v_project_data[i]);
			v_regular_active_gnutella_protection_projects[v_regular_active_gnutella_protection_projects.size()-1].m_version = 0x00000005;
			v_regular_active_gnutella_protection_projects[v_regular_active_gnutella_protection_projects.size()-1].v_poisoners = global_poisoners;
			//}
		}

		//for winmx
		if(v_project_data[i].m_project_active && (v_project_data[i].m_winmx_supply_enabled ||
			v_project_data[i].m_winmx_decoy_enabled || v_project_data[i].m_winmx_spoofing_enabled || v_project_data[i].m_winmx_swarming_enabled))
		{
			v_winmx_project_data.push_back(v_project_data[i]);
			v_winmx_project_data[v_winmx_project_data.size()-1].m_version = 0x00000006;
			
		}

		//for soulseek
		if(v_project_data[i].m_project_active && (v_project_data[i].m_soulseek_decoying_enabled ||
			v_project_data[i].m_soulseek_demand_enabled || v_project_data[i].m_soulseek_spoofing_enabled ||
			v_project_data[i].m_soulseek_supply_enabled))
		{
			v_soulseek_project_data.push_back(v_project_data[i]);
			v_soulseek_project_data[v_soulseek_project_data.size()-1].m_version = 0x00000007;
		}

		//for Bit Torrent
		if(v_project_data[i].m_project_active && (v_project_data[i].m_bittorrent_decoy_enabled ||
			v_project_data[i].m_bittorrent_interdiction_enabled || v_project_data[i].m_bittorrent_spoofing_enabled ||
			v_project_data[i].m_bittorrent_supply_enabled))
		{
			v_bittorrent_project_data.push_back(v_project_data[i]);
			v_bittorrent_project_data[v_bittorrent_project_data.size()-1].m_version = 0x00000008;
		}
		
		//for overnet
		if(v_project_data[i].m_project_active && (v_project_data[i].m_overnet_decoys_enabled ||
			v_project_data[i].m_overnet_demand_enabled || v_project_data[i].m_overnet_spoofing_enabled ||
			v_project_data[i].m_overnet_supply_enabled))
		{
			v_overnet_project_data.push_back(v_project_data[i]);
			v_overnet_project_data[v_overnet_project_data.size()-1].m_version = 0x00000009;
		}

		//for generic
		if(v_project_data[i].m_project_active)
		{
			v_generic_project_data.push_back(v_project_data[i]);
			v_generic_project_data[v_generic_project_data.size()-1].m_version = 0x00000009;
		}

		if(m_allow_to_send_projects_updates_to_splitted_gnutella_spoofers)
		{
			if(v_project_data[i].m_project_active && (v_project_data[i].m_gnutella_spoofing_enabled ||
				v_project_data[i].m_gnutella_swarming_enabled || v_project_data[i].m_gnutella_decoys_enabled) )
			{
				//if( (stricmp(v_project_data[i].m_owner.c_str(),"Warner Bros Music.Test Jan 2005")==0))
				//{
				v_active_gnutella_protection_projects.push_back(v_project_data[i]);
				v_active_gnutella_protection_projects[v_active_gnutella_protection_projects.size()-1].m_version = 0x00000005;
				v_active_gnutella_protection_projects[v_active_gnutella_protection_projects.size()-1].v_poisoners = global_poisoners;


				if(v_project_data[i].m_id!=2903) //not COMMON WORDS project
				{
					//look for project with singles
					ProjectKeywords single_project = v_project_data[i];
					single_project.v_poisoners = global_poisoners;
					vector<SupplyKeyword>::iterator supply_iter = single_project.m_supply_keywords.v_keywords.begin();
					while(supply_iter != single_project.m_supply_keywords.v_keywords.end())
					{
						if(!supply_iter->m_single)
							single_project.m_supply_keywords.v_keywords.erase(supply_iter);
						else
						{
							supply_iter++;
						}
					}
					if(single_project.m_supply_keywords.v_keywords.size()>0)
					{
						single_project.m_version=0x00000005;
						single_project.m_id+=1000000000;
						single_project.m_project_name+="_**Singles**_";
						v_singles_project_data.push_back(single_project);
					}
				}
			}
		}
		if(m_allow_to_send_projects_updates_to_gift_spoofers)
		{
			//insert active and gift spoofing project only
			if( (v_project_data[i].m_project_active) && (v_project_data[i].m_gift_spoofing_enabled) )
			{
				//if( (stricmp(v_project_data[i].m_owner.c_str(),"Warner Bros Music.Test Jan 2005")==0))
				//{
				//v_gift_spoofers_project_data.push_back(v_project_data[i]);
				//v_gift_spoofers_project_data[v_gift_spoofers_project_data.size()-1].m_version = 0x00000005;
				//}
				//look for project with singles
				ProjectKeywords project = v_project_data[i];
				project.m_version=0x00000005;
				CString owner=project.m_owner.c_str();
				owner.MakeLower();
				if( strstr(owner,"warner")==NULL && project.m_search_type==ProjectKeywords::search_type::audio)
				{
					vector<SupplyKeyword>::iterator supply_iter = project.m_supply_keywords.v_keywords.begin();
					while(supply_iter != project.m_supply_keywords.v_keywords.end())
					{
						if(!supply_iter->m_single)
							project.m_supply_keywords.v_keywords.erase(supply_iter);
						else
						{
							supply_iter++;
						}
					}
					if(project.m_supply_keywords.v_keywords.size())
						v_gift_spoofers_project_data.push_back(project);
				}
				else
					v_gift_spoofers_project_data.push_back(project);
			}
		}
		if(m_allow_to_send_projects_updates_to_gift_swarmers)
		{
			//insert active and gift swarming project only
			if( (v_project_data[i].m_project_active) && (v_project_data[i].m_gift_decoys_enabled) )
			{
				//if( (stricmp(v_project_data[i].m_owner.c_str(),"Warner Bros Music.Test Jan 2005")==0))
				//{
				//v_gift_swarmers_project_data.push_back(v_project_data[i]);
				//v_gift_swarmers_project_data[v_gift_swarmers_project_data.size()-1].m_version = 0x00000005;
				//}
				ProjectKeywords project = v_project_data[i];
				project.m_version=0x00000005;
				CString owner=project.m_owner.c_str();
				owner.MakeLower();
				if( strstr(owner,"warner")==NULL && project.m_search_type==ProjectKeywords::search_type::audio)
				{
					vector<SupplyKeyword>::iterator supply_iter = project.m_supply_keywords.v_keywords.begin();
					while(supply_iter != project.m_supply_keywords.v_keywords.end())
					{
						if(!supply_iter->m_single)
							project.m_supply_keywords.v_keywords.erase(supply_iter);
						else
						{
							supply_iter++;
						}
					}
					if(project.m_supply_keywords.v_keywords.size())
						v_gift_swarmers_project_data.push_back(project);
				}
				else
					v_gift_swarmers_project_data.push_back(project);
			}
		}
		if(m_allow_to_send_projects_updates_to_winmx_swarmers)
		{
			//for winmx swarmer
			if( (v_project_data[i].m_project_active) && (v_project_data[i].m_winmx_swarming_enabled) )
			{
				v_winmx_swarmers_project_data.push_back(v_project_data[i]);
				v_winmx_swarmers_project_data[v_winmx_swarmers_project_data.size()-1].m_version = 0x00000006;

			}
		}
		if(m_allow_to_send_projects_updates_to_winmx_spoofers)
		{
			//for winmx spoofer
			if( (v_project_data[i].m_project_active) && (v_project_data[i].m_winmx_spoofing_enabled) )
			{
				v_winmx_spoofers_project_data.push_back(v_project_data[i]);
				v_winmx_spoofers_project_data[v_winmx_spoofers_project_data.size()-1].m_version = 0x00000006;

			}
		}
		if(m_allow_to_send_projects_updates_to_piolet_spoofers)
		{
			//for piolet spoofer
			if(v_project_data[i].m_project_active && (v_project_data[i].m_piolet_spoofing_enabled))
			{
				v_piolet_spoofer_project_data.push_back(v_project_data[i]);
				v_piolet_spoofer_project_data[v_piolet_spoofer_project_data.size()-1].m_version = 0x00000009;
			}
		}
		if(m_allow_to_send_projects_updates_to_bearshare_spoofers)
		{
			//for bearshare
			if(v_project_data[i].m_project_active && (v_project_data[i].m_gnutella_spoofing_enabled ||
				v_project_data[i].m_gnutella_swarming_enabled || v_project_data[i].m_gnutella_decoys_enabled))
			{
				//look for project with singles
				ProjectKeywords single_project = v_project_data[i];
				single_project.v_poisoners = global_poisoners;
				vector<SupplyKeyword>::iterator supply_iter = single_project.m_supply_keywords.v_keywords.begin();
				while(supply_iter != single_project.m_supply_keywords.v_keywords.end())
				{
					if(!supply_iter->m_single)
						single_project.m_supply_keywords.v_keywords.erase(supply_iter);
					else
					{
						supply_iter++;
					}
				}
				if(single_project.m_supply_keywords.v_keywords.size()>0)
				{
					single_project.m_version=0x00000005;
					v_bearshare_spoofer_project_data.push_back(single_project);
				}
			}
		}
	}
	//prepare project keywords checksums overnet
	for(UINT i=0; i<v_overnet_project_data.size();i++)
	{
		ProjectChecksum  checksum;
		checksum = v_overnet_project_data[i].CalculateChecksum();
		checksum.m_project_name = v_overnet_project_data[i].m_project_name;
		m_overnet_project_checksums.v_checksums.push_back(checksum);
	}
	//prepare project keywords checksums generic
	for(UINT i=0; i<v_generic_project_data.size();i++)
	{
		ProjectChecksum  checksum;
		checksum = v_generic_project_data[i].CalculateChecksum();
		checksum.m_project_name = v_generic_project_data[i].m_project_name;
		m_generic_project_checksums.v_checksums.push_back(checksum);
	}
	//prepare project keywords checksums BitTorrent
	for(UINT i=0; i<v_bittorrent_project_data.size();i++)
	{
		ProjectChecksum  checksum;
		checksum = v_bittorrent_project_data[i].CalculateChecksum();
		checksum.m_project_name = v_bittorrent_project_data[i].m_project_name;
		m_bittorrent_project_checksums.v_checksums.push_back(checksum);
	}
	//prepare project keywords checksums soulseek
	for(UINT i=0; i<v_soulseek_project_data.size();i++)
	{
		ProjectChecksum  checksum;
		checksum = v_soulseek_project_data[i].CalculateChecksum();
		checksum.m_project_name = v_soulseek_project_data[i].m_project_name;
		m_soulseek_project_checksums.v_checksums.push_back(checksum);
	}

	//prepare project keywords checksums winmx
	for(UINT i=0; i<v_winmx_project_data.size();i++)
	{
		ProjectChecksum  checksum;
		checksum = v_winmx_project_data[i].CalculateChecksum();
		checksum.m_project_name = v_winmx_project_data[i].m_project_name;
		m_winmx_project_checksums.v_checksums.push_back(checksum);
	}

	//prepare project keywords checksums gift dc
	for(UINT i=0; i<v_gift_dc_project_data.size();i++)
	{
		ProjectChecksum  checksum;
		checksum = v_gift_dc_project_data[i].CalculateChecksum();
		checksum.m_project_name = v_gift_dc_project_data[i].m_project_name;
		m_gift_dc_project_checksums.v_checksums.push_back(checksum);
	}
	//prepare project keywords checksums regular gnutella protector
	for(UINT i=0; i<v_regular_active_gnutella_protection_projects.size();i++)
	{
		ProjectChecksum  checksum;
		checksum = v_regular_active_gnutella_protection_projects[i].CalculateChecksum();
		checksum.m_project_name = v_regular_active_gnutella_protection_projects[i].m_project_name;
		m_regular_active_gnutella_protection_project_checksums.v_checksums.push_back(checksum);
	}

	//prepare project keywords checksums splitted gnutella protector
	if(m_allow_to_send_projects_updates_to_splitted_gnutella_spoofers)
	{
		for(UINT i=0; i<v_active_gnutella_protection_projects.size();i++)
		{
			ProjectChecksum  checksum;
			checksum = v_active_gnutella_protection_projects[i].CalculateChecksum();
			checksum.m_project_name = v_active_gnutella_protection_projects[i].m_project_name;
			m_active_gnutella_protection_project_checksums.v_checksums.push_back(checksum);
		}
	}
	if(m_allow_to_send_projects_updates_to_gift_spoofers)
	{
		//prepare project keywords checksums gift spoofing
		for(UINT i=0; i<v_gift_spoofers_project_data.size();i++)
		{
			ProjectChecksum  checksum;
			checksum = v_gift_spoofers_project_data[i].CalculateChecksum();
			checksum.m_project_name = v_gift_spoofers_project_data[i].m_project_name;
			m_gift_spoofers_project_checksums.v_checksums.push_back(checksum);
		}
	}
	if(m_allow_to_send_projects_updates_to_gift_swarmers)
	{
		//prepare project keywords checksums gift swarming
		for(UINT i=0; i<v_gift_swarmers_project_data.size();i++)
		{
			ProjectChecksum  checksum;
			checksum = v_gift_swarmers_project_data[i].CalculateChecksum();
			checksum.m_project_name = v_gift_swarmers_project_data[i].m_project_name;
			m_gift_swarmers_project_checksums.v_checksums.push_back(checksum);
		}
	}
	if(m_allow_to_send_projects_updates_to_winmx_swarmers)
	{
		//prepare project keywords checksums winmx swarmers
		for(UINT i=0; i<v_winmx_swarmers_project_data.size();i++)
		{
			ProjectChecksum  checksum;
			checksum = v_winmx_swarmers_project_data[i].CalculateChecksum();
			checksum.m_project_name = v_winmx_swarmers_project_data[i].m_project_name;
			m_winmx_swarmers_project_checksums.v_checksums.push_back(checksum);
		}
	}
	if(m_allow_to_send_projects_updates_to_winmx_spoofers)
	{
		//prepare project keywords checksums winmx swarmers
		for(UINT i=0; i<v_winmx_spoofers_project_data.size();i++)
		{
			ProjectChecksum  checksum;
			checksum = v_winmx_spoofers_project_data[i].CalculateChecksum();
			checksum.m_project_name = v_winmx_spoofers_project_data[i].m_project_name;
			m_winmx_spoofers_project_checksums.v_checksums.push_back(checksum);
		}
	}
	if(m_allow_to_send_projects_updates_to_piolet_spoofers)
	{
		//prepare project keywords checksums piolet spoofers
		for(UINT i=0; i<v_piolet_spoofer_project_data.size();i++)
		{
			ProjectChecksum  checksum;
			checksum = v_piolet_spoofer_project_data[i].CalculateChecksum();
			checksum.m_project_name = v_piolet_spoofer_project_data[i].m_project_name;
			m_piolet_spoofer_project_checksums.v_checksums.push_back(checksum);
		}
	}
	if(m_allow_to_send_projects_updates_to_bearshare_spoofers)
	{
		//prepare project keywords checksums bearshare spoofers
		for(UINT i=0; i<v_bearshare_spoofer_project_data.size();i++)
		{
			ProjectChecksum  checksum;
			checksum = v_bearshare_spoofer_project_data[i].CalculateChecksum();
			checksum.m_project_name = v_bearshare_spoofer_project_data[i].m_project_name;
			m_bearshare_spoofer_project_checksums.v_checksums.push_back(checksum);
		}
	}
	char str[128];
	sprintf(str, "Total Projects: %d", (int)v_project_data.size());
	m_dlg.GetDlgItem(IDC_TOTAL_PROJECTS_STATIC)->SetWindowText(str);
	sprintf(str, "Active Projects: %d", active_project_count );
	m_dlg.GetDlgItem(IDC_ACTIVE_PROJECTS_STATIC)->SetWindowText(str);	
	sprintf(str, "gIFT DC Projects: %d", (int)v_gift_dc_project_data.size() );
	m_dlg.GetDlgItem(IDC_GIFT_DC_PROJECTS_STATIC)->SetWindowText(str);	
	sprintf(str, "gIFT Spoofing Projects: %d", (int)v_gift_spoofers_project_data.size() );
	m_dlg.GetDlgItem(IDC_GIFT_SPOOFING_PROJECTS_STATIC)->SetWindowText(str);	
	sprintf(str, "gIFT Swarming Projects: %d", (int)v_gift_swarmers_project_data.size() );
	m_dlg.GetDlgItem(IDC_GIFT_SWARMING_PROJECTS_STATIC)->SetWindowText(str);
	sprintf(str, "Gnutella Protection Projects: %d", (int)v_regular_active_gnutella_protection_projects.size());
	m_dlg.GetDlgItem(IDC_GNUTELLA_PROTECTION_PROJECTS_STATIC)->SetWindowText(str);	
	sprintf(str, "Singles Projects: %d", (int)v_singles_project_data.size());
	m_dlg.GetDlgItem(IDC_Singles_Projects_STATIC)->SetWindowText(str);	
	sprintf(str, "WinMX Spoofing Projects: %d", (int)v_winmx_spoofers_project_data.size());
	m_dlg.GetDlgItem(IDC_WINMX_SPOOFING_PROJECTS_STATIC)->SetWindowText(str);	
	sprintf(str, "WinMX Swarming Projects: %d", (int)v_winmx_swarmers_project_data.size());
	m_dlg.GetDlgItem(IDC_WINMX_SWARMING_PROJECTS_STATIC)->SetWindowText(str);	
	sprintf(str, "BearShare Projects: %d", (int)v_bearshare_spoofer_project_data.size());
	m_dlg.GetDlgItem(IDC_BearShare_Projects_STATIC)->SetWindowText(str);	
	return true;
}

//
//
//
void DCMasterDll::SendInitToAllDataCollectors()
{
	vector<string> IPs;
	m_nameserver.RequestIP("DATA-COLLECTOR",IPs);
	for(UINT i=0; i<IPs.size();i++)
	{
		DCHeader header;
		header.op = DCHeader::Init;
		header.size = 0;
		char IP[16];
		strcpy(IP, IPs[i].c_str());
		m_com.SendUnreliableData(IP, &header, sizeof(header));
		/*
		bool found = false;
		UINT j=0;
		for(j=0;j<v_emule_network_status.size();j++)
		{
			if(strcmp(v_emule_network_status[j].m_ip, IPs[i].c_str())==0)
			{
				found = true;
				break;
			}
		}
		if(!found)
		{
			for(j=0;j<v_gnutella_network_status.size();j++)
			{
				if(strcmp(v_gnutella_network_status[j].m_ip, IPs[i].c_str())==0)
				{
					found = true;
					break;
				}
			}
		}
		if(!found)
		{
			for(j=0;j<v_kazaa_network_status.size();j++)
			{
				if(strcmp(v_kazaa_network_status[j].m_ip, IPs[i].c_str())==0)
				{
					found = true;
					break;
				}
			}
		}
		if(!found)
		{
			for(j=0;j<v_piolet_network_status.size();j++)
			{
				if(strcmp(v_piolet_network_status[j].m_ip, IPs[i].c_str())==0)
				{
					found = true;
					break;
				}
			}
		}
		if(!found)
		{
			for(j=0;j<v_piolet_spoofer_status.size();j++)
			{
				if(strcmp(v_piolet_spoofer_status[j].m_ip, IPs[i].c_str())==0)
				{
					found = true;
					break;
				}
			}
		}
		if(!found)
		{
			for(j=0;j<v_gnutella_protectors_status.size();j++)
			{
				if(strcmp(v_gnutella_protectors_status[j].m_ip, IPs[i].c_str())==0)
				{
					found = true;
					break;
				}
			}
		}
		if(!found)
		{
			for(j=0;j<v_direct_connect_status.size();j++)
			{
				if(strcmp(v_direct_connect_status[j].m_ip, IPs[i].c_str())==0)
				{
					found = true;
					break;
				}
			}
		}
		if(!found)
		{
			for(j=0;j<v_fasttrack_dc_status.size();j++)
			{
				if(strcmp(v_fasttrack_dc_status[j].m_ip, IPs[i].c_str())==0)
				{
					found = true;
					break;
				}
			}
		}
		if(!found)
		{
			for(j=0;j<v_fasttrack_spoofers_status.size();j++)
			{
				if(strcmp(v_fasttrack_spoofers_status[j].m_ip, IPs[i].c_str())==0)
				{
					found = true;
					break;
				}
			}
		}
		if(!found)
		{
			for(j=0;j<v_fasttrack_swarmers_status.size();j++)
			{
				if(strcmp(v_fasttrack_swarmers_status[j].m_ip, IPs[i].c_str())==0)
				{
					found = true;
					break;
				}
			}
		}
		if(!found)
		{
			for(j=0;j<v_gnutella_spoofers_splited_status.size();j++)
			{
				if(strcmp(v_gnutella_spoofers_splited_status[j].m_ip, IPs[i].c_str())==0)
				{
					found = true;
					break;
				}
			}
		}
		if(!found)
		{
			for(j=0;j<v_winmx_dc_status.size();j++)
			{
				if(strcmp(v_winmx_dc_status[j].m_ip, IPs[i].c_str())==0)
				{
					found = true;
					break;
				}
			}
		}
		if(!found)
		{
			for(j=0;j<v_winmx_spoofer_status.size();j++)
			{
				if(strcmp(v_winmx_spoofer_status[j].m_ip, IPs[i].c_str())==0)
				{
					found = true;
					break;
				}
			}
		}
		if(!found)
		{
			for(j=0;j<v_winmx_swarmer_status.size();j++)
			{
				if(strcmp(v_winmx_swarmer_status[j].m_ip, IPs[i].c_str())==0)
				{
					found = true;
					break;
				}
			}
		}
		if(!found)
		{
			for(j=0;j<v_winmx_decoyer_status.size();j++)
			{
				if(strcmp(v_winmx_decoyer_status[j].m_ip, IPs[i].c_str())==0)
				{
					found = true;
					break;
				}
			}
		}
		if(!found)
		{
			for(j=0;j<v_soulseek_spoofer_status.size();j++)
			{
				if(strcmp(v_soulseek_spoofer_status[j].m_ip, IPs[i].c_str())==0)
				{
					found = true;
					break;
				}
			}
		}
		if(!found)
		{
			for(j=0;j<v_bittorrent_status.size();j++)
			{
				if(strcmp(v_bittorrent_status[j].m_ip, IPs[i].c_str())==0)
				{
					found = true;
					break;
				}
			}
		}
		if(!found)
		{
			for(j=0;j<v_overnet_dc_status.size();j++)
			{
				if(strcmp(v_overnet_dc_status[j].m_ip, IPs[i].c_str())==0)
				{
					found = true;
					break;
				}
			}
		}
		if(!found)
		{
			for(j=0;j<v_overnet_spoofer_status.size();j++)
			{
				if(strcmp(v_overnet_spoofer_status[j].m_ip, IPs[i].c_str())==0)
				{
					found = true;
					break;
				}
			}
		}
		if(!found)
		{
			for(j=0;j<v_generic_status.size();j++)
			{
				if(strcmp(v_generic_status[j].m_ip, IPs[i].c_str())==0)
				{
					found = true;
					break;
				}
			}
		}
		if(!found)
		{
			for(j=0;j<v_piolet_spoofer_splitted_status.size();j++)
			{
				if(strcmp(v_piolet_spoofer_splitted_status[j].m_ip, IPs[i].c_str())==0)
				{
					found = true;
					break;
				}
			}			
		}
		if(!found)
		{
			for(j=0;j<v_bearshare_spoofer_status.size();j++)
			{
				if(strcmp(v_bearshare_spoofer_status[j].m_ip, IPs[i].c_str())==0)
				{
					found = true;
					break;
				}
			}			
		}
		if(!found)
		{
			for(j=0;j<v_fasttrack_decoyers_status.size();j++)
			{
				if(strcmp(v_fasttrack_decoyers_status[j].m_ip, IPs[i].c_str())==0)
				{
					found = true;
					break;
				}
			}			
		}
		if(!found)
		{
//			DCStatus status;
//			strcpy(status.m_ip, IPs[i].c_str());
//			m_dlg.UpdateCollectorStatus(status, "*UNKNOWN*");
		}
		*/
	}
}

//
//
//
void DCMasterDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
	char* ptr = (char*)data;
	DCHeader* header = (DCHeader*)ptr;
	ptr+=sizeof(DCHeader);
	switch(header->op)
	{
		case DCHeader::RequestProjectChecksums:
		{
			ReceivedRequestProjectChecksums(source_name);
			break;
		}
		case DCHeader::Emule_DC_Init_Response:
		{
			ReceivedEmuleInitResponse(source_name);
			SendDBInfo(source_name);
			break;
		}
		case DCHeader::Gnutella_DC_Init_Response:
		{
			ReceivedGnutellaInitResponse(source_name);
			//m_gnutella_raw_db_info.m_db_host = source_name;
			SendDBInfo(source_name);
			break;
		}
		case DCHeader::Kazaa_DC_Init_Response:
		{
			ReceivedKazaaInitResponse(source_name);
			SendDBInfo(source_name);
			break;
		}
		case DCHeader::Piolet_DC_Init_Response:
		{
			ReceivedPioletInitResponse(source_name);
			m_piolet_raw_db_info.m_db_host = source_name;
			SendDBInfo(source_name);
			break;
		}
		case DCHeader::UpdateProjectsRequest:
		{
			ProjectChecksums requested_projects;
			requested_projects.ReadFromBuffer(ptr);
			SendRequestedProjects(source_name, requested_projects);
			break;
		}
		case DCHeader::Piolet_Spoofer_Init_Response:
		{
			ReceivedPioletSpooferInitResponse(source_name);
			SendDBInfo(source_name);
			SendPioletRawDBInfo(source_name);
			break;
		}
		case DCHeader::Piolet_Spoofer_Splitted_Init_Response:
		{
			ReceivedPioletSpooferSplittedInitResponse(source_name);
			SendDBInfo(source_name);
			SendPioletRawDBInfo(source_name);
			break;
		}
		case DCHeader::Gnutella_Protector_Init_Response:
		{
			ReceivedGnutellaProtectorInitResponse(source_name);
			SendDBInfo(source_name);
			SendGnutellaRawDBInfo(source_name);
			break;
		}
		case DCHeader::Gnutella_Spoofer_Spited_Init_Response:
		{
			ReceivedGnutellaSpoofersSplitedInitResponse(source_name);
			SendDBInfo(source_name);
			SendGnutellaRawDBInfo(source_name);
			break;
		}
		case DCHeader::Direct_Connect_Init_Response:
		{
			ReceivedDirectConnectInitResponse(source_name);
			SendDBInfo(source_name);
			break;
		}
		case DCHeader::Direct_Connect_Spoofer_Init_Response:
		{
			ReceivedDirectConnectSpooferInitResponse(source_name);
			SendDBInfo(source_name);
			break;
		}
		case DCHeader::FastTrack_DC_Init_Response:
		{
			ReceivedFastTrackDCInitResponse(source_name);
			m_gift_raw_db_info.m_db_host = source_name;
			SendDBInfo(source_name);
			break;
		}
		case DCHeader::FastTrack_Spoofer_Init_Response:
		{
			ReceivedFastTrackSpooferInitResponse(source_name);
			SendDBInfo(source_name);
			SendGiftRawDBInfo(source_name);
			break;
		}
		case DCHeader::FastTrack_Swarmer_Init_Response:
		{
			ReceivedFastTrackSwarmerInitResponse(source_name);
			SendDBInfo(source_name);
			SendGiftRawDBInfo(source_name);
			break;
		}
		case DCHeader::GiftSupplyRetrievalStarted:
		{
			m_dlg.ReceivedGiftSupplyStarted(source_name);
			break;
		}
		case DCHeader::GiftSupplyRetrievalEnded:
		{
			m_dlg.ReceivedGiftSupplyEnded(source_name);
			break;
		}
		case DCHeader::GiftSupplyRetrievalProgress:
		{
			int progress = *(int*)ptr;
			m_dlg.ReceivedGiftSupplyProgress(source_name,progress);
			break;
		}
		case DCHeader::WinMx_DC_Init_Response:
		{
			ReceivedWinmxDCInitResponse(source_name);
			SendDBInfo(source_name);
			break;
		}
		case DCHeader::WinMx_Swarmer_Init_Response:
		{
			ReceivedWinmxSwarmerInitResponse(source_name);
			break;
		}
		case DCHeader::WinMx_Decoyer_Init_Response:
		{
			ReceivedWinmxDecoyerInitResponse(source_name);
			break;
		}
		case DCHeader::WinMx_Spoofer_Init_Response:
		{
			ReceivedWinmxSpooferInitResponse(source_name);
			break;
		}
		case DCHeader::SoulSeek_Spoofer_Init_Response:
		{
			ReceivedSoulSeekSpooferInitResponse(source_name);
			break;
		}
		case DCHeader::BitTorrent_Init_Response:
		{
			ReceivedBitTorrentInitResponse(source_name);
			break;
		}
		case DCHeader::Overnet_DC_Init_Response:
		{
			ReceivedOvernetDCInitResponse(source_name);
			break;
		}
		case DCHeader::Overnet_Spoofer_Init_Response:
		{
			ReceivedOvernetSpooferInitResponse(source_name);
			break;
		}
		case DCHeader::BearShare_Protector_Init_Response:
		{
			ReceivedBearshareSpooferInitResponse(source_name);
			break;
		}
		case DCHeader::FastTrack_Decoyer_Init_Response:
		{
			ReceivedFastTrackDecoyerInitResponse(source_name);
			SendDBInfo(source_name);
			SendGiftRawDBInfo(source_name);
			break;
		}
		case DCHeader::Generic_Init_Response:
		{
			ReceivedGenericInitResponse(source_name);
			break;
		}
	}
}

//
//
//
void DCMasterDll::OnTimer(UINT nIDEvent)
{
	switch(nIDEvent)
	{
		case 1:
		{
			ReassignProjectsForEmule();
			ReassignProjectsForGnutella();
			ReassignProjectsForKazaa();
			ReassignProjectsForPiolet();
			ReassignProjectsForPioletSpoofer();
			LoadBalanceProjectsForPioletSpoofers();
			ReassignProjectsForGnutellaProtector();
			ReassignProjectsForDirectConnect();
			ReassignProjectsForDirectConnectSpoofer();
			ReassignProjectsForFastTrackDC();
			ReassignProjectsForWinmxDC();
			//ReassignProjectsForWinmxSpoofer();
			ReassignProjectsForWinmxDecoyer();
			ReassignProjectsForSoulSeekSpoofer();
			ReassignProjectsForBitTorrent();
			ReassignProjectsForGeneric();
			ReassignProjectsForOvernetDC();
			ReassignProjectsForOvernetSpoofer();

			//ReassignProjectsForFastTrackSpoofers();
			//ReassignProjectsForFastTrackSwarmers();
			LoadBalanceProjectsForGiftSwarmers();
			LoadBalanceProjectsForGiftSpoofers();
			//ReassignProjectsForGnutellaSpoofersSplited();
			LoadBalanceProjectsForGnutellaSplitedSpoofers();
			LoadBalanceProjectsForWinmxSwarmers();
			LoadBalanceProjectsForWinmxSpoofers();
			LoadBalanceProjectsForBearShareSpoofers();
			LoadBalanceProjectsForGiftDecoyers();
			m_dlg.KillTimer(1);
			m_dlg.SetTimer(3, 30*60*1000, NULL); // check for down racks
			break;
		}
		case 2:
		{
			int hour = CTime::GetCurrentTime().GetHour();
			int min = CTime::GetCurrentTime().GetMinute();
			if(hour >=4 && hour <=7) //send projects updates only between 4am to 8am
			{
				m_allow_to_send_projects_updates_to_splitted_gnutella_spoofers=true;
				m_allow_to_send_projects_updates_to_gift_spoofers=true;
				m_allow_to_send_projects_updates_to_winmx_spoofers=true;
				m_allow_to_send_projects_updates_to_gift_swarmers=true;
				m_allow_to_send_projects_updates_to_winmx_swarmers=true;
				m_allow_to_send_projects_updates_to_piolet_spoofers=true;
				m_allow_to_send_projects_updates_to_bearshare_spoofers=true;
			}
			else
			{
				m_allow_to_send_projects_updates_to_splitted_gnutella_spoofers=false;
				m_allow_to_send_projects_updates_to_gift_spoofers=false;
				m_allow_to_send_projects_updates_to_winmx_spoofers=false;
				m_allow_to_send_projects_updates_to_gift_swarmers=false;
				m_allow_to_send_projects_updates_to_winmx_swarmers=false;
				m_allow_to_send_projects_updates_to_piolet_spoofers=false;
				m_allow_to_send_projects_updates_to_bearshare_spoofers=false;
			}
/*
			//if it is 4am, send project checksums to all splitted gnutella spoofers
			if(hour==4 && min==0)
			{
				for(i=0; i<v_gnutella_spoofers_splited_status.size(); i++)
				{
					SendProjectChecksums(v_gnutella_spoofers_splited_status[i]);
					m_dlg.UpdateCollectorStatus(v_gnutella_spoofers_splited_status[i],"Gnutella Spoofer Splitted");
				}
			}
*/			SendInitToAllDataCollectors();
			break;
		}
		case 3:
		{
			CheckForCollectorsStatus();
			break;
		}
		case 4:
		{
			ProjectChecksums old_checksums = m_project_checksums;
			ProjectChecksums old_active_checksums = m_active_gnutella_protection_project_checksums;
			ProjectChecksums old_regular_active_checksums = m_regular_active_gnutella_protection_project_checksums;
			ProjectChecksums old_gift_dc_checksums = m_gift_dc_project_checksums;
			ProjectChecksums old_gift_spoofers_checksums = m_gift_spoofers_project_checksums;
			ProjectChecksums old_gift_swarmers_checksums = m_gift_swarmers_project_checksums;
			ProjectChecksums old_winmx_checksums = m_winmx_project_checksums;
			ProjectChecksums old_winmx_swarmers_checksums = m_winmx_swarmers_project_checksums;
			ProjectChecksums old_winmx_spoofers_checksums = m_winmx_spoofers_project_checksums;
			ProjectChecksums old_soulseek_checksums = m_soulseek_project_checksums;
			ProjectChecksums old_bittorrent_checksums = m_bittorrent_project_checksums;
			ProjectChecksums old_overnet_checksums = m_overnet_project_checksums;
			ProjectChecksums old_generic_checksums = m_generic_project_checksums;
			ProjectChecksums old_piolet_spoofer_checksums = m_piolet_spoofer_project_checksums;
			ProjectChecksums old_bearshare_spoofer_checksums = m_bearshare_spoofer_project_checksums;
			if(!ReadInKeywordData())
				break;
			if(old_checksums != m_project_checksums)
			{
				//WriteOutKeywordData();
				ReassignProjectsForEmule();
				ReassignProjectsForGnutella();
				ReassignProjectsForKazaa();
				ReassignProjectsForPiolet();
				//ReassignProjectsForPioletSpoofer();
				//ReassignProjectsForGnutellaProtector();
				ReassignProjectsForDirectConnect();
				ReassignProjectsForDirectConnectSpoofer();
			}
			if(old_gift_dc_checksums != m_gift_dc_project_checksums)
			{
				//WriteOutGiftDCKeywordData();
				ReassignProjectsForFastTrackDC();
			}
			if(old_gift_spoofers_checksums != m_gift_spoofers_project_checksums)
			{
				//WriteOutGiftSpoofersKeywordData();
				//ReassignUpdatedProjectsForFastTrackSpoofers();
				LoadBalanceProjectsForGiftSpoofers();
				LoadBalanceProjectsForGiftDecoyers();
			}
			if(old_gift_swarmers_checksums != m_gift_swarmers_project_checksums)
			{
				//WriteOutGiftSwarmersKeywordData();
				//ReassignUpdatedProjectsForFastTrackSwarmers();
				LoadBalanceProjectsForGiftSwarmers();
			}
			if(old_active_checksums != m_active_gnutella_protection_project_checksums)
			{
				//ReassignUpdatedProjectsForGnutellaSpoofersSplited();
				LoadBalanceProjectsForGnutellaSplitedSpoofers();
			}
			if(old_regular_active_checksums != m_regular_active_gnutella_protection_project_checksums)
			{
				ReassignProjectsForGnutellaProtector();
			}
			if(old_winmx_checksums != m_winmx_project_checksums)
			{
				ReassignProjectsForWinmxDC();
				//ReassignProjectsForWinmxSpoofer();
				ReassignProjectsForWinmxDecoyer();
				//ReassignProjectsForWinmxSwarmer();
			}
			if(old_winmx_swarmers_checksums != m_winmx_swarmers_project_checksums)
			{
				LoadBalanceProjectsForWinmxSwarmers();
			}
			if(old_winmx_spoofers_checksums != m_winmx_spoofers_project_checksums)
			{
				LoadBalanceProjectsForWinmxSpoofers();
			}
			if(old_soulseek_checksums != m_soulseek_project_checksums)
			{
				ReassignProjectsForSoulSeekSpoofer();
			}
			if(old_bittorrent_checksums != m_bittorrent_project_checksums)
			{
				ReassignProjectsForBitTorrent();
			}
			if(old_overnet_checksums != m_overnet_project_checksums)
			{
				ReassignProjectsForOvernetDC();
				ReassignProjectsForOvernetSpoofer();
			}
			if(old_generic_checksums != m_generic_project_checksums)
			{
				ReassignProjectsForGeneric();
			}
			if(old_piolet_spoofer_checksums != m_piolet_spoofer_project_checksums)
			{
				ReassignProjectsForPioletSpoofer();
				LoadBalanceProjectsForPioletSpoofers();
			}
			if(old_bearshare_spoofer_checksums != m_bearshare_spoofer_project_checksums)
			{
				LoadBalanceProjectsForBearShareSpoofers();
			}

			CheckForCollectorsStatus();
			break;
		}
		case 6:
		{
			RestartCrashedRacks();
			break;
		}
		case 7:
		{
			for(UINT i=0; i<v_fasttrack_dc_status.size(); i++)
			{
					SendProjectChecksums(v_fasttrack_dc_status[i]);
			}
			for(UINT i=0; i<v_fasttrack_spoofers_status.size(); i++)
			{
					SendProjectChecksums(v_fasttrack_spoofers_status[i]);
			}
			for(UINT i=0; i<v_fasttrack_decoyers_status.size(); i++)
			{
					SendProjectChecksums(v_fasttrack_decoyers_status[i]);
			}
			for(UINT i=0; i<v_fasttrack_swarmers_status.size(); i++)
			{
					SendProjectChecksums(v_fasttrack_swarmers_status[i]);
			}
			for(UINT i=0; i<v_gnutella_spoofers_splited_status.size(); i++)
			{
					SendProjectChecksums(v_gnutella_spoofers_splited_status[i]);
			}
			for(UINT i=0; i<v_gnutella_protectors_status.size(); i++)
			{
					SendProjectChecksums(v_gnutella_protectors_status[i]);
			}
			for(UINT i=0; i<v_winmx_dc_status.size(); i++)
			{
					SendProjectChecksums(v_winmx_dc_status[i]);
			}
			for(UINT i=0; i<v_winmx_spoofer_status.size(); i++)
			{
					SendProjectChecksums(v_winmx_spoofer_status[i]);
			}
			for(UINT i=0; i<v_winmx_swarmer_status.size(); i++)
			{
					SendProjectChecksums(v_winmx_swarmer_status[i]);
			}
			for(UINT i=0; i<v_winmx_decoyer_status.size(); i++)
			{
					SendProjectChecksums(v_winmx_decoyer_status[i]);
			}
			for(UINT i=0; i<v_soulseek_spoofer_status.size(); i++)
			{
					SendProjectChecksums(v_soulseek_spoofer_status[i]);
			}
			for(UINT i=0; i<v_emule_network_status.size(); i++)
			{
					SendProjectChecksums(v_emule_network_status[i]);
			}
			for(UINT i=0; i<v_gnutella_network_status.size(); i++)
			{
					SendProjectChecksums(v_gnutella_network_status[i]);
			}
			for(UINT i=0; i<v_kazaa_network_status.size(); i++)
			{
					SendProjectChecksums(v_kazaa_network_status[i]);
			}
			for(UINT i=0; i<v_piolet_network_status.size(); i++)
			{
					SendProjectChecksums(v_piolet_network_status[i]);
			}
			for(UINT i=0; i<v_piolet_spoofer_status.size(); i++)
			{
					SendProjectChecksums(v_piolet_spoofer_status[i]);
			}
			for(UINT i=0; i<v_piolet_spoofer_splitted_status.size(); i++)
			{
					SendProjectChecksums(v_piolet_spoofer_splitted_status[i]);
			}
			for(UINT i=0; i<v_direct_connect_status.size(); i++)
			{
					SendProjectChecksums(v_direct_connect_status[i]);
			}
			for(UINT i=0; i<v_direct_connect_spoofer_status.size(); i++)
			{
					SendProjectChecksums(v_direct_connect_spoofer_status[i]);
			}
			for(UINT i=0; i<v_bittorrent_status.size(); i++)
			{
					SendProjectChecksums(v_bittorrent_status[i]);
			}
			for(UINT i=0; i<v_generic_status.size(); i++)
			{
					SendProjectChecksums(v_generic_status[i]);
			}
			for(UINT i=0; i<v_overnet_spoofer_status.size(); i++)
			{
					SendProjectChecksums(v_overnet_spoofer_status[i]);
			}
			for(UINT i=0; i<v_overnet_dc_status.size(); i++)
			{
					SendProjectChecksums(v_overnet_dc_status[i]);
			}
			for(UINT i=0; i<v_bearshare_spoofer_status.size(); i++)
			{
					SendProjectChecksums(v_bearshare_spoofer_status[i]);
			}
			break;
		}
	}
}

//
//
//
void DCMasterDll::ReceivedFastTrackSwarmerInitResponse(char* from)
{

	bool found = false;
	for(UINT i=0;i<v_fasttrack_swarmers_status.size();i++)
	{
		if(v_fasttrack_swarmers_status[i] == from)
		{
			found = true;
			v_fasttrack_swarmers_status[i].m_last_init_response_time = CTime::GetCurrentTime();
			m_dlg.UpdateCollectorStatus(v_fasttrack_swarmers_status[i],"FastTrack Swarmer");
			break;
		}
	}
	if(!found)
	{
		DCStatus status;
		strcpy(status.m_ip,from);
		v_fasttrack_swarmers_status.push_back(status);
		m_dlg.UpdateCollectorStatus(status,"FastTrack Swarmer");
	}
}

//
//
//
void DCMasterDll::ReceivedFastTrackSpooferInitResponse(char* from)
{

	bool found = false;
	for(UINT i=0;i<v_fasttrack_spoofers_status.size();i++)
	{
		if(v_fasttrack_spoofers_status[i] == from)
		{
			found = true;
			v_fasttrack_spoofers_status[i].m_last_init_response_time = CTime::GetCurrentTime();
			m_dlg.UpdateCollectorStatus(v_fasttrack_spoofers_status[i],"FastTrack Spoofer");
			break;
		}
	}
	if(!found)
	{
		DCStatus status;
		strcpy(status.m_ip,from);
		v_fasttrack_spoofers_status.push_back(status);
		m_dlg.UpdateCollectorStatus(status,"FastTrack Spoofer");
	}
}

//
//
//
void DCMasterDll::ReceivedFastTrackDecoyerInitResponse(char* from)
{

	bool found = false;
	for(UINT i=0;i<v_fasttrack_decoyers_status.size();i++)
	{
		if(v_fasttrack_decoyers_status[i] == from)
		{
			found = true;
			v_fasttrack_decoyers_status[i].m_last_init_response_time = CTime::GetCurrentTime();
			m_dlg.UpdateCollectorStatus(v_fasttrack_decoyers_status[i],"FastTrack Decoyer");
			break;
		}
	}
	if(!found)
	{
		DCStatus status;
		strcpy(status.m_ip,from);
		v_fasttrack_decoyers_status.push_back(status);
		m_dlg.UpdateCollectorStatus(status,"FastTrack Decoyer");
	}
}

//
//
//
void DCMasterDll::ReceivedFastTrackDCInitResponse(char* from)
{

	bool found = false;
	for(UINT i=0;i<v_fasttrack_dc_status.size();i++)
	{
		if(v_fasttrack_dc_status[i] == from)
		{
			found = true;
			v_fasttrack_dc_status[i].m_last_init_response_time = CTime::GetCurrentTime();
			m_dlg.UpdateCollectorStatus(v_fasttrack_dc_status[i],"FastTrack DC");
			break;
		}
	}
	if(!found)
	{
		DCStatus status;
		strcpy(status.m_ip,from);
		v_fasttrack_dc_status.push_back(status);
		m_dlg.UpdateCollectorStatus(status,"FastTrack DC");
	}
}

//
//
//
void DCMasterDll::ReceivedDirectConnectInitResponse(char* from)
{

	bool found = false;
	for(UINT i=0;i<v_direct_connect_status.size();i++)
	{
		if(v_direct_connect_status[i] == from)
		{
			found = true;
			v_direct_connect_status[i].m_last_init_response_time = CTime::GetCurrentTime();
			m_dlg.UpdateCollectorStatus(v_direct_connect_status[i],"Direct Connect");
			break;
		}
	}
	if(!found)
	{
		DCStatus status;
		strcpy(status.m_ip,from);
		v_direct_connect_status.push_back(status);
		m_dlg.UpdateCollectorStatus(status,"Direct Connect");
	}
}

//
//
//
void DCMasterDll::ReceivedDirectConnectSpooferInitResponse(char* from)
{

	bool found = false;
	for(UINT i=0;i<v_direct_connect_spoofer_status.size();i++)
	{
		if(v_direct_connect_spoofer_status[i] == from)
		{
			found = true;
			v_direct_connect_spoofer_status[i].m_last_init_response_time = CTime::GetCurrentTime();
			m_dlg.UpdateCollectorStatus(v_direct_connect_spoofer_status[i],"Direct Connect Spoofer");
			break;
		}
	}
	if(!found)
	{
		DCStatus status;
		strcpy(status.m_ip,from);
		v_direct_connect_spoofer_status.push_back(status);
		m_dlg.UpdateCollectorStatus(status,"Direct Connect Spoofer");
	}
}

//
//
//
void DCMasterDll::ReceivedEmuleInitResponse(char* from)
{

	bool found = false;
	for(UINT i=0;i<v_emule_network_status.size();i++)
	{
		if(v_emule_network_status[i] == from)
		{
			found = true;
			v_emule_network_status[i].m_last_init_response_time = CTime::GetCurrentTime();
			m_dlg.UpdateCollectorStatus(v_emule_network_status[i],"Emule");
			break;
		}
	}
	if(!found)
	{
		DCStatus status;
		strcpy(status.m_ip,from);
		v_emule_network_status.push_back(status);
		m_dlg.UpdateCollectorStatus(status,"Emule");
	}
}

//
//
//
void DCMasterDll::ReceivedGnutellaInitResponse(char* from)
{
	bool found = false;
	for(UINT i=0;i<v_gnutella_network_status.size();i++)
	{
		if(v_gnutella_network_status[i] == from)
		{
			found = true;
			v_gnutella_network_status[i].m_last_init_response_time = CTime::GetCurrentTime();
			m_dlg.UpdateCollectorStatus(v_gnutella_network_status[i],"Gnutella");
			break;
		}
	}
	if(!found)
	{
		DCStatus status;
		strcpy(status.m_ip,from);
		v_gnutella_network_status.push_back(status);
		m_dlg.UpdateCollectorStatus(status,"Gnutella");
	}
}

//
//
//
void DCMasterDll::ReceivedKazaaInitResponse(char* from)
{
	bool found = false;
	for(UINT i=0;i<v_kazaa_network_status.size();i++)
	{
		if(v_kazaa_network_status[i] == from)
		{
			found = true;
			v_kazaa_network_status[i].m_last_init_response_time = CTime::GetCurrentTime();
			m_dlg.UpdateCollectorStatus(v_kazaa_network_status[i],"Kazaa");
			break;
		}
	}
	if(!found)
	{
		DCStatus status;
		strcpy(status.m_ip,from);
		v_kazaa_network_status.push_back(status);
		m_dlg.UpdateCollectorStatus(status,"Kazaa");
	}

	/*
	//send kazaa controller ip
	vector<string> kazaa_controller_ip;
	m_nameserver.RequestIP("KAZAA-CONTROLLER",kazaa_controller_ip);
	if(kazaa_controller_ip.size()!=0)
	{
		byte* buf = new byte[sizeof(DCHeader)+(UINT)kazaa_controller_ip[0].length()+1];
		DCHeader* header = (DCHeader*)buf;
		header->op = DCHeader::KazaaControllerIP;
		header->size = (UINT)kazaa_controller_ip[0].length()+1;
		char* ip = new char[kazaa_controller_ip[0].length()+1];
		strcpy(ip, kazaa_controller_ip[0].c_str());
		memcpy(&buf[sizeof(DCHeader)], ip, kazaa_controller_ip[0].length()+1);
		delete ip;
		m_com.SendReliableData(from, buf, sizeof(DCHeader)+(UINT)kazaa_controller_ip[0].length()+1);
		delete buf;
	}
	*/
}

//
//
//
void DCMasterDll::ReceivedPioletInitResponse(char* from)
{
	bool found = false;
	for(UINT i=0;i<v_piolet_network_status.size();i++)
	{
		if(v_piolet_network_status[i] == from)
		{
			found = true;
			v_piolet_network_status[i].m_last_init_response_time = CTime::GetCurrentTime();
			m_dlg.UpdateCollectorStatus(v_piolet_network_status[i],"Piolet");
			break;
		}
	}
	if(!found)
	{
		DCStatus status;
		strcpy(status.m_ip,from);
		v_piolet_network_status.push_back(status);
		m_dlg.UpdateCollectorStatus(status,"Piolet");
	}
}

//
//
//
void DCMasterDll::ReceivedPioletSpooferInitResponse(char* from)
{
	bool found = false;
	for(UINT i=0;i<v_piolet_spoofer_status.size();i++)
	{
		if(v_piolet_spoofer_status[i] == from)
		{
			found = true;
			v_piolet_spoofer_status[i].m_last_init_response_time = CTime::GetCurrentTime();
			m_dlg.UpdateCollectorStatus(v_piolet_spoofer_status[i],"Piolet Spoofer");
			break;
		}
	}
	if(!found)
	{
		DCStatus status;
		strcpy(status.m_ip,from);
		v_piolet_spoofer_status.push_back(status);
		m_dlg.UpdateCollectorStatus(status,"Piolet Spoofer");
	}
}

//
//
//
void DCMasterDll::ReceivedPioletSpooferSplittedInitResponse(char* from)
{
	bool found = false;
	for(UINT i=0;i<v_piolet_spoofer_splitted_status.size();i++)
	{
		if(v_piolet_spoofer_splitted_status[i] == from)
		{
			found = true;
			v_piolet_spoofer_splitted_status[i].m_last_init_response_time = CTime::GetCurrentTime();
			m_dlg.UpdateCollectorStatus(v_piolet_spoofer_splitted_status[i],"Piolet Spoofer Splitted");
			break;
		}
	}
	if(!found)
	{
		DCStatus status;
		strcpy(status.m_ip,from);
		v_piolet_spoofer_splitted_status.push_back(status);
		m_dlg.UpdateCollectorStatus(status,"Piolet Spoofer Splitted");
	}
}

//
//
//
void DCMasterDll::ReceivedGnutellaProtectorInitResponse(char* from)
{
	bool found = false;
	for(UINT i=0;i<v_gnutella_protectors_status.size();i++)
	{
		if(v_gnutella_protectors_status[i] == from)
		{
			found = true;
			v_gnutella_protectors_status[i].m_last_init_response_time = CTime::GetCurrentTime();
			m_dlg.UpdateCollectorStatus(v_gnutella_protectors_status[i],"Gnutella Protector");
			break;
		}
	}
	if(!found)
	{
		DCStatus status;
		strcpy(status.m_ip,from);
		v_gnutella_protectors_status.push_back(status);
		m_dlg.UpdateCollectorStatus(status,"Gnutella Protector");
	}
}

//
//
//
void DCMasterDll::ReceivedGnutellaSpoofersSplitedInitResponse(char* from)
{
	bool found = false;
	for(UINT i=0;i<v_gnutella_spoofers_splited_status.size();i++)
	{
		if(v_gnutella_spoofers_splited_status[i] == from)
		{
			found = true;
			v_gnutella_spoofers_splited_status[i].m_last_init_response_time = CTime::GetCurrentTime();
			m_dlg.UpdateCollectorStatus(v_gnutella_spoofers_splited_status[i],"Gnutella Spoofer Splitted");
			break;
		}
	}
	if(!found)
	{
		DCStatus status;
		strcpy(status.m_ip,from);
		v_gnutella_spoofers_splited_status.push_back(status);
		m_dlg.UpdateCollectorStatus(status,"Gnutella Spoofer Splitted");
	}
}

//
//
//
void DCMasterDll::ReceivedWinmxDCInitResponse(char* from)
{

	bool found = false;
	for(UINT i=0;i<v_winmx_dc_status.size();i++)
	{
		if(v_winmx_dc_status[i] == from)
		{
			found = true;
			v_winmx_dc_status[i].m_last_init_response_time = CTime::GetCurrentTime();
			m_dlg.UpdateCollectorStatus(v_winmx_dc_status[i],"WinMX DC");
			break;
		}
	}
	if(!found)
	{
		DCStatus status;
		strcpy(status.m_ip,from);
		v_winmx_dc_status.push_back(status);
		m_dlg.UpdateCollectorStatus(status,"WinMX DC");
	}
}

//
//
//
void DCMasterDll::ReceivedWinmxSpooferInitResponse(char* from)
{

	bool found = false;
	for(UINT i=0;i<v_winmx_spoofer_status.size();i++)
	{
		if(v_winmx_spoofer_status[i] == from)
		{
			found = true;
			v_winmx_spoofer_status[i].m_last_init_response_time = CTime::GetCurrentTime();
			m_dlg.UpdateCollectorStatus(v_winmx_spoofer_status[i],"WinMX Spoofer");
			break;
		}
	}
	if(!found)
	{
		DCStatus status;
		strcpy(status.m_ip,from);
		v_winmx_spoofer_status.push_back(status);
		m_dlg.UpdateCollectorStatus(status,"WinMX Spoofer");
	}
}

//
//
//
void DCMasterDll::ReceivedWinmxSwarmerInitResponse(char* from)
{

	bool found = false;
	for(UINT i=0;i<v_winmx_swarmer_status.size();i++)
	{
		if(v_winmx_swarmer_status[i] == from)
		{
			found = true;
			v_winmx_swarmer_status[i].m_last_init_response_time = CTime::GetCurrentTime();
			m_dlg.UpdateCollectorStatus(v_winmx_swarmer_status[i],"WinMX Swarmer");
			break;
		}
	}
	if(!found)
	{
		DCStatus status;
		strcpy(status.m_ip,from);
		v_winmx_swarmer_status.push_back(status);
		m_dlg.UpdateCollectorStatus(status,"WinMX Swarmer");
	}
}

//
//
//
void DCMasterDll::ReceivedWinmxDecoyerInitResponse(char* from)
{

	bool found = false;
	for(UINT i=0;i<v_winmx_decoyer_status.size();i++)
	{
		if(v_winmx_decoyer_status[i] == from)
		{
			found = true;
			v_winmx_decoyer_status[i].m_last_init_response_time = CTime::GetCurrentTime();
			m_dlg.UpdateCollectorStatus(v_winmx_decoyer_status[i],"WinMX Decoyer");
			break;
		}
	}
	if(!found)
	{
		DCStatus status;
		strcpy(status.m_ip,from);
		v_winmx_decoyer_status.push_back(status);
		m_dlg.UpdateCollectorStatus(status,"WinMX Decoyer");
	}
}

//
//
//
void DCMasterDll::ReceivedSoulSeekSpooferInitResponse(char* from)
{

	bool found = false;
	for(UINT i=0;i<v_soulseek_spoofer_status.size();i++)
	{
		if(v_soulseek_spoofer_status[i] == from)
		{
			found = true;
			v_soulseek_spoofer_status[i].m_last_init_response_time = CTime::GetCurrentTime();
			m_dlg.UpdateCollectorStatus(v_soulseek_spoofer_status[i],"SoulSeek Spoofer");
			break;
		}
	}
	if(!found)
	{
		DCStatus status;
		strcpy(status.m_ip,from);
		v_soulseek_spoofer_status.push_back(status);
		m_dlg.UpdateCollectorStatus(status,"SoulSeek Spoofer");
	}
}

//
//
//
void DCMasterDll::ReceivedBitTorrentInitResponse(char* from)
{

	bool found = false;
	for(UINT i=0;i<v_bittorrent_status.size();i++)
	{
		if(v_bittorrent_status[i] == from)
		{
			found = true;
			v_bittorrent_status[i].m_last_init_response_time = CTime::GetCurrentTime();
			m_dlg.UpdateCollectorStatus(v_bittorrent_status[i],"BitTorrent");
			break;
		}
	}
	if(!found)
	{
		DCStatus status;
		strcpy(status.m_ip,from);
		v_bittorrent_status.push_back(status);
		m_dlg.UpdateCollectorStatus(status,"BitTorrent");
	}
}

//
//
//
void DCMasterDll::ReceivedOvernetDCInitResponse(char* from)
{

	bool found = false;
	for(UINT i=0;i<v_overnet_dc_status.size();i++)
	{
		if(v_overnet_dc_status[i] == from)
		{
			found = true;
			v_overnet_dc_status[i].m_last_init_response_time = CTime::GetCurrentTime();
			m_dlg.UpdateCollectorStatus(v_overnet_dc_status[i],"Overnet DC");
			break;
		}
	}
	if(!found)
	{
		DCStatus status;
		strcpy(status.m_ip,from);
		v_overnet_dc_status.push_back(status);
		m_dlg.UpdateCollectorStatus(status,"Overnet DC");
	}
}

//
//
//
void DCMasterDll::ReceivedOvernetSpooferInitResponse(char* from)
{

	bool found = false;
	for(UINT i=0;i<v_overnet_spoofer_status.size();i++)
	{
		if(v_overnet_spoofer_status[i] == from)
		{
			found = true;
			v_overnet_spoofer_status[i].m_last_init_response_time = CTime::GetCurrentTime();
			m_dlg.UpdateCollectorStatus(v_overnet_spoofer_status[i],"Overnet Spoofer");
			break;
		}
	}
	if(!found)
	{
		DCStatus status;
		strcpy(status.m_ip,from);
		v_overnet_spoofer_status.push_back(status);
		m_dlg.UpdateCollectorStatus(status,"Overnet Spoofer");
	}
}

//
//
//
void DCMasterDll::ReceivedBearshareSpooferInitResponse(char* from)
{
	bool found = false;
	for(UINT i=0;i<v_bearshare_spoofer_status.size();i++)
	{
		if(v_bearshare_spoofer_status[i] == from)
		{
			found = true;
			v_bearshare_spoofer_status[i].m_last_init_response_time = CTime::GetCurrentTime();
			m_dlg.UpdateCollectorStatus(v_bearshare_spoofer_status[i],"BearShare Spoofer");
			break;
		}
	}
	if(!found)
	{
		DCStatus status;
		strcpy(status.m_ip,from);
		v_bearshare_spoofer_status.push_back(status);
		m_dlg.UpdateCollectorStatus(status,"BearShare Spoofer");
	}
}

//
//
//
void DCMasterDll::ReceivedGenericInitResponse(char* from)
{

	bool found = false;
	for(UINT i=0;i<v_generic_status.size();i++)
	{
		if(v_generic_status[i] == from)
		{
			found = true;
			v_generic_status[i].m_last_init_response_time = CTime::GetCurrentTime();
			m_dlg.UpdateCollectorStatus(v_generic_status[i],"Generic");
			break;
		}
	}
	if(!found)
	{
		DCStatus status;
		strcpy(status.m_ip,from);
		v_generic_status.push_back(status);
		m_dlg.UpdateCollectorStatus(status,"Generic");
	}
}

//
//
//
void DCMasterDll::SendRequestedProjects(char* dest, ProjectChecksums requested_projects)
{
	UINT i;
	ProjectKeywordsVector projects;
	int version = UsingProjectKeywordsVersion(dest);
	for(i=0; i<requested_projects.v_checksums.size(); i++)
	{
		switch(version)
		{
			case 0:
			{
				for(UINT j=0; j<v_project_data.size(); j++)
				{
					if(requested_projects.v_checksums[i].m_project_name == v_project_data[j].m_project_name)
					{
						//Check for checksum
						ProjectChecksum pc = v_project_data[j].CalculateChecksum();
						if(requested_projects.v_checksums[i] == pc)
							projects.v_projects.push_back(v_project_data[j]);
					}
				}
				break;
			}
			case 1:
			{
				for(UINT j=0; j<v_gift_dc_project_data.size(); j++)
				{
					if(requested_projects.v_checksums[i].m_project_name == v_gift_dc_project_data[j].m_project_name)
					{
						//Check for checksum
						ProjectChecksum pc = v_gift_dc_project_data[j].CalculateChecksum();
						if(requested_projects.v_checksums[i] == pc)
							projects.v_projects.push_back(v_gift_dc_project_data[j]);
					}
				}
				break;
			}
			case 2:
			{
				for(UINT j=0; j<v_gift_spoofers_project_data.size(); j++)
				{
					if(requested_projects.v_checksums[i].m_project_name == v_gift_spoofers_project_data[j].m_project_name)
					{
						//Check for checksum
						ProjectChecksum pc = v_gift_spoofers_project_data[j].CalculateChecksum();
						if(requested_projects.v_checksums[i] == pc)
							projects.v_projects.push_back(v_gift_spoofers_project_data[j]);
					}
				}
				break;
			}
			case 3:
			{
				for(UINT j=0; j<v_gift_swarmers_project_data.size(); j++)
				{
					if(requested_projects.v_checksums[i].m_project_name == v_gift_swarmers_project_data[j].m_project_name)
					{
						//Check for checksum
						ProjectChecksum pc = v_gift_swarmers_project_data[j].CalculateChecksum();
						if(requested_projects.v_checksums[i] == pc)
							projects.v_projects.push_back(v_gift_swarmers_project_data[j]);
					}
				}
				break;
			}
			case 4:
			{
				bool found =false;
				for(UINT j=0; j<v_active_gnutella_protection_projects.size(); j++)
				{
					if(requested_projects.v_checksums[i].m_project_name == v_active_gnutella_protection_projects[j].m_project_name)
					{
						found=true;
						//Check for checksum
						ProjectChecksum pc = v_active_gnutella_protection_projects[j].CalculateChecksum();
						if(requested_projects.v_checksums[i] == pc)
							projects.v_projects.push_back(v_active_gnutella_protection_projects[j]);
					}
				}
				if(!found)
				{
					for(UINT j=0; j<v_singles_project_data.size(); j++)
					{
						if(requested_projects.v_checksums[i].m_project_name == v_singles_project_data[j].m_project_name)
						{
							//Check for checksum
							ProjectChecksum pc = v_singles_project_data[j].CalculateChecksum();
							if(requested_projects.v_checksums[i] == pc)
								projects.v_projects.push_back(v_singles_project_data[j]);
						}
					}
				}
				break;
			}
			case 5:
			{
				for(UINT j=0; j<v_regular_active_gnutella_protection_projects.size(); j++)
				{
					if(requested_projects.v_checksums[i].m_project_name == v_regular_active_gnutella_protection_projects[j].m_project_name)
					{
						//Check for checksum
						ProjectChecksum pc = v_regular_active_gnutella_protection_projects[j].CalculateChecksum();
						if(requested_projects.v_checksums[i] == pc)
							projects.v_projects.push_back(v_regular_active_gnutella_protection_projects[j]);
					}
				}
				break;
			}
			case 6: //winmx
			{
				for(UINT j=0; j<v_winmx_project_data.size(); j++)
				{
					if(requested_projects.v_checksums[i].m_project_name == v_winmx_project_data[j].m_project_name)
					{
						//Check for checksum
						ProjectChecksum pc = v_winmx_project_data[j].CalculateChecksum();
						if(requested_projects.v_checksums[i] == pc)
							projects.v_projects.push_back(v_winmx_project_data[j]);
					}
				}
				break;
			}
			case 7: //soulseek
			{
				for(UINT j=0; j<v_soulseek_project_data.size(); j++)
				{
					if(requested_projects.v_checksums[i].m_project_name == v_soulseek_project_data[j].m_project_name)
					{
						//Check for checksum
						ProjectChecksum pc = v_soulseek_project_data[j].CalculateChecksum();
						if(requested_projects.v_checksums[i] == pc)
							projects.v_projects.push_back(v_soulseek_project_data[j]);
					}
				}
				break;
			}
			case 8: //bittorrent
			{
				for(UINT j=0; j<v_bittorrent_project_data.size(); j++)
				{
					if(requested_projects.v_checksums[i].m_project_name == v_bittorrent_project_data[j].m_project_name)
					{
						//Check for checksum
						ProjectChecksum pc = v_bittorrent_project_data[j].CalculateChecksum();
						if(requested_projects.v_checksums[i] == pc)
							projects.v_projects.push_back(v_bittorrent_project_data[j]);
					}
				}
				break;
			}
			case 9: //Overnet
			{
				for(UINT j=0; j<v_overnet_project_data.size(); j++)
				{
					if(requested_projects.v_checksums[i].m_project_name == v_overnet_project_data[j].m_project_name)
					{
						//Check for checksum
						ProjectChecksum pc = v_overnet_project_data[j].CalculateChecksum();
						if(requested_projects.v_checksums[i] == pc)
							projects.v_projects.push_back(v_overnet_project_data[j]);
					}
				}
				break;
			}
			case 10: //Generic
			{
				for(UINT j=0; j<v_generic_project_data.size(); j++)
				{
					if(requested_projects.v_checksums[i].m_project_name == v_generic_project_data[j].m_project_name)
					{
						//Check for checksum
						ProjectChecksum pc = v_generic_project_data[j].CalculateChecksum();
						if(requested_projects.v_checksums[i] == pc)
							projects.v_projects.push_back(v_generic_project_data[j]);
					}
				}
				break;
			}
			case 11: //piolet splitted spoofer
			{
				for(UINT j=0; j<v_piolet_spoofer_project_data.size(); j++)
				{
					if(requested_projects.v_checksums[i].m_project_name == v_piolet_spoofer_project_data[j].m_project_name)
					{
						//Check for checksum
						ProjectChecksum pc = v_piolet_spoofer_project_data[j].CalculateChecksum();
						if(requested_projects.v_checksums[i] == pc)
							projects.v_projects.push_back(v_piolet_spoofer_project_data[j]);
					}
				}
				break;
			}
			case 12: //bearshare
			{
				for(UINT j=0; j<v_bearshare_spoofer_project_data.size(); j++)
				{
					if(requested_projects.v_checksums[i].m_project_name == v_bearshare_spoofer_project_data[j].m_project_name)
					{
						//Check for checksum
						ProjectChecksum pc = v_bearshare_spoofer_project_data[j].CalculateChecksum();
						if(requested_projects.v_checksums[i] == pc)
							projects.v_projects.push_back(v_bearshare_spoofer_project_data[j]);
					}
				}
				break;
			}
		}
	}

	int buf_len = sizeof(DCHeader) + projects.GetBufferLength();
	char* buf = new char[buf_len];

	DCHeader* header = (DCHeader*)buf;
	header->op = DCHeader::ProjectKeywords;
	header->size = projects.GetBufferLength();

	projects.WriteToBuffer(&buf[sizeof(DCHeader)]);

	m_com.SendReliableData(dest, buf, buf_len);
	delete [] buf;
}

//
//
//
//void DCMasterDll::ReassignProjectsForFastTrackSwarmersRacksDown(vector<ProjectKeywords>& v_projects)
//{
//	m_dlg.ClearProjectsNumber("FastTrack Swarmer");
//	UINT i,j;
//	for(i=0;i<v_projects.size();i++)
//	{
//		vector<UINT> assigned_collectors;
//		bool found_rack_to_assign=false;
//		while(!found_rack_to_assign)
//		{
//			if(assigned_collectors.size() == v_fasttrack_swarmers_status.size())
//				break; // all racks already have this project
//			int index = GetLeastNumberOfProjects(assigned_collectors,v_fasttrack_swarmers_status);
//			bool found=false;
//			if(index >= 0)
//			{
//				for(j=0;j<v_fasttrack_swarmers_status[index].v_projects.size();j++)
//				{
//					if(v_fasttrack_swarmers_status[index].v_projects[j].m_id==v_projects[i].m_id)
//					{
//						found=true;
//						break;
//					}
//				}
//				assigned_collectors.push_back(index);
//				if(!found)
//				{
//					v_fasttrack_swarmers_status[index].v_projects.push_back(v_projects[i]);
//					found_rack_to_assign=true;
//				}
//			}
//			else
//				break;
//		}
//		/*
//		int index = FindMinimumProjectsGiftSwarmerRack();
//        v_fasttrack_swarmers_status[index].v_projects.push_back(v_projects[i]);
//		*/
//
//	}
//	for(i=0; i<v_fasttrack_swarmers_status.size(); i++)
//	{
//		SendProjectChecksums(v_fasttrack_swarmers_status[i]);
//		m_dlg.UpdateCollectorStatus(v_fasttrack_swarmers_status[i],"FastTrack Swarmer");
//	}
//	m_browse_projects_dlg.BuildProjectTree("Gift Swarmer",v_fasttrack_swarmers_status);
//}
//
////
////
////
//void DCMasterDll::ReassignProjectsForFastTrackSpoofersRacksDown(vector<ProjectKeywords>& v_projects)
//{
//	m_dlg.ClearProjectsNumber("FastTrack Spoofer");
//	UINT i,j;
//	for(i=0;i<v_projects.size();i++)
//	{
//		vector<UINT> assigned_collectors;
//		bool found_rack_to_assign=false;
//		while(!found_rack_to_assign)
//		{
//			if(assigned_collectors.size() == v_fasttrack_spoofers_status.size())
//				break; // all racks already have this project
//			int index = GetLeastNumberOfProjects(assigned_collectors,v_fasttrack_spoofers_status);
//			if(index>=0)
//			{
//				bool found=false;
//				for(j=0;j<v_fasttrack_spoofers_status[index].v_projects.size();j++)
//				{
//					if(v_fasttrack_spoofers_status[index].v_projects[j].m_id==v_projects[i].m_id)
//					{
//						found=true;
//						break;
//					}
//				}
//				assigned_collectors.push_back(index);
//				if(!found)
//				{
//					v_fasttrack_spoofers_status[index].v_projects.push_back(v_projects[i]);
//					found_rack_to_assign=true;
//				}
//			}
//			else
//				break;
//		}
//		/*
//		int index = FindMinimumProjectsGiftSwarmerRack();
//        v_fasttrack_swarmers_status[index].v_projects.push_back(v_projects[i]);
//		*/
//
//	}
//	for(i=0; i<v_fasttrack_spoofers_status.size(); i++)
//	{
//		SendProjectChecksums(v_fasttrack_spoofers_status[i]);
//		m_dlg.UpdateCollectorStatus(v_fasttrack_spoofers_status[i],"FastTrack Spoofer");
//	}
//	m_browse_projects_dlg.BuildProjectTree("Gift Spoofer",v_fasttrack_spoofers_status);
//}
//
////
////
////
//void DCMasterDll::ReassignProjectsForFastTrackDecoyersRacksDown(vector<ProjectKeywords>& v_projects)
//{
//	m_dlg.ClearProjectsNumber("FastTrack Decoyer");
//	UINT i,j;
//	for(i=0;i<v_projects.size();i++)
//	{
//		vector<UINT> assigned_collectors;
//		bool found_rack_to_assign=false;
//		while(!found_rack_to_assign)
//		{
//			if(assigned_collectors.size() == v_fasttrack_decoyers_status.size())
//				break; // all racks already have this project
//			int index = GetLeastNumberOfProjects(assigned_collectors,v_fasttrack_decoyers_status);
//			if(index>=0)
//			{
//				bool found=false;
//				for(j=0;j<v_fasttrack_decoyers_status[index].v_projects.size();j++)
//				{
//					if(v_fasttrack_decoyers_status[index].v_projects[j].m_id==v_projects[i].m_id)
//					{
//						found=true;
//						break;
//					}
//				}
//				assigned_collectors.push_back(index);
//				if(!found)
//				{
//					v_fasttrack_decoyers_status[index].v_projects.push_back(v_projects[i]);
//					found_rack_to_assign=true;
//				}
//			}
//			else
//				break;
//		}
//		/*
//		int index = FindMinimumProjectsGiftSwarmerRack();
//        v_fasttrack_swarmers_status[index].v_projects.push_back(v_projects[i]);
//		*/
//
//	}
//	for(i=0; i<v_fasttrack_decoyers_status.size(); i++)
//	{
//		SendProjectChecksums(v_fasttrack_decoyers_status[i]);
//		m_dlg.UpdateCollectorStatus(v_fasttrack_decoyers_status[i],"FastTrack Decoyer");
//	}
//	m_browse_projects_dlg.BuildProjectTree("Gift Decoyer",v_fasttrack_decoyers_status);
//}
//
////
////
////
//void DCMasterDll::ReassignProjectsForGnutellaSpoofersSplitedRacksDown(vector<ProjectKeywords>& v_projects)
//{
//	m_dlg.ClearProjectsNumber("Gnutella Spoofer Splitted");
//	UINT i,j;
//	for(i=0;i<v_projects.size();i++)
//	{
//		vector<UINT> assigned_collectors;
//		bool found_rack_to_assign=false;
//		while(!found_rack_to_assign)
//		{
//			if(assigned_collectors.size() == v_gnutella_spoofers_splited_status.size())
//				break; // all racks already have this project
//			int index = GetLeastNumberOfProjects(assigned_collectors,v_gnutella_spoofers_splited_status);
//			bool found=false;
//			if(index >= 0)
//			{
//				for(j=0;j<v_gnutella_spoofers_splited_status[index].v_projects.size();j++)
//				{
//					if(v_gnutella_spoofers_splited_status[index].v_projects[j].m_id==v_projects[i].m_id)
//					{
//						found=true;
//						break;
//					}
//				}
//				assigned_collectors.push_back(index);
//				if(!found)
//				{
//					v_gnutella_spoofers_splited_status[index].v_projects.push_back(v_projects[i]);
//					found_rack_to_assign=true;
//				}
//			}
//			else
//				break;
//		}
//	}
//	for(i=0; i<v_gnutella_spoofers_splited_status.size(); i++)
//	{
//		SendProjectChecksums(v_gnutella_spoofers_splited_status[i]);
//		m_dlg.UpdateCollectorStatus(v_gnutella_spoofers_splited_status[i],"Gnutella Spoofer Splitted");
//	}
//	m_browse_projects_dlg.BuildProjectTree("Gnutella Spoofer Splitted",v_gnutella_spoofers_splited_status);
//}

//
//
//
//void DCMasterDll::ReassignUpdatedProjectsForFastTrackSwarmers()
//{
//	UINT i,j,k;
//	m_dlg.ClearProjectsNumber("FastTrack Swarmer");
//
//	//remove the removed projects from the protectors
//	for(i=0; i<v_fasttrack_swarmers_status.size(); i++)
//	{
//		vector<ProjectKeywords>::iterator iter = v_fasttrack_swarmers_status[i].v_projects.begin();
//		while(iter!=v_fasttrack_swarmers_status[i].v_projects.end())
//		{
//			bool found=false;
//			for(j=0;j<v_gift_swarmers_project_data.size();j++)
//			{
//				if(iter->m_id == v_gift_swarmers_project_data[j].m_id)
//				{
//					found=true;
//					break;
//				}
//			}
//			if(!found)
//				v_fasttrack_swarmers_status[i].v_projects.erase(iter);
//			else
//				iter++;
//		}
//	}
//
//	//update the projects from the protectors
//	if(v_fasttrack_swarmers_status.size()>0)
//	{
//		for(i=0; i<v_gift_swarmers_project_data.size(); i++)
//		{
//			bool found = false;
//			for(j=0; j<v_fasttrack_swarmers_status.size(); j++)
//			{
//				for(k=0;k<v_fasttrack_swarmers_status[j].v_projects.size();k++)
//				{
//					if(v_gift_swarmers_project_data[i].m_id == v_fasttrack_swarmers_status[j].v_projects[k].m_id)
//					{
//						found=true;
//						v_fasttrack_swarmers_status[j].v_projects[k] = v_gift_swarmers_project_data[i];
//						break;
//					}
//				}
//
//			}
//			if(found) //we need to see if the kazaa swarm load got changed
//			{
//				vector<UINT>assigned_collectors;
//				int num_racks_supposed_to_assigned = (int)((((float)v_gift_swarmers_project_data[i].m_kazaa_swarm_load / (float)100)*(float)v_fasttrack_swarmers_status.size())/m_gift_swarmer_load_balancing_divider);
//				if(num_racks_supposed_to_assigned <= 0)
//					num_racks_supposed_to_assigned = 1;
//				int num_racks_actually_assigned = GetNumRacksAssignedForProject(v_gift_swarmers_project_data[i].m_id, v_fasttrack_swarmers_status,assigned_collectors);
//				if(num_racks_supposed_to_assigned > num_racks_actually_assigned) //need to assign more racks to this project
//				{
//					for(j=0;j<(num_racks_supposed_to_assigned-num_racks_actually_assigned);j++)
//					{
//						int index = GetLeastNumberOfProjects(assigned_collectors,v_fasttrack_swarmers_status);
//						if(index>=0)
//						{
//							assigned_collectors.push_back(index);
//							v_fasttrack_swarmers_status[index].v_projects.push_back(v_gift_swarmers_project_data[i]);
//						}
//						else
//							break;
//					}
//				}
//				else if(num_racks_supposed_to_assigned < num_racks_actually_assigned)// need to remove some racks from this project
//				{
//					for(j=0;j<(num_racks_actually_assigned-num_racks_supposed_to_assigned);j++)
//					{
//						if(j<assigned_collectors.size())
//						{
//							if(assigned_collectors[j] < v_fasttrack_swarmers_status.size())
//							{
//								vector<ProjectKeywords>::iterator iter = v_fasttrack_swarmers_status[assigned_collectors[j]].v_projects.begin();
//								while(iter!=v_fasttrack_swarmers_status[assigned_collectors[j]].v_projects.end())
//								{
//									if(iter->m_id == v_gift_swarmers_project_data[i].m_id)
//									{
//										v_fasttrack_swarmers_status[assigned_collectors[j]].v_projects.erase(iter);
//										break;
//									}
//									iter++;
//								}
//							}
//						}
//					}
//				}
//			}
//			else //add this new project
//			{
//				vector<UINT>assigned_collectors;
//
//				int num_racks_to_use = (int)((((float)v_gift_swarmers_project_data[i].m_kazaa_swarm_load / (float)100)*(float)v_fasttrack_swarmers_status.size())/m_gift_swarmer_load_balancing_divider);
//				if(num_racks_to_use<=0)
//					num_racks_to_use=1;
//				for(j=0;j<num_racks_to_use;j++)
//				{
//					int index = GetLeastNumberOfProjects(assigned_collectors,v_fasttrack_swarmers_status);
//					if(index >= 0)
//					{
//						assigned_collectors.push_back(index);
//						v_fasttrack_swarmers_status[index].v_projects.push_back(v_gift_swarmers_project_data[i]);
//					}
//					else
//						break;
//				}
//				/*
//				int index=FindMinimumProjectsGiftSwarmerRack();
//                v_fasttrack_swarmers_status[index].v_projects.push_back(v_gift_swarmers_project_data[i]);
//				*/
//			}
//		}
//		for(i=0; i<v_fasttrack_swarmers_status.size(); i++)
//		{
//			SendProjectChecksums(v_fasttrack_swarmers_status[i]);
//			m_dlg.UpdateCollectorStatus(v_fasttrack_swarmers_status[i],"FastTrack Swarmer");
//		}
//	}
//	m_browse_projects_dlg.BuildProjectTree("Gift Swarmer",v_fasttrack_swarmers_status);
//}
//
////
////
////
//void DCMasterDll::ReassignUpdatedProjectsForGnutellaSpoofersSplited()
//{
//	UINT i,j,k;
//	m_dlg.ClearProjectsNumber("Gnutella Spoofer Splitted");
//
//	//remove the removed projects from the protectors
//	for(i=0; i<v_gnutella_spoofers_splited_status.size(); i++)
//	{
//		vector<ProjectKeywords>::iterator iter = v_gnutella_spoofers_splited_status[i].v_projects.begin();
//		while(iter!=v_gnutella_spoofers_splited_status[i].v_projects.end())
//		{
//			bool found=false;
//			for(j=0;j<v_active_gnutella_protection_projects.size();j++)
//			{
//				if(iter->m_id == v_active_gnutella_protection_projects[j].m_id)
//				{
//					found=true;
//					break;
//				}
//			}
//			//remove the removed singles projects from the protectors
//			if(!found)
//			{
//				for(j=0;j<v_singles_project_data.size();j++)
//				{
//					if(iter->m_id == v_singles_project_data[j].m_id)
//					{
//						found=true;
//						break;
//					}
//				}
//			}
//
//			if(!found)
//				v_gnutella_spoofers_splited_status[i].v_projects.erase(iter);
//			else
//				iter++;
//		}
//	}
//
//	//update the projects from the protectors
//	if(v_gnutella_spoofers_splited_status.size()>0)
//	{
//		for(i=0; i<v_active_gnutella_protection_projects.size(); i++)
//		{
//			if(v_active_gnutella_protection_projects[i].m_id != 2903 )//not Common Words project
//			{
//				bool found = false;
//				for(j=0; j<v_gnutella_spoofers_splited_status.size(); j++)
//				{
//					for(k=0;k<v_gnutella_spoofers_splited_status[j].v_projects.size();k++)
//					{
//						if(v_active_gnutella_protection_projects[i].m_id == v_gnutella_spoofers_splited_status[j].v_projects[k].m_id)
//						{
//							found=true;
//							v_gnutella_spoofers_splited_status[j].v_projects[k] = v_active_gnutella_protection_projects[i];
//							break;
//						}
//					}
//
//				}
//				if(found) //we need to see if the kazaa swarm load got changed
//				{
//					vector<UINT>assigned_collectors;
//					int num_racks_supposed_to_assigned = ((int)(((float)v_active_gnutella_protection_projects[i].m_gnutella_load / (float)100)*(float)v_gnutella_spoofers_splited_status.size()))/m_gnutella_load_balancing_divider;
//					if(num_racks_supposed_to_assigned <= 0)
//						num_racks_supposed_to_assigned = 1;
//					int num_racks_actually_assigned = GetNumRacksAssignedForProject(v_active_gnutella_protection_projects[i].m_id, v_gnutella_spoofers_splited_status,assigned_collectors);
//					if(num_racks_supposed_to_assigned > num_racks_actually_assigned) //need to assign more racks to this project
//					{
//						for(j=0;j<(num_racks_supposed_to_assigned-num_racks_actually_assigned);j++)
//						{
//							int index = GetLeastNumberOfProjects(assigned_collectors,v_gnutella_spoofers_splited_status);
//							if(index>=0)
//							{
//								assigned_collectors.push_back(index);
//								v_gnutella_spoofers_splited_status[index].v_projects.push_back(v_active_gnutella_protection_projects[i]);
//							}
//							else
//								break;
//						}
//					}
//					else if(num_racks_supposed_to_assigned < num_racks_actually_assigned)// need to remove some racks from this project
//					{
//						for(j=0;j<(num_racks_actually_assigned-num_racks_supposed_to_assigned);j++)
//						{
//							if(j<assigned_collectors.size())
//							{
//								if(assigned_collectors[j] < v_gnutella_spoofers_splited_status.size())
//								{
//									vector<ProjectKeywords>::iterator iter = v_gnutella_spoofers_splited_status[assigned_collectors[j]].v_projects.begin();
//									while(iter!=v_gnutella_spoofers_splited_status[assigned_collectors[j]].v_projects.end())
//									{
//										if(iter->m_id == v_active_gnutella_protection_projects[i].m_id)
//										{
//											v_gnutella_spoofers_splited_status[assigned_collectors[j]].v_projects.erase(iter);
//											break;
//										}
//										iter++;
//									}
//								}
//							}
//						}
//					}
//				}
//				else //add this new project
//				{
//					vector<UINT>assigned_collectors;
//
//					int num_racks_to_use = ((int)(((float)v_active_gnutella_protection_projects[i].m_gnutella_load / (float)100)*(float)v_gnutella_spoofers_splited_status.size()))/m_gnutella_load_balancing_divider;
//					if(num_racks_to_use<=0)
//						num_racks_to_use=1;
//					for(j=0;j<num_racks_to_use;j++)
//					{
//						int index = GetLeastNumberOfProjects(assigned_collectors,v_gnutella_spoofers_splited_status);
//						if(index >= 0)
//						{
//							assigned_collectors.push_back(index);
//							v_gnutella_spoofers_splited_status[index].v_projects.push_back(v_active_gnutella_protection_projects[i]);
//						}
//						else
//							break;
//					}
//					/*
//					int index=FindMinimumProjectsGiftSwarmerRack();
//					v_gnutella_spoofers_splited_status[index].v_projects.push_back(v_active_gnutella_protection_projects[i]);
//					*/
//				}
//			}
//		}
//		//for singles projects
//		for(i=0; i<v_singles_project_data.size(); i++)
//		{
//			bool found = false;
//			for(j=0; j<v_gnutella_spoofers_splited_status.size(); j++)
//			{
//				for(k=0;k<v_gnutella_spoofers_splited_status[j].v_projects.size();k++)
//				{
//					if(v_singles_project_data[i].m_id == v_gnutella_spoofers_splited_status[j].v_projects[k].m_id)
//					{
//						found=true;
//						v_gnutella_spoofers_splited_status[j].v_projects[k] = v_singles_project_data[i];
//						break;
//					}
//				}
//
//			}
//			if(found) //we need to see if the kazaa swarm load got changed
//			{
//				vector<UINT>assigned_collectors;
//				int num_racks_supposed_to_assigned = ((int)(((float)v_singles_project_data[i].m_gnutella_load / (float)100)*(float)v_gnutella_spoofers_splited_status.size()))/m_gnutella_singles_load_balancing_divider;
//				if(num_racks_supposed_to_assigned <= 0)
//					num_racks_supposed_to_assigned = 1;
//				int num_racks_actually_assigned = GetNumRacksAssignedForProject(v_singles_project_data[i].m_id, v_gnutella_spoofers_splited_status,assigned_collectors);
//				if(num_racks_supposed_to_assigned > num_racks_actually_assigned) //need to assign more racks to this project
//				{
//					for(j=0;j<(num_racks_supposed_to_assigned-num_racks_actually_assigned);j++)
//					{
//						int index = GetLeastNumberOfProjects(assigned_collectors,v_gnutella_spoofers_splited_status);
//						if(index>=0)
//						{
//							assigned_collectors.push_back(index);
//							v_gnutella_spoofers_splited_status[index].v_projects.push_back(v_singles_project_data[i]);
//						}
//						else
//							break;
//					}
//				}
//				else if(num_racks_supposed_to_assigned < num_racks_actually_assigned)// need to remove some racks from this project
//				{
//					for(j=0;j<(num_racks_actually_assigned-num_racks_supposed_to_assigned);j++)
//					{
//						if(j<assigned_collectors.size())
//						{
//							if(assigned_collectors[j] < v_gnutella_spoofers_splited_status.size())
//							{
//								vector<ProjectKeywords>::iterator iter = v_gnutella_spoofers_splited_status[assigned_collectors[j]].v_projects.begin();
//								while(iter!=v_gnutella_spoofers_splited_status[assigned_collectors[j]].v_projects.end())
//								{
//									if(iter->m_id == v_singles_project_data[i].m_id)
//									{
//										v_gnutella_spoofers_splited_status[assigned_collectors[j]].v_projects.erase(iter);
//										break;
//									}
//									iter++;
//								}
//							}
//						}
//					}
//				}
//			}
//			else //add this new project
//			{
//				vector<UINT>assigned_collectors;
//
//				int num_racks_to_use = ((int)(((float)v_singles_project_data[i].m_gnutella_load / (float)100)*(float)v_gnutella_spoofers_splited_status.size()))/m_gnutella_singles_load_balancing_divider;
//				if(num_racks_to_use<=0)
//					num_racks_to_use=1;
//				for(j=0;j<num_racks_to_use;j++)
//				{
//					int index = GetLeastNumberOfProjects(assigned_collectors,v_gnutella_spoofers_splited_status);
//					if(index >= 0)
//					{
//						assigned_collectors.push_back(index);
//						v_gnutella_spoofers_splited_status[index].v_projects.push_back(v_singles_project_data[i]);
//					}
//					else
//						break;
//				}
//				/*
//				int index=FindMinimumProjectsGiftSwarmerRack();
//                v_gnutella_spoofers_splited_status[index].v_projects.push_back(v_singles_project_data[i]);
//				*/
//			}
//		}
//		for(i=0; i<v_gnutella_spoofers_splited_status.size(); i++)
//		{
//			SendProjectChecksums(v_gnutella_spoofers_splited_status[i]);
//			m_dlg.UpdateCollectorStatus(v_gnutella_spoofers_splited_status[i],"Gnutella Spoofer Splitted");
//		}
//	}
//	m_browse_projects_dlg.BuildProjectTree("Gnutella Spoofer Splitted",v_gnutella_spoofers_splited_status);
//}

//
//
//
//int DCMasterDll::FindMinimumProjectsGnutellaSpooferSplited()
//{
//	if(v_gnutella_spoofers_splited_status.size()==0)
//		return -1;
//	UINT index = 0;
//	UINT min = (UINT)v_gnutella_spoofers_splited_status[0].v_projects.size();
//	//looking for the protector which has the minimum projects and add the new project to it
//	for(UINT j=1;j<v_gnutella_spoofers_splited_status.size();j++)
//	{
//		if(v_gnutella_spoofers_splited_status[j].v_projects.size() < min)
//		{
//			min = (UINT)v_gnutella_spoofers_splited_status[j].v_projects.size();
//			index = j;
//		}
//	}
//	return index;
//}

//
//
//
//int DCMasterDll::FindMinimumProjectsGiftSwarmerRack()
//{
//	if(v_fasttrack_swarmers_status.size()==0)
//		return -1;
//	UINT index = 0;
//	UINT min = (UINT)v_fasttrack_swarmers_status[0].v_projects.size();
//	//looking for the protector which has the minimum projects and add the new project to it
//	for(UINT j=1;j<v_fasttrack_swarmers_status.size();j++)
//	{
//		if(v_fasttrack_swarmers_status[j].v_projects.size() < min)
//		{
//			min = (UINT)v_fasttrack_swarmers_status[j].v_projects.size();
//			index = j;
//		}
//	}
//	return index;
//}

//
//
//
//void DCMasterDll::ReassignUpdatedProjectsForFastTrackSpoofers()
//{
//	UINT i,j,k;
//	m_dlg.ClearProjectsNumber("FastTrack Spoofer");
//
//	//remove the removed projects from the protectors
//	for(i=0; i<v_fasttrack_spoofers_status.size(); i++)
//	{
//		vector<ProjectKeywords>::iterator iter = v_fasttrack_spoofers_status[i].v_projects.begin();
//		while(iter!=v_fasttrack_spoofers_status[i].v_projects.end())
//		{
//			bool found=false;
//			for(j=0;j<v_gift_spoofers_project_data.size();j++)
//			{
//				if(iter->m_id == v_gift_spoofers_project_data[j].m_id)
//				{
//					found=true;
//					break;
//				}
//			}
//			if(!found)
//				v_fasttrack_spoofers_status[i].v_projects.erase(iter);
//			else
//				iter++;
//		}
//	}
//
//	//update the projects from the protectors
//	if(v_fasttrack_spoofers_status.size()>0)
//	{
//		for(i=0; i<v_gift_spoofers_project_data.size(); i++)
//		{
//			bool found = false;
//			for(j=0; j<v_fasttrack_spoofers_status.size(); j++)
//			{
//				for(k=0;k<v_fasttrack_spoofers_status[j].v_projects.size();k++)
//				{
//					if(v_gift_spoofers_project_data[i].m_id == v_fasttrack_spoofers_status[j].v_projects[k].m_id)
//					{
//						found=true;
//						v_fasttrack_spoofers_status[j].v_projects[k] = v_gift_spoofers_project_data[i];
//						break;
//					}
//				}
//
//			}
//			if(found) //we need to see if the kazaa swarm load got changed
//			{
//				vector<UINT>assigned_collectors;
//				int num_racks_supposed_to_assigned = (int)((((float)v_gift_spoofers_project_data[i].m_kazaa_swarm_load / (float)100)*(float)v_fasttrack_spoofers_status.size())/m_gift_spoofer_load_balancing_divider);
//				if(num_racks_supposed_to_assigned <= 0)
//					num_racks_supposed_to_assigned = 1;
//				int num_racks_actually_assigned = GetNumRacksAssignedForProject(v_gift_spoofers_project_data[i].m_id, v_fasttrack_spoofers_status,assigned_collectors);
//				if(num_racks_supposed_to_assigned > num_racks_actually_assigned) //need to assign more racks to this project
//				{
//					for(j=0;j<(num_racks_supposed_to_assigned-num_racks_actually_assigned);j++)
//					{
//						int index = GetLeastNumberOfProjects(assigned_collectors,v_fasttrack_spoofers_status);
//						if(index>=0)
//						{
//							assigned_collectors.push_back(index);
//							v_fasttrack_spoofers_status[index].v_projects.push_back(v_gift_spoofers_project_data[i]);
//						}
//						else
//							break;
//					}
//				}
//				else if(num_racks_supposed_to_assigned < num_racks_actually_assigned)// need to remove some racks from this project
//				{
//					for(j=0;j<(num_racks_actually_assigned-num_racks_supposed_to_assigned);j++)
//					{
//						if(j<assigned_collectors.size())
//						{
//							if(assigned_collectors[j] < v_fasttrack_spoofers_status.size())
//							{
//								vector<ProjectKeywords>::iterator iter = v_fasttrack_spoofers_status[assigned_collectors[j]].v_projects.begin();
//								while(iter!=v_fasttrack_spoofers_status[assigned_collectors[j]].v_projects.end())
//								{
//									if(iter->m_id == v_gift_spoofers_project_data[i].m_id)
//									{
//										v_fasttrack_spoofers_status[assigned_collectors[j]].v_projects.erase(iter);
//										break;
//									}
//									iter++;
//								}
//							}
//						}
//					}
//				}
//			}
//			else //add this new project
//			{
//				vector<UINT>assigned_collectors;
//
//				int num_racks_to_use = (int)((((float)v_gift_spoofers_project_data[i].m_kazaa_swarm_load / (float)100)*(float)v_fasttrack_spoofers_status.size())/m_gift_spoofer_load_balancing_divider);
//				if(num_racks_to_use<=0)
//					num_racks_to_use=1;
//				for(j=0;j<num_racks_to_use;j++)
//				{
//					int index = GetLeastNumberOfProjects(assigned_collectors,v_fasttrack_spoofers_status);
//					if(index >= 0)
//					{
//						assigned_collectors.push_back(index);
//						v_fasttrack_spoofers_status[index].v_projects.push_back(v_gift_spoofers_project_data[i]);
//					}
//					else
//						break;
//				}
//				/*
//				int index=FindMinimumProjectsGiftspooferRack();
//                v_fasttrack_spoofers_status[index].v_projects.push_back(v_gift_spoofers_project_data[i]);
//				*/
//			}
//		}
//		for(i=0; i<v_fasttrack_spoofers_status.size(); i++)
//		{
//			SendProjectChecksums(v_fasttrack_spoofers_status[i]);
//			m_dlg.UpdateCollectorStatus(v_fasttrack_spoofers_status[i],"FastTrack Spoofer");
//		}
//	}
//	m_browse_projects_dlg.BuildProjectTree("Gift Spoofer",v_fasttrack_spoofers_status);
//}

//
//
//
void DCMasterDll::ReassignProjectsForFastTrackDC()
{
	m_dlg.ClearProjectsNumber("FastTrack DC");
	if(v_fasttrack_dc_status.size()>0)
	{
		UINT i;
		for(i=0; i<v_fasttrack_dc_status.size(); i++)
		{
			v_fasttrack_dc_status[i].v_projects.clear();
		}

		for(i=0; i<v_gift_dc_project_data.size(); i++)
		{
			int index = i%v_fasttrack_dc_status.size();
			v_fasttrack_dc_status[index].v_projects.push_back(v_gift_dc_project_data[i]);
		}
		for(i=0; i<v_fasttrack_dc_status.size(); i++)
		{
			SendProjectChecksums(v_fasttrack_dc_status[i]);
			m_dlg.UpdateCollectorStatus(v_fasttrack_dc_status[i],"FastTrack DC");
		}
	}
	//m_browse_projects_dlg.BuildProjectTree("FastTrack DC",v_fasttrack_dc_status);
}

//
//
//
void DCMasterDll::ReassignProjectsForDirectConnectSpoofer()
{
	//send checksums to dirrect connect collectors
	m_dlg.ClearProjectsNumber("Direct Connect Spoofer");
	if(v_direct_connect_spoofer_status.size()>0)
	{
		UINT i,j;
		for(i=0; i<v_direct_connect_spoofer_status.size(); i++)
		{
			v_direct_connect_spoofer_status[i].v_projects.clear();
		}
/*
		for(i=0; i<v_project_data.size(); i++)
		{
			int index = i%v_direct_connect_spoofer_status.size();
			v_direct_connect_spoofer_status[index].v_projects.push_back(v_project_data[i]);
		}

*/
		for(i=0; i<v_project_data.size(); i++)
		{
			for(j=0; j<v_direct_connect_spoofer_status.size(); j++)
			{
				v_direct_connect_spoofer_status[j].v_projects.push_back(v_project_data[i]);
			}
		}
		for(i=0; i<v_direct_connect_spoofer_status.size(); i++)
		{
			SendProjectChecksums(v_direct_connect_spoofer_status[i]);
			m_dlg.UpdateCollectorStatus(v_direct_connect_spoofer_status[i],"Direct Connect Spoofer");
		}
	}
	//m_browse_projects_dlg.BuildProjectTree("Direct Connect Spoofer",v_direct_connect_spoofer_status);
}

//
//
//
void DCMasterDll::ReassignProjectsForDirectConnect()
{
	//send checksums to dirrect connect collectors
	m_dlg.ClearProjectsNumber("Direct Connect");
	if(v_direct_connect_status.size()>0)
	{
		UINT i;
		for(i=0; i<v_direct_connect_status.size(); i++)
		{
			v_direct_connect_status[i].v_projects.clear();
		}

		for(i=0; i<v_project_data.size(); i++)
		{
			int index = i%v_direct_connect_status.size();
			v_direct_connect_status[index].v_projects.push_back(v_project_data[i]);
		}
		for(i=0; i<v_direct_connect_status.size(); i++)
		{
			SendProjectChecksums(v_direct_connect_status[i]);
			m_dlg.UpdateCollectorStatus(v_direct_connect_status[i],"Direct Connect");
		}
	}
	//m_browse_projects_dlg.BuildProjectTree("Direct Connect",v_direct_connect_status);
}

//
//
//
void DCMasterDll::ReassignProjectsForEmule()
{
	//send checksums to emule collectors
	m_dlg.ClearProjectsNumber("Emule");
	if(v_emule_network_status.size()>0)
	{
		UINT i;
		for(i=0; i<v_emule_network_status.size(); i++)
		{
			v_emule_network_status[i].v_projects.clear();
		}
/*
		for(i=0; i<v_project_data.size(); i++)
		{
			int index = i%v_emule_network_status.size();
			v_emule_network_status[index].v_projects.push_back(v_project_data[i]);
		}
*/
		for(i=0; i<v_project_data.size(); i++)
		{
			for(UINT j=0; j<v_emule_network_status.size(); j++)
			{
				v_emule_network_status[j].v_projects.push_back(v_project_data[i]);
			}
		}

		for(i=0; i<v_emule_network_status.size(); i++)
		{
			SendProjectChecksums(v_emule_network_status[i]);
			m_dlg.UpdateCollectorStatus(v_emule_network_status[i],"Emule");
		}
	}
	//m_browse_projects_dlg.BuildProjectTree("Emule",v_emule_network_status);
}

//
//
//
void DCMasterDll::ReassignProjectsForGnutella()
{
	//send checksums to gnutella collectors
	m_dlg.ClearProjectsNumber("Gnutella");
	if(v_gnutella_network_status.size()>0)
	{
		UINT i;
		for(i=0; i<v_gnutella_network_status.size(); i++)
		{
			v_gnutella_network_status[i].v_projects.clear();
		}
/*
		for(i=0; i<v_project_data.size(); i++)
		{
			int index = i%v_gnutella_network_status.size();
			v_gnutella_network_status[index].v_projects.push_back(v_project_data[i]);
		}
*/
		for(i=0; i<v_project_data.size(); i++)
		{
			for(UINT j=0; j<v_gnutella_network_status.size(); j++)
			{
				v_gnutella_network_status[j].v_projects.push_back(v_project_data[i]);
			}
		}

		for(i=0; i<v_gnutella_network_status.size(); i++)
		{
			SendProjectChecksums(v_gnutella_network_status[i]);
			m_dlg.UpdateCollectorStatus(v_gnutella_network_status[i],"Gnutella");
		}
	}
	//m_browse_projects_dlg.BuildProjectTree("Gnutella",v_gnutella_network_status);
}

//
//
//
void DCMasterDll::ReassignProjectsForWinmxDC()
{
	m_dlg.ClearProjectsNumber("WinMX DC");
	if(v_winmx_dc_status.size()>0)
	{
		UINT i;
		for(i=0; i<v_winmx_dc_status.size(); i++)
		{
			v_winmx_dc_status[i].v_projects.clear();
		}

		for(i=0; i<v_winmx_project_data.size(); i++)
		{
			int index = i%v_winmx_dc_status.size();
			v_winmx_dc_status[index].v_projects.push_back(v_winmx_project_data[i]);
		}

		for(i=0; i<v_winmx_dc_status.size(); i++)
		{
			SendProjectChecksums(v_winmx_dc_status[i]);
			m_dlg.UpdateCollectorStatus(v_winmx_dc_status[i],"WinMX DC");
		}
	}
}

//
//
//
/*
void DCMasterDll::ReassignProjectsForWinmxSpoofer()
{
	m_dlg.ClearProjectsNumber("WinMX Spoofer");
	if(v_winmx_spoofer_status.size()>0)
	{
		//sort the swarmerss' ips
		sort(v_winmx_spoofer_status.begin(),v_winmx_spoofer_status.end());

		vector<ProjectKeywords> winmxspooferprojects;
		UINT i;
		for(i=0; i<v_winmx_project_data.size(); i++)
		{
			if(v_winmx_project_data[i].m_winmx_spoofing_enabled)
				winmxspooferprojects.push_back(v_winmx_project_data[i]);
		}
		for(i=0; i<v_winmx_spoofer_status.size(); i++)
		{
			v_winmx_spoofer_status[i].v_projects.clear();
		}

		for(i=0; i<winmxspooferprojects.size(); i++)
		{
			int index = i%v_winmx_spoofer_status.size();
			v_winmx_spoofer_status[index].v_projects.push_back(winmxspooferprojects[i]);
		}

		for(i=0; i<v_winmx_spoofer_status.size(); i++)
		{
			SendProjectChecksums(v_winmx_spoofer_status[i]);
			m_dlg.UpdateCollectorStatus(v_winmx_spoofer_status[i],"WinMX Spoofer");
		}
	}
	m_browse_projects_dlg.BuildProjectTree("WinMX Spoofer",v_winmx_spoofer_status);
}
*/
//
//
//
void DCMasterDll::ReassignProjectsForWinmxDecoyer()
{
	m_dlg.ClearProjectsNumber("WinMX Decoyer");
	if(v_winmx_decoyer_status.size()>0)
	{
		UINT i;
		vector<ProjectKeywords> winmxdecoyerprojects;
		for(i=0; i<v_winmx_project_data.size(); i++)
		{
			if(v_winmx_project_data[i].m_winmx_decoy_enabled)
				winmxdecoyerprojects.push_back(v_winmx_project_data[i]);
		}
		for(i=0; i<v_winmx_decoyer_status.size(); i++)
		{
			v_winmx_decoyer_status[i].v_projects.clear();
		}

		for(i=0; i<winmxdecoyerprojects.size(); i++)
		{
			int index = i%v_winmx_decoyer_status.size();
			v_winmx_decoyer_status[index].v_projects.push_back(winmxdecoyerprojects[i]);
		}

		for(i=0; i<v_winmx_decoyer_status.size(); i++)
		{
			SendProjectChecksums(v_winmx_decoyer_status[i]);
			m_dlg.UpdateCollectorStatus(v_winmx_decoyer_status[i],"WinMX Decoyer");
		}
	}
}

//
//
//
/*
void DCMasterDll::ReassignProjectsForWinmxSwarmer()
{
	m_dlg.ClearProjectsNumber("WinMX Swarmer");
	if(v_winmx_swarmer_status.size()>0)
	{
		UINT i;
		vector<ProjectKeywords> winmxswarmerprojects;
		for(i=0; i<v_winmx_project_data.size(); i++)
		{
			if(v_winmx_project_data[i].m_winmx_swarming_enabled)
				winmxswarmerprojects.push_back(v_winmx_project_data[i]);
		}
		for(i=0; i<v_winmx_swarmer_status.size(); i++)
		{
			v_winmx_swarmer_status[i].v_projects.clear();
		}

		for(i=0; i<winmxswarmerprojects.size(); i++)
		{
			int index = i%v_winmx_swarmer_status.size();
			v_winmx_swarmer_status[index].v_projects.push_back(winmxswarmerprojects[i]);
		}

		for(i=0; i<v_winmx_swarmer_status.size(); i++)
		{
			SendProjectChecksums(v_winmx_swarmer_status[i]);
			m_dlg.UpdateCollectorStatus(v_winmx_swarmer_status[i],"WinMX Swarmer");
		}
	}
}
*/

//
//
//
void DCMasterDll::ReassignProjectsForGeneric()
{
	m_dlg.ClearProjectsNumber("Generic");
	if(v_generic_status.size()>0)
	{
		UINT i;
		for(i=0; i<v_generic_status.size(); i++)
		{
			v_generic_status[i].v_projects.clear();
		}

		for(i=0; i<v_generic_project_data.size(); i++)
		{
			for(UINT j=0; j<v_generic_status.size(); j++)
			{
				v_generic_status[j].v_projects.push_back(v_generic_project_data[i]);
			}
		}
		for(i=0; i<v_generic_status.size(); i++)
		{
			SendProjectChecksums(v_generic_status[i]);
			m_dlg.UpdateCollectorStatus(v_generic_status[i],"Generic");
		}
	}
}

//
//
//
void DCMasterDll::ReassignProjectsForBitTorrent()
{
	m_dlg.ClearProjectsNumber("BitTorrent");
	if(v_bittorrent_status.size()>0)
	{
		UINT i;
		for(i=0; i<v_bittorrent_status.size(); i++)
		{
			v_bittorrent_status[i].v_projects.clear();
		}

		for(i=0; i<v_bittorrent_project_data.size(); i++)
		{
			int index = i%v_bittorrent_status.size();
			v_bittorrent_status[index].v_projects.push_back(v_bittorrent_project_data[i]);
		}

		for(i=0; i<v_bittorrent_status.size(); i++)
		{
			SendProjectChecksums(v_bittorrent_status[i]);
			m_dlg.UpdateCollectorStatus(v_bittorrent_status[i],"BitTorrent");
		}
	}
}

//
//
//
void DCMasterDll::ReassignProjectsForOvernetDC()
{
	m_dlg.ClearProjectsNumber("Overnet DC");
	if(v_overnet_dc_status.size()>0)
	{
		UINT i;
		for(i=0; i<v_overnet_dc_status.size(); i++)
		{
			v_overnet_dc_status[i].v_projects.clear();
		}

		for(i=0; i<v_overnet_project_data.size(); i++)
		{
			int index = i%v_overnet_dc_status.size();
			v_overnet_dc_status[index].v_projects.push_back(v_overnet_project_data[i]);
		}

		for(i=0; i<v_overnet_dc_status.size(); i++)
		{
			SendProjectChecksums(v_overnet_dc_status[i]);
			m_dlg.UpdateCollectorStatus(v_overnet_dc_status[i],"Overnet DC");
		}
	}
}

//
//
//
void DCMasterDll::ReassignProjectsForOvernetSpoofer()
{
	m_dlg.ClearProjectsNumber("Overnet Spoofer");
	if(v_overnet_spoofer_status.size()>0)
	{
		UINT i;
		for(i=0; i<v_overnet_spoofer_status.size(); i++)
		{
			v_overnet_spoofer_status[i].v_projects.clear();
		}
		//for(i=0; i<v_overnet_project_data.size(); i++)
		//{
		//	int index = i%v_overnet_spoofer_status.size();
		//	v_overnet_spoofer_status[index].v_projects.push_back(v_overnet_project_data[i]);
		//}
		for(i=0; i<v_overnet_project_data.size(); i++)
		{
			for(UINT j=0; j<v_overnet_spoofer_status.size(); j++)
			{
				v_overnet_spoofer_status[j].v_projects.push_back(v_overnet_project_data[i]);
			}
		}

		for(i=0; i<v_overnet_spoofer_status.size(); i++)
		{
			SendProjectChecksums(v_overnet_spoofer_status[i]);
			m_dlg.UpdateCollectorStatus(v_overnet_spoofer_status[i],"Overnet Spoofer");
		}
	}
}

//
//
//
void DCMasterDll::ReassignProjectsForSoulSeekSpoofer()
{
	m_dlg.ClearProjectsNumber("SoulSeek Spoofer");
	if(v_soulseek_spoofer_status.size()>0)
	{
		UINT i;
		for(i=0; i<v_soulseek_spoofer_status.size(); i++)
		{
			v_soulseek_spoofer_status[i].v_projects.clear();
		}
/*
		for(i=0; i<v_soulseek_project_data.size(); i++)
		{
			int index = i%v_soulseek_spoofer_status.size();
			v_soulseek_spoofer_status[index].v_projects.push_back(v_soulseek_project_data[i]);
		}
*/
		for(i=0; i<v_soulseek_project_data.size(); i++)
		{
			for(UINT j=0; j<v_soulseek_spoofer_status.size(); j++)
			{
				v_soulseek_spoofer_status[j].v_projects.push_back(v_soulseek_project_data[i]);
			}
		}
		for(i=0; i<v_soulseek_spoofer_status.size(); i++)
		{
			SendProjectChecksums(v_soulseek_spoofer_status[i]);
			m_dlg.UpdateCollectorStatus(v_soulseek_spoofer_status[i],"SoulSeek Spoofer");
		}
	}
}

//
//
//
void DCMasterDll::ReassignProjectsForKazaa()
{
	//send checksums to kazaa collectors
	m_dlg.ClearProjectsNumber("Kazaa");
	if(v_kazaa_network_status.size()>0)
	{
		UINT i;
		for(i=0; i<v_kazaa_network_status.size(); i++)
		{
			v_kazaa_network_status[i].v_projects.clear();
		}

		for(i=0; i<v_project_data.size(); i++)
		{
//			int index = i%v_kazaa_network_status.size();
			
			if(v_project_data[i].m_kazaa_dist_power > 0)
			{

				//distribute the same project to other kazaa collector if kazaa dist power is greater than 1
				if(v_project_data[i].m_kazaa_dist_power > 1)
				{
					//if the number of kazaa collectors are more than the kazaa dist power
					if((int)v_kazaa_network_status.size() >= v_project_data[i].m_kazaa_dist_power)
					{
						vector<UINT>assigned_collectors;
						for(int j=0; j<v_project_data[i].m_kazaa_dist_power; j++)
						{
							//find the least number of projects of kazaa collector
							UINT index = GetLeastNumberOfProjectsOfKazaaCollector(assigned_collectors);
							assigned_collectors.push_back(index);
							v_kazaa_network_status[index].v_projects.push_back(v_project_data[i]);
						}
					}
					else //we don't have enough kazaa collectors, so just let each collector have one
					{
						for(UINT j=0; j<v_kazaa_network_status.size();j++)
						{
							v_kazaa_network_status[j].v_projects.push_back(v_project_data[i]);
						}
					}
				}
				else
				{
					vector<UINT>assigned_collectors;
					//find the least number of projects of kazaa collector
					UINT index = GetLeastNumberOfProjectsOfKazaaCollector(assigned_collectors);
					v_kazaa_network_status[index].v_projects.push_back(v_project_data[i]);
				}
			}
		}

		for(i=0; i<v_kazaa_network_status.size(); i++)
		{
			SendProjectChecksums(v_kazaa_network_status[i]);
			m_dlg.UpdateCollectorStatus(v_kazaa_network_status[i],"Kazaa");
		}
	}
	//m_browse_projects_dlg.BuildProjectTree("Kazaa",v_kazaa_network_status);
}

//
//
//
UINT DCMasterDll::GetLeastNumberOfProjectsOfKazaaCollector(vector<UINT>& exclude_collectors)
{
	UINT index=0;
	UINT least_projects=(UINT)v_kazaa_network_status[0].v_projects.size();
	for(UINT i=1; i<v_kazaa_network_status.size(); i++)
	{
		bool skip = false;
		for(UINT j=0; j<exclude_collectors.size();j++)
		{
			if(exclude_collectors[j] == i)
			{
				skip = true;
				break;
			}
		}
		if(!skip)
		{
			if(v_kazaa_network_status[i].v_projects.size() < least_projects)
			{
				index = i;
				least_projects = (UINT)v_kazaa_network_status[i].v_projects.size();
			}
		}
	}
	return index;
}

//
//
//
void DCMasterDll::ReassignProjectsForPiolet()
{
	//send checksums to kazaa collectors
	m_dlg.ClearProjectsNumber("Piolet");
	if(v_piolet_network_status.size()>0)
	{
		UINT i;
		for(i=0; i<v_piolet_network_status.size(); i++)
		{
			v_piolet_network_status[i].v_projects.clear();
		}

		for(i=0; i<v_project_data.size(); i++)
		{
			int index = i%v_piolet_network_status.size();
			v_piolet_network_status[index].v_projects.push_back(v_project_data[i]);
		}

		for(i=0; i<v_piolet_network_status.size(); i++)
		{
			SendProjectChecksums(v_piolet_network_status[i]);
			m_dlg.UpdateCollectorStatus(v_piolet_network_status[i],"Piolet");
		}
	}
	//m_browse_projects_dlg.BuildProjectTree("Piolet",v_piolet_network_status);
}

//
//
//
void DCMasterDll::ReassignProjectsForPioletSpoofer()
{
	//send checksums to kazaa collectors
	m_dlg.ClearProjectsNumber("Piolet Spoofer");
	if(v_piolet_spoofer_status.size()>0)
	{
		UINT i,j;
		for(i=0; i<v_piolet_spoofer_status.size(); i++)
		{
			v_piolet_spoofer_status[i].v_projects.clear();
		}

		for(i=0; i<v_piolet_spoofer_project_data.size(); i++)
		{
			for(j=0; j<v_piolet_spoofer_status.size(); j++)
			{
				v_piolet_spoofer_status[j].v_projects.push_back(v_piolet_spoofer_project_data[i]);
			}
		}

		for(i=0; i<v_piolet_spoofer_status.size(); i++)
		{
			SendProjectChecksums(v_piolet_spoofer_status[i]);
			m_dlg.UpdateCollectorStatus(v_piolet_spoofer_status[i],"Piolet Spoofer");
		}
	}
	//m_browse_projects_dlg.BuildProjectTree("Piolet Spoofer",v_piolet_spoofer_status);
}


//
//
//
void DCMasterDll::ReassignProjectsForGnutellaProtector()
{
	//send checksums to kazaa collectors
	m_dlg.ClearProjectsNumber("Gnutella Protector");
	if(v_gnutella_protectors_status.size()>0)
	{
		UINT i,j;
		for(i=0; i<v_gnutella_protectors_status.size(); i++)
		{
			v_gnutella_protectors_status[i].v_projects.clear();
		}
/*
		for(i=0; i<v_project_data.size(); i++)
		{
			int index = i%v_gnutella_protectors_status.size();
			v_gnutella_protectors_status[index].v_projects.push_back(v_project_data[i]);
		}
*/
		for(i=0; i<v_regular_active_gnutella_protection_projects.size(); i++)
		{
			for(j=0; j<v_gnutella_protectors_status.size(); j++)
			{
				v_gnutella_protectors_status[j].v_projects.push_back(v_regular_active_gnutella_protection_projects[i]);
			}
		}
		for(i=0; i<v_gnutella_protectors_status.size(); i++)
		{
			SendProjectChecksums(v_gnutella_protectors_status[i]);
			m_dlg.UpdateCollectorStatus(v_gnutella_protectors_status[i],"Gnutella Protector");
		}
	}
	//m_browse_projects_dlg.BuildProjectTree("Gnutella Protector",v_gnutella_protectors_status);
}

//
//
//
void DCMasterDll::SendProjectChecksums(DCStatus& status)
{
	if(status.v_projects.size()>0)
	{
		UINT i;
		ProjectChecksums checksums;

		for(i=0; i<status.v_projects.size(); i++)
		{
			ProjectChecksum checksum;
			checksum = status.v_projects[i].CalculateChecksum();
			checksum.m_project_name = status.v_projects[i].m_project_name;
			checksums.v_checksums.push_back(checksum);
		}


		int buf_len = checksums.GetBufferLength();
		unsigned char * buf = new unsigned char[sizeof(DCHeader)+buf_len];

		DCHeader* header = (DCHeader*)buf;
		header->op = DCHeader::ProjectChecksums;
		header->size = buf_len;

		checksums.WriteToBuffer((char*)(&buf[sizeof(DCHeader)]));

		m_com.SendReliableData(status.m_ip, buf, sizeof(DCHeader)+buf_len);
		delete [] buf;
	}
}

//
//
//
void DCMasterDll::CheckForCollectorsStatus(bool recalculate_project_distribution)
{
	bool new_collectors_are_up = false;
	bool some_collectors_are_down = false;
	UINT i=0;

	//check for generic network
	vector<DCStatus>::iterator iter = v_generic_status.begin();
	while(iter!= v_generic_status.end())
	{
		if(iter->v_projects.size() == 0)
		{
			new_collectors_are_up = true;
		}
		CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
		if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
		{
			some_collectors_are_down = true;
			v_generic_status.erase(iter);
		}
		else
			iter++;
	}
	if(recalculate_project_distribution)
	{
		if(new_collectors_are_up || some_collectors_are_down)
		{
			ReassignProjectsForGeneric();
		}
	}

	//check for overnet dc network
	iter = v_overnet_dc_status.begin();
	while(iter!= v_overnet_dc_status.end())
	{
		if(iter->v_projects.size() == 0)
		{
			new_collectors_are_up = true;
		}
		CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
		if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
		{
			some_collectors_are_down = true;
			v_overnet_dc_status.erase(iter);
		}
		else
			iter++;
	}
	if(recalculate_project_distribution)
	{
		if(new_collectors_are_up || some_collectors_are_down)
		{
			ReassignProjectsForOvernetDC();
		}
	}

	//check for overnet spoofer network
	iter = v_overnet_spoofer_status.begin();
	while(iter!= v_overnet_spoofer_status.end())
	{
		if(iter->v_projects.size() == 0)
		{
			new_collectors_are_up = true;
		}
		CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
		if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
		{
			some_collectors_are_down = true;
			v_overnet_spoofer_status.erase(iter);
		}
		else
			iter++;
	}
	if(recalculate_project_distribution)
	{
		if(new_collectors_are_up || some_collectors_are_down)
		{
			ReassignProjectsForOvernetSpoofer();
		}
	}

	//check for bittorrent network
	iter = v_bittorrent_status.begin();
	while(iter!= v_bittorrent_status.end())
	{
		if(iter->v_projects.size() == 0)
		{
			new_collectors_are_up = true;
		}
		CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
		if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
		{
			some_collectors_are_down = true;
			v_bittorrent_status.erase(iter);
		}
		else
			iter++;
	}
	if(recalculate_project_distribution)
	{
		if(new_collectors_are_up || some_collectors_are_down)
		{
			ReassignProjectsForBitTorrent();
		}
	}

	//check for soulseek network
	iter = v_soulseek_spoofer_status.begin();
	while(iter!= v_soulseek_spoofer_status.end())
	{
		if(iter->v_projects.size() == 0)
		{
			new_collectors_are_up = true;
		}
		CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
		if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
		{
			some_collectors_are_down = true;
			v_soulseek_spoofer_status.erase(iter);
		}
		else
			iter++;
	}
	if(recalculate_project_distribution)
	{
		if(new_collectors_are_up || some_collectors_are_down)
		{
			ReassignProjectsForSoulSeekSpoofer();
		}
	}

	//check for emule network
	iter = v_emule_network_status.begin();
	while(iter!= v_emule_network_status.end())
	{
		if(iter->v_projects.size() == 0)
		{
			new_collectors_are_up = true;
		}
		CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
		if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
		{
			some_collectors_are_down = true;
			v_emule_network_status.erase(iter);
		}
		else
			iter++;
	}
	if(recalculate_project_distribution)
	{
		if(new_collectors_are_up || some_collectors_are_down)
		{
			ReassignProjectsForEmule();
		}
	}

	//check for kazaa network
	new_collectors_are_up = false;
	some_collectors_are_down = false;
	iter = v_kazaa_network_status.begin();
	while(iter!= v_kazaa_network_status.end())
	{
		if(iter->v_projects.size() == 0)
		{
			new_collectors_are_up = true;
		}
		CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
		if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
		{
			some_collectors_are_down = true;
			v_kazaa_network_status.erase(iter);
		}
		else
			iter++;
	}
	if(recalculate_project_distribution)
	{
		if(new_collectors_are_up || some_collectors_are_down)
		{
			ReassignProjectsForKazaa();
		}
	}

	//check for gnutella network
	new_collectors_are_up = false;
	some_collectors_are_down = false;
	iter = v_gnutella_network_status.begin();
	while(iter!= v_gnutella_network_status.end())
	{
		if(iter->v_projects.size() == 0)
		{
			new_collectors_are_up = true;
		}
		CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
		if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
		{
			some_collectors_are_down = true;
			v_gnutella_network_status.erase(iter);
		}
		else
			iter++;
	}
	if(recalculate_project_distribution)
	{
		if(new_collectors_are_up || some_collectors_are_down)
		{
			ReassignProjectsForGnutella();
		}
	}

	//check for piolet network
	new_collectors_are_up = false;
	some_collectors_are_down = false;
	iter = v_piolet_network_status.begin();
	while(iter!= v_piolet_network_status.end())
	{
		if(iter->v_projects.size() == 0)
		{
			new_collectors_are_up = true;
		}
		CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
		if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
		{
			some_collectors_are_down = true;
			v_piolet_network_status.erase(iter);
		}
		else
			iter++;
	}
	if(recalculate_project_distribution)
	{
		if(new_collectors_are_up || some_collectors_are_down)
		{
			ReassignProjectsForPiolet();
		}
	}

	//check for piolet spoofer 
	new_collectors_are_up = false;
	some_collectors_are_down = false;
	iter = v_piolet_spoofer_status.begin();
	while(iter!= v_piolet_spoofer_status.end())
	{
		if(iter->v_projects.size() == 0)
		{
			new_collectors_are_up = true;
		}
		CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
		if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
		{
			some_collectors_are_down = true;
			v_piolet_spoofer_status.erase(iter);
		}
		else
			iter++;
	}
	if(recalculate_project_distribution)
	{
		if(new_collectors_are_up || some_collectors_are_down)
		{
			ReassignProjectsForPioletSpoofer();
		}
	}

	//check for Gnutella Protector 
	new_collectors_are_up = false;
	some_collectors_are_down = false;
	iter = v_gnutella_protectors_status.begin();
	while(iter!= v_gnutella_protectors_status.end())
	{
		if(iter->v_projects.size() == 0)
		{
			new_collectors_are_up = true;
		}
		CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
		if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
		{
			some_collectors_are_down = true;
			v_gnutella_protectors_status.erase(iter);
		}
		else
			iter++;
	}
	if(recalculate_project_distribution)
	{
		if(new_collectors_are_up || some_collectors_are_down)
		{
			ReassignProjectsForGnutellaProtector();
		}
	}

	//check for Direct Connect 
	new_collectors_are_up = false;
	some_collectors_are_down = false;
	iter = v_direct_connect_status.begin();
	while(iter!= v_direct_connect_status.end())
	{
		if(iter->v_projects.size() == 0)
		{
			new_collectors_are_up = true;
		}
		CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
		if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
		{
			some_collectors_are_down = true;
			v_direct_connect_status.erase(iter);
		}
		else
			iter++;
	}
	if(recalculate_project_distribution)
	{
		if(new_collectors_are_up || some_collectors_are_down)
		{
			ReassignProjectsForDirectConnect();
		}
	}

	//check for WinMX DC
	new_collectors_are_up = false;
	some_collectors_are_down = false;
	iter = v_winmx_dc_status.begin();
	while(iter!= v_winmx_dc_status.end())
	{
		if(iter->v_projects.size() == 0)
		{
			new_collectors_are_up = true;
		}
		CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
		if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
		{
			some_collectors_are_down = true;
			v_winmx_dc_status.erase(iter);
		}
		else
			iter++;
	}
	if(recalculate_project_distribution)
	{
		if(new_collectors_are_up || some_collectors_are_down)
		{
			ReassignProjectsForWinmxDC();
		}
	}
	/*
	//check for WinMX Spoofer
	new_collectors_are_up = false;
	some_collectors_are_down = false;
	iter = v_winmx_spoofer_status.begin();
	while(iter!= v_winmx_spoofer_status.end())
	{
		if(iter->v_projects.size() == 0)
		{
			new_collectors_are_up = true;
		}
		CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
		if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
		{
			some_collectors_are_down = true;
			v_winmx_spoofer_status.erase(iter);
		}
		else
			iter++;
	}
	if(new_collectors_are_up || some_collectors_are_down)
	{
		ReassignProjectsForWinmxSpoofer();
	}
	*/
	//check for WinMX Decoyer
	new_collectors_are_up = false;
	some_collectors_are_down = false;
	iter = v_winmx_decoyer_status.begin();
	while(iter!= v_winmx_decoyer_status.end())
	{
		if(iter->v_projects.size() == 0)
		{
			new_collectors_are_up = true;
		}
		CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
		if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
		{
			some_collectors_are_down = true;
			v_winmx_decoyer_status.erase(iter);
		}
		else
			iter++;
	}
	if(recalculate_project_distribution)
	{
		if(new_collectors_are_up || some_collectors_are_down)
		{
			ReassignProjectsForWinmxDecoyer();
		}
	}
	//check for Direct Connect Spoofer
	new_collectors_are_up = false;
	some_collectors_are_down = false;
	iter = v_direct_connect_spoofer_status.begin();
	while(iter!= v_direct_connect_spoofer_status.end())
	{
		if(iter->v_projects.size() == 0)
		{
			new_collectors_are_up = true;
		}
		CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
		if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
		{
			some_collectors_are_down = true;
			v_direct_connect_spoofer_status.erase(iter);
		}
		else
			iter++;
	}
	if(recalculate_project_distribution)
	{
		if(new_collectors_are_up || some_collectors_are_down)
		{
			ReassignProjectsForDirectConnectSpoofer();
		}
	}

	//check for FastTrack DC
	new_collectors_are_up = false;
	some_collectors_are_down = false;
	iter = v_fasttrack_dc_status.begin();
	while(iter!= v_fasttrack_dc_status.end())
	{
		if(iter->v_projects.size() == 0)
		{
			new_collectors_are_up = true;
		}
		CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
		if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
		{
			some_collectors_are_down = true;
			v_fasttrack_dc_status.erase(iter);
		}
		else
			iter++;
	}
	if(recalculate_project_distribution)
	{
		if(new_collectors_are_up || some_collectors_are_down)
		{
			ReassignProjectsForFastTrackDC();
		}
	}


	if(m_allow_to_send_projects_updates_to_splitted_gnutella_spoofers)
	{
		//vector<ProjectKeywords> unassigned_projects;
		//check for Gnutella Spoofer Splitted
		new_collectors_are_up = false;
		some_collectors_are_down = false;
		iter = v_gnutella_spoofers_splited_status.begin();
		//unassigned_projects.clear();
		while(iter!= v_gnutella_spoofers_splited_status.end())
		{
			if(iter->v_projects.size() == 0)
			{
				new_collectors_are_up = true;
			}
			CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
			if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
			{
				some_collectors_are_down = true;
				//for(UINT i=0; i<iter->v_projects.size(); i++)
				//	unassigned_projects.push_back( iter->v_projects[i] );
				v_gnutella_spoofers_splited_status.erase(iter);
			}
			else
				iter++;
		}
		if(recalculate_project_distribution)
		{
			if(some_collectors_are_down && !new_collectors_are_up)
			{
				//ReassignProjectsForGnutellaSpoofersSplitedRacksDown(unassigned_projects);
				LoadBalanceProjectsForGnutellaSplitedSpoofers();
			}
			if(new_collectors_are_up)
			{
				//ReassignProjectsForGnutellaSpoofersSplited();
				LoadBalanceProjectsForGnutellaSplitedSpoofers();
			}
		}
	}
	if(m_allow_to_send_projects_updates_to_gift_spoofers)
	{
		//vector<ProjectKeywords> unassigned_projects;
		//check for FastTrack Spoofer
		new_collectors_are_up = false;
		some_collectors_are_down = false;
		//unassigned_projects.clear();
		iter = v_fasttrack_spoofers_status.begin();
		while(iter!= v_fasttrack_spoofers_status.end())
		{
			if(iter->v_projects.size() == 0)
			{
				new_collectors_are_up = true;
			}
			CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
			if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
			{
				some_collectors_are_down = true;
				//for(UINT i=0; i<iter->v_projects.size(); i++)
				//	unassigned_projects.push_back( iter->v_projects[i] );
				v_fasttrack_spoofers_status.erase(iter);
			}
			else
				iter++;
		}
		if(recalculate_project_distribution)
		{
			if(some_collectors_are_down && !new_collectors_are_up)
			{
				//ReassignProjectsForFastTrackSpoofersRacksDown(unassigned_projects);
				LoadBalanceProjectsForGiftSpoofers();
			}
			if(new_collectors_are_up)
			{
				//ReassignProjectsForFastTrackSpoofers();
				LoadBalanceProjectsForGiftSpoofers();
			}
		}
	}
	if(m_allow_to_send_projects_updates_to_gift_spoofers)
	{
		//vector<ProjectKeywords> unassigned_projects;
		//check for FastTrack Spoofer
		new_collectors_are_up = false;
		some_collectors_are_down = false;
		//unassigned_projects.clear();
		iter = v_fasttrack_decoyers_status.begin();
		while(iter!= v_fasttrack_decoyers_status.end())
		{
			if(iter->v_projects.size() == 0)
			{
				new_collectors_are_up = true;
			}
			CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
			if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
			{
				some_collectors_are_down = true;
				//for(UINT i=0; i<iter->v_projects.size(); i++)
				//	unassigned_projects.push_back( iter->v_projects[i] );
				v_fasttrack_decoyers_status.erase(iter);
			}
			else
				iter++;
		}
		if(recalculate_project_distribution)
		{
			if(some_collectors_are_down && !new_collectors_are_up)
			{
				//ReassignProjectsForFastTrackSpoofersRacksDown(unassigned_projects);
				LoadBalanceProjectsForGiftDecoyers();
			}
			if(new_collectors_are_up)
			{
				//ReassignProjectsForFastTrackSpoofers();
				LoadBalanceProjectsForGiftDecoyers();
			}
		}
	}
	if(m_allow_to_send_projects_updates_to_gift_swarmers)
	{
		//vector<ProjectKeywords> unassigned_projects;
		//check for FastTrack Swamers
		new_collectors_are_up = false;
		some_collectors_are_down = false;
		//unassigned_projects.clear();
		iter = v_fasttrack_swarmers_status.begin();
		while(iter!= v_fasttrack_swarmers_status.end())
		{
			if(iter->v_projects.size() == 0)
			{
				new_collectors_are_up = true;
			}
			CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
			if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
			{
				some_collectors_are_down = true;
				//for(UINT i=0; i<iter->v_projects.size(); i++)
				//	unassigned_projects.push_back( iter->v_projects[i] );
				v_fasttrack_swarmers_status.erase(iter);
			}
			else
				iter++;
		}
		if(recalculate_project_distribution)
		{
			if(some_collectors_are_down && !new_collectors_are_up)
			{
				//ReassignProjectsForFastTrackSwarmersRacksDown(unassigned_projects);
				LoadBalanceProjectsForGiftSwarmers();
			}
			if(new_collectors_are_up)
			{
				//ReassignProjectsForFastTrackSwarmers();
				LoadBalanceProjectsForGiftSwarmers();
			}
		}
	}
	if(m_allow_to_send_projects_updates_to_winmx_swarmers)
	{
		//check for WinMX Swarmer
		vector<ProjectKeywords> unassigned_projects;
		new_collectors_are_up = false;
		some_collectors_are_down = false;
		iter = v_winmx_swarmer_status.begin();
		while(iter!= v_winmx_swarmer_status.end())
		{
			if(iter->v_projects.size() == 0)
			{
				new_collectors_are_up = true;
			}
			CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
			if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
			{
				some_collectors_are_down = true;
				v_winmx_swarmer_status.erase(iter);
			}
			else
				iter++;
		}
		if(recalculate_project_distribution)
		{
			if(new_collectors_are_up || some_collectors_are_down)
			{
				LoadBalanceProjectsForWinmxSwarmers();
			}
		}
	}
	if(m_allow_to_send_projects_updates_to_winmx_spoofers)
	{
		//check for WinMX Spoofer
		vector<ProjectKeywords> unassigned_projects;
		new_collectors_are_up = false;
		some_collectors_are_down = false;
		iter = v_winmx_spoofer_status.begin();
		while(iter!= v_winmx_spoofer_status.end())
		{
			if(iter->v_projects.size() == 0)
			{
				new_collectors_are_up = true;
			}
			CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
			if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
			{
				some_collectors_are_down = true;
				v_winmx_spoofer_status.erase(iter);
			}
			else
				iter++;
		}
		if(recalculate_project_distribution)
		{
			if(new_collectors_are_up || some_collectors_are_down)
			{
				LoadBalanceProjectsForWinmxSpoofers();
			}
		}
	}
	if(m_allow_to_send_projects_updates_to_piolet_spoofers)
	{
		//check for Piolet Spoofer
		vector<ProjectKeywords> unassigned_projects;
		new_collectors_are_up = false;
		some_collectors_are_down = false;
		iter = v_piolet_spoofer_splitted_status.begin();
		while(iter!= v_piolet_spoofer_splitted_status.end())
		{
			if(iter->v_projects.size() == 0)
			{
				new_collectors_are_up = true;
			}
			CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
			if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
			{
				some_collectors_are_down = true;
				v_piolet_spoofer_splitted_status.erase(iter);
			}
			else
				iter++;
		}
		if(recalculate_project_distribution)
		{
			if(new_collectors_are_up || some_collectors_are_down)
			{
				LoadBalanceProjectsForPioletSpoofers();
			}
		}
	}
	if(m_allow_to_send_projects_updates_to_bearshare_spoofers)
	{
		//check for bearshare Spoofer
		vector<ProjectKeywords> unassigned_projects;
		new_collectors_are_up = false;
		some_collectors_are_down = false;
		iter = v_bearshare_spoofer_status.begin();
		while(iter!= v_bearshare_spoofer_status.end())
		{
			if(iter->v_projects.size() == 0)
			{
				new_collectors_are_up = true;
			}
			CTimeSpan ts = CTime::GetCurrentTime() - iter->m_last_init_response_time;
			if(ts.GetTotalSeconds() >= COLLECTOR_TIMEOUT) //hasn't response for more than 60 mins
			{
				some_collectors_are_down = true;
				v_bearshare_spoofer_status.erase(iter);
			}
			else
				iter++;
		}
		if(recalculate_project_distribution)
		{
			if(new_collectors_are_up || some_collectors_are_down)
			{
				LoadBalanceProjectsForBearShareSpoofers();
			}
		}
	}
}

//
//
//
void DCMasterDll::Reset()
{
	//OnTimer(4);
	
	ReadInKeywordData();
	ReassignProjectsForEmule();
	ReassignProjectsForGnutella();
	ReassignProjectsForKazaa();
	ReassignProjectsForPiolet();
	ReassignProjectsForPioletSpoofer();
	LoadBalanceProjectsForPioletSpoofers();
	ReassignProjectsForGnutellaProtector();
	ReassignProjectsForDirectConnect();
	ReassignProjectsForDirectConnectSpoofer();
	ReassignProjectsForFastTrackDC();
	//ReassignProjectsForFastTrackSpoofers();
	//ReassignProjectsForFastTrackSwarmers();
	LoadBalanceProjectsForGiftSwarmers();
	LoadBalanceProjectsForGiftSpoofers();
	LoadBalanceProjectsForGiftDecoyers();
	//ReassignProjectsForGnutellaSpoofersSplited();
	LoadBalanceProjectsForGnutellaSplitedSpoofers();
	ReassignProjectsForWinmxDC();
	//ReassignProjectsForWinmxSpoofer();
	ReassignProjectsForWinmxDecoyer();
	ReassignProjectsForSoulSeekSpoofer();
	LoadBalanceProjectsForWinmxSwarmers();
	LoadBalanceProjectsForWinmxSpoofers();
	ReassignProjectsForBitTorrent();
	ReassignProjectsForGeneric();
	ReassignProjectsForOvernetDC();
	ReassignProjectsForOvernetSpoofer();
	LoadBalanceProjectsForBearShareSpoofers();
}

//
//
//
void DCMasterDll::ReadInDatabaseInfo()
{
	CStdioFile file;
	BOOL open = file.Open("db_info.ini",CFile::typeText|CFile::modeRead|CFile::shareDenyNone);

	if(open==TRUE)
	{
		char data[256];

		LPTSTR more_data = file.ReadString(data, 256);
		
		while(more_data!=NULL)
		{
			char* host = strstr(data, "<host>");
			char* user = strstr(data, "<user>");
			char* password = strstr(data, "<password>");
			if(host!=NULL)
			{
				host = strstr(data,">");
				host++;
				CString cs_host = host;
				cs_host.Trim();
				m_db_info.m_db_host = cs_host;
			}
			else if(user!=NULL)
			{
				user = strstr(data,">");
				user++;
				CString cs_user = user;
				cs_user.Trim();
				m_db_info.m_db_user = cs_user;
			}
			else if(password!=NULL)
			{
				password = strstr(data,">");
				password++;
				CString cs_password = password;
				cs_password.Trim();
				m_db_info.m_db_password = cs_password;
			}
			more_data = file.ReadString(data, 256);
		}
		file.Close();
	}
}

//
//
//
void DCMasterDll::SendDBInfo(char* dest)
{
	int len = sizeof(DCHeader) + m_db_info.GetBufferLength();
	char* buf = new char[len];
	DCHeader* header = (DCHeader*)buf;
	header->op = DCHeader::DatabaseInfo;
	header->size = len - sizeof(DCHeader);

	m_db_info.WriteToBuffer(&buf[sizeof(DCHeader)]);

	m_com.SendReliableData(dest, buf, len);
	delete [] buf;
}

//
//
//
void DCMasterDll::SendPioletRawDBInfo(char* dest)
{
	int len = sizeof(DCHeader) + m_piolet_raw_db_info.GetBufferLength();
	char* buf = new char[len];
	DCHeader* header = (DCHeader*)buf;
	header->op = DCHeader::PioletRawDatabaseInfo;
	header->size = len - sizeof(DCHeader);

	m_piolet_raw_db_info.WriteToBuffer(&buf[sizeof(DCHeader)]);

	m_com.SendReliableData(dest, buf, len);
	delete [] buf;
}

//
//
//
void DCMasterDll::SendGnutellaRawDBInfo(char* dest)
{
	int len = sizeof(DCHeader) + m_gnutella_raw_db_info.GetBufferLength();
	char* buf = new char[len];
	DCHeader* header = (DCHeader*)buf;
	header->op = DCHeader::GnutellaRawDatabaseInfo;
	header->size = len - sizeof(DCHeader);

	m_gnutella_raw_db_info.WriteToBuffer(&buf[sizeof(DCHeader)]);

	m_com.SendReliableData(dest, buf, len);
	delete [] buf;
}

//
//
//
void DCMasterDll::SendGiftRawDBInfo(char* dest)
{
	int len = sizeof(DCHeader) + m_gift_raw_db_info.GetBufferLength();
	char* buf = new char[len];
	DCHeader* header = (DCHeader*)buf;
	header->op = DCHeader::GnutellaRawDatabaseInfo;
	header->size = len - sizeof(DCHeader);

	m_gift_raw_db_info.WriteToBuffer(&buf[sizeof(DCHeader)]);

	m_com.SendReliableData(dest, buf, len);
	delete [] buf;
}

//void DCMasterDll::WriteOutActiveGnutellaKeywordData()
//{
//	CFile keyword_data_file;
//	BOOL ret=MakeSureDirectoryPathExists("Keyword Files\\");
//
//	//delete all old keyword data
//	WIN32_FIND_DATA file_data;
//	HANDLE search_handle = ::FindFirstFile("Keyword Files\\*.*", &file_data);
//	BOOL found = FALSE;
//	if (search_handle!=INVALID_HANDLE_VALUE)
//	{
//		found = TRUE;
//	}
//	while(found == TRUE)
//	{
//		CString filename = "Keyword Files\\";
//		filename += file_data.cFileName;
//		ret = DeleteFile(filename);
//		found = ::FindNextFile(search_handle, &file_data);
//	}
//	::FindClose(search_handle);
//
//	for(UINT i=0; i<v_active_gnutella_protection_projects.size(); i++)
//	{
//		BOOL open_write = FALSE;
//
//		char filename[256+1];
//		strcpy(filename, "Keyword Files\\");
//		CString project = v_active_gnutella_protection_projects[i].m_project_name.c_str();
//		project.Replace('\\','_');			// replace the backslash with _
//		project.Replace('\'', '_');		// replace the single quote "'" with _
//		project.Replace(' ', '_');
//		project.Replace('-', '_');
//		project.Replace('&', '_');
//		project.Replace('!', '_');
//		project.Replace('$', '_');
//		project.Replace('@', '_');
//		project.Replace('%', '_');
//		project.Replace('(', '_');
//		project.Replace(')', '_');
//		project.Replace('+', '_');
//		project.Replace('~', '_');
//		project.Replace('*', '_');
//		project.Replace('.', '_');
//		project.Replace(',', '_');
//		project.Replace('?', '_');
//		project.Replace(':', '_');
//		project.Replace(';', '_');
//		project.Replace('"', '_');
//		project.Replace('/', '_');
//		strcat(filename, project);
//		strcat(filename, ".kwd");
//
//		open_write = keyword_data_file.Open(filename,CFile::typeBinary|CFile::modeCreate|CFile::modeWrite|CFile::shareDenyNone);
//
//		if(open_write==TRUE)
//		{
//			int buf_len = v_active_gnutella_protection_projects[i].GetBufferLength();
//			char *buf = new char[buf_len];
//
//			v_active_gnutella_protection_projects[i].WriteToBuffer(buf);
//
//			keyword_data_file.Write(buf, buf_len);
//
//			delete [] buf;
//
//			keyword_data_file.Close();
//		}
//	}
//}

//
//
//
/*
void DCMasterDll::WriteOutKeywordData()
{
	CFile keyword_data_file;
	BOOL ret=MakeSureDirectoryPathExists("Keyword Files\\");

	//delete all old keyword data
	WIN32_FIND_DATA file_data;
	HANDLE search_handle = ::FindFirstFile("Keyword Files\\*.*", &file_data);
	BOOL found = FALSE;
	if (search_handle!=INVALID_HANDLE_VALUE)
	{
		found = TRUE;
	}
	while(found == TRUE)
	{
		CString filename = "Keyword Files\\";
		filename += file_data.cFileName;
		ret = DeleteFile(filename);
		found = ::FindNextFile(search_handle, &file_data);
	}
	::FindClose(search_handle);

	for(UINT i=0; i<v_project_data.size(); i++)
	{
		BOOL open_write = FALSE;

		char filename[256+1];
		strcpy(filename, "Keyword Files\\");
		CString project = v_project_data[i].m_project_name.c_str();
		project.Replace('\\','_');			// replace the backslash with _
		project.Replace('\'', '_');		// replace the single quote "'" with _
		project.Replace(' ', '_');
		project.Replace('-', '_');
		project.Replace('&', '_');
		project.Replace('!', '_');
		project.Replace('$', '_');
		project.Replace('@', '_');
		project.Replace('%', '_');
		project.Replace('(', '_');
		project.Replace(')', '_');
		project.Replace('+', '_');
		project.Replace('~', '_');
		project.Replace('*', '_');
		project.Replace('.', '_');
		project.Replace(',', '_');
		project.Replace('?', '_');
		project.Replace(':', '_');
		project.Replace(';', '_');
		project.Replace('"', '_');
		project.Replace('/', '_');
		strcat(filename, project);
		strcat(filename, ".kwd");

		open_write = keyword_data_file.Open(filename,CFile::typeBinary|CFile::modeCreate|CFile::modeWrite|CFile::shareDenyNone);

		if(open_write==TRUE)
		{
			int buf_len = v_project_data[i].GetBufferLength();
			char *buf = new char[buf_len];

			v_project_data[i].WriteToBuffer(buf);

			keyword_data_file.Write(buf, buf_len);

			delete [] buf;

			keyword_data_file.Close();
		}
	}
}

//
//
//
void DCMasterDll::WriteOutGiftDCKeywordData()
{
	CFile keyword_data_file;
	BOOL ret=MakeSureDirectoryPathExists("Gift DC Keyword Files\\");

	//delete all old keyword data
	WIN32_FIND_DATA file_data;
	HANDLE search_handle = ::FindFirstFile("Gift DC Keyword Files\\*.*", &file_data);
	BOOL found = FALSE;
	if (search_handle!=INVALID_HANDLE_VALUE)
	{
		found = TRUE;
	}
	while(found == TRUE)
	{
		CString filename = "Gift DC Keyword Files\\";
		filename += file_data.cFileName;
		ret = DeleteFile(filename);
		found = ::FindNextFile(search_handle, &file_data);
	}
	::FindClose(search_handle);

	for(UINT i=0; i<v_gift_dc_project_data.size(); i++)
	{
		BOOL open_write = FALSE;

		char filename[256+1];
		strcpy(filename, "Gift DC Keyword Files\\");
		CString project = v_gift_dc_project_data[i].m_project_name.c_str();
		project.Replace('\\','_');			// replace the backslash with _
		project.Replace('\'', '_');		// replace the single quote "'" with _
		project.Replace(' ', '_');
		project.Replace('-', '_');
		project.Replace('&', '_');
		project.Replace('!', '_');
		project.Replace('$', '_');
		project.Replace('@', '_');
		project.Replace('%', '_');
		project.Replace('(', '_');
		project.Replace(')', '_');
		project.Replace('+', '_');
		project.Replace('~', '_');
		project.Replace('*', '_');
		project.Replace('.', '_');
		project.Replace(',', '_');
		project.Replace('?', '_');
		project.Replace(':', '_');
		project.Replace(';', '_');
		project.Replace('"', '_');
		project.Replace('/', '_');
		strcat(filename, project);
		strcat(filename, ".kwd");

		open_write = keyword_data_file.Open(filename,CFile::typeBinary|CFile::modeCreate|CFile::modeWrite|CFile::shareDenyNone);

		if(open_write==TRUE)
		{
			int buf_len = v_gift_dc_project_data[i].GetBufferLength();
			char *buf = new char[buf_len];

			v_gift_dc_project_data[i].WriteToBuffer(buf);

			keyword_data_file.Write(buf, buf_len);

			delete [] buf;

			keyword_data_file.Close();
		}
	}
}

//
//
//
void DCMasterDll::WriteOutGiftSwarmersKeywordData()
{
	CFile keyword_data_file;
	BOOL ret=MakeSureDirectoryPathExists("Gift Swarmer Keyword Files\\");

	//delete all old keyword data
	WIN32_FIND_DATA file_data;
	HANDLE search_handle = ::FindFirstFile("Gift Swarmer Keyword Files\\*.*", &file_data);
	BOOL found = FALSE;
	if (search_handle!=INVALID_HANDLE_VALUE)
	{
		found = TRUE;
	}
	while(found == TRUE)
	{
		CString filename = "Gift Swarmer Keyword Files\\";
		filename += file_data.cFileName;
		ret = DeleteFile(filename);
		found = ::FindNextFile(search_handle, &file_data);
	}
	::FindClose(search_handle);

	for(UINT i=0; i<v_gift_swarmers_project_data.size(); i++)
	{
		BOOL open_write = FALSE;

		char filename[256+1];
		strcpy(filename, "Gift Swarmer Keyword Files\\");
		CString project = v_gift_swarmers_project_data[i].m_project_name.c_str();
		project.Replace('\\','_');			// replace the backslash with _
		project.Replace('\'', '_');		// replace the single quote "'" with _
		project.Replace(' ', '_');
		project.Replace('-', '_');
		project.Replace('&', '_');
		project.Replace('!', '_');
		project.Replace('$', '_');
		project.Replace('@', '_');
		project.Replace('%', '_');
		project.Replace('(', '_');
		project.Replace(')', '_');
		project.Replace('+', '_');
		project.Replace('~', '_');
		project.Replace('*', '_');
		project.Replace('.', '_');
		project.Replace(',', '_');
		project.Replace('?', '_');
		project.Replace(':', '_');
		project.Replace(';', '_');
		project.Replace('"', '_');
		project.Replace('/', '_');
		strcat(filename, project);
		strcat(filename, ".kwd");

		open_write = keyword_data_file.Open(filename,CFile::typeBinary|CFile::modeCreate|CFile::modeWrite|CFile::shareDenyNone);

		if(open_write==TRUE)
		{
			int buf_len = v_gift_swarmers_project_data[i].GetBufferLength();
			char *buf = new char[buf_len];

			v_gift_swarmers_project_data[i].WriteToBuffer(buf);

			keyword_data_file.Write(buf, buf_len);

			delete [] buf;

			keyword_data_file.Close();
		}
	}
}


//
//
//
void DCMasterDll::WriteOutGiftSpoofersKeywordData()
{
	CFile keyword_data_file;
	BOOL ret=MakeSureDirectoryPathExists("Gift Spoofer Keyword Files\\");

	//delete all old keyword data
	WIN32_FIND_DATA file_data;
	HANDLE search_handle = ::FindFirstFile("Gift Spoofer Keyword Files\\*.*", &file_data);
	BOOL found = FALSE;
	if (search_handle!=INVALID_HANDLE_VALUE)
	{
		found = TRUE;
	}
	while(found == TRUE)
	{
		CString filename = "Gift Spoofer Keyword Files\\";
		filename += file_data.cFileName;
		ret = DeleteFile(filename);
		found = ::FindNextFile(search_handle, &file_data);
	}
	::FindClose(search_handle);

	for(UINT i=0; i<v_gift_spoofers_project_data.size(); i++)
	{
		BOOL open_write = FALSE;

		char filename[256+1];
		strcpy(filename, "Gift Spoofer Keyword Files\\");
		CString project = v_gift_spoofers_project_data[i].m_project_name.c_str();
		project.Replace('\\','_');			// replace the backslash with _
		project.Replace('\'', '_');		// replace the single quote "'" with _
		project.Replace(' ', '_');
		project.Replace('-', '_');
		project.Replace('&', '_');
		project.Replace('!', '_');
		project.Replace('$', '_');
		project.Replace('@', '_');
		project.Replace('%', '_');
		project.Replace('(', '_');
		project.Replace(')', '_');
		project.Replace('+', '_');
		project.Replace('~', '_');
		project.Replace('*', '_');
		project.Replace('.', '_');
		project.Replace(',', '_');
		project.Replace('?', '_');
		project.Replace(':', '_');
		project.Replace(';', '_');
		project.Replace('"', '_');
		project.Replace('/', '_');
		strcat(filename, project);
		strcat(filename, ".kwd");

		open_write = keyword_data_file.Open(filename,CFile::typeBinary|CFile::modeCreate|CFile::modeWrite|CFile::shareDenyNone);

		if(open_write==TRUE)
		{
			int buf_len = v_gift_spoofers_project_data[i].GetBufferLength();
			char *buf = new char[buf_len];

			v_gift_spoofers_project_data[i].WriteToBuffer(buf);

			keyword_data_file.Write(buf, buf_len);

			delete [] buf;

			keyword_data_file.Close();
		}
	}
}
*/
//
//
//
CTime DCMasterDll::GetLastResponseTime(CString& ip, CString& network)
{
	UINT i=0;
	if(network == "Direct Connect")
	{
		for(i=0;i<v_direct_connect_status.size();i++)
		{
			if(ip==v_direct_connect_status[i].m_ip)
				return v_direct_connect_status[i].m_last_init_response_time;
		}
	}
	else if(network == "Kazaa")
	{
		for(i=0;i<v_kazaa_network_status.size();i++)
		{
			if(ip==v_kazaa_network_status[i].m_ip)
				return v_kazaa_network_status[i].m_last_init_response_time;
		}
	}
	else if(network == "Gnutella")
	{
		for(i=0;i<v_gnutella_network_status.size();i++)
		{
			if(ip==v_gnutella_network_status[i].m_ip)
				return v_gnutella_network_status[i].m_last_init_response_time;
		}
	}
	else if(network == "Gnutella Protector")
	{
		for(i=0;i<v_gnutella_protectors_status.size();i++)
		{
			if(ip==v_gnutella_protectors_status[i].m_ip)
				return v_gnutella_protectors_status[i].m_last_init_response_time;
		}
	}
	else if(network == "Emule")
	{
		for(i=0;i<v_emule_network_status.size();i++)
		{
			if(ip==v_emule_network_status[i].m_ip)
				return v_emule_network_status[i].m_last_init_response_time;
		}
	}
	else if(network == "Piolet")
	{
		for(i=0;i<v_piolet_network_status.size();i++)
		{
			if(ip==v_piolet_network_status[i].m_ip)
				return v_piolet_network_status[i].m_last_init_response_time;
		}
	}
	else if(network == "Piolet Spoofer")
	{
		for(i=0;i<v_piolet_spoofer_status.size();i++)
		{
			if(ip==v_piolet_spoofer_status[i].m_ip)
				return v_piolet_spoofer_status[i].m_last_init_response_time;
		}
	}
	else if(network == "Piolet Spoofer Splitted")
	{
		for(i=0;i<v_piolet_spoofer_splitted_status.size();i++)
		{
			if(ip==v_piolet_spoofer_splitted_status[i].m_ip)
				return v_piolet_spoofer_splitted_status[i].m_last_init_response_time;
		}
	}
	else if(network == "Direct Connect Spoofer")
	{
		for(i=0;i<v_direct_connect_spoofer_status.size();i++)
		{
			if(ip==v_direct_connect_spoofer_status[i].m_ip)
				return v_direct_connect_spoofer_status[i].m_last_init_response_time;
		}
	}
	else if(network == "FastTrack DC")
	{
		for(i=0;i<v_fasttrack_dc_status.size();i++)
		{
			if(ip==v_fasttrack_dc_status[i].m_ip)
				return v_fasttrack_dc_status[i].m_last_init_response_time;
		}
	}
	else if(network == "FastTrack Spoofer")
	{
		for(i=0;i<v_fasttrack_spoofers_status.size();i++)
		{
			if(ip==v_fasttrack_spoofers_status[i].m_ip)
				return v_fasttrack_spoofers_status[i].m_last_init_response_time;
		}
	}
	else if(network == "FastTrack Swarmer")
	{
		for(i=0;i<v_fasttrack_swarmers_status.size();i++)
		{
			if(ip==v_fasttrack_swarmers_status[i].m_ip)
				return v_fasttrack_swarmers_status[i].m_last_init_response_time;
		}
	}
	else if(network == "FastTrack Decoyer")
	{
		for(i=0;i<v_fasttrack_decoyers_status.size();i++)
		{
			if(ip==v_fasttrack_decoyers_status[i].m_ip)
				return v_fasttrack_decoyers_status[i].m_last_init_response_time;
		}
	}
	else if(network == "Gnutella Spoofer Splitted")
	{
		for(i=0;i<v_gnutella_spoofers_splited_status.size();i++)
		{
			if(ip==v_gnutella_spoofers_splited_status[i].m_ip)
				return v_gnutella_spoofers_splited_status[i].m_last_init_response_time;
		}
	}
	else if(network == "WinMX DC")
	{
		for(i=0;i<v_winmx_dc_status.size();i++)
		{
			if(ip==v_winmx_dc_status[i].m_ip)
				return v_winmx_dc_status[i].m_last_init_response_time;
		}
	}
	else if(network == "WinMX Spoofer")
	{
		for(i=0;i<v_winmx_spoofer_status.size();i++)
		{
			if(ip==v_winmx_spoofer_status[i].m_ip)
				return v_winmx_spoofer_status[i].m_last_init_response_time;
		}
	}
	else if(network == "WinMX Swarmer")
	{
		for(i=0;i<v_winmx_swarmer_status.size();i++)
		{
			if(ip==v_winmx_swarmer_status[i].m_ip)
				return v_winmx_swarmer_status[i].m_last_init_response_time;
		}
	}
	else if(network == "WinMX Decoyer")
	{
		for(i=0;i<v_winmx_decoyer_status.size();i++)
		{
			if(ip==v_winmx_decoyer_status[i].m_ip)
				return v_winmx_decoyer_status[i].m_last_init_response_time;
		}
	}
	else if(network == "SoulSeek Spoofer")
	{
		for(i=0;i<v_soulseek_spoofer_status.size();i++)
		{
			if(ip==v_soulseek_spoofer_status[i].m_ip)
				return v_soulseek_spoofer_status[i].m_last_init_response_time;
		}
	}
	else if(network == "BitTorrent")
	{
		for(i=0;i<v_bittorrent_status.size();i++)
		{
			if(ip==v_bittorrent_status[i].m_ip)
				return v_bittorrent_status[i].m_last_init_response_time;
		}
	}
	else if(network == "Generic")
	{
		for(i=0;i<v_generic_status.size();i++)
		{
			if(ip==v_generic_status[i].m_ip)
				return v_generic_status[i].m_last_init_response_time;
		}
	}
	else if(network == "Overnet DC")
	{
		for(i=0;i<v_overnet_dc_status.size();i++)
		{
			if(ip==v_overnet_dc_status[i].m_ip)
				return v_overnet_dc_status[i].m_last_init_response_time;
		}
	}
	else if(network == "Overnet Spoofer")
	{
		for(i=0;i<v_overnet_spoofer_status.size();i++)
		{
			if(ip==v_overnet_spoofer_status[i].m_ip)
				return v_overnet_spoofer_status[i].m_last_init_response_time;
		}
	}
	else if(network == "BearShare Spoofer")
	{
		for(i=0;i<v_bearshare_spoofer_status.size();i++)
		{
			if(ip==v_bearshare_spoofer_status[i].m_ip)
				return v_bearshare_spoofer_status[i].m_last_init_response_time;
		}
	}
	return CTime::GetCurrentTime() - CTimeSpan(0,1,0,0);
}

//
//
//
int DCMasterDll::UsingProjectKeywordsVersion(const char* dest)
{
	for(UINT i=0; i<v_fasttrack_dc_status.size(); i++)
	{
		if(stricmp(dest, v_fasttrack_dc_status[i].m_ip)==0)
			return 1;
	}
	for(UINT i=0; i<v_fasttrack_spoofers_status.size(); i++)
	{
		if(stricmp(dest, v_fasttrack_spoofers_status[i].m_ip)==0)
			return 2;
	}
	for(UINT i=0; i<v_fasttrack_decoyers_status.size(); i++)
	{
		if(stricmp(dest, v_fasttrack_decoyers_status[i].m_ip)==0)
			return 2;
	}
	for(UINT i=0; i<v_fasttrack_swarmers_status.size(); i++)
	{
		if(stricmp(dest, v_fasttrack_swarmers_status[i].m_ip)==0)
			return 3;
	}
	for(UINT i=0; i<v_gnutella_spoofers_splited_status.size(); i++)
	{
		if(stricmp(dest, v_gnutella_spoofers_splited_status[i].m_ip)==0)
			return 4;
	}
	for(UINT i=0; i<v_gnutella_protectors_status.size(); i++)
	{
		if(stricmp(dest, v_gnutella_protectors_status[i].m_ip)==0)
			return 5;
	}
	for(UINT i=0; i<v_winmx_dc_status.size(); i++)
	{
		if(stricmp(dest, v_winmx_dc_status[i].m_ip)==0)
			return 6;
	}
	for(UINT i=0; i<v_winmx_spoofer_status.size(); i++)
	{
		if(stricmp(dest, v_winmx_spoofer_status[i].m_ip)==0)
			return 6;
	}
	for(UINT i=0; i<v_winmx_swarmer_status.size(); i++)
	{
		if(stricmp(dest, v_winmx_swarmer_status[i].m_ip)==0)
			return 6;
	}
	for(UINT i=0; i<v_winmx_decoyer_status.size(); i++)
	{
		if(stricmp(dest, v_winmx_decoyer_status[i].m_ip)==0)
			return 6;
	}
	for(UINT i=0; i<v_soulseek_spoofer_status.size(); i++)
	{
		if(stricmp(dest, v_soulseek_spoofer_status[i].m_ip)==0)
			return 7;
	}	
	for(UINT i=0; i<v_bittorrent_status.size(); i++)
	{
		if(stricmp(dest, v_bittorrent_status[i].m_ip)==0)
			return 8;
	}	
	for(UINT i=0; i<v_overnet_dc_status.size(); i++)
	{
		if(stricmp(dest, v_overnet_dc_status[i].m_ip)==0)
			return 9;
	}	
	for(UINT i=0; i<v_overnet_spoofer_status.size(); i++)
	{
		if(stricmp(dest, v_overnet_spoofer_status[i].m_ip)==0)
			return 9;
	}
	for(UINT i=0; i<v_generic_status.size(); i++)
	{
		if(stricmp(dest, v_generic_status[i].m_ip)==0)
			return 10;
	}	
	for(UINT i=0; i<v_piolet_spoofer_status.size(); i++)
	{
		if(stricmp(dest, v_piolet_spoofer_status[i].m_ip)==0)
			return 11;
	}	
	for(UINT i=0; i<v_piolet_spoofer_splitted_status.size(); i++)
	{
		if(stricmp(dest, v_piolet_spoofer_splitted_status[i].m_ip)==0)
			return 11;
	}	
	for(UINT i=0; i<v_bearshare_spoofer_status.size(); i++)
	{
		if(stricmp(dest, v_bearshare_spoofer_status[i].m_ip)==0)
			return 12;
	}	

	return 0;
}

//
//
//
void DCMasterDll::OnBnClickedBrowseProjectsButton()
{
	m_browse_projects_dlg.ShowWindow(SW_NORMAL);
	m_browse_projects_dlg.SetForegroundWindow();
}

//
//
//
void DCMasterDll::OnRestart(vector<CString>& selected_racks)
{
	RackRecoverInterface rri;

	for(UINT i=0;i<selected_racks.size();i++)
	{
		rri.RestartRack(selected_racks[i]);
		rri.RestartRack(selected_racks[i]);
		rri.RestartRack(selected_racks[i]);
	}
}


//
//
//
void DCMasterDll::LoadBalanceProjectsForGiftSwarmers()
{
	m_dlg.ClearProjectsNumber("FastTrack Swarmer");

	if(v_fasttrack_swarmers_status.size()>0)
	{
		
		int num_swarmers = v_fasttrack_swarmers_status.size();
		/*
		//randomize the order of the swarmers
		vector<DCStatus> temp;
		vector<DCStatus>::iterator iter = v_fasttrack_swarmers_status.begin();
		while(iter!=v_fasttrack_swarmers_status.end())
		{
			int index=rand()%v_fasttrack_swarmers_status.size();
			iter+=index;
			iter->v_projects.clear();
			temp.push_back(*(iter));
			v_fasttrack_swarmers_status.erase(iter);
			iter = v_fasttrack_swarmers_status.begin();
		}
		v_fasttrack_swarmers_status = temp;
		*/
		//sort the spoofers' ips
		sort(v_fasttrack_swarmers_status.begin(),v_fasttrack_swarmers_status.end());



		UINT i,j;
		//clear all existing distributed projects
		for(i=0; i<num_swarmers; i++)
		{
			v_fasttrack_swarmers_status[i].v_projects.clear();
		}

		for(i=0; i<v_gift_swarmers_project_data.size(); i++)
		{
			vector<UINT>assigned_collectors;

			//int num_racks_to_use = (int)(((float)v_gift_swarmers_project_data[i].m_kazaa_swarm_load / (float)100)*(float)num_swarmers);
			int num_racks_to_use = ((int)(((float)v_gift_swarmers_project_data[i].m_kazaa_swarm_load / (float)100)*(float)num_swarmers))/m_gift_swarmer_load_balancing_divider;
			if(num_racks_to_use<=0)
				num_racks_to_use=1;
			for(j=0;j<num_racks_to_use;j++)
			{
				int index = GetLeastNumberOfProjects(assigned_collectors,v_fasttrack_swarmers_status);
				if(index>=0)
				{
					assigned_collectors.push_back(index);
					v_fasttrack_swarmers_status[index].v_projects.push_back(v_gift_swarmers_project_data[i]);
				}
				else
					break;
			}
		}
		for(i=0; i<v_fasttrack_swarmers_status.size(); i++)
		{
			SendProjectChecksums(v_fasttrack_swarmers_status[i]);
			m_dlg.UpdateCollectorStatus(v_fasttrack_swarmers_status[i],"FastTrack Swarmer");
		}
	}
	m_browse_projects_dlg.BuildProjectTree("Gift Swarmer",v_fasttrack_swarmers_status);
}

//
//
//
void DCMasterDll::LoadBalanceProjectsForGiftSpoofers()
{
	m_dlg.ClearProjectsNumber("FastTrack Spoofer");

	if(v_fasttrack_spoofers_status.size()>0)
	{
		
		int num_spoofers = v_fasttrack_spoofers_status.size();
		/*
		//randomize the order of the spoofers
		vector<DCStatus> temp;
		vector<DCStatus>::iterator iter = v_fasttrack_spoofers_status.begin();
		while(iter!=v_fasttrack_spoofers_status.end())
		{
			int index=rand()%v_fasttrack_spoofers_status.size();
			iter+=index;
			iter->v_projects.clear();
			temp.push_back(*(iter));
			v_fasttrack_spoofers_status.erase(iter);
			iter = v_fasttrack_spoofers_status.begin();
		}
		v_fasttrack_spoofers_status = temp;
		*/

		//sort the spoofers' ips
		sort(v_fasttrack_spoofers_status.begin(),v_fasttrack_spoofers_status.end());



		UINT i,j;
		//clear all existing distributed projects
		for(i=0; i<num_spoofers; i++)
		{
			v_fasttrack_spoofers_status[i].v_projects.clear();
		}

		for(i=0; i<v_gift_spoofers_project_data.size(); i++)
		{
			vector<UINT>assigned_collectors;
			int num_racks_to_use = ((int)(((float)v_gift_spoofers_project_data[i].m_kazaa_swarm_load / (float)100)*(float)num_spoofers))/m_gift_spoofer_load_balancing_divider;
			if(num_racks_to_use<=0)
				num_racks_to_use=1;
			for(j=0;j<num_racks_to_use;j++)
			{
				int index = GetLeastNumberOfProjects(assigned_collectors,v_fasttrack_spoofers_status);
				if(index>=0)
				{
					assigned_collectors.push_back(index);
					v_fasttrack_spoofers_status[index].v_projects.push_back(v_gift_spoofers_project_data[i]);
				}
				else
					break;
			}
		}
		for(i=0; i<v_fasttrack_spoofers_status.size(); i++)
		{
			SendProjectChecksums(v_fasttrack_spoofers_status[i]);
			m_dlg.UpdateCollectorStatus(v_fasttrack_spoofers_status[i],"FastTrack Spoofer");
		}
	}
	m_browse_projects_dlg.BuildProjectTree("Gift Spoofer",v_fasttrack_spoofers_status);
}

//
//
//
void DCMasterDll::LoadBalanceProjectsForGiftDecoyers()
{
	m_dlg.ClearProjectsNumber("FastTrack Decoyer");

	if(v_fasttrack_decoyers_status.size()>0)
	{
		
		int num_spoofers = v_fasttrack_decoyers_status.size();
		/*
		//randomize the order of the spoofers
		vector<DCStatus> temp;
		vector<DCStatus>::iterator iter = v_fasttrack_spoofers_status.begin();
		while(iter!=v_fasttrack_spoofers_status.end())
		{
			int index=rand()%v_fasttrack_spoofers_status.size();
			iter+=index;
			iter->v_projects.clear();
			temp.push_back(*(iter));
			v_fasttrack_spoofers_status.erase(iter);
			iter = v_fasttrack_spoofers_status.begin();
		}
		v_fasttrack_spoofers_status = temp;
		*/

		//sort the spoofers' ips
		sort(v_fasttrack_decoyers_status.begin(),v_fasttrack_decoyers_status.end());



		UINT i,j;
		//clear all existing distributed projects
		for(i=0; i<num_spoofers; i++)
		{
			v_fasttrack_decoyers_status[i].v_projects.clear();
		}

		for(i=0; i<v_gift_spoofers_project_data.size(); i++)
		{
			vector<UINT>assigned_collectors;
			int num_racks_to_use = ((int)(((float)v_gift_spoofers_project_data[i].m_kazaa_swarm_load / (float)100)*(float)num_spoofers))/m_gift_spoofer_load_balancing_divider;
			if(num_racks_to_use<=0)
				num_racks_to_use=1;
			for(j=0;j<num_racks_to_use;j++)
			{
				int index = GetLeastNumberOfProjects(assigned_collectors,v_fasttrack_decoyers_status);
				if(index>=0)
				{
					assigned_collectors.push_back(index);
					v_fasttrack_decoyers_status[index].v_projects.push_back(v_gift_spoofers_project_data[i]);
				}
				else
					break;
			}
		}
		for(i=0; i<v_fasttrack_decoyers_status.size(); i++)
		{
			SendProjectChecksums(v_fasttrack_decoyers_status[i]);
			m_dlg.UpdateCollectorStatus(v_fasttrack_decoyers_status[i],"FastTrack Decoyer");
		}
	}
	m_browse_projects_dlg.BuildProjectTree("Gift Decoyer",v_fasttrack_decoyers_status);
}

//
//
//
int DCMasterDll::GetLeastNumberOfProjects(vector<UINT>& exclude_collectors, vector<DCStatus>& network)
{
	int index=-1;
	int least_projects=999999;
	for(UINT i=0; i<network.size(); i++)
	{
		bool skip = false;
		for(UINT j=0; j<exclude_collectors.size();j++)
		{
			if(exclude_collectors[j] == i)
			{
				skip = true;
				break;
			}
		}
		if(!skip)
		{
			if(network[i].v_projects.size() < least_projects)
			{
				index = i;
				least_projects = (UINT)network[i].v_projects.size();
			}
		}
	}
	return index;
}

//
//
//
int DCMasterDll::GetNumRacksAssignedForProject(int project_id, vector<DCStatus>& network, vector<UINT>& assigned_racks)
{
	int count=0;
	for(UINT i=0; i<network.size(); i++)
	{
		for(UINT j=0; j<network[i].v_projects.size(); j++)
		{
			if(project_id == network[i].v_projects[j].m_id)
			{
				count++;
				assigned_racks.push_back(i);
				break;
			}
		}
	}
	return count;
}

//
//
//
//
//
//
void DCMasterDll::LoadBalanceProjectsForGnutellaSplitedSpoofers()
{
	m_dlg.ClearProjectsNumber("Gnutella Spoofer Splitted");

	if(v_gnutella_spoofers_splited_status.size()>0)
	{
		//sort the spoofers' ips
		sort(v_gnutella_spoofers_splited_status.begin(),v_gnutella_spoofers_splited_status.end());

		UINT i,j;
		UINT num_swarmers = v_gnutella_spoofers_splited_status.size();
		//clear all existing distributed projects
		for(i=0; i<num_swarmers; i++)
		{
			v_gnutella_spoofers_splited_status[i].v_projects.clear();
		}

		//go through all the gnutella protection projects
		for(i=0; i<v_active_gnutella_protection_projects.size(); i++)
		{
			if(v_active_gnutella_protection_projects[i].m_id == 2903 )//Common Words project
			{
				for(j=0;j<v_gnutella_spoofers_splited_status.size();j++)
					v_gnutella_spoofers_splited_status[j].v_projects.push_back(v_active_gnutella_protection_projects[i]);
			}
			else
			{
				vector<UINT>assigned_collectors;
				int num_racks_to_use = ((int)(((float)v_active_gnutella_protection_projects[i].m_gnutella_load / (float)100)*(float)num_swarmers))/m_gnutella_load_balancing_divider;
				if(num_racks_to_use<=0)
					num_racks_to_use=1;
				for(j=0;j<num_racks_to_use;j++)
				{
					int index = GetLeastNumberOfProjects(assigned_collectors,v_gnutella_spoofers_splited_status);
					if(index>=0)
					{
						assigned_collectors.push_back(index);
						v_gnutella_spoofers_splited_status[index].v_projects.push_back(v_active_gnutella_protection_projects[i]);
					}
					else
						break;
				}
			}
		}
		//go through all the gnutella singles projects
		for(i=0; i<v_singles_project_data.size(); i++)
		{
			vector<UINT>assigned_collectors;

			int num_racks_to_use = ((int)(((float)v_singles_project_data[i].m_kazaa_swarm_load / (float)100)*(float)num_swarmers))/m_gnutella_singles_load_balancing_divider;
			if(num_racks_to_use<=0)
				num_racks_to_use=1;
			for(j=0;j<num_racks_to_use;j++)
			{
				int index = GetLeastNumberOfProjects(assigned_collectors,v_gnutella_spoofers_splited_status);
				if(index>=0)
				{
					assigned_collectors.push_back(index);
					v_gnutella_spoofers_splited_status[index].v_projects.push_back(v_singles_project_data[i]);
				}
				else
					break;
			}
		}
		for(i=0; i<v_gnutella_spoofers_splited_status.size(); i++)
		{
			SendProjectChecksums(v_gnutella_spoofers_splited_status[i]);
			m_dlg.UpdateCollectorStatus(v_gnutella_spoofers_splited_status[i],"Gnutella Spoofer Splitted");
		}
	}
	m_browse_projects_dlg.BuildProjectTree("Gnutella Spoofer Splitted",v_gnutella_spoofers_splited_status);
}

//
//
//
int DCMasterDll::AlterGnutellaSinglesLoadBalancingDivider(int value)
{
	m_gnutella_singles_load_balancing_divider+=value;
	// Store in the registry
	RegistryKeys::SetDividers(m_gnutella_load_balancing_divider,m_gnutella_singles_load_balancing_divider);
	return m_gnutella_singles_load_balancing_divider;

}

//
//
//
int DCMasterDll::AlterGnutellaLoadBalancingDivider(int value)
{
	m_gnutella_load_balancing_divider+=value;
	// Store in the registry
	RegistryKeys::SetDividers(m_gnutella_load_balancing_divider,m_gnutella_singles_load_balancing_divider);
	return m_gnutella_load_balancing_divider;
}

//
//
//
int DCMasterDll::AlterPioletLoadBalancingDivider(int value)
{
	m_piolet_load_balancing_divider+=value;
	// Store in the registry
	RegistryKeys::SetPioletDivider(m_piolet_load_balancing_divider);
	return m_piolet_load_balancing_divider;
}

//
//
//
int DCMasterDll::AlterBearShareLoadBalancingDivider(int value)
{
	m_bearshare_load_balancing_divider+=value;
	// Store in the registry
	RegistryKeys::SetBearShareDivider(m_bearshare_load_balancing_divider);
	return m_bearshare_load_balancing_divider;
}

//
//
//
int DCMasterDll::AlterGiftSwarmerLoadBalancingDivider(int value)
{
	m_gift_swarmer_load_balancing_divider+=value;
	// Store in the registry
	RegistryKeys::SetGiftSwarmerDivider(m_gift_swarmer_load_balancing_divider);
	return m_gift_swarmer_load_balancing_divider;
}

//
//
//
int DCMasterDll::AlterGiftSpooferLoadBalancingDivider(int value)
{
	m_gift_spoofer_load_balancing_divider+=value;
	// Store in the registry
	RegistryKeys::SetGiftSpooferDivider(m_gift_spoofer_load_balancing_divider);
	return m_gift_spoofer_load_balancing_divider;
}

//
//
//
void DCMasterDll::ResetSplittedGnutellaProjects()
{
	if(!m_allow_to_send_projects_updates_to_splitted_gnutella_spoofers)
	{
		m_allow_to_send_projects_updates_to_splitted_gnutella_spoofers=true;
		ReadInKeywordData();
		CheckForCollectorsStatus(false);
		m_allow_to_send_projects_updates_to_splitted_gnutella_spoofers=false;
	}
	else
	{
		ReadInKeywordData();
		CheckForCollectorsStatus(false);
	}
	LoadBalanceProjectsForGnutellaSplitedSpoofers();
	//LoadBalanceProjectsForGiftSwarmers();
	//LoadBalanceProjectsForGiftSpoofers();
}

//
//
//
void DCMasterDll::ResetSplittedPioletProjects()
{
	if(!m_allow_to_send_projects_updates_to_piolet_spoofers)
	{
		m_allow_to_send_projects_updates_to_piolet_spoofers=true;
		ReadInKeywordData();
		CheckForCollectorsStatus(false);
		m_allow_to_send_projects_updates_to_piolet_spoofers=false;
	}
	else
	{
		ReadInKeywordData();
		CheckForCollectorsStatus(false);
	}
	LoadBalanceProjectsForPioletSpoofers();
}

//
//
//
void DCMasterDll::ResetGiftSpooferProjects()
{
	if(!m_allow_to_send_projects_updates_to_gift_spoofers)
	{
		m_allow_to_send_projects_updates_to_gift_spoofers=true;
		ReadInKeywordData();
		CheckForCollectorsStatus(false);
		m_allow_to_send_projects_updates_to_gift_spoofers=false;
	}
	else
	{
		ReadInKeywordData();
		CheckForCollectorsStatus(false);
	}
	LoadBalanceProjectsForGiftSpoofers();
	LoadBalanceProjectsForGiftDecoyers();
}

//
//
//
void DCMasterDll::ResetGiftSwarmerProjects()
{
	if(!m_allow_to_send_projects_updates_to_gift_swarmers)
	{
		m_allow_to_send_projects_updates_to_gift_swarmers=true;
		ReadInKeywordData();
		CheckForCollectorsStatus(false);
		m_allow_to_send_projects_updates_to_gift_swarmers=false;
	}
	else
	{
		ReadInKeywordData();
		CheckForCollectorsStatus(false);
	}
	LoadBalanceProjectsForGiftSwarmers();
}

//
//
//
void DCMasterDll::ResetWinmxSpooferProjects()
{
	if(!m_allow_to_send_projects_updates_to_winmx_spoofers)
	{
		m_allow_to_send_projects_updates_to_winmx_spoofers=true;
		ReadInKeywordData();
		CheckForCollectorsStatus(false);
		m_allow_to_send_projects_updates_to_winmx_spoofers=false;
	}
	else
	{
		ReadInKeywordData();
		CheckForCollectorsStatus(false);
	}
	LoadBalanceProjectsForWinmxSpoofers();
}

//
//
//
void DCMasterDll::ResetWinmxSwarmerProjects()
{
	if(!m_allow_to_send_projects_updates_to_winmx_swarmers)
	{
		m_allow_to_send_projects_updates_to_winmx_swarmers=true;
		ReadInKeywordData();
		CheckForCollectorsStatus(false);
		m_allow_to_send_projects_updates_to_winmx_swarmers=false;
	}
	else
	{
		ReadInKeywordData();
		CheckForCollectorsStatus(false);
	}
	LoadBalanceProjectsForWinmxSwarmers();
}

//
//
//
void DCMasterDll::ResetBearShareProjects()
{
	if(!m_allow_to_send_projects_updates_to_bearshare_spoofers)
	{
		m_allow_to_send_projects_updates_to_bearshare_spoofers=true;
		ReadInKeywordData();
		CheckForCollectorsStatus(false);
		m_allow_to_send_projects_updates_to_bearshare_spoofers=false;
	}
	else
	{
		ReadInKeywordData();
		CheckForCollectorsStatus(false);
	}
	LoadBalanceProjectsForBearShareSpoofers();
}

//
//
//
void DCMasterDll::RestartCrashedRacks()
{
	RackRecoverInterface rri;
	UINT i;
	for(i=0;i<v_gnutella_spoofers_splited_status.size();i++)
	{
		CTimeSpan ts = CTime::GetCurrentTime() - v_gnutella_spoofers_splited_status[i].m_last_init_response_time;
		if(ts.GetTotalSeconds() >= 30*60) //hasn't response for more than 30 mins
		{
			rri.RestartRack(v_gnutella_spoofers_splited_status[i].m_ip);
			rri.RestartRack(v_gnutella_spoofers_splited_status[i].m_ip);
			rri.RestartRack(v_gnutella_spoofers_splited_status[i].m_ip);
			CStdioFile file;
			if(file.Open("rack_restarted_log.txt",CFile::typeText|CFile::modeWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::shareDenyWrite)!=0)
			{
				file.SeekToEnd();
				CString str = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S ");
				str.AppendFormat("Gnutella Splitted Spoofer - %s\n",v_gnutella_spoofers_splited_status[i].m_ip);
				file.WriteString(str);
				file.Close();
			}
		}
	}
	for(i=0;i<v_fasttrack_spoofers_status.size();i++)
	{
		CTimeSpan ts = CTime::GetCurrentTime() - v_fasttrack_spoofers_status[i].m_last_init_response_time;
		if(ts.GetTotalSeconds() >= 30*60) //hasn't response for more than 30 mins
		{
			rri.RestartRack(v_fasttrack_spoofers_status[i].m_ip);
			rri.RestartRack(v_fasttrack_spoofers_status[i].m_ip);
			rri.RestartRack(v_fasttrack_spoofers_status[i].m_ip);
			CStdioFile file;
			if(file.Open("rack_restarted_log.txt",CFile::typeText|CFile::modeWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::shareDenyWrite)!=0)
			{
				file.SeekToEnd();
				CString str = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S ");
				str.AppendFormat("Fasttrack Spoofer - %s\n",v_fasttrack_spoofers_status[i].m_ip);
				file.WriteString(str);
				file.Close();
			}
		}
	}
	for(i=0;i<v_fasttrack_decoyers_status.size();i++)
	{
		CTimeSpan ts = CTime::GetCurrentTime() - v_fasttrack_decoyers_status[i].m_last_init_response_time;
		if(ts.GetTotalSeconds() >= 30*60) //hasn't response for more than 30 mins
		{
			rri.RestartRack(v_fasttrack_decoyers_status[i].m_ip);
			rri.RestartRack(v_fasttrack_decoyers_status[i].m_ip);
			rri.RestartRack(v_fasttrack_decoyers_status[i].m_ip);
			CStdioFile file;
			if(file.Open("rack_restarted_log.txt",CFile::typeText|CFile::modeWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::shareDenyWrite)!=0)
			{
				file.SeekToEnd();
				CString str = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S ");
				str.AppendFormat("FastTrack Decoyer - %s\n",v_fasttrack_decoyers_status[i].m_ip);
				file.WriteString(str);
				file.Close();
			}
		}
	}
	for(i=0;i<v_fasttrack_swarmers_status.size();i++)
	{
		CTimeSpan ts = CTime::GetCurrentTime() - v_fasttrack_swarmers_status[i].m_last_init_response_time;
		if(ts.GetTotalSeconds() >= 30*60) //hasn't response for more than 30 mins
		{
			rri.RestartRack(v_fasttrack_swarmers_status[i].m_ip);
			rri.RestartRack(v_fasttrack_swarmers_status[i].m_ip);
			rri.RestartRack(v_fasttrack_swarmers_status[i].m_ip);
			CStdioFile file;
			if(file.Open("rack_restarted_log.txt",CFile::typeText|CFile::modeWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::shareDenyWrite)!=0)
			{
				file.SeekToEnd();
				CString str = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S ");
				str.AppendFormat("Fasttrack Swarmer - %s\n",v_fasttrack_swarmers_status[i].m_ip);
				file.WriteString(str);
				file.Close();
			}
		}
	}
	for(i=0;i<v_winmx_spoofer_status.size();i++)
	{
		CTimeSpan ts = CTime::GetCurrentTime() - v_winmx_spoofer_status[i].m_last_init_response_time;
		if(ts.GetTotalSeconds() >= 30*60) //hasn't response for more than 30 mins
		{
			rri.RestartRack(v_winmx_spoofer_status[i].m_ip);
			rri.RestartRack(v_winmx_spoofer_status[i].m_ip);
			rri.RestartRack(v_winmx_spoofer_status[i].m_ip);
			CStdioFile file;
			if(file.Open("rack_restarted_log.txt",CFile::typeText|CFile::modeWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::shareDenyWrite)!=0)
			{
				file.SeekToEnd();
				CString str = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S ");
				str.AppendFormat("WinMX Spoofer - %s\n",v_winmx_spoofer_status[i].m_ip);
				file.WriteString(str);
				file.Close();
			}
		}
	}
	for(i=0;i<v_winmx_swarmer_status.size();i++)
	{
		CTimeSpan ts = CTime::GetCurrentTime() - v_winmx_swarmer_status[i].m_last_init_response_time;
		if(ts.GetTotalSeconds() >= 30*60) //hasn't response for more than 30 mins
		{
			rri.RestartRack(v_winmx_swarmer_status[i].m_ip);
			rri.RestartRack(v_winmx_swarmer_status[i].m_ip);
			rri.RestartRack(v_winmx_swarmer_status[i].m_ip);
			CStdioFile file;
			if(file.Open("rack_restarted_log.txt",CFile::typeText|CFile::modeWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::shareDenyWrite)!=0)
			{
				file.SeekToEnd();
				CString str = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S ");
				str.AppendFormat("WinMX Swarmer - %s\n",v_winmx_swarmer_status[i].m_ip);
				file.WriteString(str);
				file.Close();
			}
		}
	}
	for(i=0;i<v_overnet_spoofer_status.size();i++)
	{
		CTimeSpan ts = CTime::GetCurrentTime() - v_overnet_spoofer_status[i].m_last_init_response_time;
		if(ts.GetTotalSeconds() >= 30*60) //hasn't response for more than 30 mins
		{
			rri.RestartRack(v_overnet_spoofer_status[i].m_ip);
			rri.RestartRack(v_overnet_spoofer_status[i].m_ip);
			rri.RestartRack(v_overnet_spoofer_status[i].m_ip);
			CStdioFile file;
			if(file.Open("rack_restarted_log.txt",CFile::typeText|CFile::modeWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::shareDenyWrite)!=0)
			{
				file.SeekToEnd();
				CString str = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S ");
				str.AppendFormat("Overnet Spoofer - %s\n",v_overnet_spoofer_status[i].m_ip);
				file.WriteString(str);
				file.Close();
			}
		}
	}
	for(i=0;i<v_piolet_spoofer_splitted_status.size();i++)
	{
		CTimeSpan ts = CTime::GetCurrentTime() - v_piolet_spoofer_splitted_status[i].m_last_init_response_time;
		if(ts.GetTotalSeconds() >= 30*60) //hasn't response for more than 30 mins
		{
			rri.RestartRack(v_piolet_spoofer_splitted_status[i].m_ip);
			rri.RestartRack(v_piolet_spoofer_splitted_status[i].m_ip);
			rri.RestartRack(v_piolet_spoofer_splitted_status[i].m_ip);
			CStdioFile file;
			if(file.Open("rack_restarted_log.txt",CFile::typeText|CFile::modeWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::shareDenyWrite)!=0)
			{
				file.SeekToEnd();
				CString str = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S ");
				str.AppendFormat("Piolet Spoofer Splitted - %s\n",v_piolet_spoofer_splitted_status[i].m_ip);
				file.WriteString(str);
				file.Close();
			}
		}
	}
	for(i=0;i<v_bearshare_spoofer_status.size();i++)
	{
		CTimeSpan ts = CTime::GetCurrentTime() - v_bearshare_spoofer_status[i].m_last_init_response_time;
		if(ts.GetTotalSeconds() >= 30*60) //hasn't response for more than 30 mins
		{
			rri.RestartRack(v_bearshare_spoofer_status[i].m_ip);
			rri.RestartRack(v_bearshare_spoofer_status[i].m_ip);
			rri.RestartRack(v_bearshare_spoofer_status[i].m_ip);
			CStdioFile file;
			if(file.Open("rack_restarted_log.txt",CFile::typeText|CFile::modeWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::shareDenyWrite)!=0)
			{
				file.SeekToEnd();
				CString str = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S ");
				str.AppendFormat("BearShare Spoofer Splitted - %s\n",v_bearshare_spoofer_status[i].m_ip);
				file.WriteString(str);
				file.Close();
			}
		}
	}
}

//
//
//
void DCMasterDll::LoadBalanceProjectsForWinmxSwarmers()
{
	m_dlg.ClearProjectsNumber("WinMX Swarmer");

	if(v_winmx_swarmer_status.size()>0)
	{
		
		int num_swarmers = v_winmx_swarmer_status.size();
		//sort the swarmerss' ips
		sort(v_winmx_swarmer_status.begin(),v_winmx_swarmer_status.end());

		UINT i,j;
		//clear all existing distributed projects
		for(i=0; i<num_swarmers; i++)
		{
			v_winmx_swarmer_status[i].v_projects.clear();
		}

		for(i=0; i<v_winmx_swarmers_project_data.size(); i++)
		{
			vector<UINT>assigned_collectors;

			int num_racks_to_use = (int)(((float)v_winmx_swarmers_project_data[i].m_kazaa_swarm_load / (float)100)*(float)num_swarmers);
			if(num_racks_to_use<=0)
				num_racks_to_use=1;
			for(j=0;j<num_racks_to_use;j++)
			{
				int index = GetLeastNumberOfProjects(assigned_collectors,v_winmx_swarmer_status);
				if(index>=0)
				{
					assigned_collectors.push_back(index);
					v_winmx_swarmer_status[index].v_projects.push_back(v_winmx_swarmers_project_data[i]);
				}
				else
					break;
			}
		}
		for(i=0; i<v_winmx_swarmer_status.size(); i++)
		{
			SendProjectChecksums(v_winmx_swarmer_status[i]);
			m_dlg.UpdateCollectorStatus(v_winmx_swarmer_status[i],"WinMX Swarmer");
		}
	}
	m_browse_projects_dlg.BuildProjectTree("WinMX Swarmer",v_winmx_swarmer_status);
}

//
//
//
void DCMasterDll::LoadBalanceProjectsForWinmxSpoofers()
{
	m_dlg.ClearProjectsNumber("WinMX Spoofer");

	if(v_winmx_spoofer_status.size()>0)
	{
		
		int num_swarmers = v_winmx_spoofer_status.size();
		//sort the swarmerss' ips
		sort(v_winmx_spoofer_status.begin(),v_winmx_spoofer_status.end());

		UINT i,j;
		//clear all existing distributed projects
		for(i=0; i<num_swarmers; i++)
		{
			v_winmx_spoofer_status[i].v_projects.clear();
		}

		for(i=0; i<v_winmx_spoofers_project_data.size(); i++)
		{
			vector<UINT>assigned_collectors;

			int num_racks_to_use = (int)(((float)v_winmx_spoofers_project_data[i].m_kazaa_swarm_load / (float)100)*(float)num_swarmers);
			if(num_racks_to_use<=0)
				num_racks_to_use=1;
			for(j=0;j<num_racks_to_use;j++)
			{
				int index = GetLeastNumberOfProjects(assigned_collectors,v_winmx_spoofer_status);
				if(index>=0)
				{
					assigned_collectors.push_back(index);
					v_winmx_spoofer_status[index].v_projects.push_back(v_winmx_spoofers_project_data[i]);
				}
				else
					break;
			}
		}
		for(i=0; i<v_winmx_spoofer_status.size(); i++)
		{
			SendProjectChecksums(v_winmx_spoofer_status[i]);
			m_dlg.UpdateCollectorStatus(v_winmx_spoofer_status[i],"WinMX Spoofer");
		}
	}
	m_browse_projects_dlg.BuildProjectTree("WinMX Spoofer",v_winmx_spoofer_status);
}

//
//
//
void DCMasterDll::ReceivedRequestProjectChecksums(char* from)
{
	for(UINT i=0; i<v_fasttrack_dc_status.size(); i++)
	{
		if(stricmp(from, v_fasttrack_dc_status[i].m_ip)==0)
		{
			SendProjectChecksums(v_fasttrack_dc_status[i]);
			return;
		}
	}
	for(UINT i=0; i<v_fasttrack_spoofers_status.size(); i++)
	{
		if(stricmp(from, v_fasttrack_spoofers_status[i].m_ip)==0)
		{
			SendProjectChecksums(v_fasttrack_spoofers_status[i]);
			return ;
		}
	}
	for(UINT i=0; i<v_fasttrack_decoyers_status.size(); i++)
	{
		if(stricmp(from, v_fasttrack_decoyers_status[i].m_ip)==0)
		{
			SendProjectChecksums(v_fasttrack_decoyers_status[i]);
			return ;
		}
	}
	for(UINT i=0; i<v_fasttrack_swarmers_status.size(); i++)
	{
		if(stricmp(from, v_fasttrack_swarmers_status[i].m_ip)==0)
		{
			SendProjectChecksums(v_fasttrack_swarmers_status[i]);
			return ;
		}
	}
	for(UINT i=0; i<v_gnutella_spoofers_splited_status.size(); i++)
	{
		if(stricmp(from, v_gnutella_spoofers_splited_status[i].m_ip)==0)
		{
			SendProjectChecksums(v_gnutella_spoofers_splited_status[i]);
			return ;
		}
	}
	for(UINT i=0; i<v_gnutella_protectors_status.size(); i++)
	{
		if(stricmp(from, v_gnutella_protectors_status[i].m_ip)==0)
		{
			SendProjectChecksums(v_gnutella_protectors_status[i]);
			return ;
		}
	}
	for(UINT i=0; i<v_winmx_dc_status.size(); i++)
	{
		if(stricmp(from, v_winmx_dc_status[i].m_ip)==0)
		{
			SendProjectChecksums(v_winmx_dc_status[i]);
			return ;
		}
	}
	for(UINT i=0; i<v_winmx_spoofer_status.size(); i++)
	{
		if(stricmp(from, v_winmx_spoofer_status[i].m_ip)==0)
		{
			SendProjectChecksums(v_winmx_spoofer_status[i]);
			return ;
		}
	}
	for(UINT i=0; i<v_winmx_swarmer_status.size(); i++)
	{
		if(stricmp(from, v_winmx_swarmer_status[i].m_ip)==0)
		{
			SendProjectChecksums(v_winmx_swarmer_status[i]);
			return ;
		}
	}
	for(UINT i=0; i<v_winmx_decoyer_status.size(); i++)
	{
		if(stricmp(from, v_winmx_decoyer_status[i].m_ip)==0)
		{
			SendProjectChecksums(v_winmx_decoyer_status[i]);
			return ;
		}
	}
	for(UINT i=0; i<v_soulseek_spoofer_status.size(); i++)
	{
		if(stricmp(from, v_soulseek_spoofer_status[i].m_ip)==0)
		{
			SendProjectChecksums(v_soulseek_spoofer_status[i]);
			return ;
		}
	}
	for(UINT i=0; i<v_emule_network_status.size(); i++)
	{
		if(stricmp(from, v_emule_network_status[i].m_ip)==0)
		{
			SendProjectChecksums(v_emule_network_status[i]);
			return ;
		}
	}
	for(UINT i=0; i<v_gnutella_network_status.size(); i++)
	{
		if(stricmp(from, v_gnutella_network_status[i].m_ip)==0)
		{
			SendProjectChecksums(v_gnutella_network_status[i]);
			return ;
		}
	}
	for(UINT i=0; i<v_kazaa_network_status.size(); i++)
	{
		if(stricmp(from, v_kazaa_network_status[i].m_ip)==0)
		{
			SendProjectChecksums(v_kazaa_network_status[i]);
			return ;
		}
	}
	for(UINT i=0; i<v_piolet_network_status.size(); i++)
	{
		if(stricmp(from, v_piolet_network_status[i].m_ip)==0)
		{
			SendProjectChecksums(v_piolet_network_status[i]);
			return ;
		}
	}
	for(UINT i=0; i<v_piolet_spoofer_status.size(); i++)
	{
		if(stricmp(from, v_piolet_spoofer_status[i].m_ip)==0)
		{
			SendProjectChecksums(v_piolet_spoofer_status[i]);
			return ;
		}
	}
	for(UINT i=0; i<v_piolet_spoofer_splitted_status.size(); i++)
	{
		if(stricmp(from, v_piolet_spoofer_splitted_status[i].m_ip)==0)
		{
			SendProjectChecksums(v_piolet_spoofer_splitted_status[i]);
			return ;
		}
	}
	for(UINT i=0; i<v_direct_connect_status.size(); i++)
	{
		if(stricmp(from, v_direct_connect_status[i].m_ip)==0)
		{
			SendProjectChecksums(v_direct_connect_status[i]);
			return ;
		}
	}
	for(UINT i=0; i<v_direct_connect_spoofer_status.size(); i++)
	{
		if(stricmp(from, v_direct_connect_spoofer_status[i].m_ip)==0)
		{
			SendProjectChecksums(v_direct_connect_spoofer_status[i]);
			return ;
		}
	}
	for(UINT i=0; i<v_bittorrent_status.size(); i++)
	{
		if(stricmp(from, v_bittorrent_status[i].m_ip)==0)
		{
			SendProjectChecksums(v_bittorrent_status[i]);
			return ;
		}
	}
	for(UINT i=0; i<v_generic_status.size(); i++)
	{
		if(stricmp(from, v_generic_status[i].m_ip)==0)
		{
			SendProjectChecksums(v_generic_status[i]);
			return ;
		}
	}
	for(UINT i=0; i<v_overnet_spoofer_status.size(); i++)
	{
		if(stricmp(from, v_overnet_spoofer_status[i].m_ip)==0)
		{
			SendProjectChecksums(v_overnet_spoofer_status[i]);
			return ;
		}
	}
	for(UINT i=0; i<v_overnet_dc_status.size(); i++)
	{
		if(stricmp(from, v_overnet_dc_status[i].m_ip)==0)
		{
			SendProjectChecksums(v_overnet_dc_status[i]);
			return ;
		}
	}
	for(UINT i=0; i<v_bearshare_spoofer_status.size(); i++)
	{
		if(stricmp(from, v_bearshare_spoofer_status[i].m_ip)==0)
		{
			SendProjectChecksums(v_bearshare_spoofer_status[i]);
			return ;
		}
	}
}

//
//
//
void DCMasterDll::LoadBalanceProjectsForPioletSpoofers()
{
	m_dlg.ClearProjectsNumber("Piolet Spoofer Splitted");

	if(v_piolet_spoofer_splitted_status.size()>0)
	{
		//sort the spoofers' ips
		sort(v_piolet_spoofer_splitted_status.begin(),v_piolet_spoofer_splitted_status.end());

		UINT i,j;
		UINT num_swarmers = v_piolet_spoofer_splitted_status.size();
		//clear all existing distributed projects
		for(i=0; i<num_swarmers; i++)
		{
			v_piolet_spoofer_splitted_status[i].v_projects.clear();
		}

		for(i=0; i<v_piolet_spoofer_project_data.size(); i++)
		{
			vector<UINT>assigned_collectors;
			int num_racks_to_use = ((int)(((float)v_piolet_spoofer_project_data[i].m_kazaa_swarm_load / (float)100)*(float)num_swarmers))/m_piolet_load_balancing_divider;
			if(num_racks_to_use<=0)
				num_racks_to_use=1;
			for(j=0;j<num_racks_to_use;j++)
			{
				int index = GetLeastNumberOfProjects(assigned_collectors,v_piolet_spoofer_splitted_status);
				if(index>=0)
				{
					assigned_collectors.push_back(index);
					v_piolet_spoofer_splitted_status[index].v_projects.push_back(v_piolet_spoofer_project_data[i]);
				}
				else
					break;
			}
		}

		for(i=0; i<v_piolet_spoofer_splitted_status.size(); i++)
		{
			SendProjectChecksums(v_piolet_spoofer_splitted_status[i]);
			m_dlg.UpdateCollectorStatus(v_piolet_spoofer_splitted_status[i],"Piolet Spoofer Splitted");
		}
	}
	m_browse_projects_dlg.BuildProjectTree("Piolet Spoofer Splitted",v_piolet_spoofer_splitted_status);
}

//
//
//
void DCMasterDll::LoadBalanceProjectsForBearShareSpoofers()
{
	m_dlg.ClearProjectsNumber("BearShare Spoofer");

	if(v_bearshare_spoofer_status.size()>0)
	{
		//sort the spoofers' ips
		sort(v_bearshare_spoofer_status.begin(),v_bearshare_spoofer_status.end());

		UINT i,j;
		UINT num_swarmers = v_bearshare_spoofer_status.size();
		//clear all existing distributed projects
		for(i=0; i<num_swarmers; i++)
		{
			v_bearshare_spoofer_status[i].v_projects.clear();
		}

		for(i=0; i<v_bearshare_spoofer_project_data.size(); i++)
		{
			vector<UINT>assigned_collectors;
			int num_racks_to_use = ((int)(((float)v_bearshare_spoofer_project_data[i].m_gnutella_load / (float)100)*(float)num_swarmers))/m_bearshare_load_balancing_divider;
			if(num_racks_to_use<=0)
				num_racks_to_use=1;
			for(j=0;j<num_racks_to_use;j++)
			{
				int index = GetLeastNumberOfProjects(assigned_collectors,v_bearshare_spoofer_status);
				if(index>=0)
				{
					assigned_collectors.push_back(index);
					v_bearshare_spoofer_status[index].v_projects.push_back(v_bearshare_spoofer_project_data[i]);
				}
				else
					break;
			}
		}

		for(i=0; i<v_bearshare_spoofer_status.size(); i++)
		{
			SendProjectChecksums(v_bearshare_spoofer_status[i]);
			m_dlg.UpdateCollectorStatus(v_bearshare_spoofer_status[i],"BearShare Spoofer");
		}
	}
	m_browse_projects_dlg.BuildProjectTree("BearShare Spoofer",v_bearshare_spoofer_status);
}