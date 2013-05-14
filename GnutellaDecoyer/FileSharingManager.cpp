#include "StdAfx.h"
#include "filesharingmanager.h"
#include "GnutellaDecoyerDll.h"
#include "imagehlp.h"	// for MakeSureDirectoryPathExists
#include "ID3v1Tag.h"
#include "MP3Info.h"
#include <io.h>	// for _finddata_t stuff
#include "DatabaseHelper.h"

FileSharingManager::FileSharingManager(void)
{
	srand((unsigned)time(NULL));
	m_rng.Reseed(true,32);

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
// Scrambles the bits of a 32 bit value, except for the MSB so that the case is unchanged
//
unsigned int BitScramble(unsigned int val)
{
	unsigned int ret=0;

	ret|=((val>> 15 )&0x0001)<<0;
	ret|=((val>> 29 )&0x0001)<<1;
	ret|=((val>> 6  )&0x0001)<<2;
	ret|=((val>> 11 )&0x0001)<<3;
	ret|=((val>> 7  )&0x0001)<<4;
	ret|=((val>> 21 )&0x0001)<<5;
	ret|=((val>> 23 )&0x0001)<<6;
	ret|=((val>> 2  )&0x0001)<<7;
	ret|=((val>> 30 )&0x0001)<<8;
	ret|=((val>> 14 )&0x0001)<<9;
	ret|=((val>> 26 )&0x0001)<<10;
	ret|=((val>> 18 )&0x0001)<<11;
	ret|=((val>> 0  )&0x0001)<<12;
	ret|=((val>> 27 )&0x0001)<<13;
	ret|=((val>> 12 )&0x0001)<<14;
	ret|=((val>> 10 )&0x0001)<<15;
	ret|=((val>> 22 )&0x0001)<<16;
	ret|=((val>> 28 )&0x0001)<<17;
	ret|=((val>> 1  )&0x0001)<<18;
	ret|=((val>> 25 )&0x0001)<<19;
	ret|=((val>> 5  )&0x0001)<<20;
	ret|=((val>> 9  )&0x0001)<<21;
	ret|=((val>> 20 )&0x0001)<<22;
	ret|=((val>> 13 )&0x0001)<<23;
	ret|=((val>> 8  )&0x0001)<<24;
	ret|=((val>> 19 )&0x0001)<<25;
	ret|=((val>> 17 )&0x0001)<<26;
	ret|=((val>> 4  )&0x0001)<<27;
	ret|=((val>> 24 )&0x0001)<<28;
	ret|=((val>> 16 )&0x0001)<<29;
	ret|=((val>> 3  )&0x0001)<<30;

	return ret;
}

//
//
//
void GetID3TagInfo(vector<QueryHitResult>::iterator* file)
{
	ID3v1Tag id3v1_tag;
	CString filename = (*file)->m_path.c_str();
	filename += (*file)->m_file_name.c_str();

	// See if there is an ID3v1 tag
	if(id3v1_tag.ExtractTag(filename)==1)
	{
		if(id3v1_tag.ReturnTitle().length() > 0)
			(*file)->m_title=id3v1_tag.ReturnTitle().c_str();
		//else
		//	(*file)->m_title=GetTitleFromFilename((*file)->m_filename);
		(*file)->m_artist=id3v1_tag.ReturnArtist().c_str();
		(*file)->m_album=id3v1_tag.ReturnAlbum().c_str();
		(*file)->m_comment=id3v1_tag.ReturnComment().c_str();
	}
	MP3Info info(filename);
	(*file)->m_bitrate = info.GetBitRate();
	(*file)->m_duration = info.GetDuration();
}

//
//
//
void ExtractMetaData(vector<QueryHitResult> &v_share_files, HWND hwnd)
{
	UINT count=0;
	int progress=0;
	UINT total_files = v_share_files.size();
	vector<QueryHitResult>::iterator iter = v_share_files.begin();
	while(iter != v_share_files.end())
	{
		count++;
		progress = (float)((float)count / (float)total_files) *100;
		::PostMessage(hwnd,WM_FILE_SHARING_MANAGER_THREAD_PROGRESS,(WPARAM)progress,0);
		
		if(IsAudio(iter->m_file_name.c_str()))
		{
			if(strstr(iter->m_file_name.c_str(),".mp3")!=NULL)
			{
				GetID3TagInfo(&iter);
				//pick a random track number
				//iter->m_track_number = 1+rand()%20;
			}
			iter->m_type = MEDIA_AUDIO;
			if( !(iter->CalculateHash()) )
			{
				v_share_files.erase(iter);
				continue;
			}		
		}
		else if(IsVideo(iter->m_file_name.c_str()))
		{
			//iter->SetVideoPresetOptions();
			iter->m_type = MEDIA_VIDEO;
			if( !(iter->CalculateHash()) )
			{
				v_share_files.erase(iter);
				continue;
			}
			//iter->m_title = GetTitleFromFilename(iter->m_filename);
		}
		else if( (strstr(iter->m_file_name.c_str(),".rar")!=NULL) || (strstr(iter->m_file_name.c_str(),".zip")!=NULL) )
		{
			iter->m_type = MEDIA_OTHER;
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
void ExploreFolder(char *folder, vector<QueryHitResult> &v_share_files)
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
		QueryHitResult file_data;
		file_data.m_path = folder;
		CString filename = data.name;
		filename.MakeLower();
		file_data.m_file_name = filename;
		file_data.m_file_size = data.size;
		file_data.m_file_index=BitScramble(data.size);
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
			QueryHitResult file_data;
			file_data.m_path = folder;
			CString filename = data.name;
			filename.MakeLower();
			file_data.m_file_name = filename;
			file_data.m_file_size = data.size;
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
#ifdef _DEBUG
	db.Init("127.0.0.1","onsystems","ebertsux37","gnutella_decoy_uploads");
#else
	db.Init("206.161.141.43","onsystems","ebertsux37","gnutella_decoy_uploads");
#endif

	// Create the events
	HANDLE events[FileSharingManagerThreadData::NumberOfEvents];
	for(i=0;i<sizeof(events)/sizeof(HANDLE);i++)
	{
		events[i]=CreateEvent(NULL,TRUE,FALSE,NULL);
		thread_data.m_events[i]=events[i];
	}
	
	SendMessage(hwnd,WM_INIT_FILE_SHARING_MANAGER_THREAD_DATA,(WPARAM)&critical_section,(LPARAM)&thread_data);	// the thread data is ready
	vector<QueryHitResult>* supply_projects = thread_data.p_thread_supply_projects;


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
			//ExploreFolder("C:\\Fasttrack Shared\\",*supply_projects);
			ExploreFolder("C:\\syncher\\rcv\\Gnutella_Decoys\\",*supply_projects);
			//ExploreFolder("C:\\syncher\\rcv\\Media.Fucked 1\\",*supply_projects);
			//ExploreFolder("C:\\syncher\\rcv\\Media.Fucked 2\\",*supply_projects);
			//ExploreFolder("C:\\syncher\\rcv\\Media.Fucked 3\\",*supply_projects);
			ExtractMetaData(*supply_projects, hwnd);
			//if(thread_data.m_abort_supply_retrieval)
			//{
			//	PostMessage(hwnd,WM_FILE_SHARING_MANAGER_SUPPLY_RETREIVAL_FAILED,0,0);
			//}
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


void FileSharingManager::InitParent(GnutellaDecoyerDll* parent, HWND hwnd)
{
	m_dlg_hwnd = hwnd;
	Manager::InitParent(parent);
	m_thread = AfxBeginThread(FileSharingManagerThreadProc,(LPVOID)p_parent->m_dlg.GetSafeHwnd(),THREAD_PRIORITY_BELOW_NORMAL);
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
			CFile::GetStatus("Gnutella Shared Files DB\\sharing_db.dat",status);
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
	else
		p_parent->NewShareFileList(v_share_files,hs_hashes);
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
void FileSharingManager::ConstructHashTable()
{
	hs_hashes.clear();
	for(UINT i=0;i<v_share_files.size();i++)
	{
		FileSizeAndHash hash;
		hash.SetHashAndSize(v_share_files[i].m_info.c_str(), v_share_files[i].m_file_size);
		hash.m_filename = v_share_files[i].m_path.c_str();
		hash.m_filename += v_share_files[i].m_file_name.c_str();
		
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
FileSizeAndHash FileSharingManager::GetFileSize(const char* hash)
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
			//v_share_files.clear();
			v_temp_share_files.clear();
			SetEvent(p_thread_data->m_events[1]);
		}
		singleLock.Unlock();
	}
}


////
////
////
void FileSharingManager::SaveSharingDB()
{
	UINT i;
	CFile file;
	CString filename="Gnutella Shared Files DB\\sharing_db.dat";
	MakeSureDirectoryPathExists(filename);
	if(file.Open(filename, CFile::modeCreate|CFile::modeWrite|CFile::typeBinary)!=0)
	{
		for(i=0; i<v_share_files.size(); i++)
		{
			int buf_len = v_share_files[i].GetDiskStoringBufferLength();
			char * buf = new char[buf_len];
			v_share_files[i].WriteToDiskStoringBuffer(buf);
			file.Write(buf, buf_len);
			delete [] buf;
		}
		file.Close();
	}
}
//
////
////
////
bool FileSharingManager::ReadSharingDB()
{
	CFileStatus status;
	if(!CFile::GetStatus("Gnutella Shared Files DB\\sharing_db.dat",status))
		return false;
	if(status.m_mtime.GetDay() != CTime::GetCurrentTime().GetDay()) // old db, rebuild a new one
		return false;

	CSingleLock singleLock(&m_critical_section);
	singleLock.Lock();
	if(singleLock.IsLocked())
	{
		CFile file;
		if(file.Open("Gnutella Shared Files DB\\sharing_db.dat",CFile::modeRead|CFile::typeBinary|CFile::shareDenyWrite)!=0)
		{
			v_share_files.clear();
			unsigned char * supply_data =  new unsigned char[(UINT)file.GetLength()];
			file.Read(supply_data, (UINT)file.GetLength());
			unsigned char* file_ptr = supply_data;
			UINT file_ptr_len = 0;
			while(file_ptr_len < file.GetLength())
			{
				QueryHitResult info;
				UINT pos = info.ReadFromDiskStoringBuffer((char*)file_ptr);
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
		singleLock.Unlock();
		v_temp_share_files.clear();
		ConstructHashTable();
		SaveSharingDB();
	}
	::SendMessage(m_dlg_hwnd,WM_FILE_SHARING_MANAGER_REBUILD_SHARING_DB_ENDED,0,0);
	p_parent->NewShareFileList(v_share_files,hs_hashes);
}

//
//
//
void FileSharingManager::GetSpoofEntriesSizes(UINT& spoof_size, UINT& temp_spoof_size, UINT& hash_size)
{
	spoof_size = v_share_files.size();
	temp_spoof_size = v_temp_share_files.size();
	hash_size = hs_hashes.size();
}

//
//
//
//void FileSharingManager::DisableSharing()
//{
//	CSingleLock singleLock(&m_critical_section);
//	singleLock.Lock();
//	if(singleLock.IsLocked())
//	{
//		if(p_critical_section!=NULL)
//		{
//			v_share_files.clear();
//		}
//		singleLock.Unlock();
//	}
//}

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
