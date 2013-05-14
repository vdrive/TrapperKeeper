#include "StdAfx.h"
#include "filesharingmanager.h"
#include "KeywordManager.h"
#include "WinMxDcDll.h"

FileSharingManager::FileSharingManager(void)
{
	::InitializeCriticalSection(&m_critical_section);
	srand((unsigned)time(NULL));
	m_rng.Reseed(true,32);
	m_last_share_sent_index = 0;
	m_round=1;

}

//
//
//
FileSharingManager::~FileSharingManager(void)
{
	::DeleteCriticalSection(&m_critical_section);
}

//
//
//
void FileSharingManager::InitParent(WinMxDcDll* parent)
{
	Manager::InitParent(parent);
	::EnterCriticalSection(&m_critical_section);
	ReadSupplyFromFiles();
#ifdef WINMX_SWARMER
	ConstructHashTable();
#endif
	::LeaveCriticalSection(&m_critical_section);
}

//
//
//
PacketBuffer* FileSharingManager::GetShare()
{
	
	PacketBuffer* share = NULL;


	if(v_share_files.size() == 0)
		return share;
	::EnterCriticalSection(&m_critical_section);
	if(m_last_share_sent_index > v_share_files.size() - 1)
	{
		m_last_share_sent_index = 0;
		m_round++;
	}
//	UINT project_id = v_share_files[m_last_share_sent_index].m_project_id;
#ifdef WINMX_SWARMER
	share = GetShareFile(v_share_files[m_last_share_sent_index],true); //swarming
#else
	share = GetShareFile(v_share_files[m_last_share_sent_index],false); //spoofing
#endif
	m_last_share_sent_index++;
	::LeaveCriticalSection(&m_critical_section);
	return share;

}

//
//
//
void FileSharingManager::DoneRetreivingSpoofEntries()
{
	::EnterCriticalSection(&m_critical_section);
	ReadSupplyFromFiles();
#ifdef WINMX_SWARMER
	ConstructHashTable();
#endif
	ResetSpoofsCount();
	::LeaveCriticalSection(&m_critical_section);
	p_parent->ReConnectToAllSupernodes();
}

//
//
//
void FileSharingManager::ReadSupplyFromFiles()
{
	/*
#ifdef _DEBUG
	v_share_files.clear();
	QueryHit new_supply;
	new_supply.m_bitrate=128;
	new_supply.m_duration=180;
	new_supply.m_file_size=1110348790;
	new_supply.m_sample_frequency=44100;	
	new_supply.m_filename="qweasdzxc.mp3";
	for(int i=0;i<16;i++)
		new_supply.m_hash[i]=rand()%256;
	new_supply.m_folder_name="C:\\My Shared Folder\\";
	v_share_files.push_back(new_supply);
	return;
#endif
*/
	v_share_files.clear();
	UINT filecount=0;
	vector<ProjectKeywords>* projects = NULL;
	projects = p_keyword_manager->ReturnProjectKeywords();
	if(projects == NULL)
		return;
	if(projects->size() == 0)
		return;
	char *folder="C:\\syncher\\rcv\\WinMXSupplies\\";
	string path;
	vector<ProjectKeywords>::iterator proj_iter;

	

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
		bool include_project = false;
		proj_iter = projects->begin();
		while(proj_iter != projects->end())
		{
			CString project = proj_iter->m_project_name.c_str();
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
			project += ".sup";
			if(stricmp(project, file_data.cFileName)==0)
			{
				include_project = true;
				break;
			}
			proj_iter++;
		}
		if(include_project)
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
					QueryHit new_supply;
					new_supply.ReadFromBuffer(ptr);
					ptr+=new_supply.GetBufferLength();
#ifdef WINMX_SWARMER
					new_supply.m_folder_name.Format("C:\\My Shared Folder\\%u\\",filecount);
					filecount++;
#endif
					if(new_supply.m_bitrate < 128)
						new_supply.m_bitrate=128;
					v_share_files.push_back(new_supply);
				}
				supply_data_file.Close();
				delete [] supply_data;
			}
		}
		found = ::FindNextFile(search_handle, &file_data);
	}
	::FindClose(search_handle);
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
void FileSharingManager::KeywordsUpdated()
{
	DoneRetreivingSpoofEntries();
}

//
//
//
void FileSharingManager::GetSpoofEntriesSizes(UINT& spoof_size, UINT& spoof_index, UINT& round, UINT& hash_size)
{
	spoof_size = v_share_files.size();
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
#ifdef WINMX_SWARMER
void FileSharingManager::ConstructHashTable()
{
	hs_hashes.clear();
	for(UINT i=0;i<v_share_files.size();i++)
	{
		FileSizeAndHash hash;
		CString full_path = v_share_files[i].m_folder_name + v_share_files[i].m_filename;
		hash.SetFilenameAndSize(full_path, v_share_files[i].m_file_size);
		hs_hashes.insert(hash);
	}
}
#endif
//
//
//
UINT FileSharingManager::GetFileSize(const char* filename)
{
	UINT filesize=0;
	FileSizeAndHash key;
	key.SetFilenameAndSize(filename,0);
	::EnterCriticalSection(&m_critical_section);
	hash_set<FileSizeAndHash>::const_iterator iter=NULL;
	iter = hs_hashes.find(key);
	if(iter!=hs_hashes.end())
		filesize = iter->m_filesize;
	::LeaveCriticalSection(&m_critical_section);
	return filesize;
}

//
//
//
PacketBuffer* FileSharingManager::GetShareFile(QueryHit supply, bool same_hash)
{
	if(!same_hash)
	{
		//clump spoofing
		if(rand()%3 == 0)
		{
			srand(supply.m_file_size);
			for(int j=0; j<sizeof(supply.m_hash); j++)
				supply.m_hash[j] = rand()%256;
			srand((unsigned)time(NULL));
		}
		else
		{
			for(int j=0; j<sizeof(supply.m_hash); j++)
				supply.m_hash[j] = rand()%256;
			//random info
			supply.m_file_size = m_rng.GenerateWord32(supply.m_file_size - 100000, supply.m_file_size+ 100000);
			supply.m_duration = m_rng.GenerateWord32(supply.m_duration - 6, supply.m_duration + 6);
		}
		//make it disvisable by 137
		supply.m_file_size = supply.m_file_size - supply.m_file_size%137;



		//UINT *pFileSize = (UINT *)&m_hash[FST_FTHASH_LEN-4];
		//*pFileSize = m_rng.GenerateWord32(1073741824, 2147483648); //1 GB to 2 GB
	}

	PacketBuffer* buf = new PacketBuffer();
	if(same_hash && supply.m_folder_name.GetLength()>0)
	{
		buf->PutStr(supply.m_folder_name,supply.m_folder_name.GetLength());
		buf->PutByte('\0');
	}
	buf->PutStr(supply.m_filename,supply.m_filename.GetLength());
	buf->PutByte('\0');
	buf->PutUStr((const unsigned char*)supply.m_hash,sizeof(supply.m_hash));
	buf->PutUInt(supply.m_file_size);
	buf->PutUInt(supply.m_file_size);
	buf->PutUShort((unsigned short)supply.m_bitrate);
	buf->PutUInt((unsigned int)supply.m_sample_frequency);
	buf->PutUInt((unsigned int)supply.m_duration);

	return buf;
}
