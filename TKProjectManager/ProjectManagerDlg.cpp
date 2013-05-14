// ProjectManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ProjectManagerDlg.h"


// CProjectManagerDlg dialog

IMPLEMENT_DYNAMIC(CProjectManagerDlg, CDialog)
CProjectManagerDlg::CProjectManagerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CProjectManagerDlg::IDD, pParent)
{
}

CProjectManagerDlg::~CProjectManagerDlg()
{
}

void CProjectManagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_Project_Keyword_Tree, m_project_keyword_tree);
}


BEGIN_MESSAGE_MAP(CProjectManagerDlg, CDialog)
END_MESSAGE_MAP()


// CProjectManagerDlg message handlers
//
//
void CProjectManagerDlg::UpdateProjectKeyowrdTree(vector<ProjectKeywords>* keywords)
{
	UINT i,j,k;
	
	m_project_keyword_tree.DeleteAllItems();

	HTREEITEM item1,item2,item3,item4;
	char buf[1024];

	for(i=0;i<keywords->size();i++)
	{
		// Project name
		item1=m_project_keyword_tree.InsertItem((*keywords)[i].m_project_name.c_str(),0,0,TVI_ROOT);
		sprintf(buf,"Project ID: %u",(*keywords)[i].m_id);
		m_project_keyword_tree.InsertItem(buf,0,0,item1);
		sprintf(buf,"Artist: %s",(*keywords)[i].m_artist_name.c_str());
		m_project_keyword_tree.InsertItem(buf,0,0,item1);
		sprintf(buf,"Album: %s",(*keywords)[i].m_album_name.c_str());
		m_project_keyword_tree.InsertItem(buf,0,0,item1);
		sprintf(buf,"Owner: %s",(*keywords)[i].m_owner.c_str());
		m_project_keyword_tree.InsertItem(buf,0,0,item1);

		item4=m_project_keyword_tree.InsertItem("Viewers",0,0,item1);
		for(j=0;j<(*keywords)[i].m_viewers.v_strings.size();j++)
		{
			m_project_keyword_tree.InsertItem((*keywords)[i].m_viewers.v_strings[j].c_str(),0,0,item4);
		}

		sprintf(buf,"Version: %u",(*keywords)[i].m_version);
		m_project_keyword_tree.InsertItem(buf,0,0,item1);



		// Flags
		item2=m_project_keyword_tree.InsertItem("Flags",0,0,item1);

		sprintf(buf,"Project Active : %u",(*keywords)[i].m_project_active);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Auto Multiplier : %u",(*keywords)[i].m_auto_multiplier_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"BearShare Disting : %u",(*keywords)[i].m_bearshare_disting_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"GUID Spoofing : %u",(*keywords)[i].m_guid_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Poisoning : %u",(*keywords)[i].m_poisoning_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Uber Disting: %u",(*keywords)[i].m_uber_disting_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"Emule Spoofing: %u",(*keywords)[i].m_emule_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Emule Swarming: %u",(*keywords)[i].m_emule_swarming_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Emule Decoying: %u",(*keywords)[i].m_emule_decoys_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Emule Supply: %u",(*keywords)[i].m_emule_supply_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"DirectConnect Decoying: %u",(*keywords)[i].m_directconnect_decoys_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"DirectConnect Spoofing: %u",(*keywords)[i].m_directconnect_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"DirectConnect Supply: %u",(*keywords)[i].m_directconnect_supply_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"FastTrack Decoying: %u",(*keywords)[i].m_fasttrack_decoys_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"FastTrack Spoofing: %u",(*keywords)[i].m_fasttrack_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"FastTrack Swarming: %u",(*keywords)[i].m_fasttrack_swarming_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"FastTrack Supply: %u",(*keywords)[i].m_fasttrack_supply_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"FastTrack Find More: %u",(*keywords)[i].m_find_more);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"Gnutella Supply: %u",(*keywords)[i].m_gnutella_supply_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Gnutella Demand: %u",(*keywords)[i].m_gnutella_demand_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Gnutella Decoying: %u",(*keywords)[i].m_gnutella_decoys_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Gnutella Spoofing: %u",(*keywords)[i].m_gnutella_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Gnutella Swarming: %u",(*keywords)[i].m_gnutella_swarming_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);


		sprintf(buf,"Piolet Spoofing: %u",(*keywords)[i].m_piolet_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Piolet Demand: %u",(*keywords)[i].m_piolet_demand_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Piolet Supply: %u",(*keywords)[i].m_piolet_supply_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"Gift Spoofing: %u",(*keywords)[i].m_gift_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Gift Decoys: %u",(*keywords)[i].m_gift_decoys_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Gift Supply: %u",(*keywords)[i].m_gift_supply_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"WinMX Supply: %u",(*keywords)[i].m_winmx_supply_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"WinMX Spoofing: %u",(*keywords)[i].m_winmx_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"WinMX Swarming: %u",(*keywords)[i].m_winmx_swarming_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"WinMX Decoying: %u",(*keywords)[i].m_winmx_decoy_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"SoulSeek Supply: %u",(*keywords)[i].m_soulseek_supply_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"SoulSeek Demand: %u",(*keywords)[i].m_soulseek_demand_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"SoulSeek Spoofing: %u",(*keywords)[i].m_soulseek_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"SoulSeek Decoying: %u",(*keywords)[i].m_soulseek_decoying_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"BitTorrent Decoying: %u",(*keywords)[i].m_bittorrent_decoy_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"BitTorrent Interdiction: %u",(*keywords)[i].m_bittorrent_interdiction_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"BitTorrent Spoofing: %u",(*keywords)[i].m_bittorrent_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"BitTorrent Supply: %u",(*keywords)[i].m_bittorrent_supply_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		sprintf(buf,"Overnet Demand: %u",(*keywords)[i].m_overnet_demand_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Overnet Supply: %u",(*keywords)[i].m_overnet_supply_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Overnet Decoying: %u",(*keywords)[i].m_overnet_decoys_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Overnet Spoofing: %u",(*keywords)[i].m_overnet_spoofing_enabled);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);

		//Kazaa Search Type
		char type[32];
		switch((*keywords)[i].m_search_type)
		{
			case ProjectKeywords::search_type::everything:
			{
				strcpy(type, "Everything");
				break;
			}
			case ProjectKeywords::search_type::audio:
			{
				strcpy(type, "Audio");
				break;
			}
			case ProjectKeywords::search_type::video:
			{
				strcpy(type, "Video");
				break;
			}
			case ProjectKeywords::search_type::images:
			{
				strcpy(type, "Images");
				break;
			}
			case ProjectKeywords::search_type::documents:
			{
				strcpy(type, "Documents");
				break;
			}
			case ProjectKeywords::search_type::software:
			{
				strcpy(type, "Software");
				break;
			}
			case ProjectKeywords::search_type::playlists:
			{
				strcpy(type, "Playlists");
				break;
			}
			case ProjectKeywords::search_type::none:
			{
				strcpy(type, "None");
				break;
			}
		}
		sprintf(buf,"FastTrack Search Type : %s",type);
		m_project_keyword_tree.InsertItem(buf,0,0,item1);
		sprintf(buf,"FastTrack Dist Power : %d",(*keywords)[i].m_kazaa_dist_power);
		m_project_keyword_tree.InsertItem(buf,0,0,item1);

		// Hash Count
		item2=m_project_keyword_tree.InsertItem("FastTrack Hash Count",0,0,item1);
		for(j=0;j<(*keywords)[i].v_tracks_hash_counts.size();j++)
		{
			sprintf(buf,"%2d : %d",(*keywords)[i].v_tracks_hash_counts[j].m_track, (*keywords)[i].v_tracks_hash_counts[j].m_hash_count);
			m_project_keyword_tree.InsertItem(buf,0,0,item2);
		}

		// Query multiplier
		sprintf(buf,"Query Multiplier : %u",(*keywords)[i].m_query_multiplier);
		m_project_keyword_tree.InsertItem(buf,0,0,item1);

		// Supply Interval
		sprintf(buf,"Supply Interval : %u",(*keywords)[i].m_interval);
		m_project_keyword_tree.InsertItem(buf,0,0,item1);

		// Query keywords
		item2=m_project_keyword_tree.InsertItem("Query Keywords",0,0,item1);
		// Query Keywords : Keywords
		item3=m_project_keyword_tree.InsertItem("Keywords : Weight",0,0,item2);
		for(j=0;j<(*keywords)[i].m_query_keywords.v_keywords.size();j++)
		{
			sprintf(buf,"%s : %u",(*keywords)[i].m_query_keywords.v_keywords[j].keyword.c_str(),(*keywords)[i].m_query_keywords.v_keywords[j].weight);
			m_project_keyword_tree.InsertItem(buf,0,0,item3);
		}
		// Query Keywords : Exact Keywords
		item3=m_project_keyword_tree.InsertItem("Exact Keywords : Weight",0,0,item2);
		for(j=0;j<(*keywords)[i].m_query_keywords.v_exact_keywords.size();j++)
		{
			sprintf(buf,"%s : %u",(*keywords)[i].m_query_keywords.v_exact_keywords[j].keyword.c_str(),(*keywords)[i].m_query_keywords.v_exact_keywords[j].weight);
			m_project_keyword_tree.InsertItem(buf,0,0,item3);
		}
		// Query Keywords : Killwords
		item3=m_project_keyword_tree.InsertItem("Killwords",0,0,item2);
		for(j=0;j<(*keywords)[i].m_query_keywords.v_killwords.size();j++)
		{
			sprintf(buf,"%s",(*keywords)[i].m_query_keywords.v_killwords[j].keyword.c_str());
			m_project_keyword_tree.InsertItem(buf,0,0,item3);
		}
		// Query Keywords : Exact Killwords
		item3=m_project_keyword_tree.InsertItem("Exact Killwords",0,0,item2);
		for(j=0;j<(*keywords)[i].m_query_keywords.v_exact_killwords.size();j++)
		{
			sprintf(buf,"%s",(*keywords)[i].m_query_keywords.v_exact_killwords[j].keyword.c_str());
			m_project_keyword_tree.InsertItem(buf,0,0,item3);
		}

		// Supply Keywords
		item2=m_project_keyword_tree.InsertItem("Supply Keywords",0,0,item1);
		sprintf(buf,"Search String : %s",(*keywords)[i].m_supply_keywords.m_search_string.c_str());
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		sprintf(buf,"Supply Size Threshold : %u",(*keywords)[i].m_supply_keywords.m_supply_size_threshold);
		m_project_keyword_tree.InsertItem(buf,0,0,item2);
		// Supply Keywords : Keywords
		item3=m_project_keyword_tree.InsertItem("Keywords : Track",0,0,item2);
		for(j=0;j<(*keywords)[i].m_supply_keywords.v_keywords.size();j++)
		{
			buf[0]='\0';
			for(k=0;k<(*keywords)[i].m_supply_keywords.v_keywords[j].v_keywords.size();k++)
			{
				strcat(buf,(*keywords)[i].m_supply_keywords.v_keywords[j].v_keywords[k]);
				strcat(buf," ");
			}
			sprintf(&buf[strlen(buf)],": %u",(*keywords)[i].m_supply_keywords.v_keywords[j].m_track);
			m_project_keyword_tree.InsertItem(buf,0,0,item3);
		}
		// Supply Keywords : Killwords
		item3=m_project_keyword_tree.InsertItem("Killwords",0,0,item2);
		for(j=0;j<(*keywords)[i].m_supply_keywords.v_killwords.size();j++)
		{
			sprintf(buf,"%s",(*keywords)[i].m_supply_keywords.v_killwords[j].v_keywords[0]);	// kinda kludgey *&*
			m_project_keyword_tree.InsertItem(buf,0,0,item3);
		}
		
		// QRP Keywords
		item2=m_project_keyword_tree.InsertItem("QRP Keywords",0,0,item1);
		for(j=0;j<(*keywords)[i].m_qrp_keywords.v_keywords.size();j++)
		{
			m_project_keyword_tree.InsertItem((*keywords)[i].m_qrp_keywords.v_keywords[j].c_str(),0,0,item2);
		}

		// Poisoners
		item2=m_project_keyword_tree.InsertItem("Poisoners",0,0,item1);
		for(j=0;j<(*keywords)[i].v_poisoners.size();j++)
		{
			unsigned int ip=(*keywords)[i].v_poisoners[j].GetIPInt();
			unsigned int port=(*keywords)[i].v_poisoners[j].GetPort();
			sprintf(buf,"%u.%u.%u.%u : %u",(ip>>0)&0xFF,(ip>>8)&0xFF,(ip>>16)&0xFF,(ip>>24)&0xFF,port);
			m_project_keyword_tree.InsertItem(buf,0,0,item2);
		}


	}
	m_project_keyword_tree.SortChildren(TVI_ROOT);
	// Whenever the keywords are updated, reset the status list in case any projects were removed
	//OnBnClickedResetProjectStatusList();

	// Reset the keyword count
	if(keywords->size()==1)
	{
		sprintf(buf,"Project Keywords : ( %u Project )",keywords->size());
	}
	else
	{
		sprintf(buf,"Project Keywords : ( %u Projects )",keywords->size());
	}
	GetDlgItem(IDC_Project_Keywords_Static)->SetWindowText(buf);
}

//
//
//
void CProjectManagerDlg::SetClient(const char* client)
{
	GetDlgItem(IDC_CLIENT_STATIC)->SetWindowText(client);
}