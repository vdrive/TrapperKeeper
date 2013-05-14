#include "StdAfx.h"
#include "swarmersourcedll.h"
#include <io.h>
#include "imagehlp.h"
#include "../NameServer/NameServerInterface.h"
#include "swarmercomheader.h"
#include"../KazaaSwarmerDownloader/DatfileRecord.h"

//
//
//
SwarmerSourceDll::SwarmerSourceDll(void)
{
}

//
//
//
SwarmerSourceDll::~SwarmerSourceDll(void)
{
}

//
//
//
void SwarmerSourceDll::DllInitialize()
{
	m_dlg.Create(IDD_DISTRIBUTEDSRC_DLG,CWnd::GetDesktopWindow());
	m_dlg.InitParent(this);
	srand((unsigned int)time(NULL));
}

//
//
//
void SwarmerSourceDll::DllUnInitialize()
{

	if (p_com != NULL)
	{
		delete p_com;
	}


}

//
//
//
void SwarmerSourceDll::DllStart()
{

	p_com = new SwarmerSourceComInterface();
	p_com->Register(this,45);
	p_com->InitParent(this);

	/*
	CFile logfile;

	if (logfile.Open("whereami.txt",CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate|CFile::shareDenyNone)== TRUE)
	{
		logfile.SeekToEnd();
		logfile.Write("where am I", 10);
		logfile.Write("\r\n",2);
		logfile.Close();
	}
	*/
	m_manager.ReadMapsFile();
}

//
//
//
void SwarmerSourceDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//
//
//
void SwarmerSourceDll::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
	char dlgdata[5000];
	SwarmerComHeader* header = (SwarmerComHeader*)data;
	byte * data_received =(byte *)data;
	// Skip over the header and the size integer at the beginning of the file
	data_received += sizeof(SwarmerComHeader) +4;
	int found;
	switch(header->op)
	{
		case(SwarmerComHeader::File_Sent):
			{
				FileEntry *entry;
				DatfileRecord *record;
				record = (DatfileRecord *)data_received;
				entry = (FileEntry *)data_received;

				sprintf(dlgdata,"Received Project Record %s, %s",record->m_filename,record->m_hash);
				m_dlg.DisplayInfo(dlgdata);

				// Get Entry from Hash
				char hash[40+1];
				strcpy(hash,record->m_hash);
				found = m_manager.GetEntryFromHash(hash,entry);
				
				char project[256+1];
				char projectfilename[256+1];
				char filename[256+1];

				

				strcpy(project,entry->m_project);
				strcpy(projectfilename,entry->m_filename);

				sprintf(filename,"C:\\syncher\\file deposit\\distribute\\kazaa_swarm\\%s\\%s",project,projectfilename);
				if (found == 0)
				{
					strcpy(projectfilename,record->m_filename);
					sprintf(filename,"C:\\syncher\\file deposit\\distribute\\kazaa_swarm\\UNKNOWN\\%s",projectfilename);
				}

				if (strlen(projectfilename) == 0)
				{
					strcpy(projectfilename,"Unknownfile.mp3");
					sprintf(filename,"C:\\syncher\\file deposit\\distribute\\kazaa_swarm\\UNKNOWN\\%s",projectfilename);			

				}
				MakeSureDirectoryPathExists(filename);

				_finddata_t fdata;
				long handle=(long)_findfirst(filename,&fdata);

				while (handle != -1)
				{
					char endbuf[256+1];
					char * ptr = strrchr(filename,'.');
					strcpy(endbuf,ptr);
					*ptr = '\0';
					char endstr[6];
					sprintf(endstr," (%i)",(rand()%9));
					strcat(filename,endstr);
					strcat(filename,endbuf);
					_findclose(handle);
					handle=(long)_findfirst(filename,&fdata);
				}

				_findclose(handle);


				
				CFile file;
				if (file.Open(filename,CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate|CFile::shareDenyNone)== TRUE)
				{
					// Set the data back to the beginning
					data_received =(byte *)data;
					// Skip the header
					data_received += sizeof(SwarmerComHeader);
					file.Write(data_received,data_length - sizeof(SwarmerComHeader));
				}

				break;
			}

		default:
			{
				m_dlg.DisplayInfo("Received Unknown Data");


				break;
		
			}
	}

}

void SwarmerSourceDll::CheckRegistryTimer()
{
CHAR reg_sub_key[] = TEXT("Software\\Swarmer\\TimeStamp");
	HKEY hkey;

	time_t curtime = 0;

	DWORD cbData;
	
	if (RegOpenKeyEx(HKEY_CURRENT_USER,reg_sub_key,0,KEY_QUERY_VALUE|KEY_SET_VALUE,&hkey)!=ERROR_SUCCESS)
	{
		SetRegistryTimer();
		return;
	}

	RegQueryValueEx(hkey,"timestamp",NULL,NULL,(unsigned char *)curtime,&cbData);

	CTime regtime(curtime);
	CTime timenow;
	timenow.GetCurrentTime();

	regtime += CTimeSpan(1,0,0,0);
	if (timenow > regtime)
	{
		ProcessData();
	}



}

void SwarmerSourceDll::SetRegistryTimer()
{

	CHAR reg_sub_key[] = TEXT("Software\\Swarmer\\TimeStamp");
	HKEY hkey;

	CTime time;
	time_t curtime;
	curtime = (time_t)time.GetTime();


	DWORD cbData = sizeof(curtime);

	
	if (RegCreateKeyEx(HKEY_CURRENT_USER,reg_sub_key,0,NULL,REG_OPTION_NON_VOLATILE,KEY_QUERY_VALUE|KEY_SET_VALUE,NULL,&hkey,NULL)==ERROR_SUCCESS)
	{
		//RegQueryValueEx(hkey,"DownloadDir",NULL,NULL,(unsigned char *)downloaddir,&cbData);
		RegSetValueEx(hkey,"timestamp",NULL,REG_DWORD,(unsigned char *)curtime,cbData);
	}

}

void SwarmerSourceDll::ProcessData()
{
	int numcatalog = 0;
	int numspecial = 0;
	int numnormal = 0;
	int numrows = 0;

	v_file_list.clear();
	/*
	numcatalog = m_db.GetCatalogData(&v_file_list,1000);
	numspecial = m_db.GetSpecialData(&v_file_list);
	numnormal = m_db.GetRegularEntries(&v_file_list,1000);
	*/

//	numnormal = m_db.GetNonSpecialEntries(&v_file_list,12500);
	numspecial = m_db.GetSpecialData(&v_file_list);

	numrows = numcatalog + numspecial + numnormal;

	m_manager.ReadMapsFile();

	v_makefile_list.clear();
	// Clear out the old map file since we're rebuilding it with new data.
	m_manager.ResetMap();
	for (int i = 0; i < numrows; i ++)
	{
		//Compare here and either save and leave or save and make sig2dat;
		if ((m_manager.CompareHashWithMaps(v_file_list[i])) == true)
		{
			v_file_list[i].Save();
		}
		else
		{
			v_file_list[i].Save();
			v_makefile_list.push_back(v_file_list[i]);
		}
	}
	m_dlg.DisplayInfo("Removing leftover files");
	m_manager.RemoveLeftoverFiles();
	m_dlg.DisplayInfo("Sending Files to be Made");
	SendFilesToBeMade();

}

void SwarmerSourceDll::SendFilesToBeMade()
{
	
	EnumDestinations();
	int maxdest = 0;
	int destiter = 0;

	char disp_info[1024+1];
	
	byte* buf;
	UINT length = sizeof(SwarmerComHeader)+sizeof(FileEntry);
	buf = new byte[length]; 
	SwarmerComHeader * header = (SwarmerComHeader*)buf;
	header->op = SwarmerComHeader::Project_Record;
	header->size = length - sizeof(SwarmerComHeader);

	FileEntry *entry;
	

	maxdest = (int)v_destips.size();
	while(v_makefile_list.size() > 0)
	{	
		entry = (FileEntry *)&v_makefile_list[0];
		byte * ptr = &buf[sizeof(SwarmerComHeader)];
		//*((FileEntry*)ptr) = *entry;
		memcpy(ptr,&v_makefile_list[0],sizeof(FileEntry));
		entry = (FileEntry *)ptr;
		p_com->SendReliableData((char *)v_destips[destiter].c_str(),buf,length);
		sprintf(disp_info,"Sending to %s",(char *)v_destips[destiter].c_str());
		m_dlg.DisplayInfo(disp_info);
		v_makefile_list.erase(v_makefile_list.begin());
		destiter++;
		if (destiter == maxdest)
		{
			destiter = 0;
		}
	}

	unsigned int i = 0;
	while (i < v_destips.size())
	{
		
		SendRecordsComplete(v_destips[i]);
		i++;
	}

	delete[] buf;

}

void SwarmerSourceDll::SendRecordsComplete(string ip)
{
	byte* buf;
	UINT length = sizeof(SwarmerComHeader);
	buf = new byte[length]; 
	SwarmerComHeader * header = (SwarmerComHeader*)buf;
	header->op = SwarmerComHeader::Records_Complete;

	p_com->SendReliableData((char *)ip.c_str(),buf,length);

	delete[] buf;

}


void SwarmerSourceDll::GenerateMapFromFiles()
{
	vector<string> filenames;
	char hash[40+1];
	char tempfilename[256+1];
	FileEntry entry;
	int num_files;


	num_files = m_manager.ExploreFolder("C:\\syncher\\file deposit\\distribute\\kazaa_swarm\\",&filenames);
	for(int i = 0; i < (int)filenames.size(); i ++)
	{
		strcpy(tempfilename,filenames[i].c_str());
		m_manager.GetHashFromDeflated(tempfilename,hash);
		m_manager.GetEntryFromHash(hash,&entry);
		entry.SaveSpecial();
	}

}


void SwarmerSourceDll::EnumDestinations()
{

	v_destips.clear();
	NameServerInterface ns;
	ns.RequestIP("Swarmer",v_destips);

}