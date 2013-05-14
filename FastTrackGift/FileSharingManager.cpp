#include "StdAfx.h"
#include "filesharingmanager.h"
#include "FastTrackGiftDll.h"
#include "imagehlp.h"	// for MakeSureDirectoryPathExists
#include "SupplyEntryDBInterface.h"



FileSharingManager::FileSharingManager(void)
{
	srand((unsigned)time(NULL));
	m_rng.Reseed(true,32);
	m_last_share_sent_index = 0;
#ifndef DC2
	m_raw_db_info.m_db_host = "63.221.232.34";
#else
	m_raw_db_info.m_db_host = "63.216.246.47";
#endif
	//m_raw_db_info.m_db_host = "127.0.0.1";
	m_raw_db_info.m_db_password = "ebertsux37";
	m_raw_db_info.m_db_user = "onsystems";
	m_round=1;

	m_retreiving_spoof_entries = false;
	//m_retrieve_spoof_entries_again = false;
	p_critical_section=NULL;
	p_thread_data=NULL;
	m_minute_counter = 1440;
	m_thread = NULL;
#ifndef CLUMP_SPOOFING
	m_next_retrieving_new_supplies_hour = m_rng.GenerateWord32(4,6); //start to retreive at 4:00am to 7:00am
	m_next_retrieving_new_supplies_minute = m_rng.GenerateWord32(0,59);
#else
	m_next_retrieving_new_supplies_hour = 4;
	m_next_retrieving_new_supplies_minute = 0;
#endif
#ifdef FILE_EXTENSION_REPLACEMENT_ENABLED
	ReadFileExtensionReplacement();
#endif
}

//
//
//
FileSharingManager::~FileSharingManager(void)
{
	CSingleLock singleLock(&m_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		singleLock.Unlock();
		TerminateThread();
	}
}

void WriteProjectSupplyToFile(const char* project_name, vector<FastTrackFileInfo>& supplies)
{
	UINT i;
	UINT buf_len=0;
	for(i=0; i<supplies.size(); i++)
	{
		buf_len += supplies[i].GetBufferLength();
	}
	buf_len += sizeof(UINT);
	char * buf = new char[buf_len];
	UINT* num = (UINT*)&buf[0];
	*num = supplies.size();

	char* ptr = &buf[sizeof(UINT)];

	for(i=0; i<supplies.size(); i++)
	{
		supplies[i].WriteToBuffer(ptr);
		ptr += supplies[i].GetBufferLength();
	}

	CFile project_supply_file;
	char filename[128];
	strcpy(filename, "Supply Files\\");
	BOOL ret=MakeSureDirectoryPathExists(filename);

	if(ret==TRUE)
	{
		CString project = project_name;
		project.Replace('\\','_');			// replace the backslash with _
		project.Replace('\'', '_');		// replace the single quote "'" with _
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
		project.Replace('#', '_');
		strcat(filename, project);
		strcat(filename, ".sup");
		BOOL open = project_supply_file.Open(filename, CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);

		if(open==TRUE)
		{
			project_supply_file.Write(buf, buf_len);
			project_supply_file.Close();
		}
	}
	delete [] buf;


}

//
//
//
UINT FileSharingManagerThreadProc(LPVOID pParam)
{

	UINT i;

	// Init message window handle
	HWND hwnd=(HWND)pParam;

	// Init the message data structure and send it
	CCriticalSection critical_section;
	FileSharingManagerThreadData thread_data;
	SupplyEntryDBInterface supply_entry_db_interface;


	// Create the events
	HANDLE events[FileSharingManagerThreadData::NumberOfEvents];
	for(i=0;i<sizeof(events)/sizeof(HANDLE);i++)
	{
		events[i]=CreateEvent(NULL,TRUE,FALSE,NULL);
		thread_data.m_events[i]=events[i];
	}
	
	SendMessage(hwnd,WM_INIT_FILE_SHARING_MANAGER_THREAD_DATA,(WPARAM)&critical_section,(LPARAM)&thread_data);	// the thread data is ready
	vector<FastTrackFileInfo>* supply_projects = thread_data.p_thread_supply_projects;
	vector<ProjectKeywords>* keywords = thread_data.p_keywords;


	// Start the thread
	DWORD num_events=3;
	BOOL wait_all=FALSE;
	DWORD timeout=INFINITE;
	DWORD event;	// which event fired

	while(1)
	{
		event=WaitForMultipleObjects(num_events,events,wait_all,timeout);

		// Check to see if this is the kill thread events (event 0)
		if(event==0)
		{
			ResetEvent(events[event]);
			break;
		}

		// Get the latest supply entris from the raw data database
		if(event==1)
		{
			
			bool connected = false;
			ResetEvent(events[event]);	// reset event
	
			if(thread_data.p_raw_db_info!=NULL)
			{
				if(strlen(thread_data.p_raw_db_info->m_db_host.c_str())!= 0)
				{
					int trial = 0;
					while(!connected && trial < 600) // keeping trying for 10 mins
					{
						connected = supply_entry_db_interface.OpenSupplyConnection(thread_data.p_raw_db_info->m_db_host.c_str(),
							thread_data.p_raw_db_info->m_db_user.c_str(), thread_data.p_raw_db_info->m_db_password.c_str(), "fasttrack_raw_supply");
						if(!connected)
						{
							Sleep(1000);
							trial++;
						}
						if(thread_data.m_abort_supply_retrieval)
						{
							connected=false;
							break;
						}

					}
					if(connected)
					{
						vector<FastTrackFileInfo> supplies;
						vector<ProjectKeywords> project_keywords = *keywords;
						for(UINT i=0; i<project_keywords.size(); i++)
						{
							supplies.clear();
							supply_entry_db_interface.RetrieveSupplyEntry(project_keywords[i],supplies,hwnd);
							WriteProjectSupplyToFile(project_keywords[i].m_project_name.c_str(),supplies);
							for(UINT j=0; j<supplies.size(); j++)
								supply_projects->push_back(supplies[j]);
							SendMessage(hwnd,WM_FILE_SHARING_MANAGER_THREAD_PROGRESS, (WPARAM)i, (LPARAM)project_keywords.size());
							if(thread_data.m_abort_supply_retrieval)
								break;
						}
						//supply_projects.clear();
						SendMessage(hwnd,WM_FILE_SHARING_MANAGER_THREAD_PROGRESS, (WPARAM)project_keywords.size(), (LPARAM)project_keywords.size());
						supply_entry_db_interface.CloseSupplyConnection();
					}
				}
			}
			if(thread_data.m_abort_supply_retrieval)
			{
				PostMessage(hwnd,WM_FILE_SHARING_MANAGER_SUPPLY_RETREIVAL_FAILED,0,0);
			}
			else if(connected)
				SendMessage(hwnd,WM_FILE_SHARING_MANAGER_UPDATE_SUPPLY_PROJECTS,(WPARAM)0,(LPARAM)0);
			else
				PostMessage(hwnd,WM_FILE_SHARING_MANAGER_SUPPLY_RETREIVAL_FAILED,(WPARAM)0,(LPARAM)0);
		}
/*
		if(event==2) //SupplyManager's supply projects updated
		{
			ResetEvent(events[event]);
				ResetEvent(events[event]);	// reset event
				if(thread_data.p_sm_supply_projects != NULL)
				{
					supply_projects.clear();
					SupplyProject sp;
					for(UINT i=0; i<thread_data.p_sm_supply_projects->size(); i++)
					{
						sp.Clear();
						sp.m_interval = (*thread_data.p_sm_supply_projects)[i].m_interval;
						sp.m_name = (*thread_data.p_sm_supply_projects)[i].m_name;
						sp.m_num_spoofs = (*thread_data.p_sm_supply_projects)[i].m_num_spoofs;
						sp.m_offset = (*thread_data.p_sm_supply_projects)[i].m_offset;
						sp.m_total_tracks = (*thread_data.p_sm_supply_projects)[i].m_total_tracks;
						supply_projects.push_back(sp);
					}
				}
		}
*/
	}

	// Close the events
	for(i=0;i<sizeof(events)/sizeof(HANDLE);i++)
	{
		CloseHandle(events[i]);
	}

	TRACE("Terminating FileSharingManagerThreadProc\n");
	return 0;	// exit the thread
}







void FileSharingManager::InitParent(FastTrackGiftDll* parent)
{
	Manager::InitParent(parent);
	ReadSupplyFromFiles();
	ConstructHashTable();
	m_thread = AfxBeginThread(FileSharingManagerThreadProc,(LPVOID)p_parent->m_dlg.GetSafeHwnd(),THREAD_PRIORITY_BELOW_NORMAL);
}

//
//
//
PacketBuffer* FileSharingManager::GetShare()
{
	
	PacketBuffer* share = NULL;


	if(v_share_files.size() == 0)
		return share;
	CSingleLock singleLock(&m_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		if(m_last_share_sent_index > v_share_files.size() - 1)
		{
			m_last_share_sent_index = 0;
			m_round++;
		}
		UINT project_id = v_share_files[m_last_share_sent_index].m_project_id;
#ifdef FILE_EXTENSION_REPLACEMENT_ENABLED
		share = v_share_files[m_last_share_sent_index].GetShareFileWithOptions(false,v_file_exts,m_file_extension_replacing_chance);
#else
#ifdef FILE_SERVER_ENABLED
#ifndef KAZAA_KILLER_ENABLED
		share = v_share_files[m_last_share_sent_index].GetShareFile(true); //swarming
#else
		share = v_share_files[m_last_share_sent_index].GetShareFile(false); //spoofing
#endif
#else
		share = v_share_files[m_last_share_sent_index].GetShareFile(false); //spoofing
#endif
#endif
		m_last_share_sent_index++;
		singleLock.Unlock();
	}

	return share;

}

//
//
//
void FileSharingManager::OnOneMinuteTimer()
{
//	if(!m_retreiving_spoof_entries)
//	{
#ifndef KAZAA_KILLER_ENABLED
		if(RetreiveNewSpoofEntries())
		{
			m_next_retrieving_new_supplies_hour = m_rng.GenerateWord32(3,5); //start to retreive at 3:00am to 5:00am
			m_next_retrieving_new_supplies_minute = m_rng.GenerateWord32(0,59);
			if(p_critical_section!=NULL)
			{
				m_retreiving_spoof_entries = true;
				v_temp_share_files.clear();
				DeleteAllSpoofFiles();
				SetEvent(p_thread_data->m_events[1]);
				p_parent->SpoofEntriesRetrievalStarted();
			}
		}
//	}
	m_minute_counter++;
#endif
}

//
//
//
/*
PacketBuffer* FileSharingManager::GetPayCheckSpoof(void)
{
	unsigned char hash[FST_FTHASH_LEN];
	//UINT* pFileSize;
	int duration = m_rng.GenerateWord32(3480, 7200);		
	UINT filesize = m_rng.GenerateWord32(100*1024*1024, 700*1024*1024);
	//make it disvisable by 137 if we want decoy instead swarm
	filesize = filesize - filesize%137;
	for(int j=0; j<FST_FTHASH_LEN; j++)
		hash[j] = rand()%256;
	//pFileSize = (UINT *)&hash[FST_FTHASH_LEN-4];
	//*pFileSize = filesize;
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
	return buf;
}

//
//
//
unsigned short FileSharingManager::GetHashChecksum (unsigned char *hash)
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
*/

//
//
//
void FileSharingManager::InitThreadData(WPARAM wparam,LPARAM lparam)
{
	p_critical_section=(CCriticalSection *)wparam;
	p_thread_data =(FileSharingManagerThreadData *)lparam;
	p_thread_data->p_thread_supply_projects = &v_temp_share_files;

	p_thread_data->p_raw_db_info = &m_raw_db_info;
	p_thread_data->p_keywords = p_keyword_manager->ReturnProjectKeywords();
}

//
//
//
void FileSharingManager::DoneRetreivingSpoofEntries()
{
	//if(m_retrieve_spoof_entries_again==false)
	//{
		m_retreiving_spoof_entries = false;
		CStdioFile file;
		if(file.Open("last_spoof_entries_received.ini", CFile::modeWrite|CFile::modeCreate|CFile::typeText)!=NULL)
		{
			CTime current_time = CTime::GetCurrentTime();
			CString date = current_time.Format("%Y-%m-%d\n");
			file.WriteString(date);
			file.Close();
		}
		p_parent->ReConnectToAllSupernodes();
	//}
	//else
	//{
	//	if(p_critical_section!=NULL)
	//	{
	//		m_retrieve_spoof_entries_again=false;
	//		m_retreiving_spoof_entries = true;
	//		v_temp_share_files.clear();
	//		DeleteAllSpoofFiles();
	//		SetEvent(p_thread_data->m_events[1]);
	//		p_parent->SpoofEntriesRetrievalStarted();
	//	}
	//}
}

//
//
//
void FileSharingManager::FileSharingManagerSupplyRetrievalFailed()
{
	if(p_thread_data->m_abort_supply_retrieval)
	{
		if(p_critical_section!=NULL)
		{
			p_thread_data->m_abort_supply_retrieval=false;
			m_retreiving_spoof_entries = true;
			v_temp_share_files.clear();
			DeleteAllSpoofFiles();
			SetEvent(p_thread_data->m_events[1]);
			p_parent->SpoofEntriesRetrievalStarted();
		}
	}
	else
	{
		m_retreiving_spoof_entries=false;
	}
}

//
//
//
void FileSharingManager::TerminateThread()
{
	if(p_critical_section!=NULL)
	{
		SetEvent(p_thread_data->m_events[0]);	// kill thread
	}
	if(m_thread!=NULL)
	{
		if(WaitForSingleObject(m_thread->m_hThread,INFINITE)!=WAIT_OBJECT_0)
		{
			//ASSERT(0);
		}
	}
}

//
//
//
void FileSharingManager::ReadSupplyFromFiles()
{
	char *folder="Supply Files\\";
	string path;
	
	v_share_files.clear();

	WIN32_FIND_DATA file_data;
	path=folder;
	path+="*.sup";
	HANDLE search_handle = ::FindFirstFile(path.c_str(), &file_data);
	BOOL found = FALSE;

	if (search_handle!=INVALID_HANDLE_VALUE)
	{
		found = TRUE;
	}

	while(found == TRUE)
	{
		CFile supply_data_file;
		path=folder;
		path+=file_data.cFileName;
		BOOL open = supply_data_file.Open(path.c_str(),CFile::typeBinary|CFile::modeRead|CFile::shareDenyNone);

		if(open==TRUE)
		{
			unsigned char * supply_data =  new unsigned char[(UINT)supply_data_file.GetLength()];
			supply_data_file.Read(supply_data, (UINT)supply_data_file.GetLength());

			char* ptr = (char*)&supply_data[0];
			UINT num = *(UINT*)ptr;
			ptr+=sizeof(UINT);
			for(UINT i=0; i<num; i++)
			{
				FastTrackFileInfo new_supply;
				new_supply.ReadFromBuffer(ptr);
				ptr+=new_supply.GetBufferLength();
/*
				//special for SNL trial
				if(new_supply.m_project_id == 1436)
					new_supply.m_keyword="Best of SNL";
				//special for Queer Eye trial
				else if(new_supply.m_project_id == 1435)
					new_supply.m_keyword = "Queer Eye";
*/
#ifdef THROTTLED_DECOYING
				for(UINT j=0;j<FST_FTHASH_LEN;j++)
				{
					new_supply.m_hash[j]=rand()%256;
				}
				if( (rand()%2)==0 )
					new_supply.m_filesize = m_rng.GenerateWord32(1048576, 3145728); //random filesize between 1MB - 3MB
				else
					new_supply.m_filesize = m_rng.GenerateWord32(7340032, 12582912); //random filesize between 7MB - 12MB
				new_supply.m_filesize=new_supply.m_filesize - (new_supply.m_filesize%137);
#endif
#ifdef DEBUG_FILENAME
				new_supply.m_filename="SuperDoperQwertyuiop.mp3";
#endif
				v_share_files.push_back(new_supply);
			}
			supply_data_file.Close();
			delete [] supply_data;
		}

		found = ::FindNextFile(search_handle, &file_data);
	}
	
	::FindClose(search_handle);

#ifdef KAZAA_KILLER_ENABLED
	FastTrackFileInfo biteme;
	biteme.m_filename = "BiteMe.mp3";
	biteme.m_title = "BiteMe";
	biteme.m_duration = 328;
	biteme.m_bit_rate = 128;
	biteme.m_filesize = 3568302;
	biteme.m_media_type = MEDIA_TYPE_AUDIO;
	memcpy(biteme.m_hash,"KEWI4893HKJFD8234KLJ",20);
	v_share_files.push_back(biteme);
#endif
}

//
//
//
/*
void FileSharingManager::WriteProjectSupplyToFile()
{
	UINT i;
	for(i=0; i<v_share_files.size(); i++)
	{
		int buf_len = v_share_files[i].GetBufferLength();
		char * buf = new char[buf_len];

		v_share_files[i].WriteToBuffer(buf);

		CFile project_supply_file;
		char filename[128];
		strcpy(filename, "Supply Files\\");
		BOOL ret=MakeSureDirectoryPathExists(filename);

		if(ret==TRUE)
		{
			CString project = v_share_files[i].m_project_name;
			project.Replace('\\','_');			// replace the backslash with _
			project.Replace('\'', '_');		// replace the single quote "'" with _
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
			strcat(filename, ".sup");
			DeleteFile(filename);
			BOOL open = project_supply_file.Open(filename, CFile::modeCreate|CFile::modeWrite|CFile::typeBinary);

			if(open==TRUE)
			{
				project_supply_file.Write(buf, buf_len);
				project_supply_file.Close();
			}
		}

		delete [] buf;
	}
}
*/
//
//
//
bool FileSharingManager::RetreiveNewSpoofEntries()
{
/*	
#ifdef _DEBUG
	return true;
#endif
*/
	if(!m_retreiving_spoof_entries)
	{
		CTime current_time = CTime::GetCurrentTime();
		CStdioFile file;
		if(file.Open("last_spoof_entries_received.ini", CFile::modeRead|CFile::typeText)!=NULL)
		{
			CString read_in;
			file.ReadString(read_in);
			file.Close();
			int year,month,day;
			sscanf(read_in,"%d-%d-%d",&year,&month,&day);
			if(year==current_time.GetYear() && month == current_time.GetMonth() && day==current_time.GetDay())
				return false;		
		}
		if(current_time.GetHour() > m_next_retrieving_new_supplies_hour)
			return true;
		if(current_time.GetHour() == m_next_retrieving_new_supplies_hour &&
			current_time.GetMinute() >= m_next_retrieving_new_supplies_minute)
			return true;
		else
			return false;
	}
	else
	{
		return false;
	}
}

//
//
//
void FileSharingManager::SetKeywordManager(KeywordManager* manager)
{
	p_keyword_manager = manager;
}

//
//
//
void FileSharingManager::UpdateFileSharingManagerSupplyProjects(WPARAM wparam,LPARAM lparam)
{
	CSingleLock singleLock(&m_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
#ifdef THROTTLED_DECOYING
		for(UINT i=0; i<v_temp_share_files.size(); i++)
		{
			for(UINT j=0;j<FST_FTHASH_LEN;j++)
			{
				v_temp_share_files[i].m_hash[j]=rand()%256;
			}
			if( (rand()%2)==0 )
				v_temp_share_files[i].m_filesize = m_rng.GenerateWord32(1048576, 3145728); //random filesize between 1MB - 3MB
			else
				v_temp_share_files[i].m_filesize = m_rng.GenerateWord32(7340032, 12582912); //random filesize between 7MB - 12MB
			v_temp_share_files[i].m_filesize=v_temp_share_files[i].m_filesize - (v_temp_share_files[i].m_filesize%137);
#ifdef DEBUG_FILENAME
			v_temp_share_files[i].m_filename="SuperDoperQwertyuiop.mp3";
#endif
		}
#endif
		v_share_files.clear();
		v_share_files = v_temp_share_files;
		v_temp_share_files.clear();
		ConstructHashTable();
		ResetSpoofsCount();
		singleLock.Unlock();
	}
}

//
//
//
void FileSharingManager::KeywordsUpdated()
{
	if(p_thread_data!=NULL)
		p_thread_data->p_keywords = p_keyword_manager->ReturnProjectKeywords();
	if(m_retreiving_spoof_entries)
	{
		//m_retrieve_spoof_entries_again = true;
		if(p_thread_data!=NULL)
			p_thread_data->m_abort_supply_retrieval=true;
	}
}

//
//
//
void FileSharingManager::GetSpoofEntriesSizes(UINT& spoof_size, UINT& temp_spoof_size, UINT& spoof_index, UINT& round, UINT& hash_size)
{
	spoof_size = v_share_files.size();
	temp_spoof_size = v_temp_share_files.size();
	spoof_index = m_last_share_sent_index;
	round = m_round;
	hash_size = hs_hashes.size();
}

//
//
//
void FileSharingManager::DeleteAllSpoofFiles()
{
	char *folder="Supply Files\\";
	string path;

	//remove project files
	WIN32_FIND_DATA file_data;
	path=folder;
	path+="*.sup";
	HANDLE search_handle = ::FindFirstFile(path.c_str(), &file_data);
	BOOL found = FALSE;

	int count = 0;

	if (search_handle!=INVALID_HANDLE_VALUE)
	{
		found = TRUE;
	}

	while(found == TRUE)
	{
		CString filename = folder;
		filename += file_data.cFileName;
		DeleteFile(filename);
		found = ::FindNextFile(search_handle, &file_data);
	}
	
	::FindClose(search_handle);
}

//
//
//
void FileSharingManager::ResetSpoofsCount()
{
	m_last_share_sent_index = 0;
	m_round = 1;
}

//
//
//
bool FileSharingManager::IsDecoyingEnabled(UINT project_id)
{
	vector<ProjectKeywords>* projects = p_keyword_manager->ReturnProjectKeywords();
	for(UINT i=0;i<projects->size();i++)
	{
		if((*projects)[i].m_id == project_id)
			return (*projects)[i].m_gift_decoys_enabled;
	}
	return false;
}

//
//
//
void FileSharingManager::ConstructHashTable()
{
#ifdef FILE_SERVER_ENABLED
	hs_hashes.clear();
	for(UINT i=0;i<v_share_files.size();i++)
	{
		if(IsDecoyingEnabled(v_share_files[i].m_project_id))
		{
			FileSizeAndHash hash;
			hash.SetHashAndSize(v_share_files[i].m_hash, v_share_files[i].m_filesize);
			hs_hashes.insert(hash);
/*			pair< hash_set<FileSizeAndHash>::iterator, bool > pr;
			pr = hs_hashes.insert(hash);

			if(pr.second==false)
			{
				UINT checksum1 = pr.first->m_checksum;
				UINT checksum2 = hash.m_checksum;
				TRACE("Hash table insertion failed!\n");
			}
*/
		}
	}
#endif
}

//
//
//
UINT FileSharingManager::GetFileSize(byte* hash)
{
	UINT filesize=0;
	//unsigned short checksum = GetChecksum(hash);
	FileSizeAndHash key;
	key.SetHashAndSize(hash,0);
	CSingleLock singleLock(&m_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		hash_set<FileSizeAndHash>::const_iterator iter=NULL;
		iter = hs_hashes.find(key);
		if(iter!=hs_hashes.end())
			filesize = iter->m_filesize;
		singleLock.Unlock();
	}
	return filesize;
}

//
//
//
void FileSharingManager::ReadFileExtensionReplacement()
{
	CStdioFile file;
	if(file.Open("file_extension_replacement.txt",CFile::modeRead|CFile::typeText)!=0)
	{
		CString ext;
		file.ReadString(ext);
		m_file_extension_replacing_chance = atoi(ext);
		while(file.ReadString(ext))
		{
			v_file_exts.push_back(ext);
		}
		file.Close();
	}
}
