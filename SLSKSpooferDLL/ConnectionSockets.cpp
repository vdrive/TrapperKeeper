// ConnectionSockets.cpp

#include "stdafx.h"
#include "ConnectionSockets.h"
//#include "VendorCount.h"
#include "ConnectionModuleMessageWnd.h"
//#include "ProjectStatus.h"
#include "iphlpapi.h"
//#include "FileSharingManager.h"
#include "fstream"
#include "ProjectKeywords.h"
#include "db.h"

//
//
//
ConnectionSockets::ConnectionSockets()
{
	m_projects = NULL;
	b_inited=false;
	m_one_hour_timer=0;

	m_fill_projects = true;
	m_fill_spoofs = true;
	//p_file_sharing_manager = NULL;
	/*
	GetMyIP();
	v_spoof_filenames.push_back("vcdistro.sharedreactor paycheck cd2 good cam.avi");
	v_spoof_filenames.push_back("paycheck (complete) screener.svcd-tcf.rar good cam.avi");
	v_spoof_filenames.push_back("paycheck (complete) screener.svcd-tcf.rar screener.avi");
	v_spoof_filenames.push_back("paycheck (complete) screener.svcd-tcf.rar ts-tcf.avi");
	v_spoof_filenames.push_back("paycheck (complete) teg good cam.avi");
	v_spoof_filenames.push_back("paycheck (complete) teg telesynch.avi");
	v_spoof_filenames.push_back("paycheck (complete) teg_vcd cam.avi");
	v_spoof_filenames.push_back("paycheck (complete) teg_vcd screener.avi");
	v_spoof_filenames.push_back("paycheck (complete) ts-esoteric. cam.avi");
	v_spoof_filenames.push_back("paycheck (complete) ts-esoteric. good cam.avi");
	v_spoof_filenames.push_back("paycheck (complete) ts-esoteric. screener.avi");
	v_spoof_filenames.push_back("paycheck (complete) ts-esoteric. telesynch.avi");
	v_spoof_filenames.push_back("paycheck (complete) ts-esoteric. ts-tcf.avi");
	v_spoof_filenames.push_back("paycheck (complete) ts.centropy.svcd.rip.sbc. screener.avi");
	v_spoof_filenames.push_back("paycheck (complete) ts.centropy.svcd.rip.sbc. ts-tcf.avi");
	v_spoof_filenames.push_back("paycheck (complete) vcdistro.sharedreactor screener.avi");
	v_spoof_filenames.push_back("paycheck (complete) vcdistro.sharedreactor telesynch.avi");
	v_spoof_filenames.push_back("paycheck (complete) vcdistro.sharedreactor ts-tcf.avi");
	v_spoof_filenames.push_back("paycheck (complete) vol.1.dvd.screener.svcd-tcf good cam.avi");
	v_spoof_filenames.push_back("paycheck (complete) [tmd].(eso). screener.avi");
	v_spoof_filenames.push_back("paycheck cd1 screener.svcd-tcf.rar cam.avi");
	v_spoof_filenames.push_back("paycheck cd1 teg_vcd ts-tcf.avi");
	v_spoof_filenames.push_back("paycheck cd1 ts.centropy.svcd.rip.sbc. telesynch.avi");
	v_spoof_filenames.push_back("paycheck cd1 ts.centropy.svcd.rip.sbc. ts-tcf.avi");
	v_spoof_filenames.push_back("paycheck cd1 vol.1.dvd.screener.svcd-tcf screener.avi");
	v_spoof_filenames.push_back("paycheck cd2 screener.svcd-tcf.rar cam.avi");
	v_spoof_filenames.push_back("paycheck cd2 screener.svcd-tcf.rar telecine.avi");
	v_spoof_filenames.push_back("paycheck cd2 teg ts-tcf.avi");
	v_spoof_filenames.push_back("paycheck cd2 teg_vcd good cam.avi");
	v_spoof_filenames.push_back("paycheck cd2 ts-esoteric. cam.avi");
	v_spoof_filenames.push_back("paycheck cd2 ts-esoteric. screener.avi");
	v_spoof_filenames.push_back("paycheck cd2 ts.centropy.svcd.rip.sbc. screener.avi");
	v_spoof_filenames.push_back("paycheck cd2 ts.centropy.svcd.rip.sbc. telecine.avi");
	v_spoof_filenames.push_back("paycheck cd2 ts.centropy.svcd.rip.sbc. ts-tcf.avi");
	v_spoof_filenames.push_back("paycheck cd2 vcdistro.sharedreactor good cam.avi");
	v_spoof_filenames.push_back("paycheck cd2 vol.1.dvd.screener.svcd-tcf telecine.avi");
	v_spoof_filenames.push_back("paycheck cd2 vol.1.dvd.screener.svcd-tcf ts-tcf.avi");
	v_spoof_filenames.push_back("paycheck cd2 [tmd].(eso). telecine.avi");
	v_spoof_filenames.push_back("paycheck cd2 [tmd].(eso). telesynch.avi");
	v_spoof_filenames.push_back("Paycheck.avi");
	v_spoof_filenames.push_back("screener.svcd-tcf.rar paycheck (complete) cam.avi");
	v_spoof_filenames.push_back("SCREENER.SVCD-TCF.RAR PAYCHECK (COMPLETE) TELESYNCH.avi");
	v_spoof_filenames.push_back("SCREENER.SVCD-TCF.RAR PAYCHECK CD2 GOOD CAM.avi");
	v_spoof_filenames.push_back("TEG PAYCHECK CD2 SCREENER.avi");
	v_spoof_filenames.push_back("teg_vcd paycheck cd1 good cam.avi");
	v_spoof_filenames.push_back("teg_vcd paycheck cd1 telesynch.avi");
	v_spoof_filenames.push_back("ts-esoteric. paycheck (complete) telecine.avi");
	v_spoof_filenames.push_back("ts-esoteric. paycheck cd1 screener.avi");
	v_spoof_filenames.push_back("ts-esoteric. paycheck cd1 ts-tcf.avi");
	v_spoof_filenames.push_back("TS-ESOTERIC. PAYCHECK CD2 CAM.avi");
	v_spoof_filenames.push_back("<ts.centropy.svcd.rip.sbc. paycheck (complete) screener.avi");
	v_spoof_filenames.push_back("VCDISTRO.SHAREDREACTOR PAYCHECK (COMPLETE) CAM.avi");
	v_spoof_filenames.push_back("VCDISTRO.SHAREDREACTOR PAYCHECK CD1 TS-TCF.avi");
	v_spoof_filenames.push_back("[tmd].(eso). paycheck (complete) good cam.avi");
	v_spoof_filenames.push_back("vol.1.dvd.screener.svcd-tcf paycheck cd2 good cam.avi");
	v_spoof_filenames.push_back("[tmd].(eso). paycheck (complete) ts-tcf.avi");
	v_spoof_filenames.push_back("[TMD].(ESO). PAYCHECK CD1 TELESYNCH.avi");
	v_spoof_filenames.push_back("[tmd].(eso). paycheck cd1 ts-tcf.avi");
	v_spoof_filenames.push_back("[TMD]Paycheck.(MPT).TS.(2of2).avi");
	v_spoof_filenames.push_back("[tmd]paycheck.divx.ts.2003.asf");
*/
	//ReadInUserNames();
	m_dlg_hwnd=NULL;
	int num_reserved_events=ReturnNumberOfReservedEvents();
	int num_socket_events=ReturnNumberOfSocketEvents();

	// Init the parent pointers and message window handles
	for(int i=0;i<num_socket_events;i++)
	{
		m_sockets[i].InitParent(this);
		//m_sockets[i].InitAccountInfo(account_info);
	}
	
	// Create the reserved events
	for(i=0;i<num_reserved_events;i++)
	{
		m_events[i]=WSACreateEvent();
		if(m_events[i]==WSA_INVALID_EVENT)
		{
			char msg[1024];
			strcpy(msg,"Could not create a valid reserved event ");
			_itoa(i,&msg[strlen(msg)],10);
			::MessageBox(NULL,msg,"Error",MB_OK);
		}
	}

	// Fully initialize events array
	for(i=0;i<num_socket_events;i++)
	{
		m_events[num_reserved_events+i]=m_sockets[i].ReturnEventHandle();
	}
	
	if (spoofing_info.size() == 0 && state == 3)
	{
	
		spoofing_info.clear();
		FillSpoofs();
	}

	m_projects=NULL;
	m_spoofs_created = false;
//	m_rng.Reseed(true,32);


}

ConnectionSockets::ConnectionSockets(int socketstate, ProjectKeywordsVector *pkv)
 {
	 //TRACE("in connection sockets constructor!\n");
	 m_projects = NULL;
	 state = socketstate;
	m_one_hour_timer=0;

	m_fill_projects = true;
	m_fill_spoofs = true;
	//p_file_sharing_manager = NULL;
	/*
	GetMyIP();
	v_spoof_filenames.push_back("vcdistro.sharedreactor paycheck cd2 good cam.avi");
	v_spoof_filenames.push_back("paycheck (complete) screener.svcd-tcf.rar good cam.avi");
	v_spoof_filenames.push_back("paycheck (complete) screener.svcd-tcf.rar screener.avi");
	v_spoof_filenames.push_back("paycheck (complete) screener.svcd-tcf.rar ts-tcf.avi");
	v_spoof_filenames.push_back("paycheck (complete) teg good cam.avi");
	v_spoof_filenames.push_back("paycheck (complete) teg telesynch.avi");
	v_spoof_filenames.push_back("paycheck (complete) teg_vcd cam.avi");
	v_spoof_filenames.push_back("paycheck (complete) teg_vcd screener.avi");
	v_spoof_filenames.push_back("paycheck (complete) ts-esoteric. cam.avi");
	v_spoof_filenames.push_back("paycheck (complete) ts-esoteric. good cam.avi");
	v_spoof_filenames.push_back("paycheck (complete) ts-esoteric. screener.avi");
	v_spoof_filenames.push_back("paycheck (complete) ts-esoteric. telesynch.avi");
	v_spoof_filenames.push_back("paycheck (complete) ts-esoteric. ts-tcf.avi");
	v_spoof_filenames.push_back("paycheck (complete) ts.centropy.svcd.rip.sbc. screener.avi");
	v_spoof_filenames.push_back("paycheck (complete) ts.centropy.svcd.rip.sbc. ts-tcf.avi");
	v_spoof_filenames.push_back("paycheck (complete) vcdistro.sharedreactor screener.avi");
	v_spoof_filenames.push_back("paycheck (complete) vcdistro.sharedreactor telesynch.avi");
	v_spoof_filenames.push_back("paycheck (complete) vcdistro.sharedreactor ts-tcf.avi");
	v_spoof_filenames.push_back("paycheck (complete) vol.1.dvd.screener.svcd-tcf good cam.avi");
	v_spoof_filenames.push_back("paycheck (complete) [tmd].(eso). screener.avi");
	v_spoof_filenames.push_back("paycheck cd1 screener.svcd-tcf.rar cam.avi");
	v_spoof_filenames.push_back("paycheck cd1 teg_vcd ts-tcf.avi");
	v_spoof_filenames.push_back("paycheck cd1 ts.centropy.svcd.rip.sbc. telesynch.avi");
	v_spoof_filenames.push_back("paycheck cd1 ts.centropy.svcd.rip.sbc. ts-tcf.avi");
	v_spoof_filenames.push_back("paycheck cd1 vol.1.dvd.screener.svcd-tcf screener.avi");
	v_spoof_filenames.push_back("paycheck cd2 screener.svcd-tcf.rar cam.avi");
	v_spoof_filenames.push_back("paycheck cd2 screener.svcd-tcf.rar telecine.avi");
	v_spoof_filenames.push_back("paycheck cd2 teg ts-tcf.avi");
	v_spoof_filenames.push_back("paycheck cd2 teg_vcd good cam.avi");
	v_spoof_filenames.push_back("paycheck cd2 ts-esoteric. cam.avi");
	v_spoof_filenames.push_back("paycheck cd2 ts-esoteric. screener.avi");
	v_spoof_filenames.push_back("paycheck cd2 ts.centropy.svcd.rip.sbc. screener.avi");
	v_spoof_filenames.push_back("paycheck cd2 ts.centropy.svcd.rip.sbc. telecine.avi");
	v_spoof_filenames.push_back("paycheck cd2 ts.centropy.svcd.rip.sbc. ts-tcf.avi");
	v_spoof_filenames.push_back("paycheck cd2 vcdistro.sharedreactor good cam.avi");
	v_spoof_filenames.push_back("paycheck cd2 vol.1.dvd.screener.svcd-tcf telecine.avi");
	v_spoof_filenames.push_back("paycheck cd2 vol.1.dvd.screener.svcd-tcf ts-tcf.avi");
	v_spoof_filenames.push_back("paycheck cd2 [tmd].(eso). telecine.avi");
	v_spoof_filenames.push_back("paycheck cd2 [tmd].(eso). telesynch.avi");
	v_spoof_filenames.push_back("Paycheck.avi");
	v_spoof_filenames.push_back("screener.svcd-tcf.rar paycheck (complete) cam.avi");
	v_spoof_filenames.push_back("SCREENER.SVCD-TCF.RAR PAYCHECK (COMPLETE) TELESYNCH.avi");
	v_spoof_filenames.push_back("SCREENER.SVCD-TCF.RAR PAYCHECK CD2 GOOD CAM.avi");
	v_spoof_filenames.push_back("TEG PAYCHECK CD2 SCREENER.avi");
	v_spoof_filenames.push_back("teg_vcd paycheck cd1 good cam.avi");
	v_spoof_filenames.push_back("teg_vcd paycheck cd1 telesynch.avi");
	v_spoof_filenames.push_back("ts-esoteric. paycheck (complete) telecine.avi");
	v_spoof_filenames.push_back("ts-esoteric. paycheck cd1 screener.avi");
	v_spoof_filenames.push_back("ts-esoteric. paycheck cd1 ts-tcf.avi");
	v_spoof_filenames.push_back("TS-ESOTERIC. PAYCHECK CD2 CAM.avi");
	v_spoof_filenames.push_back("<ts.centropy.svcd.rip.sbc. paycheck (complete) screener.avi");
	v_spoof_filenames.push_back("VCDISTRO.SHAREDREACTOR PAYCHECK (COMPLETE) CAM.avi");
	v_spoof_filenames.push_back("VCDISTRO.SHAREDREACTOR PAYCHECK CD1 TS-TCF.avi");
	v_spoof_filenames.push_back("[tmd].(eso). paycheck (complete) good cam.avi");
	v_spoof_filenames.push_back("vol.1.dvd.screener.svcd-tcf paycheck cd2 good cam.avi");
	v_spoof_filenames.push_back("[tmd].(eso). paycheck (complete) ts-tcf.avi");
	v_spoof_filenames.push_back("[TMD].(ESO). PAYCHECK CD1 TELESYNCH.avi");
	v_spoof_filenames.push_back("[tmd].(eso). paycheck cd1 ts-tcf.avi");
	v_spoof_filenames.push_back("[TMD]Paycheck.(MPT).TS.(2of2).avi");
	v_spoof_filenames.push_back("[tmd]paycheck.divx.ts.2003.asf");
*/
	//ReadInUserNames();
	m_dlg_hwnd=NULL;
	int num_reserved_events=ReturnNumberOfReservedEvents();
	int num_socket_events=ReturnNumberOfSocketEvents();

	// Init the parent pointers and message window handles
	for(int i=0;i<num_socket_events;i++)
	{
		m_sockets[i].InitParent(this);
		//m_sockets[i].InitAccountInfo(account_info);
	}
	
	// Create the reserved events
	for(i=0;i<num_reserved_events;i++)
	{
		m_events[i]=WSACreateEvent();
		if(m_events[i]==WSA_INVALID_EVENT)
		{
			char msg[1024];
			strcpy(msg,"Could not create a valid reserved event ");
			_itoa(i,&msg[strlen(msg)],10);
			::MessageBox(NULL,msg,"Error",MB_OK);
		}
	}

	// Fully initialize events array
	for(i=0;i<num_socket_events;i++)
	{
		m_events[num_reserved_events+i]=m_sockets[i].ReturnEventHandle();
	}
	m_projects = pkv;
	
	b_inited=true; //ty
	ListeningPort = 2234;

	if (state == 3)
	{
		CreateSpoofs();
		//spoofing_info.clear();
		//FillSpoofs();
	}
//	m_rng.Reseed(true,32);

	m_last_status_sent = CTime::GetCurrentTime();

	//TRACE("FINISHED WITH CONNECTION SOCKETS CONSTRUCTOR!\n");
}

//
//
//
ConnectionSockets::~ConnectionSockets()
{
	/*
	vector<PacketBuffer*>::iterator iter = v_shared_files_buffers.begin();
	while(iter != v_shared_files_buffers.end())
	{
		delete *iter;
		v_shared_files_buffers.erase(iter);
	}
	*/

	int num_reserved_events=ReturnNumberOfReservedEvents();
	
	// Close the reserved events 
	for(int i=0;i<num_reserved_events;i++)
	{
		if(WSACloseEvent(m_events[i])==FALSE)
		{
			char msg[1024];
			strcpy(msg,"Could not close reserved event ");
			_itoa(i,&msg[strlen(msg)],10);
			::MessageBox(NULL,msg,"Error",MB_OK);
		}
	}

	weighted_keywords.clear();
	spoofing_info.clear();
	projects.clear();
	// close all sockets
	int num_socket_events=ReturnNumberOfSocketEvents();
	for(i=0;i<num_socket_events;i++)
	{
		m_sockets[i].Close();
	}
	delete m_projects;

	//for (i = 0; i < (int)m_track_info->size(); i++)
	//{
	//	delete m_track_info[i];
	//}

	//m_track_info->clear();

//	delete m_track_info;
}

//
//
//
DWORD ConnectionSockets::ReturnNumberOfEvents()
{
	return NumberOfEvents;
}

void ConnectionSockets::Log(const char* log)
{
	if(m_dlg_hwnd != NULL)
		::PostMessage(m_dlg_hwnd,WM_LOG_MSG,(WPARAM)log,0);
}

void ConnectionSockets::WriteToLog(const char* log)
{
	if (m_dlg_hwnd != NULL)
		::PostMessage(m_dlg_hwnd, WM_WRITE_TO_LOG, (WPARAM)log, 0);
}

void ConnectionSockets::setParentConnecting(const char* status)
{
	if (m_dlg_hwnd != NULL)
		::PostMessage(m_dlg_hwnd, WM_NUM_PARENT_CONNECTING, (WPARAM)status, 0);
}

void ConnectionSockets::setPeerConnecting(const char* status)
{
	if (m_dlg_hwnd != NULL)
		::PostMessage(m_dlg_hwnd, WM_NUM_PEER_CONNECTING, (WPARAM)status, 0);
}

void ConnectionSockets::ServerStatus(const char* status)
{
	if (m_dlg_hwnd != NULL)
		::PostMessage(m_dlg_hwnd, WM_SERVER_STATUS_MSG, (WPARAM)status, 0);
}
void ConnectionSockets::ParentStatus(const char* status)
{
	if (m_dlg_hwnd != NULL)
		::PostMessage(m_dlg_hwnd, WM_PARENT_STATUS_MSG, (WPARAM)status, 0);
}
void ConnectionSockets::PeerStatus(const char* status)
{
	if (m_dlg_hwnd != NULL)
		::PostMessage(m_dlg_hwnd, WM_PEER_STATUS_MSG, (WPARAM)status, 0);
}

void ConnectionSockets::SendTask(SLSKtask *t)
{
	if (m_dlg_hwnd != NULL)
		::PostMessage(m_dlg_hwnd, WM_SEND_TASK, (WPARAM)t, 0);
}


void ConnectionSockets::SetUserName(CString un)
{
	CString *n = new CString( un );
	if (m_dlg_hwnd != NULL)
		::PostMessage(m_dlg_hwnd, WM_USER_NAME, (WPARAM)n, 0);
}

void ConnectionSockets::AddParent(SupernodeHost newhost)
{
	SupernodeHost *nh = new SupernodeHost();
	nh->m_ip = newhost.m_ip;
	nh->m_port = newhost.m_port;
	if (m_dlg_hwnd != NULL)
		::PostMessage(m_dlg_hwnd, WM_ADD_PARENT, (WPARAM)nh, 0);
}

void ConnectionSockets::FillDailyDemandVector(void)
{
	//TRACE("FILLING IN DAILY DEMAND!\n");
	ProjectKeywords pkv;

	for (int i = 0; i < (int)m_projects->v_projects.size(); i++)
	{
		pkv = m_projects->v_projects[i];

		if (pkv.m_soulseek_demand_enabled)
		{
			demand d;
			d.m_artist = pkv.m_artist_name.c_str();
			d.m_counter = 0;
			daily_demand.push_back(d);
		}

	}

	//TRACE("DONE FILLIN IN DAILY DEMAND!\n");
}
void ConnectionSockets::ChangeConnectedSockets(void)
{
	if (m_dlg_hwnd != NULL)
		::PostMessage(m_dlg_hwnd, WM_CONNECTED_SOCKETS, (WPARAM)m_connected_sockets, 0);
}


void ConnectionSockets::ChangeConnectingSockets(void)
{
	if (m_dlg_hwnd != NULL)
		::PostMessage(m_dlg_hwnd, WM_CONNECTED_SOCKETS, (WPARAM)m_connecting_sockets, 0);
}

void ConnectionSockets::DropCachedParent(SupernodeHost *nh)
{
	if (m_dlg_hwnd != NULL)
		::PostMessage(m_dlg_hwnd, WM_DROP_CACHED_PARENT, (WPARAM)nh, 0);
}
//
//
//
int ConnectionSockets::ReturnNumberOfReservedEvents()
{
	return NumberOfReservedEvents;
}

void ConnectionSockets::SetProjects(ProjectKeywordsVector *projects)
{
	//m_projects = projects;
}

/*void ConnectionSockets::SetTracks(vector<TrackInformation *> *tracks)
{
	m_track_info = tracks;
}*/
//
//
//
int ConnectionSockets::ReturnNumberOfSocketEvents()
{
	return NumberOfSocketEvents;
}

//
//
//

/*void ConnectionSockets::InitAccountInfo(AccountInfo &nai)
{
	account_info = nai;
}
*/
/*void ConnectionSockets::CreateSpoofs()
{
//	delete task;
//	return;

	int i, j, k, l;
	CString clean_path, path, spoofname;
	buffer *buf;
	//char zipped[1024];
	//UINT zippedsize = 1024;
	//TrackInformation *track_info;

	SupplyKeywords *sk;
	int a = (int)m_projects->v_projects.size();
//	spoofs.clear();
//	for (int i = 0; i < task->tracks.size(); i++)
	//{
	for (i = 0; i < (int)m_projects->v_projects.size(); i++)
	{
		spoof s;
		ProjectKeywords *ptr_keyword = &m_projects->v_projects[i];

		s.m_artist = ptr_keyword->m_artist_name.c_str();
		Log("Generating Spoofs...");
		bool file_found;
		for (j = 0; j < 10; j++)
		{
			buf = new buffer();
			spoofname = getSpoofName();
			buf->PutInt(11);//(int)strlen((const char*)(LPCTSTR)spoofname));
			buf->PutCharPtr("HAPPYPUSSY1", 11);//(const char*)(LPCTSTR)spoofname, (int)strlen((const char*)(LPCTSTR)spoofname));
			buf->PutInt(0); // token spot


			for (int l = 0; l < 10; l++)
			{
			clean_path = GetFilePath(ptr_keyword->m_artist_name.c_str(), ptr_keyword->m_album_name.c_str());

			sk  = &(ptr_keyword->m_supply_keywords);

			buf->PutInt(10 * (int)sk->v_keywords.size());

			for (k = 0; k < (int)sk->v_keywords.size(); k++)
			{
			    
			 //   sk  = &(ptr_keyword->m_supply_keywords);
			    

				//track_info = new TrackInformation(*(*m_track_info)[k]);
					//counter = 0; 
				

                 
				path = clean_path;
				buf->PutByte(1);
				path.Append(sk->v_keywords[k].m_track_name.c_str());
				path.Append(".mp3");
				buf->PutInt((int)strlen((const char*)(LPCTSTR)path));
				buf->PutCharPtr((const char*)(LPCTSTR)path, (int)strlen((const char*)(LPCTSTR)path));
			
				//		file_found = false;

	//			file_found = true;
				buf->PutLong(4781848);
				buf->PutInt(0);
				buf->PutInt(3);
				buf->PutCharPtr("mp3", 3);
				buf->PutInt(3);
				buf->PutInt(0);
				buf->PutInt(192);
				buf->PutInt(1);
				buf->PutInt(318);
				buf->PutInt(2);
				buf->PutInt(0);
			//	break;
			
					
			}
			}
			buf->PutByte(1);
			buf->PutInt(9999);
			buf->PutInt(0);	

			
			
			s.m_spoof.push_back(buf);

		}

		m_spoofs.push_back(s);
			
	}
		
}*/

void ConnectionSockets::CreateSpoofs()
{
	
//	delete task;
//	return;
	//TRACE("Creating SPOOFS!\n");

	int i, j, k, l, bitrate;
	int songlength;
	long songsize;
	CString clean_path, path, spoofname;
	buffer *buf;
//	ofstream fout;
//	fout.open("C:\\spoofscreated.txt", ios::out);
	//char zipped[1024];
	//UINT zippedsize = 1024;
	//TrackInformation *track_info;

	SupplyKeywords *sk;
	int a = (int)m_projects->v_projects.size();
//	spoofs.clear();
//	for (int i = 0; i < task->tracks.size(); i++)
	//{
//	fout << "number of projects = " << a << endl;
	for (i = 0; i < (int)m_projects->v_projects.size(); i++)
	{
		//TRACE("IN FOR I FOR LOOP OF CREATING SPOOOFS!\n");
		spoof s;
		ProjectKeywords *ptr_keyword = &m_projects->v_projects[i];

		
		if (ptr_keyword->m_soulseek_spoofing_enabled == false)
			continue;

	//	fout << "project title = " << ptr_keyword->m_project_name << endl;

		s.m_artist = ptr_keyword->m_artist_name.c_str();
		Log("Generating Spoofs...");
//		bool file_found;
		if (ptr_keyword->m_search_type == ProjectKeywords::search_type::video)
			{
				//TRACE("CREATING SPOOF IS A VIDEO!\n");
				for (j = 0; j < 10; j++)
				{
					//TRACE("IN j loop of creating spoof!\n");
					buf = new buffer();
					spoofname = getSpoofName();
					buf->PutInt((int)strlen((char*)(LPCTSTR)spoofname));//(int)strlen((const char*)(LPCTSTR)spoofname));
					buf->PutCharPtr((char*)(LPCTSTR)spoofname, (int)strlen((char*)(LPCTSTR)spoofname));//(const char*)(LPCTSTR)spoofname, (int)strlen((const char*)(LPCTSTR)spoofname));
					buf->PutInt(0); // token spot

			
					sk  = &(ptr_keyword->m_supply_keywords);
					buf->PutInt(100);
					for (l = 0; l < 100; l++)
					{
						//TRACE("IN l loop of creating spoof!\n");
						clean_path = GetFilePath(ptr_keyword->m_artist_name.c_str(), " ");


						path = clean_path;
						buf->PutByte(1);
						path.Append(ptr_keyword->m_artist_name.c_str());
						path.Append(".mpg");
						buf->PutInt((int)strlen((const char*)(LPCTSTR)path));
						buf->PutCharPtr((const char*)(LPCTSTR)path, (int)strlen((const char*)(LPCTSTR)path));


						buf->PutLong(689789787);
						buf->PutInt(0);
						buf->PutInt(3);
						buf->PutCharPtr("mpg", 3);
						buf->PutInt(0);


						//TRACE("END OF L LOOP FOR VIDEO CREATING SPOOF!\n");
					}

					if ( j == 2 || j == 6)
					{
						buf->PutByte(0);
						buf->PutInt(random(1000, 80000));
						buf->PutInt(rand() % 10);
					}
					else
					{
					buf->PutByte(1);
					buf->PutInt(random(1000, 80000));
					buf->PutInt(0);	
					}
				//	fout << buf->GetCharPtr() << endl;
					


				
					//fout.write((const unsigned char*)buf->GetCharPtr(), buf->Size());
					s.m_spoof.push_back(buf);
					//TRACE("END OF J LOOP FOR VIDEO CREATING SPOOF!\n");
				}

				m_spoofs.push_back(s);
				//TRACE("END OF VIDEO CREATING SPOOF!\n");
			}
			else
				if (ptr_keyword->m_search_type == ProjectKeywords::search_type::audio)
			{
				//TRACE("AUDIO CREATING SPOOF!\n");
			
		for (j = 0; j < 10; j++)
		{
			//TRACE("IN J LOOP OF AUDIO CREATING SPOOF!\n");
			buf = new buffer();
			spoofname = getSpoofName();
			buf->PutInt((int)strlen((char*)(LPCTSTR)spoofname));//(int)strlen((const char*)(LPCTSTR)spoofname));
			buf->PutCharPtr((char*)(LPCTSTR)spoofname, (int)strlen((char*)(LPCTSTR)spoofname));//(const char*)(LPCTSTR)spoofname, (int)strlen((const char*)(LPCTSTR)spoofname));
			buf->PutInt(0); // token spot

		//	fout << "spoof name = " << spoofname.GetString() << endl;
			
			sk  = &(ptr_keyword->m_supply_keywords);
			buf->PutInt((int)sk->v_keywords.size() * 10);

		//	fout << "number of tracks = " << sk->v_keywords.size() << endl;
			for (l = 0; l < 10; l++)
			{

				//TRACE("IN L LOOP OF AUDIO CREATING SPOOF!\n");
			clean_path = GetFilePath(ptr_keyword->m_artist_name.c_str(), ptr_keyword->m_album_name.c_str());

			if (random(0, 1) == 0)
				bitrate = 192;
			else
				bitrate = 128;

			for (k = 0; k < (int)sk->v_keywords.size(); k++)
			{
				//if (k == 15) 
				//	continue;
			    //TRACE("IN K LOOP OF AUDIO CREATING SPOOF!\n");
			 //   sk  = &(ptr_keyword->m_supply_keywords);
			    

				//track_info = new TrackInformation(*(*m_track_info)[k]);
					//counter = 0; 
				

                 
				path = clean_path;
				buf->PutByte(1);
			//	fout << "a bit i forgot for what " << endl;
				path.Append(sk->v_keywords[k].m_track_name.c_str());
				path.Append(".mp3");
			//	fout << "length of path = " << strlen((const char*) path) << endl;
				buf->PutInt((int)strlen((const char*)(LPCTSTR)path));
			//	fout << "The path = " << path.GetString() << endl;
				buf->PutCharPtr((const char*)(LPCTSTR)path, (int)strlen((const char*)(LPCTSTR)path));
			
				//		file_found = false;
				//TRACE ("IN MIDDLE OF K LOOP OF AUDIO CREATING SPOOF!\n");

	//			file_found = true;
				/*if (k == 0)
					buf->PutLong(7354288);
				if (k == 1)
					buf->PutLong(2000230);
				if (k == 2)
					buf->PutLong(2746287);
				if (k == 3)
					buf->PutLong(6104799);
				if (k == 4)
					buf->PutLong(6080348);
				if (k == 5)
					buf->PutLong(5166898);
				if (k == 6)
					buf->PutLong(5783806);
				if (k == 7)
					buf->PutLong(4180096);
				if (k == 8)
					buf->PutLong(6899130);
				if (k == 9)
					buf->PutLong(10472681);
				if (k == 10)
					buf->PutLong(5385700);
				if (k == 11)
					buf->PutLong(4324242);
				if (k == 12)
					buf->PutLong(9532273);
				if (k == 13)
					buf->PutLong(7492841);
				if (k > 13)
				{*/
					songsize = random(2000000, 10000000);
				    songlength = (songsize / 1000000) * 69;
					buf->PutLong(songsize);
			/*	}*/


				buf->PutInt(0);
				buf->PutInt(3);
				buf->PutCharPtr("mp3", 3);
				buf->PutInt(3);
				buf->PutInt(0);
				buf->PutInt(bitrate);
				buf->PutInt(1);

			//	fout << "file size\n0\n3\nmp3\n3\n0\n" << bitrate << "\n1\nlength\n";
				/*if (k == 0)
					buf->PutInt(506);
				if (k == 1)
					buf->PutInt(123);
				if (k == 2)
					buf->PutInt(154);
				if (k == 3)
					buf->PutInt(414);
				if (k == 4)
					buf->PutInt(413);
				if (k == 5)
					buf->PutInt(335);
				if (k == 6)
					buf->PutInt(400);
				if (k == 7)
					buf->PutInt(254);
				if (k == 8)
					buf->PutInt(447);
				if (k == 9)
					buf->PutInt(716);
				if (k == 10)
					buf->PutInt(344);
				if (k == 11)
					buf->PutInt(300);
				if (k == 12)
					buf->PutInt(637);
				if (k == 13)
					buf->PutInt(512);
				if (k > 13)*/
					buf->PutInt(songlength);

				buf->PutInt(2);
				buf->PutInt(0);
			//	fout << "2\n0\n";
			//	break;
			//TRACE("END OF K LOOP\n");
					
			}
			//TRACE("END OF L LOOP\n");
			}
			if ( j == 2 || j == 6)
					{
						buf->PutByte(0);
						buf->PutInt(random(1000, 80000));
						buf->PutInt(rand() % 10);
				//		fout << "no aviable slots\nupload speed\nnum of people in que\n";
					}
					else
					{
					buf->PutByte(1);
					buf->PutInt(random(1000, 80000));
					buf->PutInt(0);	
			//		fout << "yes available slots\nupload speed\nno one in que\n";
					}

			
	//		fout << endl;
		//	fout << buf->GetCharPtr() << endl;
					/*int index = 0;
					int sl = buf->GetInt(index);
					index += 4;
					fout << "String length = " << sl << endl;
					char* st = new char[sl + 1];
					index += sl;
					strncpy(st, buf->GetCharPtr(index), sl);
					st[sl] = '/0';
					fout << "Username = " << st << endl;
					delete [] st;
					int token = buf->GetInt(index);
					fout << "token = " << token << endl;
					index += 4;
					int nof = buf->GetInt(index);
					fout << "number of files = " << nof << endl;
					index += 4;
					for (int v = 0; v < nof; v++)
					{
						fout << "code = " << buf->GetByte(index) << endl;
						index += 1;
						int sl = buf->GetInt(index);
						index += 4;
						fout << "String length = " << sl << endl;
						st = new char[sl + 1];
						strncpy(st, buf->GetCharPtr(index), sl);
						st[sl] = '\0';
						fout << "string = " << st << endl;
						index += sl;
						delete [] st;
						fout << "size1 = " << buf->GetInt(index) << endl;
						index += 4;
						fout << "size2 = " << buf->GetInt(index) << endl;
						index += 4;
						fout << "string length = " <<buf->GetInt(index) << endl;
						index += 4;
						st = new char[4];
						strncpy(st, buf->GetCharPtr(index), 3);
						st[3] = '\0';
						fout << "ext = " << st << endl;
						delete [] st;
						index += 3;
                        fout << "Num of attrib = " << buf->GetInt(index) << endl;
						index += 4;
						fout << "c0 = " << buf->GetInt(index) << endl;
						index += 4;
						fout << "value = " << buf->GetInt(index) << endl;
						index += 4;
						fout << "c1 = " << buf->GetInt(index) << endl;
						index += 4;
						fout << "value = " << buf->GetInt(index) << endl;
						index += 4;
						fout << "c2 = " << buf->GetInt(index) << endl;
						index += 4;
						fout << "value = " << buf->GetInt(index) << endl;
						index += 4;
					}
					fout << "available = " << buf->GetByte(index) << endl;
					index += 1;
					fout << "upload = " << buf->GetInt(index) << endl;
					index += 4;
					fout << "queue = " << buf->GetInt(index) << endl;*/
			s.m_spoof.push_back(buf);
			//TRACE("END OF J LOOP\n");

		}

		//TRACE("END OF AUDIO CREATING SPOOF\n");
			}

			//TRACE("FINISHED CREATING SPOOFS NOW ADDING TO m_spoofs!\n");
			TRACE("Artist = %s and num of spoofs = %d\n", s.m_artist, s.m_spoof.size());
		//	fout << s.m_spoof
		m_spoofs.push_back(s);
	//	fout << "\n\n";
			
	}

	TRACE("DONE CREATING SPOOFS!\n");
	m_spoofs_created = true;
		
}

/*			clean_path = p_sockets->GetFilePath(task->m_artist, task->m_album);

			for (j = 0; j < (int)task->tracks.size(); j++)
			{
				path = clean_path;
				buf->PutByte(1);
				path.Append(task->tracks[j].m_track_name);
				path.Append(".mp3");
				buf->PutInt((int)strlen((const char*)(LPCTSTR)path));
 				buf->PutCharPtr((const char*)(LPCTSTR)path, (int)strlen((const char*)(LPCTSTR)path));
			
				file_found = false;

				for (k = 0; k < (int)p_sockets->spoofing_info.size(); k++)
				{
					if (task->m_project_id == p_sockets->spoofing_info[i].m_project_id && task->tracks[j].m_track_number == p_sockets->spoofing_info[k].m_track_number)
					{
						file_found = true;
						buf->PutInt(p_sockets->spoofing_info[k].m_track_size);
						buf->PutInt(0);
						buf->PutInt(3);
						buf->PutCharPtr("mp3", 3);
						buf->PutInt(3);
						buf->PutInt(0);
						buf->PutInt(p_sockets->spoofing_info[k].m_bit_rate);
						buf->PutInt(1);
						buf->PutLong(p_sockets->spoofing_info[k].m_track_length);
						buf->PutInt(2);
						buf->PutInt(0);
						break;
					}
				}

				if (file_found == false)
				{
					buf->PutLong(4781848);
					buf->PutInt(0);
					buf->PutInt(3);
					buf->PutCharPtr("mp3", 3);
					buf->PutInt(3);
					buf->PutInt(0);
					buf->PutInt(192);
					buf->PutInt(1);
					buf->PutInt(318);
					buf->PutInt(2);
					buf->PutInt(0);
				}
			}
			

	//		strcpy(zipped, "");
	//		zippedsize = 1024;
	//		compsize = compress((Bytef*)zipped, (uLong *)&zippedsize, (Bytef*)buf->GetCharPtr(), (uLong)buf->Size());

	//		if (compsize != Z_OK)
	//		{
	//			::MessageBox(NULL, "ERROR COMPRESSING THE SPOOFER", "COMPRESSION ERROR!", MB_OK);
	//		}

			delete buf;
			
			buffer *sendbuffer = new buffer();
			sendbuffer->PutInt(0);
			sendbuffer->PutInt(9);
			sendbuffer->PutCharPtr(zipped, zippedsize);
			//delete buf;
			sendbuffer->SetInt((int)sendbuffer->Size() - 4, 0);
			spoofs.push_back(sendbuffer);
		
		}

		//delete task;
 		p_sockets->Log("Finished generating spoofs!");*/
//	}
//}
void ConnectionSockets::ReportStatus(ConnectionModuleStatusData& status_data)
{
	int i;
	CTime time = CTime::GetCurrentTime();// = CTime::GetHour(), min = CTime::GetMinute(), sec = CTime::GetSecond();

	if (time.GetHour() == 0 && time.GetMinute() == 0 && time.GetMinute() == 0)
	{
		db mydb;
		mydb.dbConnection("localhost", "onsystems", "sumyungguy37", "slsk");

		CString query;

		for (i = 0; i < (int)daily_demand.size(); i++)
		{
			query = "Insert into daily_demand values('";
			query.Append(daily_demand[i].m_artist);
			query.Append("', ");
			query.AppendFormat("%d", daily_demand[i].m_counter);
			query.Append(", ");
			query.Append((CTime::GetCurrentTime()).Format("%Y%m%d%H%M%S"));
			query.Append(");");
			mydb.dbOQuery((char*)(LPCTSTR)query);
		}

		mydb.dbClose();
		//daily_demand.clear();
		//FillDailyDemandVector();
	}

	/*if ((CTime::GetCurrentTime() - m_last_status_sent).GetTotalSeconds() >= 7)
	{
		if ( state == 1)
		{
			int serverconnections = 0;

			for (int i = 0; i < 2; i++)
			{
				if (m_sockets[i].m_logged_in == true)
					serverconnections++;
			}

			char* status = new char[10];
			sprintf(status, "%d", serverconnections);
		    ServerStatus(status);
		}

		if (state == 2)
		{
			int connected = 0, connecting = 0;
			int num_socket_events=ReturnNumberOfSocketEvents();

			for (int i = 0; i < (UINT)num_socket_events; i++)
			{
				if (m_sockets[i].m_state_connecting == 1)
					connecting++;
				if (m_sockets[i].m_state_connecting == 2)
					connected++;
			}

			char *status1 = new char[10];
			sprintf(status1, "%d", connecting);
			setParentConnecting(status1);

			char*status2 = new char[10];
			sprintf(status2, "%d", connected);
			ParentStatus(status2);

		}

		if (state == 3)
		{

			int  connected = 0, connecting = 0;
			int num_socket_events=ReturnNumberOfSocketEvents();

			for (int i = 0; i < (UINT)num_socket_events; i++)
			{
				if (m_sockets[i].m_state_connecting == 1)
					connecting++;
				if (m_sockets[i].m_state_connecting == 2)
					connected++;
			}

			char *status1 = new char[10];
			sprintf(status1, "%d", connecting);
			setPeerConnecting(status1);

			char *status2 = new char[10];
			sprintf(status2, "%d", connected);
			PeerStatus(status2);
		}

		m_last_status_sent = CTime::GetCurrentTime();
	}*/

	if (state == 1 && (CTime::GetCurrentTime() - m_attempted_server_connection).GetTotalSeconds() >= 60)
	{
		SupernodeHost nh;
		nh.SetIP("38.115.131.131");
		nh.setport(2240);

		for (int i = 0; i < 2; i++)
		{
			if(m_sockets[i].m_logged_in == false)
			{
				//m_sockets[j].Create();
				m_sockets[i].Connect(nh);
			//	break;
			}
		}
		m_attempted_server_connection = CTime::GetCurrentTime();
	}
	m_one_hour_timer++;
	

	int num_socket_events=ReturnNumberOfSocketEvents();



	// See how many sockets are idle (and therefore need ips).  Any non-idle sockets, get their host connection status
	for(i=0;i<num_socket_events;i++)
	{
		if(m_sockets[i].IsSocket()==false)
		{
			m_status_data.m_idle_socket_count++;
		}
		else
		{
			if(m_sockets[i].IsConnected())
			{
				m_sockets[i].TimerHasFired();
				m_status_data.m_connected_socket_count++;
			}
			else	// connecting
			{
				m_status_data.m_connecting_socket_count++;
			}

			// Get the ip of the connection and the connection status of the socket
			if(m_sockets[i].IsConnected())
				m_status_data.v_connected_host_ips.push_back(m_sockets[i].ReturnHostIP());

			//m_status_data.m_cached_query_hit_count+=m_sockets[i].ReturnCachedQueryHitCount();
		}
	}

	//ConnectionModuleStatusData ret=m_status_data;
	//status_data=m_status_data;
	
	//m_status_data.ClearCounters();

//	if( (m_one_hour_timer%3600) == 0 )// reload username from file every hour
//		ReadInUserNames();
	
	//return ret;
}

//
//
//
void ConnectionSockets::GetPeerIP(SLSKtask *task)
{
	if (task->m_state == 1001)
		m_sockets[0].SendCantConnect(task);
	else
		m_sockets[0].GetPeerIP(task);
}

void ConnectionSockets::ConnectToHosts(vector<SupernodeHost> &hosts)
{
	UINT i,j;
	// Find idle sockets to tell these hosts to
	int num_socket_events=ReturnNumberOfSocketEvents();
	for(i=0;i<hosts.size();i++)
	{
		for(j=0;j<(UINT)num_socket_events;j++)
		//for(j=0;j<1;j++)
		{
			if(m_sockets[j].IsSocket()==false)
			{
				//m_sockets[j].Create();
				m_sockets[j].ListeningPort = ListeningPort + i;
				m_sockets[j].Connect(hosts[i]);
				if (j == 0)
				{
					m_sockets[j].m_main_server = true;
				}
				else 
					m_sockets[j].m_main_server = false;
				break;
			}
		}
	}

	m_attempted_server_connection = CTime::GetCurrentTime();
}


CString ConnectionSockets::GetFilePath(CString artist, CString album)
{
	CString paths[10] = {"C:\\downloads\\", "C:\\mydownloads\\", "C:\\music\\", "C:\\Documents and Settings\\All Users\\Documents\\My Music\\", "C:\\", "D:\\my music\\", "D:\\downloads\\", "E:\\downloads\\", "D:\\musics\\", "C:\\downloaded\\"};

	CString filepath = paths[rand() % 10];
	filepath.Append(artist);
	filepath.Append("\\");
	if (album.TrimLeft().IsEmpty())
		return filepath;
	
	filepath.Append(album);
	filepath.Append("\\");
//	filepath.Append(track);
//	filepath.Append("mp3");

	return filepath;

}

/*void ConnectionSockets::FillKeywords(void)
{
	ifstream fin;
	fin.open("C:\\SLSKInfo\\keywords.txt", ios::in);
	char k[100];

	weighted_keywords.clear();
	while(!fin.eof())
	{
		w_keyword newkeyword;
		fin >> newkeyword.m_project_id;
		fin.getline(k, 100);
		fin.getline(k, 100);
		fin >> newkeyword.m_weight;
		newkeyword.m_keyword = k;
		weighted_keywords.push_back(newkeyword);
	}
	fin.close();
}

void ConnectionSockets::FillProjects(void)
{
	ifstream fin;
	fin.open("C:\\SLSKInfo\\projectsInfo.txt", ios::in);

	char temp[100];

	projects.clear();
	while(!fin.eof())
	{
		project_info project;
		fin >> project.m_project_id;
		fin.getline(temp, 100);
		fin.getline(temp, 100);
		//fin.getline(temp, 100);
		project.m_artist = temp;
		fin.getline(temp, 100);
		project.m_album = temp;
		int num_of_tracks;
		fin >> num_of_tracks;

		for (int i = 0; i < num_of_tracks; i++)
		{
			track t;
			fin >> t.m_track_number;
			fin.getline(temp, 100);
			fin.getline(temp, 100);
			t.m_track_name = temp;
			project.tracks.push_back(t);
		}

		projects.push_back(project);
	}
fin.close();
}
*/
void ConnectionSockets::FillSpoofs(void)
{
	
/*	ifstream fin;
	fin.open("C:\\SLSKInfo\\SpoofingInfo.txt", ios::out);

	char temp[100];

	while(!fin.eof())
	{
		spoof_info newinfo;
		fin >> newinfo.m_project_id;
		fin >> newinfo.m_track_number;
		fin >> newinfo.m_track_size;
		fin >> newinfo.m_bit_rate;
		fin >> newinfo.m_track_length;
		spoofing_info.push_back(newinfo);
	}
	fin.close();
	*/

	CStdioFile spoof_file;
	BOOL open = spoof_file.Open("C:\\SLSKInfo\\SpoofingInfo.txt", CFile::typeText|CFile::modeRead|CFile::shareDenyNone);

	spoofing_info.clear();
	if (open==TRUE)
	{
		spoof_info newinfo;
		CString line;

		BOOL line_exists = true;
		
		while(line_exists == TRUE)
		{
			spoof_file.ReadString(line);
			newinfo.m_project_id = atoi((const char *)(LPCTSTR)line);

			if(abs(newinfo.m_project_id) > 9999)
			{
				int asdf = 0;
			}
			
			spoof_file.ReadString(line);
			newinfo.m_track_number = atoi((const char *)(LPCTSTR)line);

			if(abs(newinfo.m_track_number) > 50)
			{
				int asdf = 0;
			}

			spoof_file.ReadString(line);
			newinfo.m_track_size = atoi((const char *)(LPCTSTR)line);

			if(abs(newinfo.m_track_size) > 20000000)
			{
				int asdf = 0;
			}

			spoof_file.ReadString(line);
			newinfo.m_bit_rate = atoi((const char *)(LPCTSTR)line);

			if(abs(newinfo.m_bit_rate) > 500)
			{
				int asdf = 0;
			}

			line_exists = spoof_file.ReadString(line);
			newinfo.m_track_length = atoi((const char *)(LPCTSTR)line);

			if(abs(newinfo.m_track_length) > 1000)
			{
				int asdf = 0;
			}

			if(line_exists==TRUE)
				spoofing_info.push_back(newinfo);
		}

		spoof_file.Close();



		

	}

}

CString ConnectionSockets::getUserName(void)
{
	CString username;
//	int temp;

	fillName();
	
	username = names[rand() % 100];
	username.AppendFormat("%d", rand() % 10);
	username.AppendFormat("%d", rand() % 10); 
	username.AppendFormat("%d", rand() % 10);
	username.AppendFormat("%d", rand() % 10);

	names.clear();
	return username;
}

CString ConnectionSockets::getPassword(void)
{
	CString password;
	int temp;
  
	for (int j = 0; j < 8; j++)
	{
		temp = random(48, 122);

		while (temp >= 58 && temp <= 64 || temp >= 91 && temp <= 96 || temp > 122)
		   temp = random(48, 122);

		password.AppendChar((char)temp);//strcat(password, ctemp);
	}

	return password;
}

CString ConnectionSockets::getSpoofName(void)
{
	CString username;
//	int temp;

	fillName();
	
	username = names[rand() % 100];
	username.AppendFormat("%d", rand() % 10);
	username.AppendFormat("%d", rand() % 10); 
	username.AppendFormat("%d", (username.GetLength() % 10));//rand() % 10);
	//username.AppendFormat("%d", rand() % 10);
	//username.AppendFormat("%d", 5);

	names.clear();
	return username;
}

void ConnectionSockets::fillName(void)
{
	names.push_back("Jacob");
	names.push_back("Emily");
	names.push_back("Michael");
	names.push_back("Emma");
	names.push_back("Joshua");
	names.push_back("Madison");
	names.push_back("Matthew");
	names.push_back("Hannah");
	names.push_back("Andrew");
	names.push_back("Olivia");
	names.push_back("Joseph");
	names.push_back("Abigail");
	names.push_back("Ethan");
	names.push_back("Alexis");
	names.push_back("Daniel");
	names.push_back("Ashley");
	names.push_back("Christopher");
	names.push_back("Elizabeth");
	names.push_back("Anthony");
	names.push_back("Samantha");
	names.push_back("William");
	names.push_back("Isabella");
	names.push_back("Ryan");
	names.push_back("Sarah");
	names.push_back("Nicholas");
	names.push_back("Grace");
	names.push_back("David");
	names.push_back("Alyssa");
	names.push_back("David");
	names.push_back("Tyler");
	names.push_back("Lauren");
	names.push_back("Alexander");
	names.push_back("Kayla");
	names.push_back("John");
	names.push_back("Brianna");
	names.push_back("James");
	names.push_back("Jessica");
	names.push_back("Dylan");
	names.push_back("Taylor");
	names.push_back("Zachary");
	names.push_back("Sophia");
	names.push_back("Brandon");
	names.push_back("Anna");
	names.push_back("Jonathan");
	names.push_back("Victoria");
	names.push_back("Samuel");
	names.push_back("Natalie");
	names.push_back("Christian");
	names.push_back("Chloe");
	names.push_back("Benjamin");
	names.push_back("Sydney");
	names.push_back("Justin");
	names.push_back("Hailey");
	names.push_back("Nathan");
	names.push_back("Jasmine");
	names.push_back("Jose");
	names.push_back("Rachel");
	names.push_back("Logan");
	names.push_back("Morgan");
	names.push_back("Gabriel");
	names.push_back("Megan");
	names.push_back("Kevin");
	names.push_back("Jennifer");
	names.push_back("Noah");
	names.push_back("Kaitlyn");
	names.push_back("Austin");
	names.push_back("Julia");
	names.push_back("Caleb");
	names.push_back("Haley");
	names.push_back("Robert");
	names.push_back("Mia");
	names.push_back("Thomas");
	names.push_back("Katherine");
	names.push_back("Elijah");
	names.push_back("Destiny");
	names.push_back("Jordan");
	names.push_back("Alexandra");
	names.push_back("Aidan");
	names.push_back("Nicole");
	names.push_back("Cameron");
	names.push_back("Maria");
	names.push_back("Hunter");
	names.push_back("Ava");
	names.push_back("Jason");
	names.push_back("Savannah");
	names.push_back("Angel");
	names.push_back("Brooke");
	names.push_back("Connor");
	names.push_back("Ella");
	names.push_back("Evan");
	names.push_back("Allison");
	names.push_back("Jack");
	names.push_back("MacKenzie");
	names.push_back("Luke");
	names.push_back("Paige");
	names.push_back("Isaac");
	names.push_back("Stephanie");
	names.push_back("Aaron");
	names.push_back("Jordan");
	names.push_back("Isaiah");
	names.push_back("Kylie");
}

int ConnectionSockets::random(int lowest, int highest)
{
	double range = (highest - lowest) + 1.0;
    return lowest + (int) (range * rand() / RAND_MAX + 1.0);
}
void ConnectionSockets::ConnectToHosts(CString un, vector<SupernodeHost> &hosts)
{


	if (projects.size() == 0 && state == 2)
	{
		projects.clear();
		weighted_keywords.clear();
	//	FillProjects();
	//	FillKeywords();
	}
	
	UINT i, j;

	int num_socket_events = ReturnNumberOfSocketEvents();
	for (i = 0; i< hosts.size(); i++)
	{
		for (j = 0;j < (UINT)num_socket_events; j++)
		{
			if (m_sockets[j].IsSocket() == false)
			{
				m_sockets[j].Connect(un, hosts[i]);
				break;
			}
		}
	}
}

void ConnectionSockets::ConnectToHosts(CString un, SLSKtask *t)
{
	if (m_spoofs_created == false)
	{
		delete t;
		return;
	}

	UINT /*i,*/ j;
	
	int num_socket_events = ReturnNumberOfSocketEvents();
	for (j = 0; j < (UINT)num_socket_events; j++)
		if (m_sockets[j].IsSocket() == false)
		{
			m_sockets[j].Connect(un, t);
			break;
		}
}
//
//
//
void ConnectionSockets::ReportHosts(vector<SupernodeHost> &hosts)
{

	int i,j;
	
	

	// Add these hosts to the hosts vector
	for(i=0;i<(int)hosts.size();i++)
	{
		// Check to see if they are already in the hosts vector
		bool found=false;
		for(j=(int)m_status_data.v_host_cache.size()-1;j>=0;j--)
		{
			if(hosts[i]==m_status_data.v_host_cache[j])
			{
				found=true;
				break;
			}
		}

		if(!found)
		{
			m_status_data.v_host_cache.push_back(hosts[i]);
		}
	}
}

//
//
//
/*void ConnectionSockets::PerformProjectSupplyQuery(ProjectSupplyQuery &psq)
{
	int i;
	hash_set<ProjectSupplyQuery>::iterator iter = hs_project_supply_queries.begin();
	while(iter!=hs_project_supply_queries.end())
	{
		if((strcmp(iter->m_project.c_str(),psq.m_project.c_str())==0))
		{
			hs_project_supply_queries.erase(iter);
			break;
		}
		iter++;
	}
	hs_project_supply_queries.insert(psq);
	// Tell all of the sockets to perform this query, unless this is a searcher query, where we only tell 5 ultrapeer sockets
	int num_socket_events=ReturnNumberOfSocketEvents();
	for(i=0;i<num_socket_events;i++)
	{
		m_sockets[i].PerformProjectSupplyQuery(psq);
	}

	// Remove any existing queries for this project so that all returns from the new query will not count the returns for any old queries
	// Make sure that if this query is a searcher query, that we only remove searcher queries...same with supply queries
/*
	vector<ProjectSupplyQuery>::iterator psq_iter=v_project_supply_queries.begin();
	while(psq_iter!=v_project_supply_queries.end())
	{
		if((strcmp(psq_iter->m_project.c_str(),psq.m_project.c_str())==0))
		{
			v_project_supply_queries.erase(psq_iter);
			break;
		}
		psq_iter++;
	}

	// Save this query to the list of project supply queries that we've done 
	v_project_supply_queries.push_back(psq);

}
*/
//
//
//

/*void ConnectionSockets::UpdateKeywords(vector<ProjectKeywords> &keywords)
{
	UINT i,j;
	
	v_keywords=keywords;

	// Make sure that the project status data is still valid since the keywords have been updated...basically the indexes for each project in
	// the keywords vector have to be the same as the indexes for the same project in the project status vector.
	vector<ProjectStatus> old_project_status=m_status_data.v_project_status;
	m_status_data.v_project_status.clear();

	for(i=0;i<v_keywords.size();i++)
	{
		bool found=false;
		for(j=0;j<old_project_status.size();j++)
		{
			if(strcmp(old_project_status[j].ProjectName(),v_keywords[i].m_project_name.c_str())==0)
			{
				found=true;
				m_status_data.v_project_status.push_back(old_project_status[j]);
				break;
			}
		}

		// If not found, create a new project status object (this must be a new project, then)
		if(!found)
		{
			ProjectStatus ps;
			ps.ProjectName((char *)keywords[i].m_project_name.c_str());
			m_status_data.v_project_status.push_back(ps);
		}
	}

	// Make sure all of the project keyword data of the project supply queries are still correct
	hash_set<ProjectSupplyQuery>::iterator psq_iter=hs_project_supply_queries.begin();
	while(psq_iter!=hs_project_supply_queries.end())
	{
		bool found=false;
		for(i=0;i<v_keywords.size();i++)
		{
			if(strcmp(psq_iter->m_project.c_str(),v_keywords[i].m_project_name.c_str())==0)
			{
				found=true;
				psq_iter->ExtractProjectKeywordData(&v_keywords[i]);
				psq_iter->m_project_status_index=i;
				break;
			}
		}

		// If not found, then this project no longer exists, so remove the query from the vector
		if(!found)
		{
			hs_project_supply_queries.erase(psq_iter);
			psq_iter=hs_project_supply_queries.begin();
		}
		else
		{
			psq_iter++;
		}
	}

	// Tell all of the sockets that the keywords have been updated, so that they can re-send their QRP table
	/*
	int num_socket_events=ReturnNumberOfSocketEvents();
	for(i=0;i<(UINT)num_socket_events;i++)
	{
		m_sockets[i].KeywordsUpdated();
	}
	
}
*/
//
//
//
/*
void ConnectionSockets::UpdateSupply(vector<SupplyProject> &supply)
{
	// Else it's not an update, so just replace the local vector
	v_supply=supply;
}
*/

//
//
//
/*vector<VendorCount> *ConnectionSockets::ReturnVendorCounts()
{
	UINT i,j;

	// Create a vector on the heap...the memory will be freed in the main thread
	vector<VendorCount> *vendor_counts=new vector<VendorCount>;

	// Get the vendor name from all of the connected sockets. 
	int num_socket_events=ReturnNumberOfSocketEvents();
	for(i=0;i<(UINT)num_socket_events;i++)
	{
		if(m_sockets[i].IsConnected())
		{
			string vendor=m_sockets[i].ReturnRemoteVendor();

			// Check the vector to see if this vendor is already in there
			bool found=false;
			for(j=0;j<vendor_counts->size();j++)
			{
				if(strcmp(vendor.c_str(),(*vendor_counts)[j].m_vendor.c_str())==0)
				{
					found=true;
					(*vendor_counts)[j].m_count++;
					break;
				}
			}

			// If not found, then create a new vendor count object
			if(!found)
			{
				VendorCount vc;
				vc.m_vendor=vendor;
				vc.m_count=1;
				vendor_counts->push_back(vc);
			}
		}
	}
	
	return vendor_counts;
}

//
//
//
void ConnectionSockets::Log(const char* log)
{
	if(m_dlg_hwnd != NULL)
		::SendMessage(m_dlg_hwnd,WM_LOG_MSG,(WPARAM)log,0);
}

//
//
//
/*
void ConnectionSockets::Search(const char* search)
{
	int num_socket_events=ReturnNumberOfSocketEvents();
	for(int j=0;j<num_socket_events;j++)
	{
		if(m_sockets[j].IsSocket())
		{
			m_sockets[j].Search(search);
		}
	}
}
*/
/*
void ConnectionSockets::SpoofIP(unsigned int ip)
{
	v_spoof_ips.push_back(ip);

	// Limit the vector size to 100 ips
	while(v_spoof_ips.size()>100)
	{
		v_spoof_ips.erase(v_spoof_ips.begin());
	}
}

//
//
//
unsigned int ConnectionSockets::SpoofIP()
{
	unsigned int ip = 0;
	// Pick a random ip that we've recently seen
	if(v_spoof_ips.size() > 0)
	{
		ip = v_spoof_ips[rand() % v_spoof_ips.size()];

		// Alter the last two ip digits (ie. 192.168.x.x)
		ip&=0x0000FFFF;
		ip|=(rand()%256)<<16;
		ip|=(rand()%256)<<24;
	}
	else
	{
		ip&=0xFFFFFFFF;
		ip|=(rand()%256)<<0;
		ip|=(rand()%256)<<8;
		ip|=(rand()%256)<<16;
		ip|=(rand()%256)<<24;
	}

	return ip;
}
*/

//
//
//
void ConnectionSockets::ReConnectAll()
{
	UINT j;
	int num_socket_events=ReturnNumberOfSocketEvents();
	for(j=0;j<(UINT)num_socket_events;j++)
	{
		m_sockets[j].Close();
	}
}


//
//
//
/*void ConnectionSockets::GetRandomUserName(CString& username)
{
	if(v_usernames.size() > 0)
	{
		int index = m_rng.GenerateWord32(0,v_usernames.size()-1);
		char* new_username = new char[v_usernames[index].GetLength()+6];
		strcpy(new_username,v_usernames[index]);
		ChangeUserName(new_username);
		username = new_username;
		delete [] new_username;
	}
	else
		username = "KazaaLiteK++";
}

//
//
//
void ConnectionSockets::ChangeUserName(char * username)
{
	CString string = username;
	char* tempuser = new char[strlen(username)+6];
	int len;
	int num;
	int randnum;
	char numchar[1+1];

	//Remove all the numbers
	string.Remove('0');
	string.Remove('1');
	string.Remove('2');
	string.Remove('3');
	string.Remove('4');
	string.Remove('5');
	string.Remove('6');
	string.Remove('7');
	string.Remove('8');
	string.Remove('9');

	len = string.GetLength();
	randnum = rand()%4;

	strcpy(tempuser,string);
	for (int i = 0;i<randnum;i++)
	{
		num = rand()%10;
		_itoa(num,numchar,10);
		strcat(tempuser,numchar);
	}

	num = (len%10);
	_itoa(num,numchar,10);
	strcat(tempuser,numchar);

	strcpy(username,tempuser);
	delete [] tempuser;
}

//
//
//
void ConnectionSockets::ReadInUserNames()
{
	CStdioFile file;
	CString filestring;
	char tempstring[256+1];
	if(file.Open("c:\\syncher\\rcv\\Kazaa-Launcher\\namelist.txt", CFile::modeRead|CFile::typeText) != 0)
	{
		v_usernames.clear();
		LPTSTR data = file.ReadString(tempstring, 256);
		while (data!= NULL)
		{
			filestring = tempstring;
			filestring.TrimRight();
			filestring.TrimLeft();
			v_usernames.push_back(filestring);
			data = file.ReadString(tempstring, 256);
		}
	}
	else
	{
//		MessageBox(NULL, "Coudn't find the file \"namelist.txt\"", "ERROR", MB_OK|MB_ICONERROR);
		return;
	}
	file.Close();
}
/*
//
//
//
void ConnectionSockets::CreateSpoofShares()
{
	vector<PacketBuffer*>::iterator iter = v_shared_files_buffers.begin();
	while(iter != v_shared_files_buffers.end())
	{
		delete *iter;
		v_shared_files_buffers.erase(iter);
	}

	unsigned char hash[FST_FTHASH_LEN];
	UINT filesize = 0;
	UINT* pFileSize;
	int duration;
/*
	//spoofs for Matchbox 20 Unwell
	for(UINT i=0; i<2600; i++)
	{
		filesize = m_rng.GenerateWord32(2000000, 6000000); //random filesize between 2MB - 6 MB
		//make it disvisable by 137 if we want decoy instead swarm
		//filesize = filesize - filesize%137;
		for(int j=0; j<FST_FTHASH_LEN-4; j++)
			hash[j] = rand()%256;

		pFileSize = (UINT *)&hash[FST_FTHASH_LEN-4];
		*pFileSize = filesize;
		int song_duration = rand()%7 + 227; //3:47 to 3:53
		PacketBuffer* buf = new PacketBuffer();
		char* filename = GetRandomFilename("Matchbox 20","More Than You Think You Are","Unwell",4);
		
		
		bool send_rating = false;
		if(rand()%2 == 0)
			send_rating = true;


		buf->PutByte(0x00); //unknown
		buf->PutByte(MEDIA_TYPE_AUDIO); //media type
		buf->PutStr("\x00\x00",2);
		buf->PutUStr(hash,FST_FTHASH_LEN); //hash
		buf->PutDynInt(GetHashChecksum(hash)); //file_id
		buf->PutDynInt(filesize); //file size
		if(send_rating)
			buf->PutDynInt(8);	//number of tag
		else
			buf->PutDynInt(7);	//number of tag
		
		//filename
		buf->PutDynInt(FILE_TAG_FILENAME);	//tag type
		buf->PutDynInt((UINT)strlen(filename));	//tag length
		buf->PutStr(filename,strlen(filename)); //tag content

		//artist
		buf->PutDynInt(FILE_TAG_ARTIST);	//tag type
		if(rand()%2==0)
		{
			buf->PutDynInt((UINT)strlen("Matchbox 20"));	//tag length
			buf->PutStr("Matchbox 20",strlen("Matchbox 20")); //tag content
		}
		else
		{
			buf->PutDynInt((UINT)strlen("Matchbox Twenty"));	//tag length
			buf->PutStr("Matchbox Twenty",strlen("Matchbox Twenty")); //tag content
		}

		//album
		buf->PutDynInt(FILE_TAG_ALBUM);	//tag type
		buf->PutDynInt((UINT)strlen("More Than You Think You Are"));	//tag length
		buf->PutStr("More Than You Think You Are",strlen("More Than You Think You Are")); //tag content

		//title
		buf->PutDynInt(FILE_TAG_TITLE);	//tag type
		buf->PutDynInt((UINT)strlen("Unwell"));	//tag length
		buf->PutStr("Unwell",strlen("Unwell")); //tag content

		//category
		buf->PutDynInt(FILE_TAG_CATEGORY);	//tag type
		int category = rand()%5;
		switch(category)
		{
			case 0:
			{
				buf->PutDynInt((UINT)strlen("AlternRock"));	//tag length
				buf->PutStr("AlternRock",strlen("AlternRock")); //tag content
				break;
			}
			case 1:
			{
				buf->PutDynInt((UINT)strlen("Rock"));	//tag length
				buf->PutStr("Rock",strlen("Rock")); //tag content
				break;
			}
			case 2:
			{
				buf->PutDynInt((UINT)strlen("Other"));	//tag length
				buf->PutStr("Other",strlen("Other")); //tag content
				break;
			}
			case 3:
			{
				buf->PutDynInt((UINT)strlen("Pop"));	//tag length
				buf->PutStr("Pop",strlen("Pop")); //tag content
				break;
			}
			case 4:
			{
				buf->PutDynInt((UINT)strlen("Rock/Pop"));	//tag length
				buf->PutStr("Rock/Pop",strlen("Rock/Pop")); //tag content
				break;
			}
		}

		//song duration
		buf->PutDynInt(FILE_TAG_TIME);	//tag type
		PacketBuffer* temp_buf = new PacketBuffer();
		temp_buf->PutDynInt(song_duration);
		buf->PutDynInt(temp_buf->Size());
		temp_buf->Rewind();
		buf->Append(*temp_buf);

		//bitrate
		buf->PutDynInt(FILE_TAG_QUALITY);	//tag type
		delete temp_buf;
		temp_buf = new PacketBuffer();
		if(rand()%2==0)
			temp_buf->PutDynInt(128);
		else
			temp_buf->PutDynInt(192);
		buf->PutDynInt(temp_buf->Size());
		temp_buf->Rewind();
		buf->Append(*temp_buf);

		//rating
		if(send_rating)
		{
			buf->PutDynInt(FILE_TAG_RATING);	//tag type
			delete temp_buf;
			temp_buf = new PacketBuffer();
			temp_buf->PutDynInt(4);
			buf->PutDynInt(temp_buf->Size());
			temp_buf->Rewind();
			buf->Append(*temp_buf);
		}
		delete [] filename;
		delete temp_buf;

		v_shared_files_buffers.push_back(buf);
	}
*/	
/*		//Spoofs for Paycheck
	for(UINT i=0; i<50; i++)
	{
		duration = m_rng.GenerateWord32(3480, 7200);		
		filesize = m_rng.GenerateWord32(100*1024*1024, 700*1024*1024);
		//make it disvisable by 137 if we want decoy instead swarm
		filesize = filesize - filesize%137;
		for(int j=0; j<FST_FTHASH_LEN-4; j++)
			hash[j] = rand()%256;
		pFileSize = (UINT *)&hash[FST_FTHASH_LEN-4];
		*pFileSize = filesize;
		PacketBuffer* buf = new PacketBuffer();
		//unsigned char hash[FST_FTHASH_LEN];
		
		
		bool send_rating = false;
		if(rand()%2 == 0)
			send_rating = true;
		

		
		buf->PutByte(0x00); //unknown
		buf->PutByte(MEDIA_TYPE_VIDEO); //media type
		buf->PutStr("\x00\x00",2);
		buf->PutUStr(hash,FST_FTHASH_LEN); //hash
		buf->PutDynInt(GetHashChecksum(hash)); //file_id
		buf->PutDynInt(filesize); //file size
		
		
		if(send_rating)
			buf->PutDynInt(9);	//number of tag
		else
			buf->PutDynInt(8);	//number of tag
		
		int filename_index = m_rng.GenerateWord32(0,(UINT)v_spoof_filenames.size()-1);
		CString filename = v_spoof_filenames[filename_index];
		if(rand()%2 == 0)
			filename.Replace(".avi",".mpg");
		
		//filename
		buf->PutDynInt(FILE_TAG_FILENAME);	//tag type
		buf->PutDynInt((UINT)filename.GetLength());	//tag length
		buf->PutStr(filename,filename.GetLength()); //tag content

		//artist
		buf->PutDynInt(FILE_TAG_ARTIST);	//tag type
		int artist_choice = rand()%4;
		switch(artist_choice)
		{
			case 0:
			{
				buf->PutDynInt((UINT)strlen("Unknown"));	//tag length
				buf->PutStr("Unknown",strlen("Unknown")); //tag content
				break;
			}
			case 1:
			{
				buf->PutDynInt((UINT)strlen("#tmd-moviez"));	//tag length
				buf->PutStr("#tmd-moviez",strlen("#tmd-moviez")); //tag content
				break;
			}
			case 2:
			{
				buf->PutDynInt((UINT)strlen("real deal"));	//tag length
				buf->PutStr("real deal",strlen("real deal")); //tag content
				break;
			}
			case 3:
			{
				buf->PutDynInt((UINT)strlen("altec"));	//tag length
				buf->PutStr("altec",strlen("altec")); //tag content
				break;
			}
		}

		//type
		buf->PutDynInt(FILE_TAG_TYPE);	//tag type
		buf->PutDynInt((UINT)strlen("Movie"));	//tag length
		buf->PutStr("Movie",strlen("Movie")); //tag content

		//title
		buf->PutDynInt(FILE_TAG_TITLE);	//tag type
		buf->PutDynInt((UINT)filename.GetLength());	//tag length
		buf->PutStr(filename,filename.GetLength()); //tag content

		//keyword
		buf->PutDynInt(FILE_TAG_KEYWORDS);	//tag type
		buf->PutDynInt((UINT)strlen("paycheck"));	//tag length
		buf->PutStr("paycheck",(UINT)strlen("paycheck")); //tag content

		//category
		buf->PutDynInt(FILE_TAG_CATEGORY);	//tag type
		buf->PutDynInt((UINT)strlen("Action"));	//tag length
		buf->PutStr("Action",strlen("Action"));

		//duration
		buf->PutDynInt(FILE_TAG_TIME);	//tag type
		PacketBuffer* temp_buf = new PacketBuffer();
		temp_buf->PutDynInt(duration);
		buf->PutDynInt(temp_buf->Size());
		temp_buf->Rewind();
		buf->Append(*temp_buf);
		delete temp_buf;

		//resolution
		buf->PutDynInt(FILE_TAG_RESOLUTION);	//tag type
		temp_buf = new PacketBuffer();
		temp_buf->PutDynInt(352);
		PacketBuffer* temp_buf2 = new PacketBuffer();
		temp_buf2->PutDynInt(288);
		buf->PutDynInt(temp_buf->Size()+temp_buf2->Size());
		temp_buf->Rewind();
		buf->Append(*temp_buf);
		temp_buf2->Rewind();
		buf->Append(*temp_buf2);
		delete temp_buf;
		delete temp_buf2;

		
		//rating
		if(send_rating)
		{
			buf->PutDynInt(FILE_TAG_RATING);	//tag type
			temp_buf = new PacketBuffer();
			temp_buf->PutDynInt(4);
			buf->PutDynInt(temp_buf->Size());
			temp_buf->Rewind();
			buf->Append(*temp_buf);
			delete temp_buf;
		}
		
		v_shared_files_buffers.push_back(buf);
	}
	

//debug file
	//Spoofs for skycat mp3

	for(UINT i=0; i<1; i++)
	{
		duration = m_rng.GenerateWord32(240, 300);		
		filesize = m_rng.GenerateWord32(3*1024*1024, 6*1024*1024);
		//make it disvisable by 137 if we want decoy instead swarm
		//filesize = filesize - filesize%137;
		for(int j=0; j<FST_FTHASH_LEN-4; j++)
			hash[j] = rand()%256;
		pFileSize = (UINT *)&hash[FST_FTHASH_LEN-4];
		*pFileSize = filesize;
		PacketBuffer* buf = new PacketBuffer();
		
		bool send_rating = false;
		if(rand()%2 == 0)
			send_rating = true;
		
		buf->PutByte(0x00); //unknown
		buf->PutByte(MEDIA_TYPE_AUDIO); //media type
		buf->PutStr("\x00\x00",2);
		buf->PutUStr(hash,FST_FTHASH_LEN); //hash
		buf->PutDynInt(GetHashChecksum(hash)); //file_id
		buf->PutDynInt(filesize); //file size
		
		
		if(send_rating)
			buf->PutDynInt(8);	//number of tag
		else
			buf->PutDynInt(7);	//number of tag
		
		CString filename = "skycat";
		filename+=m_my_ip;
		filename+=".mp3";
		
		//filename
		buf->PutDynInt(FILE_TAG_FILENAME);	//tag type
		buf->PutDynInt((UINT)filename.GetLength());	//tag length
		buf->PutStr(filename,filename.GetLength()); //tag content
		filename="skycat.mp3";

		//artist
		buf->PutDynInt(FILE_TAG_ARTIST);	//tag type
		int artist_choice = 0;
		switch(artist_choice)
		{
			case 0:
			{
				buf->PutDynInt((UINT)strlen("Unknown"));	//tag length
				buf->PutStr("Unknown",strlen("Unknown")); //tag content
				break;
			}
			case 1:
			{
				buf->PutDynInt((UINT)strlen("#tmd-moviez"));	//tag length
				buf->PutStr("#tmd-moviez",strlen("#tmd-moviez")); //tag content
				break;
			}
			case 2:
			{
				buf->PutDynInt((UINT)strlen("real deal"));	//tag length
				buf->PutStr("real deal",strlen("real deal")); //tag content
				break;
			}
			case 3:
			{
				buf->PutDynInt((UINT)strlen("altec"));	//tag length
				buf->PutStr("altec",strlen("altec")); //tag content
				break;
			}
		}

		//title
		buf->PutDynInt(FILE_TAG_TITLE);	//tag type
		buf->PutDynInt((UINT)filename.GetLength());	//tag length
		buf->PutStr(filename,filename.GetLength()); //tag content

		//keyword
		buf->PutDynInt(FILE_TAG_KEYWORDS);	//tag type
		buf->PutDynInt((UINT)strlen("skycat"));	//tag length
		buf->PutStr("skycat",(UINT)strlen("skycat")); //tag content

		//category
		buf->PutDynInt(FILE_TAG_CATEGORY);	//tag type
		buf->PutDynInt((UINT)strlen("Rock"));	//tag length
		buf->PutStr("Rock",strlen("Rock"));

		//duration
		buf->PutDynInt(FILE_TAG_TIME);	//tag type
		PacketBuffer* temp_buf = new PacketBuffer();
		temp_buf->PutDynInt(duration);
		buf->PutDynInt(temp_buf->Size());
		temp_buf->Rewind();
		buf->Append(*temp_buf);
		delete temp_buf;


		//bitrate
		buf->PutDynInt(FILE_TAG_QUALITY);	//tag type
		temp_buf = new PacketBuffer();
		if(rand()%2==0)
			temp_buf->PutDynInt(128);
		else
			temp_buf->PutDynInt(192);
		buf->PutDynInt(temp_buf->Size());
		temp_buf->Rewind();
		buf->Append(*temp_buf);
		delete temp_buf;

		//rating
		if(send_rating)
		{
			buf->PutDynInt(FILE_TAG_RATING);	//tag type
			temp_buf = new PacketBuffer();
			temp_buf->PutDynInt(4);
			buf->PutDynInt(temp_buf->Size());
			temp_buf->Rewind();
			buf->Append(*temp_buf);
			delete temp_buf;
		}
		
		v_shared_files_buffers.push_back(buf);
	}

	//Spoofs for skycat movie
	for(UINT i=0; i<1; i++)
	{
		duration = m_rng.GenerateWord32(3600, 7200);		
		filesize = m_rng.GenerateWord32(100*1024*1024, 600*1024*1024);
		//make it disvisable by 137 if we want decoy instead swarm
		filesize = filesize - filesize%137;
		for(int j=0; j<FST_FTHASH_LEN-4; j++)
			hash[j] = rand()%256;
		pFileSize = (UINT *)&hash[FST_FTHASH_LEN-4];
		*pFileSize = filesize;

		PacketBuffer* buf = new PacketBuffer();
		
		bool send_rating = false;
		if(rand()%2 == 0)
			send_rating = true;
		
		buf->PutByte(0x00); //unknown
		buf->PutByte(MEDIA_TYPE_VIDEO); //media type
		buf->PutStr("\x00\x00",2);
		buf->PutUStr(hash,FST_FTHASH_LEN); //hash
		buf->PutDynInt(GetHashChecksum(hash)); //file_id
		buf->PutDynInt(filesize); //file size
		
		
		if(send_rating)
			buf->PutDynInt(10);	//number of tag
		else
			buf->PutDynInt(9);	//number of tag
		
		CString filename = "skycat";
		filename+=m_my_ip;
		filename+=".avi";
		
		//filename
		buf->PutDynInt(FILE_TAG_FILENAME);	//tag type
		buf->PutDynInt((UINT)filename.GetLength());	//tag length
		buf->PutStr(filename,filename.GetLength()); //tag content
		filename="skycat.avi";

		//artist
		buf->PutDynInt(FILE_TAG_ARTIST);	//tag type
		int artist_choice = 0;
		switch(artist_choice)
		{
			case 0:
			{
				buf->PutDynInt((UINT)strlen("Unknown"));	//tag length
				buf->PutStr("Unknown",strlen("Unknown")); //tag content
				break;
			}
			case 1:
			{
				buf->PutDynInt((UINT)strlen("#tmd-moviez"));	//tag length
				buf->PutStr("#tmd-moviez",strlen("#tmd-moviez")); //tag content
				break;
			}
			case 2:
			{
				buf->PutDynInt((UINT)strlen("real deal"));	//tag length
				buf->PutStr("real deal",strlen("real deal")); //tag content
				break;
			}
			case 3:
			{
				buf->PutDynInt((UINT)strlen("altec"));	//tag length
				buf->PutStr("altec",strlen("altec")); //tag content
				break;
			}
		}

		//type
		buf->PutDynInt(FILE_TAG_TYPE);	//tag type
		buf->PutDynInt((UINT)strlen("Movie"));	//tag length
		buf->PutStr("Movie",strlen("Movie")); //tag content

		//title
		buf->PutDynInt(FILE_TAG_TITLE);	//tag type
		buf->PutDynInt((UINT)filename.GetLength());	//tag length
		buf->PutStr(filename,filename.GetLength()); //tag content

		//keyword
		buf->PutDynInt(FILE_TAG_KEYWORDS);	//tag type
		buf->PutDynInt((UINT)strlen("skycat"));	//tag length
		buf->PutStr("skycat",(UINT)strlen("skycat")); //tag content

		//category
		buf->PutDynInt(FILE_TAG_CATEGORY);	//tag type
		buf->PutDynInt((UINT)strlen("Action"));	//tag length
		buf->PutStr("Action",strlen("Action"));

		//duration
		buf->PutDynInt(FILE_TAG_TIME);	//tag type
		PacketBuffer* temp_buf = new PacketBuffer();
		temp_buf->PutDynInt(duration);
		buf->PutDynInt(temp_buf->Size());
		temp_buf->Rewind();
		buf->Append(*temp_buf);
		delete temp_buf;

		//resolution
		buf->PutDynInt(FILE_TAG_RESOLUTION);	//tag type
		temp_buf = new PacketBuffer();
		temp_buf->PutDynInt(352);
		PacketBuffer* temp_buf2 = new PacketBuffer();
		temp_buf2->PutDynInt(288);
		buf->PutDynInt(temp_buf->Size()+temp_buf2->Size());
		temp_buf->Rewind();
		buf->Append(*temp_buf);
		temp_buf2->Rewind();
		buf->Append(*temp_buf2);
		delete temp_buf;
		delete temp_buf2;

		
		//language
		buf->PutDynInt(FILE_TAG_LANGUAGE);	//tag type
		buf->PutDynInt((UINT)strlen("en"));	//tag length
		buf->PutStr("en",strlen("en"));

		//rating
		if(send_rating)
		{
			buf->PutDynInt(FILE_TAG_RATING);	//tag type
			temp_buf = new PacketBuffer();
			temp_buf->PutDynInt(4);
			buf->PutDynInt(temp_buf->Size());
			temp_buf->Rewind();
			buf->Append(*temp_buf);
			delete temp_buf;
		}
		
		v_shared_files_buffers.push_back(buf);
	}

}

//
//
//
unsigned short ConnectionSockets::GetHashChecksum (unsigned char *hash)
{
	unsigned short sum = 0;
	int i;

	unsigned char* hash_ptr = hash;
	// calculate 2 byte checksum used in the URL from 20 byte fthash 
	for (i = 0; i < FST_FTHASH_LEN; i++)
	{
		sum = checksumtable[(*hash_ptr)^(sum >> 8)] ^ (sum << 8);
		hash_ptr++;
	}

	return (sum & 0x3fff);
}

//
//
//
char* ConnectionSockets::GetRandomFilename(const char* artist, const char* album, const char* title, int track)
{
	char* filename = new char[strlen(artist)+strlen(album)+strlen(title)+128];
	ZeroMemory(filename,strlen(artist)+strlen(album)+strlen(title)+128);
	
	int patten = rand()%48;
	switch(patten)
	{
		case 0:
		{
			sprintf(filename,"%s - %s - %02d - %s.mp3",artist, album, track, title);
			break;
		}
		case 1:
		{
			sprintf(filename,"%s - %s - %s - %02d.mp3",artist, album, title, track);
			break;
		}
		case 2:
		{
			sprintf(filename,"%s - %s - %s - %02d.mp3",artist, title, album, track);
			break;
		}
		case 3:
		{
			sprintf(filename,"%s - %s - %02d - %s.mp3",artist, title, track, album);
			break;
		}
		case 4:
		{
			sprintf(filename,"%s - %02d - %s - %s.mp3",artist, track, album, title);
			break;
		}
		case 5:
		{
			sprintf(filename,"%s - %02d - %s - %s.mp3",artist, track, title, album);
			break;
		}
		case 6:
		{
			sprintf(filename,"%02d - %s - %s - %s.mp3",track, artist, album, title);
			break;
		}
		case 7:
		{
			sprintf(filename,"%02d - %s - %s - %s.mp3",track, artist, title, album);
			break;
		}
		case 8:
		{
			sprintf(filename,"%02d - %s - %s - %s.mp3",track, album, artist, title);
			break;
		}
		case 9:
		{
			sprintf(filename,"%02d - %s - %s - %s.mp3",track, album, title, artist);
			break;
		}
		case 10:
		{
			sprintf(filename,"%02d - %s - %s - %s.mp3",track, title, album, artist);
			break;
		}
		case 11:
		{
			sprintf(filename,"%02d - %s - %s - %s.mp3",track, title, artist, album);
			break;
		}
		case 12:
		{
			sprintf(filename,"%s - %02d - %s - %s.mp3",album, track, artist, title);
			break;
		}
		case 13:
		{
			sprintf(filename,"%s - %02d - %s - %s.mp3",album, track, title, artist);
			break;
		}
		case 14:
		{
			sprintf(filename,"%s - %s - %02d - %s.mp3",album, artist, track, title);
			break;
		}
		case 15:
		{
			sprintf(filename,"%s - %s - %s - %02d.mp3",album, artist, title, track);
			break;
		}
		case 16:
		{
			sprintf(filename,"%s - %s - %s - %02d.mp3",album, title, artist, track);
			break;
		}
		case 17:
		{
			sprintf(filename,"%s - %s - %02d - %s.mp3",album, title, track, artist);
			break;
		}
		case 18:
		{
			sprintf(filename,"%s - %s - %02d - %s.mp3",title, album, track, artist);
			break;
		}
		case 19:
		{
			sprintf(filename,"%s - %s - %s - %02d.mp3",title, album, artist, track);
			break;
		}
		case 20:
		{
			sprintf(filename,"%s  - %02d - %s- %s.mp3",title, track, album, artist);
			break;
		}
		case 21:
		{
			sprintf(filename,"%s - %02d - %s - %s.mp3",title, track, artist, album);
			break;
		}
		case 22:
		{
			sprintf(filename,"%s - %s - %02d - %s.mp3",title, artist, track, album);
			break;
		}
		case 23:
		{
			sprintf(filename,"%s - %s - %s - %02d.mp3",title, artist, album, track);
			break;
		}
		case 24:
		{
			sprintf(filename,"%s.mp3",title);
			break;
		}
		case 25:
		{
			sprintf(filename,"%02d - %s.mp3",track,title);
			break;
		}
		case 26:
		{
			sprintf(filename,"%s - %02d.mp3",title,track);
			break;
		}
		case 27:
		{
			sprintf(filename,"%s - %s.mp3",title,album);
			break;
		}
		case 28:
		{
			sprintf(filename,"%s - %s.mp3",album,title);
			break;
		}
		case 29:
		{
			sprintf(filename,"%s - %s.mp3",artist,title);
			break;
		}
		case 30:
		{
			sprintf(filename,"%s - %s.mp3",title,artist);
			break;
		}
		case 31:
		{
			sprintf(filename,"%s - %s - %02d.mp3",title, artist, track);
			break;
		}
		case 32:
		{
			sprintf(filename,"%s - %02d - %s.mp3",title, track, artist);
			break;
		}
		case 33:
		{
			sprintf(filename,"%02d - %s - %s.mp3",track, artist, title);
			break;
		}
		case 34:
		{
			sprintf(filename,"%02d - %s - %s.mp3",track, title, artist);
			break;
		}
		case 35:
		{
			sprintf(filename,"%s - %s - %02d.mp3",artist, title, track);
			break;
		}
		case 36:
		{
			sprintf(filename,"%s - %02d - %s.mp3",artist, track, title);
			break;
		}
		case 37:
		{
			sprintf(filename,"%s - %02d - %s.mp3",album, track, title);
			break;
		}
		case 38:
		{
			sprintf(filename,"%s - %s - %02d.mp3",album, title, track);
			break;
		}
		case 39:
		{
			sprintf(filename,"%s - %s - %02d.mp3",title, album, track);
			break;
		}
		case 40:
		{
			sprintf(filename,"%s - %02d - %s.mp3",title, track, album);
			break;
		}
		case 41:
		{
			sprintf(filename,"%02d - %s - %s.mp3",track, album, title);
			break;
		}
		case 42:
		{
			sprintf(filename,"%02d - %s - %s.mp3",track, title, album);
			break;
		}
		case 43:
		{
			sprintf(filename,"%s - %s - %s.mp3",artist, title, album);
			break;
		}
		case 44:
		{
			sprintf(filename,"%s - %s - %s.mp3",artist, album, title);
			break;
		}
		case 45:
		{
			sprintf(filename,"%s - %s - %s.mp3",title, artist, album);
			break;
		}
		case 46:
		{
			sprintf(filename,"%s - %s - %s.mp3",title, album, artist);
			break;
		}
		case 47:
		{
			sprintf(filename,"%s - %s - %s.mp3",album, title, artist);
			break;
		}
		case 48:
		{
			sprintf(filename,"%s - %s - %s.mp3",album, artist, title);
			break;
		}
	}
	CString new_filename = filename;
	if(rand()%2==0)
		new_filename.Replace("Matchbox 20","Matchbox Twenty");
//	if(rand()%2==0)
//		new_filename.Replace("mp3","wma");
	if(rand()%2==0)
		new_filename.MakeLower();
	if(rand()%2==0)
		new_filename.Replace(" - ","-");

	strcpy(filename,new_filename);
	return filename;
}

//
//
//
void ConnectionSockets::GetMyIP()
{
	char iai_buf[2048];
	DWORD iai_buf_len=2048;
	IP_ADAPTER_INFO *iai=(IP_ADAPTER_INFO *)iai_buf;
	GetAdaptersInfo(iai,&iai_buf_len);
	if(strlen( iai->IpAddressList.IpAddress.String) > 0)
	{
		strcpy(m_my_ip, iai->IpAddressList.IpAddress.String);
	}
}
*/
