#include "StdAfx.h"
#include "filesharingmanager.h"
#include "FastTrackGiftDll.h"
#include "imagehlp.h"	// for MakeSureDirectoryPathExists
#include "ID3v1Tag.h"
#include "MP3Info.h"
#include <io.h>	// for _finddata_t stuff
#include "DatabaseHelper.h"

FileSharingManager::FileSharingManager(void)
{
	srand((unsigned)time(NULL));
	m_rng.Reseed(true,32);
	m_last_share_sent_index = 0;
	m_round=1;

	p_critical_section=NULL;
	p_thread_data=NULL;
	m_thread = NULL;
	m_rebuilding_db = false;
	m_minute_counter=0;
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
	}
}

//
//
//
bool IsAudio(const char* filename)
{
	if(strstr(filename,".mp3")!=NULL)
		return true;
	if(strstr(filename,".wma")!=NULL)
		return true;
	if(strstr(filename,".wav")!=NULL)
		return true;

	return false;
}

//
//
//
bool IsVideo(const char* filename)
{
	if(strstr(filename,".avi")!=NULL)
		return true;
	if(strstr(filename,".mov")!=NULL)
		return true;
	if(strstr(filename,".wmv")!=NULL)
		return true;
	if(strstr(filename,".mpg")!=NULL)
		return true;

	return false;
}

//
//
//
CString GetTitleFromFilename(const char* filename)
{
	CString f=filename;
	int index = f.ReverseFind('.');
	if(index > 0)
		f.Delete(index,f.GetLength()-index);
	return f;
}

//
//
//
void GetID3TagInfo(vector<FastTrackFileInfo>::iterator* file)
{
	ID3v1Tag id3v1_tag;
	CString filename = (*file)->m_path + (*file)->m_filename;

	// See if there is an ID3v1 tag
	if(id3v1_tag.ExtractTag(filename)==1)
	{
		if(id3v1_tag.ReturnTitle().length() > 0)
			(*file)->m_title=id3v1_tag.ReturnTitle().c_str();
		else
			(*file)->m_title=GetTitleFromFilename((*file)->m_filename);
		(*file)->m_artist=id3v1_tag.ReturnArtist().c_str();
		(*file)->m_album=id3v1_tag.ReturnAlbum().c_str();
		(*file)->m_description=id3v1_tag.ReturnComment().c_str();
		(*file)->m_category=id3v1_tag.ReturnGenre().c_str();
	}
	MP3Info info(filename);
	(*file)->m_bit_rate = info.GetBitRate();
	(*file)->m_duration = info.GetDuration();
}

//
//
//
void ExtractMetaData(vector<FastTrackFileInfo> &v_share_files, HWND hwnd)
{
	UINT count=0;
	int progress=0;
	UINT total_files = v_share_files.size();
	vector<FastTrackFileInfo>::iterator iter = v_share_files.begin();
	while(iter != v_share_files.end())
	{
		count++;
		progress = (float)((float)count / (float)total_files) *100;
		::PostMessage(hwnd,WM_FILE_SHARING_MANAGER_THREAD_PROGRESS,(WPARAM)progress,0);
		
		if(IsAudio(iter->m_filename))
		{
			if(strstr(iter->m_filename,".mp3")!=NULL)
			{
				GetID3TagInfo(&iter);
				//pick a random track number
				iter->m_track_number = 1+rand()%20;
			}
			iter->m_media_type = MEDIA_TYPE_AUDIO;
			if( !(iter->CalculateHash()) )
			{
				v_share_files.erase(iter);
				continue;
			}		
		}
		else if(IsVideo(iter->m_filename))
		{
			iter->SetVideoPresetOptions();
			if( !(iter->CalculateHash()) )
			{
				v_share_files.erase(iter);
				continue;
			}
			iter->m_title = GetTitleFromFilename(iter->m_filename);
		}
		else if( (strstr(iter->m_filename,".rar")!=NULL) || (strstr(iter->m_filename,".zip")!=NULL) )
		{
			iter->m_media_type = MEDIA_TYPE_SOFTWARE;
			if( !(iter->CalculateHash()) )
			{
				v_share_files.erase(iter);
				continue;
			}		
		}
		else
		{
			v_share_files.erase(iter);
			continue;
		}
		iter++;
	}
}

//
//
//
void ExploreFolder(char *folder, vector<FastTrackFileInfo> &v_share_files)
{
	char buf[4096];
	strcpy(buf,folder);
	strcat(buf,"*.*");
	
	_finddata_t data;
	long handle=(long)_findfirst(buf,&data);

	if(handle==-1)
	{
		return;
	}

	if(data.attrib & _A_SUBDIR)
	{
		if((strcmp(data.name,".")!=0) && (strcmp(data.name,"..")!=0))
		{
			strcpy(buf,folder);
			strcat(buf,data.name);
			strcat(buf,"\\");
			ExploreFolder(buf,v_share_files);
		}
	}
	else
	{
		FastTrackFileInfo file_data;
		file_data.m_path = folder;
		file_data.m_filename = data.name;
		file_data.m_filename.MakeLower();
		file_data.m_filesize = data.size;
		v_share_files.push_back(file_data);
	}

	while(_findnext(handle,&data)==0)
	{
		if(data.attrib & _A_SUBDIR)
		{
			if((strcmp(data.name,".")!=0) && (strcmp(data.name,"..")!=0))
			{
				strcpy(buf,folder);
				strcat(buf,data.name);
				strcat(buf,"\\");
				ExploreFolder(buf,v_share_files);
			}
		}
		else
		{
			FastTrackFileInfo file_data;
			file_data.m_path = folder;
			file_data.m_filename = data.name;
			file_data.m_filename.MakeLower();
			file_data.m_filesize = data.size;
			v_share_files.push_back(file_data);
		}
	}
	_findclose(handle);
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
	DatabaseHelper db;
	db.Init("63.216.246.36","onsystems","ebertsux37","fasttrack_decoy_uploads");

	// Create the events
	HANDLE events[FileSharingManagerThreadData::NumberOfEvents];
	for(i=0;i<sizeof(events)/sizeof(HANDLE);i++)
	{
		events[i]=CreateEvent(NULL,TRUE,FALSE,NULL);
		thread_data.m_events[i]=events[i];
	}
	
	SendMessage(hwnd,WM_INIT_FILE_SHARING_MANAGER_THREAD_DATA,(WPARAM)&critical_section,(LPARAM)&thread_data);	// the thread data is ready
	vector<FastTrackFileInfo>* supply_projects = thread_data.p_thread_supply_projects;


	// Start the thread
	DWORD num_events=3;
	BOOL wait_all=FALSE;
	DWORD timeout=INFINITE;
	DWORD event;	// which event fired
	vector<UploadInfo> upload_logs;

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
			
			ResetEvent(events[event]);	// reset event
			ExploreFolder("C:\\Fasttrack Shared\\",*supply_projects);
			ExploreFolder("C:\\syncher\\rcv\\Media.Fucked\\",*supply_projects);
			ExploreFolder("C:\\syncher\\rcv\\Media.Fucked 1\\",*supply_projects);
			ExploreFolder("C:\\syncher\\rcv\\Media.Fucked 2\\",*supply_projects);
			ExploreFolder("C:\\syncher\\rcv\\Media.Fucked 3\\",*supply_projects);
			ExtractMetaData(*supply_projects, hwnd);
			if(thread_data.m_abort_supply_retrieval)
			{
				//PostMessage(hwnd,WM_FILE_SHARING_MANAGER_SUPPLY_RETREIVAL_FAILED,0,0);
			}
			SendMessage(hwnd,WM_FILE_SHARING_MANAGER_UPDATE_SHARED_FILES_DONE,(WPARAM)0,(LPARAM)0);
		}
		if(event==2) //writing upload logs to db
		{
			CSingleLock singleLock(&critical_section);
			singleLock.Lock();
			if(singleLock.IsLocked())
			{
				upload_logs=thread_data.v_upload_logs;
				thread_data.v_upload_logs.clear();
				singleLock.Unlock();
			}
			if(upload_logs.size())
			{
				CString big_query="insert ignore into decoy_uploads (filename,hash_value,IP) values";
				for(i=0;i<upload_logs.size();i++)
				{
					if(i != 0) 
						big_query += ", ";
					upload_logs[i].GetValueQuery(big_query);	
				}
				upload_logs.clear();
				db.Execute(big_query);
			}
		}
	}

	// Close the events
	for(i=0;i<sizeof(events)/sizeof(HANDLE);i++)
	{
		CloseHandle(events[i]);
	}

	TRACE("Terminating FileSharingManagerThreadProc\n");
	return 0;	// exit the thread
}


void FileSharingManager::InitParent(FastTrackGiftDll* parent, HWND hwnd)
{
	m_dlg_hwnd = hwnd;
	Manager::InitParent(parent);
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
		//UINT project_id = v_share_files[m_last_share_sent_index].m_project_id;
		share = v_share_files[m_last_share_sent_index].GetShareFile();
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
	m_minute_counter++;
	if(m_minute_counter % 60 == 0)
	{
		if(!m_rebuilding_db)
		{
			CFileStatus status;
			CFile::GetStatus("Shared Files DB\\sharing_db.dat",status);
			if(status.m_mtime.GetDay() != CTime::GetCurrentTime().GetDay()) // old db, rebuild a new one
				RebuildSharedFilesList();
		}
		m_minute_counter=0;
	}
	if(p_critical_section!=NULL)
	{
		//signal to write upload logs to DB
		CSingleLock singleLock(p_critical_section);
		singleLock.Lock();
		if(singleLock.IsLocked())
		{
			CSingleLock upload_singleLock(&m_upload_log_critical_section);
			upload_singleLock.Lock();
			if(upload_singleLock.IsLocked())
			{
				p_thread_data->v_upload_logs=v_upload_logs;
				v_upload_logs.clear();
				SetEvent(p_thread_data->m_events[2]);
				upload_singleLock.Unlock();
			}
			singleLock.Unlock();
		}
	}
}


//
//
//
void FileSharingManager::InitThreadData(WPARAM wparam,LPARAM lparam)
{
	p_critical_section=(CCriticalSection *)wparam;
	p_thread_data =(FileSharingManagerThreadData *)lparam;
	p_thread_data->p_thread_supply_projects = &v_temp_share_files;

	if(!ReadSharingDB())
		RebuildSharedFilesList();
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
void FileSharingManager::ResetSpoofsCount()
{
	m_last_share_sent_index = 0;
	m_round = 1;
}


//
//
//
void FileSharingManager::ConstructHashTable()
{
	hs_hashes.clear();
	for(UINT i=0;i<v_share_files.size();i++)
	{
		FileSizeAndHash hash;
		hash.SetHashAndSize(v_share_files[i].m_hash, v_share_files[i].m_filesize);
		hash.m_filename = v_share_files[i].m_path + v_share_files[i].m_filename;
		
		pair< hash_set<FileSizeAndHash>::iterator, bool > pr;
		pr = hs_hashes.insert(hash);
		if(pr.second==false)
		{
			TRACE("Hash table insertion failed!\n");
		}
	}
}

//
//
//
FileSizeAndHash FileSharingManager::GetFileSize(byte* hash)
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
			key = *iter;
		singleLock.Unlock();
	}
	return key;
}

//
//
//
void FileSharingManager::RebuildSharedFilesList(void)
{
	::SendMessage(m_dlg_hwnd,WM_FILE_SHARING_MANAGER_REBUILD_SHARING_DB_STARTED,0,0);
	CSingleLock singleLock(&m_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		if(p_critical_section!=NULL)
		{
			m_rebuilding_db = true;
			v_share_files.clear();
			v_temp_share_files.clear();
			SetEvent(p_thread_data->m_events[1]);
		}
		//ExtractMetaData(v_share_files);
		//ConstructHashTable();
		//SaveSharingDB();
		singleLock.Unlock();
	}
	//::SendMessage(m_dlg_hwnd,WM_FILE_SHARING_MANAGER_REBUILD_SHARING_DB_ENDED,0,0);
}


//
//
//
void FileSharingManager::SaveSharingDB()
{
	UINT i;
	CFile file;
	CString filename="Shared Files DB\\sharing_db.dat";
	MakeSureDirectoryPathExists(filename);
	if(file.Open(filename, CFile::modeCreate|CFile::modeWrite|CFile::typeBinary)!=0)
	{
		for(i=0; i<v_share_files.size(); i++)
		{
			int buf_len = v_share_files[i].GetBufferLength();
			char * buf = new char[buf_len];
			v_share_files[i].WriteToBuffer(buf);
			file.Write(buf, buf_len);
			delete [] buf;
		}
		file.Close();
	}
}

//
//
//
bool FileSharingManager::ReadSharingDB()
{
	CFileStatus status;
	CFile::GetStatus("Shared Files DB\\sharing_db.dat",status);
	if(status.m_mtime.GetDay() != CTime::GetCurrentTime().GetDay()) // old db, rebuild a new one
		return false;

	CSingleLock singleLock(&m_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		CFile file;
		if(file.Open("Shared Files DB\\sharing_db.dat",CFile::modeRead|CFile::typeBinary|CFile::shareDenyWrite)!=0)
		{
			v_share_files.clear();
			unsigned char * supply_data =  new unsigned char[(UINT)file.GetLength()];
			file.Read(supply_data, (UINT)file.GetLength());
			unsigned char* file_ptr = supply_data;
			UINT file_ptr_len = 0;
			while(file_ptr_len < file.GetLength())
			{
				FastTrackFileInfo info;
				UINT pos = info.ReadFromBuffer((char*)file_ptr);
				file_ptr += pos;
				file_ptr_len += pos;

				v_share_files.push_back(info);
			}
			delete [] supply_data;
			file.Close();
		}
		ConstructHashTable();
		singleLock.Unlock();
	}
	return true;
}

//
//
//
void FileSharingManager::FileSharingManagerUpdateSharedFilesDone()
{
	CSingleLock singleLock(&m_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		m_rebuilding_db = false;
		v_share_files.clear();
		v_share_files = v_temp_share_files;
		v_temp_share_files.clear();
		ConstructHashTable();
		SaveSharingDB();
		singleLock.Unlock();
	}
	::SendMessage(m_dlg_hwnd,WM_FILE_SHARING_MANAGER_REBUILD_SHARING_DB_ENDED,0,0);
	p_parent->ReConnectToAllSupernodes();
}

//
//
//
void FileSharingManager::DisableSharing()
{
	CSingleLock singleLock(&m_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		if(p_critical_section!=NULL)
		{
			v_share_files.clear();
		}
		singleLock.Unlock();
	}
}

//
//
//
void FileSharingManager::StoreUploadLog(UploadInfo& info)
{
	CSingleLock singleLock(&m_upload_log_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		v_upload_logs.push_back(info);
		singleLock.Unlock();
	}

}
