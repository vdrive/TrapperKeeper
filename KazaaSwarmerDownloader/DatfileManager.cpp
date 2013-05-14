#include "StdAfx.h"
#include "datfilemanager.h"
#include "sig2datfile.h"
#include "SwarmerDownloader.h"
#include "imagehlp.h"
#include <io.h>


DatfileManager::DatfileManager(void)
{
}

DatfileManager::~DatfileManager(void)
{
}

void DatfileManager::InitParent(SwarmerDownloader * parent)
{
	p_parent = parent;

}

string DatfileManager::MakeSig2Dat(char * filename, char * hash, int filesize)
{
	string datfilename;
	Sig2DatFile datfile;
	
	datfile.SetFilename(filename);
	char binHash[20];
//	char uuHash[20];
	AsciiHashtoBinHash(hash,binHash);
//	BinHashtoUUHash(binHash,uuHash);
	
	datfile.SetHash(binHash);
	datfile.SetLength(filesize);
	datfilename = datfile.WriteFile();

	return datfilename;
}

void DatfileManager::CreateNewDatfile(char * filename, char * hash, int filesize)
{
	DatfileRecord Record;
	string datfilename;
	datfilename = MakeSig2Dat(filename,hash,filesize);
	Record.SetRecord(filename,hash,filesize,datfilename);
	char destpath[256+1];


	if (v_current_datfiles.size() < 50)
	{
		v_current_datfiles.push_back(Record);
		// Also move the datfile to the Fast Track Shared directory
		strcpy(destpath,"C:\\FastTrack Shared\\");
		strcat(destpath,datfilename.c_str());
		MakeSureDirectoryPathExists(destpath);
		rename(datfilename.c_str(),destpath);
		
	}
	else
	{
		v_waiting_datfiles.push_back(Record);
		// Also move the datfile to the waiting directory
		strcpy(destpath,"Waiting Files\\");
		strcat(destpath,datfilename.c_str());
		MakeSureDirectoryPathExists(destpath);
		rename(datfilename.c_str(),destpath);

	}

}

bool DatfileManager::CopyFile(char * fullpath, char * destpath)
{

	CFile copyfile;

	return true;
}

void DatfileManager::AsciiHashtoBinHash(char * hashin, char * hashout)
{
	char ascii_hash[40+1];
	unsigned char bin_hash[20];

	strcpy(ascii_hash, hashin);
	memset(bin_hash,0,sizeof(bin_hash));
	for(int k=0;k<20;k++)
	{
		char tmp[3];
		tmp[0]=ascii_hash[2*k];
		tmp[1]=ascii_hash[2*k+1];
		tmp[2]='\0';

		unsigned int c;
		sscanf(tmp,"%x",&c);
		bin_hash[k]=(unsigned char)c;
	}

	memcpy(hashout,bin_hash,20);
}

void DatfileManager::BinHashtoUUHash(char * hashin, char * hashout)
{
// Init lookup table
	char lookup[64];
	memset(lookup,0,sizeof(lookup));
	for(int k=0;k<26;k++)
	{
		lookup[k]='A'+k;
		lookup[k+26]='a'+k;
	}
	for(k=0;k<10;k++)
	{
		lookup[k+26+26]='0'+k;
	}
	lookup[26+26+10]='+';
	lookup[26+26+10+1]='/';

	// Convert the binary hash to the uuhash
	unsigned char bin_hash[20];
	memcpy(bin_hash, hashin, 20);
	CString uuhash="";
	for(k=0;k<sizeof(bin_hash);k+=3)
	{
		uuhash+=lookup[(bin_hash[k]>>2)&0x3F];
		uuhash+=lookup[((bin_hash[k+1]>>4)&0x0F)+((bin_hash[k]<<4)&0x30)];
		
		if(k<sizeof(bin_hash)-3)
		{
			uuhash+=lookup[((bin_hash[k+1]<<2)&0x3C)+((bin_hash[k+2]>>6)&0x03)];
			uuhash+=lookup[bin_hash[k+2]&0x3F];
		}
		else
		{
			uuhash+=lookup[((bin_hash[k+1]<<2)&0x3C)];
		}
	}

	strcpy(hashout,uuhash);

}

bool DatfileManager::WriteCurrentDatfile()
{
	remove("currentdatfilelist.txt");
	CFile datfile;
	if (datfile.Open("currentdatfilelist.txt",CFile::modeWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeBinary|CFile::shareDenyWrite) == FALSE)
	{
		DWORD damnerror = ::GetLastError();
		return false;
	}
	for (int i = 0;i < (int)v_current_datfiles.size();i++)
	{
		datfile.Write(&v_current_datfiles[i], sizeof(DatfileRecord));
	}

	datfile.Close();
	return true;
}

bool DatfileManager::WriteWaitingDatfile()
{
	remove("waitingdatfilelist.txt");
	CFile datfile;
	if (datfile.Open("waitingdatfilelist.txt",CFile::modeWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeBinary|CFile::shareDenyWrite) == FALSE)
	{
		DWORD damnerror = ::GetLastError();
		return false;
	}
	for (int i = 0;i < (int)v_waiting_datfiles.size();i++)
	{
		datfile.Write(&v_waiting_datfiles[i], sizeof(DatfileRecord));
	}

	datfile.Close();
	return true;
}


bool DatfileManager::ReadCurrentDatfile()
{
	CFile datfile;
	DatfileRecord record;
	if (datfile.Open("currentdatfilelist.txt",CFile::modeRead|CFile::typeBinary|CFile::shareDenyWrite) == FALSE)
	{
		DWORD damnerror = ::GetLastError();
		return false;
	}
	while (datfile.Read(&record,sizeof(record)) != NULL)
	{
		v_current_datfiles.push_back(record);
		record.ClearRecord();
	}

	datfile.Close();

	return true;
}

bool DatfileManager::ReadWaitingDatfile()
{
	CFile datfile;
	DatfileRecord record;
	if (datfile.Open("waitingdatfilelist.txt",CFile::modeRead|CFile::typeBinary|CFile::shareDenyWrite) == FALSE)
	{
		DWORD damnerror = ::GetLastError();
		return false;
	}
	while (datfile.Read(&record,sizeof(record)) != NULL)
	{
		v_waiting_datfiles.push_back(record);
		record.ClearRecord();
	}

	datfile.Close();
	return true;
}

// This must be called AFTER you deflate the files so you get the hashes from the current list
// since this function will remove all of the downloaded files from the current list.
void DatfileManager::RefreshDownloadFiles()
{
	vector<string>  filenames;
	vector<DatfileRecord> Records;
	DatfileRecord Record;
	char tempname[256+1];
	char displaydata[5000];
	ExploreFolder("C:\\FastTrack Shared\\",&filenames);
	char * ptr;


	// Kill Kazaa
	p_parent->KillKazaa();

	Sleep(1000);

	char debugtemp[256+1];
	char exactname[256+1];

	Records.clear();
	// Check all of the datfiles that haven't been downloaded
	for(int i = 0; i < (int)filenames.size(); i++)
	{
		strcpy(tempname,filenames[i].c_str());
		ptr = strstr(tempname,".dat");
		if (ptr != NULL)
		{
			ptr = strrchr(tempname,'\\');
			strcpy(exactname,ptr+1);
			for (int j = 0; j < (int)v_current_datfiles.size(); j++)
			{
				strcpy(debugtemp,v_current_datfiles[j].m_datfilename);
				if (strcmp(exactname,v_current_datfiles[j].m_datfilename) == 0)
				{
					Record = v_current_datfiles[j];
					Records.push_back(Record);
				}
			}
		}
	}

	v_current_datfiles = Records;

	char destpath[256+1];
	char sourcepath[256+1];

	// Clear the list if the number of records is close to 50

	int found = 0;
	if ( (int)v_current_datfiles.size() >= 48)
	{
		int sizedat = (int)v_current_datfiles.size();
		filenames.clear();
		ExploreFolder("C:\\FastTrack Shared\\",&filenames);
		for(int i = 0; i < (int)filenames.size(); i++)
		{
			strcpy(tempname,filenames[i].c_str());

			ptr = strrchr(tempname,'\\');
			strcpy(exactname,ptr+1);
			found = 0;
			for (int j = 0; j < (int)v_current_datfiles.size(); j++)
			{
			
				strcpy(debugtemp,v_current_datfiles[j].m_datfilename);
				if (strcmp(exactname,v_current_datfiles[j].m_datfilename) == 0)
				{
					Record = v_current_datfiles[j];
					v_waiting_datfiles.push_back(Record);
					strcpy(sourcepath,"C:\\FastTrack Shared\\");
					strcat(sourcepath,Record.m_datfilename);
					strcpy(destpath,"Waiting Files\\");
					strcat(destpath,Record.m_datfilename);
					rename(sourcepath,destpath);
					sprintf(displaydata,"moving %s to %s because size is %i",sourcepath,destpath, sizedat);
					p_parent->DisplayInfo(displaydata);
					found = 1;
				}
			}
			if (found == 0)
			{
				strcpy(sourcepath,"C:\\FastTrack Shared\\");
				strcat(sourcepath,exactname);
				strcpy(destpath,"Waiting Files\\");
				strcat(destpath,exactname);
				rename(sourcepath,destpath);
				sprintf(displaydata,"moving unknown record %s to %s because size is %i",sourcepath,destpath, sizedat);
				p_parent->DisplayInfo(displaydata);

			}

		}
		v_current_datfiles.clear();
	}



	for (i = (int)v_current_datfiles.size(); i < 50; i++)
	{
		if (v_waiting_datfiles.size() > 0)
		{
			// Pull waiting records and make them current
			Record.ClearRecord();
			Record = *v_waiting_datfiles.begin();
			v_waiting_datfiles.erase(v_waiting_datfiles.begin());



			strcpy(sourcepath,"Waiting Files\\");
			strcat(sourcepath,Record.m_datfilename);
			strcpy(destpath,"C:\\FastTrack Shared\\");
			strcat(destpath,Record.m_datfilename);

			//check the timestamp to see if we should just dump this record
			CTime curtime = CTime::GetCurrentTime();
			CTimeSpan elapsed = curtime - Record.m_time ;
			CString oldtime = Record.m_time.Format("%H:%M  %B %d, %Y ");
			CString currenttime = curtime.Format("%H:%M  %B %d, %Y ");
			int hours = (int)elapsed.GetTotalHours();
			if (hours > (24*14)) // it's been two weeks, give up on this one
			{
				remove(sourcepath);
			}
			else
			{
				v_current_datfiles.push_back(Record);
				// copy the waiting datfile to the current datfile area
				rename(sourcepath,destpath);
			}
		}
		else
		{
			WriteCurrentDatfile();
			WriteWaitingDatfile();
			p_parent->LaunchKazaa();
			return;
		}
	}
	WriteCurrentDatfile();
	WriteWaitingDatfile();
	p_parent->LaunchKazaa();
}

void DatfileManager::ExploreFolder(char * folder, vector<string>  *filenames)
{
	char buf[1024];
	strcpy(buf,folder);
	strcat(buf,"*.*");
	string tempname;
	
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
			tempname = folder;
			tempname +=data.name;
			tempname += "/";

			ExploreFolder((char *)tempname.c_str(),filenames);
			data.size;

		}
	}
	else
	{
		tempname = folder;
		tempname += data.name;

		filenames->push_back(tempname);
	}

	while(_findnext(handle,&data)==0)
	{
		if(data.attrib & _A_SUBDIR)
		{
			if((strcmp(data.name,".")!=0) && (strcmp(data.name,"..")!=0))
			{
				tempname = folder;
				tempname +=data.name;
				tempname += "/";

				ExploreFolder((char * )tempname.c_str(),filenames);
				data.size;

			}
		}

		else
		{
			tempname = folder;
			tempname += data.name;

			filenames->push_back(tempname);
		}
	}
	_findclose(handle);
}

DatfileRecord DatfileManager::GetRecordFromFilename(char * filename)
{
	DatfileRecord record;
	record.ClearRecord();

	char displaydata[5000];
	char tempname[256+1];

	for (int i = 0; i < (int)v_current_datfiles.size(); i++)
	{
		strcpy(tempname,"C:\\FastTrack Shared\\");
		strcat(tempname,v_current_datfiles[i].m_filename);
		if ((strcmp(filename,tempname)) == 0)
		{
			record = v_current_datfiles[i];
			sprintf(displaydata,"Returning record by name current %s ",record.m_filename);
			p_parent->DisplayInfo(displaydata);
			return record;
		}
	
	}

	for (i = 0; i < (int)v_waiting_datfiles.size(); i++)
	{
		strcpy(tempname,"C:\\FastTrack Shared\\");
		strcat(tempname,v_waiting_datfiles[i].m_filename);
		if ((strcmp(filename,tempname)) == 0)
		{
			record = v_waiting_datfiles[i];
			sprintf(displaydata,"Returning record by namewaiting %s ",record.m_filename);
			p_parent->DisplayInfo(displaydata);
			return record;
		}
	
	}

	return record;
}

DatfileRecord DatfileManager::GetRecordFromHash(char * hash)
{
	DatfileRecord record;

	char oldhash[40+1];
	char newhash[40+1];

	
	/*
	strcpy(oldhash,hash);
	AsciiHashtoBinHash(oldhash,newhash);
	*/
	char displaydata[5000];
	record.ClearRecord();

	//sprintf(displaydata,"comparing orighash %s , to type %s", hash, v_current_datfiles[0].m_hash);
	//p_parent->DisplayInfo(displaydata);

	strcpy(newhash,hash);

	for (int i = 0; i < (int)v_current_datfiles.size(); i++)
	{
		strcpy(oldhash,v_current_datfiles[i].m_hash);
		if ((strcmp(newhash,v_current_datfiles[i].m_hash)) == 0)
		{
			record = v_current_datfiles[i];
			sprintf(displaydata,"Returning record by hash current %s ",record.m_filename);
			p_parent->DisplayInfo(displaydata);
			return record;
		}
	
	}
	for (int i = 0; i < (int)v_waiting_datfiles.size(); i++)
	{
		strcpy(oldhash,v_waiting_datfiles[i].m_hash);
		if ((strcmp(newhash,v_waiting_datfiles[i].m_hash)) == 0)
		{
			record = v_waiting_datfiles[i];
			sprintf(displaydata,"Returning record by hash waiting %s ",record.m_filename);
			p_parent->DisplayInfo(displaydata);
			return record;
		}
	
	}



	return record;
}

