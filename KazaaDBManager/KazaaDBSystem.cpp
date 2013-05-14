#include "StdAfx.h"
#include "kazaadbsystem.h"
#include "giFThash.h"
#include "giFTmd5.h"
#include "..\tkcom\Timer.h"
#include <Psapi.h>
#include <mmsystem.h>

KazaaDBSystem::KazaaDBSystem(void)
{
	md5_buf=new byte[300*1024];
	md5_buf2=new byte[300*1024];
	mb_invalid=false;
	mb_changed=false;

	mb_user_request_rescan=false;
	mb_user_request_delete=false;
	mb_user_request_clear_files=false;
	mb_user_request_write_databases=false;
	mb_db_files_current=false;

	m_status="Idle";
	m_progress=0;
	m_max_progress=100;
	m_last_rescan_time=CTime::GetCurrentTime();
	mb_file_list_gui_should_change=false;
}

KazaaDBSystem::~KazaaDBSystem(void)
{
	delete []md5_buf;
	delete []md5_buf2;
}

UINT KazaaDBSystem::Run(void)
{
	Timer shared_rescan_timer;
	Timer periodic_rescan_timer;
	srand(timeGetTime());

	int periodic_interval=60*60*12;  //do it at least every 10 hours
	periodic_interval+=(rand()%(60*60*12));  //up to 12 hours more, too offset all the racks

	//main loop
	while(!this->b_killThread){
		if(mb_user_request_delete){
			mb_user_request_delete=false;
			//CSingleLock lock(&m_data_lock,TRUE);
			DeleteDatabases();
			shared_rescan_timer.Refresh();
			m_last_rescan_time=CTime::GetCurrentTime();
		}
		if(mb_user_request_clear_files){
			mb_user_request_clear_files=false;
			CSingleLock lock(&m_data_lock,TRUE);
			ClearFiles();
			shared_rescan_timer.Refresh();
			m_last_rescan_time=CTime::GetCurrentTime();
		}
		if(mb_user_request_write_databases){
			mb_user_request_write_databases=false;
			//CSingleLock lock(&m_data_lock,TRUE);
			WriteDBs();
			shared_rescan_timer.Refresh();
			m_last_rescan_time=CTime::GetCurrentTime();
		}
		if(shared_rescan_timer.HasTimedOut(60*10) || mb_user_request_rescan){  //every 10 minutes do a folder scan, then do work based on this
			mb_user_request_rescan=false;
			//time to check the contents of the shared folders, to see if we need to update the kazaa dbs
			
			RescanInputFile();
			if(!this->b_killThread)
				RescanFolders();

			if(periodic_rescan_timer.HasTimedOut(periodic_interval)){
				mb_invalid=true;  //make sure we write out the dbs ritualistically at least every 'periodic_interval' seconds
			}

			if(mb_invalid && !this->b_killThread){
				WriteDBs();  //time to write out the dbs, since we have a stable shared folder
				periodic_rescan_timer.Refresh();  //put off doing the forced rewrite, since we just did one
			}
			shared_rescan_timer.Refresh();
			m_last_rescan_time=CTime::GetCurrentTime();
		}
		Sleep(100);  //take a break so we don't use any cpu running this loop
	}

	return 0;
}

void KazaaDBSystem::RescanFolders(void)
{
	m_status="Enumerating files in shared folder...";
	m_progress=0;
	m_max_progress=100;

	vector <ManagedFileInfo> v_files;
	EnumerateFilesInFolder("c:\\fasttrack shared",v_files);
	EnumerateFilesInFolder("c:\\syncher\\rcv\\Media.fucked",v_files);
	m_progress=40;

	bool b_changed=false;

	//remove any managed files, that aren't really in our shared directories
	for(int i=0;i<(int)mv_mapped_files.size() && !this->b_killThread;i++){  //go through and remove any files that are no more
		ManagedFileInfo *file_info=&mv_mapped_files[i];
		bool b_found=false;
		for(int j=0;j<(int)v_files.size();j++){
			if( stricmp(file_info->m_file_name.c_str(),v_files[j].m_file_name.c_str())==0 
				&& stricmp(file_info->m_folder.c_str(),v_files[j].m_folder.c_str())==0
				&& memcmp(&file_info->m_last_write_time,&v_files[j].m_last_write_time,sizeof(file_info->m_last_write_time))==0)
			{
				//we already have this file in our managed file vector
				b_found=true;
				break;
			}
		}

		if(!b_found){
			b_changed=true;
			CSingleLock lock(&m_data_lock,TRUE);
			mv_mapped_files.erase(mv_mapped_files.begin()+i);
			i--;
		}
	}
	m_progress=75;

	vector <ManagedFileInfo> v_new_files;

	//go through the loops again, except in different orders and add any new files that have shown up
	for(int j=0;j<(int)v_files.size() && !this->b_killThread;j++){
		bool b_found=false;
		for(int i=0;i<(int)mv_mapped_files.size();i++){
			ManagedFileInfo *file_info=&v_files[i];
			if( stricmp(file_info->m_file_name.c_str(),v_files[j].m_file_name.c_str())==0 
				&& stricmp(file_info->m_folder.c_str(),v_files[j].m_folder.c_str())==0
				&& memcmp(&file_info->m_last_write_time,&v_files[j].m_last_write_time,sizeof(file_info->m_last_write_time))==0)
			{
				//we already have this file in our managed file vector
				b_found=true;
				break;
			}
		}
		if(!b_found){
			v_new_files.push_back(v_files[j]);  //we need to add a new file to our managed files
		}
	}

	m_status="Extracting info for new/changed files";
	m_progress=0;
	m_max_progress=(int)v_new_files.size();
	for(int i=0;i<(int)v_new_files.size() && !this->b_killThread;i++){
		mb_db_files_current=false;
		m_progress=i;
		if(GetDetailedFileInfo(&v_new_files[i])){  //try to read in detailed info about the file

			CSingleLock lock(&m_data_lock,TRUE);
			mv_mapped_files.push_back(v_new_files[i]);
			mb_file_list_gui_should_change=true;
			b_changed=true;  //we have a new managed file that we can potentially be putting into the kazaa db
			mb_changed=true;  //map the member as true too so that the display shows that files are changing during the long scans
		}
		//Sleep(5);
	}

	if(!b_changed && mb_changed){
		//we have a situation where the shared folders contents have stopped changing, and we should write out the kazaa db folder now.
		mb_invalid=true;  //signal to write the kazaa db out
		mb_changed=false;
	}
	else if(b_changed){
		mb_changed=true;  //flag to check on the next rescan folder, so we can track when things have stopped changing
	}

	m_status="Idle";
	m_progress=0;
}


//recursively enumerates all files and their respective info.
void KazaaDBSystem::EnumerateFilesInFolder(const char* directory,vector<KazaaDBSystem::ManagedFileInfo> &v_file_info)
{
	CString tmp=directory;
	tmp+="\\";
	CString path=tmp;
	tmp+="*";

	if(tmp.Find("\\windows")!=-1 || tmp.GetLength()<3){  //rule out windows and c drive
		return;
	}

	WIN32_FIND_DATA info;
	HANDLE hFind=FindFirstFile(tmp,&info);
	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}

	while(FindNextFile(hFind,&info)){ //add all the rest
		if(stricmp(info.cFileName,".")==0 || stricmp(info.cFileName,"..")==0)
			continue;
		bool b_directory=false;
		CString full_name=path+info.cFileName;
		if(((GetFileAttributes(full_name) & FILE_ATTRIBUTE_DIRECTORY) != 0)){
			EnumerateFilesInFolder(full_name,v_file_info);
		}
		else{
			ManagedFileInfo file_info;
			file_info.m_file_name=info.cFileName;
			file_info.m_folder=directory;
			file_info.m_file_size=info.nFileSizeLow;
			file_info.m_last_write_time=info.ftLastWriteTime;
			v_file_info.push_back(file_info);
		}
	}
	FindClose(hFind);  //we wouldn't want any handle leaks. it kills pre win2k.
}

bool KazaaDBSystem::GetDetailedFileInfo(KazaaDBSystem::ManagedFileInfo* file_info)
{
	static int the_index=0;
	the_index++;


	CString full_path=file_info->m_folder.c_str();
	full_path+="\\";
	full_path+=file_info->m_file_name.c_str();
	int mp3_quality;
	if(!CalculateHash(full_path,file_info->m_hash,file_info->m_file_size,mp3_quality)){  //we should at least be able to calculate the hash
		return false;
	}
	if(rand()%100<30){  //30% chance to set the integrity
		file_info->m_integrity=2;
	}
	else{
		file_info->m_integrity=-1;
	}
	
	//set the default title
	CString file_name=file_info->m_file_name.c_str();
	if(file_name.GetLength()>4){
		file_info->m_title=file_name.Left(file_name.GetLength()-4);
	}

	if(full_path.Right(4).CompareNoCase(".avi")==0){
		file_info->mb_movie=true;		
	}
	else if(file_info->m_file_size>20000000){  //its something wierd
		//use the default title which is the filename, don't set any of the other metadata
	}
	else{  //it is presumably an mp3, we need to extract the appropriate information
		//testing info, we still need to add real extraction
		CString album,title,category,comments,artist,keywords;

		/*
		album.Format("TheAlbum %d",the_index);
		title.Format("TheTitle %d",the_index);
		artist.Format("TheArtist %d",the_index);
		comments.Format("TheComments %d",the_index);
		keywords.Format("TheKeywords %d",the_index);
		category.Format("TheCategory %d",the_index);

		file_info->m_album=album;
		file_info->m_artist=artist;
		file_info->m_category=category;
		file_info->m_title=title;
		file_info->m_comment=comments;
		file_info->m_quality=224;
		file_info->m_keywords=keywords;
		*/

		file_info->m_quality=mp3_quality;
		ExtractMetaData(file_info);
	}

	return true;
}

bool KazaaDBSystem::CalculateHash(const char* full_file_name,byte *hash,UINT &file_length,int &mp3_quality)
{
	HANDLE file_handle = CreateFile((LPCSTR)full_file_name, // open file at local_path 
				GENERIC_READ,              // open for reading 
				FILE_SHARE_READ,           // share for reading 
				NULL,                      // no security 
				OPEN_EXISTING,             // existing file only 
				FILE_ATTRIBUTE_NORMAL,     // normal file 
				NULL);                     // no attr. template 

	if(file_handle==INVALID_HANDLE_VALUE || file_handle==NULL){
		return false;
	}

	DWORD hsize=0;
	file_length=(UINT)GetFileSize(file_handle,&hsize);   //we will only work with up to 2^32 size files for now.  Thats a 4 gig file.

	if(file_length==0){
		CloseHandle(file_handle);
		return false;
	}

	// Read in first 300K


	
	DWORD num_read=0;
	ReadFile(file_handle,md5_buf,300*1024,&num_read,NULL);

	CString tmp_str=full_file_name;
	mp3_quality=128;
	if(tmp_str.GetLength()>4 && tmp_str.Right(4).CompareNoCase(".mp3")==0){
		mp3_quality=ExtractMP3Quality(md5_buf,num_read);
		if(mp3_quality<64)
			mp3_quality=128;
	}

	MD5Context md5;
	MD5Init(&md5);
	MD5Update(&md5,&md5_buf[0],num_read);
	MD5Final(hash,&md5);	// first 16 bytes of that hash

	// Stolen stolen code, begin

	// Calculate the 4-byte small hash.
	unsigned int smallhash = 0xffffffff;
	unsigned int chunk_size=300*1024;
	

	if(file_length > chunk_size)
	{
		size_t offset = 0x100000;	// 1 MB
		size_t lastpos = chunk_size;	// 300K
		size_t endlen;
		while(offset+2*chunk_size < file_length && !this->b_killThread)
		{
			LONG high_move=0;
			LONG low_move=(UINT)(offset);
			DWORD dwPtr=SetFilePointer(file_handle,low_move,&high_move,FILE_BEGIN);  //move to where we want to start reading
			
			ReadFile(file_handle,md5_buf2,chunk_size,&num_read,NULL);
	
			smallhash = hashSmallHash(md5_buf2, chunk_size, smallhash);
			lastpos = offset+chunk_size;
			offset <<= 1;
		}

		endlen = (size_t)file_length - lastpos;
		if (endlen > chunk_size) 
		{
			endlen = chunk_size;
		}
	
		LONG high_move=0;
		LONG low_move=(UINT)(file_length-endlen);
		DWORD dwPtr=SetFilePointer(file_handle,low_move,&high_move,FILE_BEGIN);  //move to where we want to start reading
		
		ReadFile(file_handle,md5_buf2,(DWORD)endlen,&num_read,NULL);

		smallhash = hashSmallHash(md5_buf2, endlen, smallhash);
	}

	smallhash ^= file_length;
	hash[16] = smallhash & 0xff;
	hash[17] = (smallhash >> 8) & 0xff;
	hash[18] = (smallhash >> 16) & 0xff;
	hash[19] = (smallhash >> 24) & 0xff;
	CloseHandle(file_handle);
	return true;
}

void KazaaDBSystem::WriteDBs(void)
{
	m_kazaa_launcher.KillAllKazaa();
	for(int i=0;i<100 && !this->b_killThread;i++){
		Sleep(100);  //sleep for 10 seconds to allow the files to all become unlocked.  This is required because the system keeps files locked by an application, locked for a given time period after the application is terminated
	}
	//make sure they are dead
	//kill the kazaas
	try{
		DWORD ids[1000];
		HMODULE modules[1000];
		DWORD count;
		EnumProcesses(ids,sizeof(DWORD)*1000,&count);  //enumerate the processes over and over again until kazaa shows up
		count/=(sizeof(DWORD));
		for(int i=0;i<(int)count;i++){ //for each of the processes
			DWORD nmod;
			
			HANDLE handle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD)ids[i]);

			EnumProcessModules(handle,modules,sizeof(HMODULE)*1000,&nmod);
			nmod/=(sizeof(HMODULE));

			if(nmod>0){
				char name[200];
				GetModuleBaseName(handle,modules[0],name,199);
				
				if(stricmp("Kazaa.exe",name)==0 || stricmp("Kazaa.kpp",name)==0 || stricmp("KazaaLite.kpp",name)==0 || stricmp("KazaaLite.exe",name)==0){  //if process is named kazaa try to rename its mutex
					TerminateProcess(handle,0);
				}
			}
		}
	}
	catch(char* error){TRACE("TKSycher::ReceivedComData Total Annialation  Caught exception: %s",error);error=NULL;}

	for(int i=0;i<100 && !this->b_killThread;i++){
		Sleep(100);  //sleep for 10 seconds to allow the files to all become unlocked.  This is required because the system keeps files locked by an application, locked for a given time period after the application is terminated
	}	


	m_db_writer.ClearDatabases();
	m_progress=0;
	m_max_progress=(int)mv_mapped_files.size();
	m_status="Writing Kazaa Databases...";

	for(int i=0;i<(int)mv_mapped_files.size() && !this->b_killThread;i++){
		//throw random data into metadata right now
		string project=ExtractProjectFromPath(mv_mapped_files[i].m_folder.c_str());
		if(project.size()>0){
			byte desc_buf[451];
			byte keyword_buf[451];

			m_input_file.GetMetaData(project.c_str(),desc_buf,keyword_buf);
			CString tmp=(char*)desc_buf;
			tmp=tmp.Trim();
			if(tmp.GetLength()>0)
				mv_mapped_files[i].m_comment=(LPCSTR)tmp;
			tmp=(char*)keyword_buf;
			tmp=tmp.Trim();
			if(tmp.GetLength()>0)
				mv_mapped_files[i].m_keywords=(LPCSTR)tmp;

			mb_file_list_gui_should_change=true;
		}

		m_progress++;
		m_db_writer.WriteKnownFile(mv_mapped_files[i]);
	}

	
	m_progress=0;
	mb_invalid=false;
	mb_db_files_current=true;
	m_status="Copying database...";
	m_db_writer.CopyDatabases();
	m_status="Relaunching Kazaas...";
	m_kazaa_launcher.ResumeLaunchingKazaa();
	m_status="Idle";
}

const char* KazaaDBSystem::GetStatus(void)
{
	return m_status.c_str();
}

void KazaaDBSystem::GetProgress(int& current, int& max)
{
	current=m_progress;
	max=m_max_progress;
}

void KazaaDBSystem::DeleteDatabases(void)
{
	m_status="Deleting Kazaa Databases...";
	m_progress=0;
	m_max_progress=100;
	mb_db_files_current=false;

	//kill the kazaas
	try{
		DWORD ids[1000];
		HMODULE modules[1000];
		DWORD count;
		EnumProcesses(ids,sizeof(DWORD)*1000,&count);  //enumerate the processes over and over again until kazaa shows up
		count/=(sizeof(DWORD));
		for(int i=0;i<(int)count;i++){ //for each of the processes
			DWORD nmod;
			
			HANDLE handle=OpenProcess(PROCESS_ALL_ACCESS,FALSE,(DWORD)ids[i]);

			EnumProcessModules(handle,modules,sizeof(HMODULE)*1000,&nmod);
			nmod/=(sizeof(HMODULE));

			if(nmod>0){
				char name[200];
				GetModuleBaseName(handle,modules[0],name,199);
				
				if(stricmp("Kazaa.exe",name)==0 || stricmp("Kazaa.kpp",name)==0 || stricmp("KazaaLite.kpp",name)==0 || stricmp("KazaaLite.exe",name)==0){  //if process is named kazaa try to rename its mutex
					TerminateProcess(handle,0);
				}
			}
		}
	}
	catch(char* error){TRACE("TKSycher::ReceivedComData Total Annialation  Caught exception: %s",error);error=NULL;}
	m_progress=20;

	for(int i=0;i<100;i++){
		CString onsystems_dir;
		onsystems_dir.Format("c:\\onsystems%d",i);
		RemoveDBFilesFromDirectory(onsystems_dir);
		if(m_progress<100)
			m_progress++;
	}

	m_progress=0;
	m_status="Idle";
}

void KazaaDBSystem::ClearFiles(void)
{
	m_status="Idle";
	m_progress=0;
	mv_mapped_files.clear();
//	mv_last_file_check.clear();
	mb_db_files_current=false;
	mb_file_list_gui_should_change=true;
}

void KazaaDBSystem::GetTimeToRescan(int &minutes,int &seconds)
{
	CTime cur_time=CTime::GetCurrentTime();
	CTime next_rescan=m_last_rescan_time+CTimeSpan(0,0,10,0);
	CTimeSpan dif_time=next_rescan-cur_time;
	seconds=dif_time.GetSeconds();
	minutes=dif_time.GetMinutes();
}

bool KazaaDBSystem::GetFilesChanging(void)
{
	return mb_changed;
}

CString FormatNumber(CString num){
	if (num.GetLength() < 4)
		return num;

	bool b_neg = false;
	if (num[0]=='-') {
		b_neg = true;
		num = num.Mid(1);
	}

	int count = num.GetLength() / 3;

	for(int i=num.GetLength()-3;i>=0;i-=3){
		if(i!=0){
			num.Insert(i,',');
		}
	}

	if (b_neg){
		num="-"+num;
	}
	return num;
}

void KazaaDBSystem::FillFileListCtrl(CListCtrl& file_list_ctrl)
{
	if(!mb_file_list_gui_should_change)
		return;

	mb_file_list_gui_should_change=false;

	CSingleLock lock(&m_data_lock,TRUE);
	file_list_ctrl.DeleteAllItems();
	

	for(int i=0;i<(int)mv_mapped_files.size();i++){
		int index=file_list_ctrl.InsertItem(i,mv_mapped_files[i].m_folder.c_str());
		file_list_ctrl.SetItemData(index,index);
		file_list_ctrl.SetItemText(i,1,mv_mapped_files[i].m_file_name.c_str());
		CString tmp;
		tmp.Format("%d",mv_mapped_files[i].m_file_size);
		tmp=FormatNumber(tmp);
		file_list_ctrl.SetItemText(i,2,tmp);
		string hash=ConvertHashToString(mv_mapped_files[i].m_hash);
		file_list_ctrl.SetItemText(i,3,hash.c_str());
		file_list_ctrl.SetItemText(i,4,mv_mapped_files[i].m_comment.c_str());
		file_list_ctrl.SetItemText(i,5,mv_mapped_files[i].m_keywords.c_str());
		file_list_ctrl.SetItemText(i,6,mv_mapped_files[i].m_title.c_str());
		file_list_ctrl.SetItemText(i,7,mv_mapped_files[i].m_artist.c_str());
		file_list_ctrl.SetItemText(i,8,mv_mapped_files[i].m_album.c_str());
		file_list_ctrl.SetItemText(i,9,mv_mapped_files[i].m_category.c_str());
		tmp.Format("%d",mv_mapped_files[i].m_quality);
		file_list_ctrl.SetItemText(i,10,tmp);
		tmp.Format("%d",mv_mapped_files[i].m_integrity);
		file_list_ctrl.SetItemText(i,11,tmp);
		tmp.Format("%s",(mv_mapped_files[i].mb_movie)?"Yes":"No");
		file_list_ctrl.SetItemText(i,12,tmp);
	}

	/*
	m_file_list.InsertColumn(0,"Folder",LVCFMT_LEFT,120);
	m_file_list.InsertColumn(1,"File Name",LVCFMT_LEFT,120);
	m_file_list.InsertColumn(2,"File Size (KB)",LVCFMT_LEFT,80);
	m_file_list.InsertColumn(3,"Kazaa Hash",LVCFMT_LEFT,120);
	m_file_list.InsertColumn(4,"Description",LVCFMT_LEFT,120);
	m_file_list.InsertColumn(5,"Keywords",LVCFMT_LEFT,120);
	m_file_list.InsertColumn(6,"Title",LVCFMT_LEFT,120);
	m_file_list.InsertColumn(7,"Artist",LVCFMT_LEFT,120);
	m_file_list.InsertColumn(8,"Album",LVCFMT_LEFT,120);
	m_file_list.InsertColumn(9,"Category",LVCFMT_LEFT,120);
	m_file_list.InsertColumn(10,"Quality",LVCFMT_LEFT,90);
	m_file_list.InsertColumn(11,"Integrity",LVCFMT_LEFT,70);
	m_file_list.InsertColumn(12,"Movie",LVCFMT_LEFT,70);
	*/
}

string KazaaDBSystem::ConvertHashToString(byte* hash)
{
	string str;
	for(int i=0;i<20;i++){
		switch((hash[i]>>4)&0xf){
			case 0: str+="0"; break;
			case 1: str+="1"; break;
			case 2: str+="2"; break;
			case 3: str+="3"; break;
			case 4: str+="4"; break;
			case 5: str+="5"; break;
			case 6: str+="6"; break;
			case 7: str+="7"; break;
			case 8: str+="8"; break;
			case 9: str+="9"; break;
			case 10: str+="A"; break;
			case 11: str+="B"; break;
			case 12: str+="C"; break;
			case 13: str+="D"; break;
			case 14: str+="E"; break;
			case 15: str+="F"; break;
		}
		switch((hash[i])&0xf){
			case 0: str+="0"; break;
			case 1: str+="1"; break;
			case 2: str+="2"; break;
			case 3: str+="3"; break;
			case 4: str+="4"; break;
			case 5: str+="5"; break;
			case 6: str+="6"; break;
			case 7: str+="7"; break;
			case 8: str+="8"; break;
			case 9: str+="9"; break;
			case 10: str+="A"; break;
			case 11: str+="B"; break;
			case 12: str+="C"; break;
			case 13: str+="D"; break;
			case 14: str+="E"; break;
			case 15: str+="F"; break;
		}
	}
	return str;
}

void KazaaDBSystem::ExtractMetaData(KazaaDBSystem::ManagedFileInfo* file_info)
{
	CString full_path=file_info->m_folder.c_str();
	full_path+="\\";
	full_path+=file_info->m_file_name.c_str();
	HANDLE file_handle = CreateFile((LPCSTR)full_path, // open file at local_path 
				GENERIC_READ,              // open for reading 
				FILE_SHARE_READ,           // share for reading 
				NULL,                      // no security 
				OPEN_EXISTING,             // existing file only 
				FILE_ATTRIBUTE_NORMAL,     // normal file 
				NULL);                     // no attr. template 

	if(file_handle==INVALID_HANDLE_VALUE || file_handle==NULL){
		return;
	}

	DWORD hsize=0;
	UINT file_length=(UINT)GetFileSize(file_handle,&hsize);   //we will only work with up to 2^32 size files for now.  Thats a 4 gig file.

	if(file_length<128){  //can't possibly contain any metadata
		CloseHandle(file_handle);
		return;
	}

	LONG high_move=0;
	LONG low_move=(UINT)(file_length-128);
	DWORD dwPtr=SetFilePointer(file_handle,low_move,&high_move,FILE_BEGIN);  //move to where we want to start reading

	DWORD num_read=0;
	byte buf[128];
	ReadFile(file_handle,buf,128,&num_read,NULL);
	CloseHandle(file_handle);
	if(num_read!=128)
		return;

	//unsigned char m_tag[3];		// "TAG"
	//unsigned char m_title[30];
	//unsigned char m_artist[30];
	//unsigned char m_album[30];
	//unsigned char m_year[4];
	//unsigned char m_comment[30];
	//unsigned char m_genre[1];

	if(buf[0]!='T' && buf[1]!='A' && buf[2]!='G')
		return;


	byte *data=buf;

	data+=3;  //skip past tag

	//do title
	file_info->m_title="";  //clear it since we set it to a default value before
	for(int i=0;i<30;i++){
		if(data[i]>0 && data[i]<128){  //does it fall within an ascii range
			file_info->m_title+=data[i];
		}
		else break;
	}

	data+=30; //skip past title

	//do artist
	for(int i=0;i<30;i++){
		if(data[i]>0 && data[i]<128){  //does it fall within an ascii range
			file_info->m_artist+=data[i];
		}
		else break;
	}

	data+=30; //skip past artist

	//do album
	for(int i=0;i<30;i++){
		if(data[i]>0 && data[i]<128){  //does it fall within an ascii range
			file_info->m_album+=data[i];
		}
		else break;
	}

	data+=30; //skip past album

	data+=4;  //skip past year


	for(int i=0;i<30;i++){
		if(data[i]>0 && data[i]<128){  //does it fall within an ascii range
			file_info->m_comment+=data[i];
		}
		else break;
	}

	data+=30;  //skip past comment

	byte genre=*data;

	switch(genre){
		case 0: file_info->m_category="Blues"; break;
		case 1: file_info->m_category="Classic Rock"; break;
		case 2: file_info->m_category="Country"; break;
		case 3: file_info->m_category="Dance"; break;
		case 4: file_info->m_category="Disco"; break;
		case 5: file_info->m_category="Funk"; break;
		case 6: file_info->m_category="Grunge"; break;
		case 7: file_info->m_category="Hip-Hop"; break;
		case 8: file_info->m_category="Jazz"; break;
		case 9: file_info->m_category="Metal"; break;
		case 10: file_info->m_category="New Age"; break;
		case 11: file_info->m_category="Oldies"; break;
		case 12: file_info->m_category="Other"; break;
		case 13: file_info->m_category="Pop"; break;
		case 14: file_info->m_category="R&B"; break;
		case 15: file_info->m_category="Rap"; break;
		case 16: file_info->m_category="Reggae"; break;
		case 17: file_info->m_category="Rock"; break;
		case 18: file_info->m_category="Techno"; break;
		case 19: file_info->m_category="Industrial"; break;
		case 20: file_info->m_category="Alternative"; break;
		case 21: file_info->m_category="Ska"; break;
		case 22: file_info->m_category="Death Metal"; break;
		case 23: file_info->m_category="Pranks"; break;
		case 24: file_info->m_category="Soundtrack"; break;
		case 25: file_info->m_category="Euro-Techno"; break;
		case 26: file_info->m_category="Ambient"; break;
		case 27: file_info->m_category="Trip-Hop"; break;
		case 28: file_info->m_category="Vocal"; break;
		case 29: file_info->m_category="Jazz+Funk"; break;
		case 30: file_info->m_category="Fusion"; break;
		case 31: file_info->m_category="Trance"; break;
		case 32: file_info->m_category="Classical"; break;
		case 33: file_info->m_category="Instrumental"; break;
		case 34: file_info->m_category="Acid"; break;
		case 35: file_info->m_category="House"; break;
		case 36: file_info->m_category="Game"; break;
		case 37: file_info->m_category="Sound Clip"; break;
		case 38: file_info->m_category="Gospel"; break;
		case 39: file_info->m_category="Noise"; break;
		case 40: file_info->m_category="Alternative Rock"; break;
		case 41: file_info->m_category="Bass"; break;
		case 42: file_info->m_category="Soul"; break;
		case 43: file_info->m_category="Punk"; break;
		case 44: file_info->m_category="Space"; break;
		case 45: file_info->m_category="Meditative"; break;
		case 46: file_info->m_category="Instrumental Pop"; break;
		case 47: file_info->m_category="Instrumental Rock"; break;
		case 48: file_info->m_category="Ethnic"; break;
		case 49: file_info->m_category="Gothic"; break;
		case 50: file_info->m_category="Darkwave"; break;
		case 51: file_info->m_category="Techno-Industrial"; break;
		case 52: file_info->m_category="Electronic"; break;
		case 53: file_info->m_category="Pop-Folk"; break;
		case 54: file_info->m_category="Eurodance"; break;
		case 55: file_info->m_category="Dream"; break;
		case 56: file_info->m_category="Southern Rock"; break;
		case 57: file_info->m_category="Comedy"; break;
		case 58: file_info->m_category="Cult"; break;
		case 59: file_info->m_category="Gangsta"; break;
		case 60: file_info->m_category="Top 40"; break;
		case 61: file_info->m_category="Christian Rap"; break;
		case 62: file_info->m_category="Pop/Funk"; break;
		case 63: file_info->m_category="Jungle"; break;
		case 64: file_info->m_category="Native US"; break;
		case 65: file_info->m_category="Cabaret"; break;
		case 66: file_info->m_category="New Wave"; break;
		case 67: file_info->m_category="Psychadelic"; break;
		case 68: file_info->m_category="Rave"; break;
		case 69: file_info->m_category="Showtunes"; break;
		case 70: file_info->m_category="Trailer"; break;
		case 71: file_info->m_category="Lo-Fi"; break;
		case 72: file_info->m_category="Tribal"; break;
		case 73: file_info->m_category="Acid Punk"; break;
		case 74: file_info->m_category="Acid Jazz"; break;
		case 75: file_info->m_category="Polka"; break;
		case 76: file_info->m_category="Retro"; break;
		case 77: file_info->m_category="Musical"; break;
		case 78: file_info->m_category="Rock & Roll"; break;
		case 79: file_info->m_category="Hard Rock"; break;
		case 80: file_info->m_category="Folk"; break;
		case 81: file_info->m_category="Folk-Rock"; break;
		case 82: file_info->m_category="National Folk"; break;
		case 83: file_info->m_category="Swing"; break;
		case 84: file_info->m_category="Fast Fusion"; break;
		case 85: file_info->m_category="Bebob"; break;
		case 86: file_info->m_category="Latin"; break;
		case 87: file_info->m_category="Revival"; break;
		case 88: file_info->m_category="Celtic"; break;
		case 89: file_info->m_category="Bluegrass"; break;
		case 90: file_info->m_category="Avantgarde"; break;
		case 91: file_info->m_category="Gothic Rock"; break;
		case 92: file_info->m_category="Progressive Rock"; break;
		case 93: file_info->m_category="Psychedelic Rock"; break;
		case 94: file_info->m_category="Symphonic Rock"; break;
		case 95: file_info->m_category="Slow Rock"; break;
		case 96: file_info->m_category="Big Band"; break;
		case 97: file_info->m_category="Chorus"; break;
		case 98: file_info->m_category="Easy Listening"; break;
		case 99: file_info->m_category="Acoustic"; break;
		case 100: file_info->m_category="Humour"; break;
		case 101: file_info->m_category="Speech"; break;
		case 102: file_info->m_category="Chanson"; break;
		case 103: file_info->m_category="Opera"; break;
		case 104: file_info->m_category="Chamber Music"; break;
		case 105: file_info->m_category="Sonata"; break;
		case 106: file_info->m_category="Symphony"; break;
		case 107: file_info->m_category="Booty Bass"; break;
		case 108: file_info->m_category="Primus"; break;
		case 109: file_info->m_category="Porn Groove"; break;
		case 110: file_info->m_category="Satire"; break;
		case 111: file_info->m_category="Slow Jam"; break;
		case 112: file_info->m_category="Club"; break;
		case 113: file_info->m_category="Tango"; break;
		case 114: file_info->m_category="Samba"; break;
		case 115: file_info->m_category="Folklore"; break;
		case 116: file_info->m_category="Ballad"; break;
		case 117: file_info->m_category="Power Ballad"; break;
		case 118: file_info->m_category="Rhytmic Soul"; break;
		case 119: file_info->m_category="Freestyle"; break;
		case 120: file_info->m_category="Duet"; break;
		case 121: file_info->m_category="Punk Rock"; break;
		case 122: file_info->m_category="Drum Solo"; break;
		case 123: file_info->m_category="Acapella"; break;
		case 124: file_info->m_category="Euro-House"; break;
		case 125: file_info->m_category="Dance Hall"; break;
		case 126: file_info->m_category="Goa"; break;
		case 127: file_info->m_category="Drum & Bass"; break;
		case 128: file_info->m_category="Club-House"; break;
		case 129: file_info->m_category="Hardcore"; break;
		case 130: file_info->m_category="Terror"; break;
		case 131: file_info->m_category="Indie"; break;
		case 132: file_info->m_category="BritPop"; break;
		case 133: file_info->m_category="Negerpunk"; break;
		case 134: file_info->m_category="Polsk Punk"; break;
		case 135: file_info->m_category="Beat"; break;
		case 136: file_info->m_category="Christian Gangsta Rap"; break;
		case 137: file_info->m_category="Heavy Metal"; break;
		case 138: file_info->m_category="Black Metal"; break;
		case 139: file_info->m_category="Crossover"; break;
		case 140: file_info->m_category="Contemporary Christian"; break;
		case 141: file_info->m_category="Christian Rock"; break;
		case 142: file_info->m_category="Merengue"; break;
		case 143: file_info->m_category="Salsa"; break;
		case 144: file_info->m_category="Trash Metal"; break;
		case 145: file_info->m_category="Anime"; break;
		case 146: file_info->m_category="Jpop"; break;
		case 147: file_info->m_category="Synthpop"; break;
		default: file_info->m_category="";
	}
	

	data+=1;  //skip past the genre

	//we are done
}

bool KazaaDBSystem::GetDBFilesCurrent(void)
{
	return mb_db_files_current;
}

void KazaaDBSystem::RemoveDBFilesFromDirectory(const char * directory)
{
	CString tmp=directory;
	tmp+="\\";
	CString path=tmp;
	tmp+="*";

	if(tmp.Find("\\windows")!=-1 || tmp.GetLength()<3){  //rule out windows and c drive
		return;
	}

	WIN32_FIND_DATA info;
	HANDLE hFind=FindFirstFile(tmp,&info);
	if (hFind == INVALID_HANDLE_VALUE) {
		return;
	}

	while(FindNextFile(hFind,&info)){ //add all the rest
		if(stricmp(info.cFileName,".")==0 || stricmp(info.cFileName,"..")==0)
			continue;

		CString full_name=path+info.cFileName;
		if(((GetFileAttributes(full_name) & FILE_ATTRIBUTE_DIRECTORY) != 0)){
			RemoveDBFilesFromDirectory(full_name);
		}
		else{
			CString tmp_file_name=full_name.MakeLower();
			if(tmp_file_name.Find(".dbb")!=-1){  //only check .dbb files
				DeleteFile(full_name);
			}
		}
	}
	FindClose(hFind);  //we wouldn't want any handle leaks. it kills pre win2k.
}

void KazaaDBSystem::RescanInputFile(void)
{
	if(m_input_file.HasChanged()){  //has the metadata input file changed???
		mb_invalid=true;  //signal to write the kazaa db file out
		m_input_file.RescanInputFile();
	}
}

string KazaaDBSystem::ExtractProjectFromPath(const char* path)
{
	//find last directory
	CString tmp=path;
	tmp=tmp.Trim();
	tmp=tmp.MakeLower();
	if(tmp.Find("\\swarmer.distribute")==-1)  //is it a distributed swarmed project? (meaning are we dealing with a swarmed file that has been organized by project)
		return string("");
	if(tmp[tmp.GetLength()-1]=='\\')  //if it ends with a bandkslash then get rid of that one, because the project must be right before it
		tmp=tmp.Left(tmp.GetLength()-1);

	int index=tmp.ReverseFind('\\');  //find the last backslash , the project will be the remaining string going forward from this backslash
	if(index!=-1){
		return string((LPCSTR)tmp.Mid(index+1));
	}
	return string("");
}

int KazaaDBSystem::ExtractMP3Quality(const byte* buffer,UINT buffer_length)
{
	for(int i=0;i<4096 && i<(int)buffer_length-8;i++){
		if(buffer[i]==0xff &&(buffer[i+1]&224)==224){  //start of mp3 frame
			byte mpeg_version_info=(buffer[i+1]>>3)&0x3;
			byte layer_info=(buffer[i+1]>>1)&0x3;
			byte bitrate_info=(buffer[i+2]>>4)&0xf;
			int version=2;
			if(mpeg_version_info==0x3)
				version=1;
			
			int layer=1;
			if(layer_info==0x2)
				layer=2;
			if(layer_info==0x1)
				layer=3;

			switch(bitrate_info){
				case 3: if(version==1) {
							if(layer==1)
								return 96;
							else if(layer==2)
								return 56;
							else if(layer==3)
								return 48;
						}
						else if(version==2){
							if(layer==1)
								return 56;
							else if(layer==2 || layer==3)
								return 24;
						}
						break;
				case 4: if(version==1) {
							if(layer==1)
								return 128;
							else if(layer==2)
								return 64;
							else if(layer==3)
								return 56;
						}
						else if(version==2){
							if(layer==1)
								return 64;
							else if(layer==2 || layer==3)
								return 32;
						}
						break;
				case 5: if(version==1) {
							if(layer==1)
								return 160;
							else if(layer==2)
								return 80;
							else if(layer==3)
								return 64;
						}
						else if(version==2){
							if(layer==1)
								return 80;
							else if(layer==2 || layer==3)
								return 40;
						}
						break;
				case 6: if(version==1) {
							if(layer==1)
								return 192;
							else if(layer==2)
								return 96;
							else if(layer==3)
								return 80;
						}
						else if(version==2){
							if(layer==1)
								return 96;
							else if(layer==2 || layer==3)
								return 48;
						}
						break;
				case 7: if(version==1) {
							if(layer==1)
								return 224;
							else if(layer==2)
								return 112;
							else if(layer==3)
								return 96;
						}
						else if(version==2){
							if(layer==1)
								return 112;
							else if(layer==2 || layer==3)
								return 56;
						}
						break;
				case 8: if(version==1) {
							if(layer==1)
								return 256;
							else if(layer==2)
								return 128;
							else if(layer==3)
								return 112;
						}
						else if(version==2){
							if(layer==1)
								return 128;
							else if(layer==2 || layer==3)
								return 64;
						}
						break;
				case 9: if(version==1) {
							if(layer==1)
								return 288;
							else if(layer==2)
								return 160;
							else if(layer==3)
								return 128;
						}
						else if(version==2){
							if(layer==1)
								return 144;
							else if(layer==2 || layer==3)
								return 80;
						}
						break;
				case 10: if(version==1) {
							if(layer==1)
								return 320;
							else if(layer==2)
								return 192;
							else if(layer==3)
								return 160;
						}
						else if(version==2){
							if(layer==1)
								return 160;
							else if(layer==2 || layer==3)
								return 96;
						}
						break;
				case 11: if(version==1) {
							if(layer==1)
								return 352;
							else if(layer==2)
								return 224;
							else if(layer==3)
								return 192;
						}
						else if(version==2){
							if(layer==1)
								return 176;
							else if(layer==2 || layer==3)
								return 112;
						}
						break;
				case 12: if(version==1) {
							if(layer==1)
								return 384;
							else if(layer==2)
								return 256;
							else if(layer==3)
								return 224;
						}
						else if(version==2){
							if(layer==1)
								return 192;
							else if(layer==2 || layer==3)
								return 128;
						}
						break;
				case 13: if(version==1) {
							if(layer==1)
								return 416;
							else if(layer==2)
								return 320;
							else if(layer==3)
								return 256;
						}
						else if(version==2){
							if(layer==1)
								return 224;
							else if(layer==2 || layer==3)
								return 144;
						}
						break;
				case 14: if(version==1) {
							if(layer==1)
								return 448;
							else if(layer==2)
								return 384;
							else if(layer==3)
								return 320;
						}
						else if(version==2){
							if(layer==1)
								return 256;
							else if(layer==2 || layer==3)
								return 160;
						}
						break;
				default: return 128;
			}


			break;
		}
	}
	return 128;
}
