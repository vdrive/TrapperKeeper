// ConnectionSockets.cpp

#include "stdafx.h"
#include "ConnectionSockets.h"
#include "VendorCount.h"
#include "ConnectionModuleMessageWnd.h"
#include "ProjectStatus.h"
#include "iphlpapi.h"
#include "FileSharingManager.h"

//
//
//
ConnectionSockets::ConnectionSockets()
{
	m_one_hour_timer=0;
	p_file_sharing_manager = NULL;
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
	ReadInUserNames();
	m_dlg_hwnd=NULL;
	int num_reserved_events=ReturnNumberOfReservedEvents();
	int num_socket_events=ReturnNumberOfSocketEvents();

	// Init the parent pointers and message window handles
	for(int i=0;i<num_socket_events;i++)
	{
		m_sockets[i].InitParent(this);
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
	
	m_rng.Reseed(true,32);

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
	// close all sockets
	int num_socket_events=ReturnNumberOfSocketEvents();
	for(i=0;i<num_socket_events;i++)
	{
		m_sockets[i].Close();
	}
}

//
//
//
DWORD ConnectionSockets::ReturnNumberOfEvents()
{
	return NumberOfEvents;
}

//
//
//
int ConnectionSockets::ReturnNumberOfReservedEvents()
{
	return NumberOfReservedEvents;
}

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
void ConnectionSockets::ReportStatus(ConnectionModuleStatusData& status_data)
{
	m_one_hour_timer++;
	int i;

	int num_socket_events=ReturnNumberOfSocketEvents();

	// First tell all of the sockets that the timer has fired
	for(i=0;i<num_socket_events;i++)
	{
		m_sockets[i].TimerHasFired();
	}

	// See how many sockets are idle (and therefore need ips).  Any non-idle sockets, get their host connection status
	for(i=0;i<num_socket_events;i++)
	{
		UINT ip=m_sockets[i].ReturnHostIP();
		if(ip!=0)
			m_status_data.v_connected_host_ips.push_back(ip);

		if(m_sockets[i].IsSocket()==false)
		{
			m_status_data.m_idle_socket_count++;
		}
		else
		{
			if(m_sockets[i].IsConnected())
			{
				m_status_data.m_connected_socket_count++;
			}
			else	// connecting
			{
				m_status_data.m_connecting_socket_count++;
			}

			// Get the ip of the connection and the connection status of the socket
			//if(m_sockets[i].IsConnected())
			//	m_status_data.v_connected_host_ips.push_back(m_sockets[i].ReturnHostIP());

			//m_status_data.m_cached_query_hit_count+=m_sockets[i].ReturnCachedQueryHitCount();
		}
	}

	//ConnectionModuleStatusData ret=m_status_data;
	status_data=m_status_data;
	
	m_status_data.ClearCounters();

	if( (m_one_hour_timer%3600) == 0 )// reload username from file every hour
		ReadInUserNames();
	
	//return ret;
}

//
//
//
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
				m_sockets[j].Connect(hosts[i]);
				break;
			}
		}
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
void ConnectionSockets::PerformProjectSupplyQuery(ProjectSupplyQuery &psq)
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
*/
}

//
//
//

void ConnectionSockets::UpdateKeywords(vector<ProjectKeywords> &keywords)
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
	*/
}

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
vector<VendorCount> *ConnectionSockets::ReturnVendorCounts()
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
void ConnectionSockets::GetRandomUserName(CString& username)
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
