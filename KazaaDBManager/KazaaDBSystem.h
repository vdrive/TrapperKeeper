#pragma once
#include "..\tkcom\threadedobject.h"
#include "FileMetaData.h"
#include <afxmt.h>
#include "DBWriter.h"
#include "InputFile.h"
#include "..\KazaaLauncher\KazaaLauncherInterface.h"

class KazaaDBSystem : public ThreadedObject
{
private:
	class ManagedFileInfo : public FileMetaData
	{
	public:
		FILETIME m_last_write_time;
		bool mb_new;

		inline bool IsSameFile(ManagedFileInfo* other_file){
			if( other_file->m_file_size==m_file_size
				&& stricmp(other_file->m_file_name.c_str(),m_file_name.c_str())==0
				&& stricmp(other_file->m_folder.c_str(),m_folder.c_str())==0
				&& memcmp(&other_file->m_last_write_time,&m_last_write_time,sizeof(FILETIME))==0)
			{
				return true;
			}
			else{
				return false;
			}
		}
		ManagedFileInfo(){
			mb_new=true;
		}

	};

public:
	KazaaDBSystem(void);
	~KazaaDBSystem(void);

protected:
	UINT Run(void);

private:
	byte *md5_buf;
	byte *md5_buf2;

	void RemoveDBFilesFromDirectory(const char * directory);
	void ExtractMetaData(KazaaDBSystem::ManagedFileInfo* file_info);
	string ConvertHashToString(byte* hash);
	void RescanFolders(void);
	void DeleteDatabases(void);
	void ClearFiles(void);
	bool GetDetailedFileInfo(KazaaDBSystem::ManagedFileInfo* file_info);
	bool CalculateHash(const char* full_file_name,byte *hash,UINT &file_length,int &mp3_quality);
	void WriteDBs(void);
	void EnumerateFilesInFolder(const char* directory,vector<KazaaDBSystem::ManagedFileInfo> &v_file_info);
	//vector <ManagedFileInfo> mv_last_file_check;
	vector <ManagedFileInfo> mv_mapped_files;
	string m_status;
	int m_progress;
	int m_max_progress;
	bool mb_db_files_current;
	bool mb_invalid;  //if set to true, a kazaa db write will occur
	bool mb_changed;  //set to true when a scan indicated the shared folder contents have changed.  a write won't occur until the contents stop changing
	bool mb_file_list_gui_should_change;
	CCriticalSection m_data_lock;
	CTime m_last_rescan_time;
	InputFile m_input_file;
	DbWriter m_db_writer;
	KazaaLauncherInterface m_kazaa_launcher;

public:
	bool mb_user_request_rescan;
	bool mb_user_request_delete;
	bool mb_user_request_clear_files;
	bool mb_user_request_write_databases;

	const char* GetStatus(void);
	void GetProgress(int& current, int& max);


	void GetTimeToRescan(int &minutes,int &seconds);
	bool GetFilesChanging(void);
	void FillFileListCtrl(CListCtrl& file_list_ctrl);
	bool GetDBFilesCurrent(void);

	void RescanInputFile(void);
	string ExtractProjectFromPath(const char* file_name);
	int ExtractMP3Quality(const byte* buffer,UINT buffer_length);
};
