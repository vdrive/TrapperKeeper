#include "StdAfx.h"
#include "filemanager.h"
#include <io.h>
#include "imagehlp.h"
#include "gifthash.h"
#include "giftmd5.h"
#include"../KazaaSwarmerDownloader/DatfileRecord.h"

FileManager::FileManager(void)
{
}

FileManager::~FileManager(void)
{
}

void FileManager::ReadMapsFile()
{
	CFile list;
	FileEntry fa;

	v_old_file_list.clear();

	//  read each line of the file associations and place it onto the  vector.
	if( list.Open("swarmer_source_maps.dat", CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone) == TRUE )
	{
		while(list.Read(&fa, sizeof(fa)) != 0)
		{
			v_old_file_list.push_back(fa);

		}
		list.Close();
	}

}

bool FileManager::CompareHashWithMaps(FileEntry entry)
{
	vector<FileEntry>::iterator file_iter = v_old_file_list.begin();
	char iterhash[256];

	while (file_iter != v_old_file_list.end())
	{
		strcpy(iterhash,file_iter->m_hash);
		if ((strcmp(entry.m_hash,iterhash)) == 0)
		{
			v_old_file_list.erase(file_iter);
			return true;
		}
		file_iter++;
	}
	return false;
}

void FileManager::RemoveLeftoverFiles()
{

	char displaydata[1024+1];

	strcpy(displaydata,"Checking for file Deletion");
	//m_dlg.DisplayInfo(displaydata);


	//ReadMapsFile();

	FileEntry cur_entry;
	_finddata_t data;

	char filename[256];
	// first two found will be . and ..
	// Change to Explore Folder recursive with C:\syncher\file deposit\distribute\kazaa_swarm\ as the base
	vector<string> filenames;
	ExploreFolder("C:\\syncher\\file deposit\\distribute\\kazaa_swarm\\",&filenames);
/*
	_findnext(handle,&data);
*/
	int found;
	char oldfilename[256+1];
	char newfilename[256+1];
	int num_found = 0;
	ReadMapsFile();

	for (int i = 0; i < (int)filenames.size(); i++)
	{

		//strcpy(filename,"C:\\syncher\\file deposit\\distribute\\kazaa_swarm\\");
		strcpy(filename,filenames[i].c_str());
		intptr_t handle = _findfirst(filename,&data);
		sprintf(displaydata,"Checking: %s",filename);
		//m_dlg.DisplayInfo(displaydata);

		strcpy(cur_entry.m_filename,data.name);
		GetHashFromDeflated(filename,cur_entry.m_hash);
		cur_entry.m_size = data.size;

		found = 0;

		FileEntry entry;
		found = GetEntryFromHashAndRemove(cur_entry.m_hash, &entry);

		/*
		for (int j = 0; j < (int)v_old_file_list.size(); j++)
		{
			char hash[40+1];
			strcpy(hash,v_old_file_list[j].m_hash);
			if ((strcmp(v_old_file_list[j].m_hash,cur_entry.m_hash)) == 0)
			{
				found = true;
				num_found++;
				break;
			}
		}
		*/
		if (found == 0)
		{
			// don't do anything till this is fixed
			//strcpy(oldfilename,"C:\\syncher\\src\\Swarmer.new\\");
			strcpy(oldfilename,cur_entry.m_filename);
			strcpy(newfilename,"C:\\SwarmerCache\\");
			strcat(newfilename,cur_entry.m_filename);
			MakeSureDirectoryPathExists(newfilename);
			rename(filename,newfilename);
			sprintf(displaydata,"Moving: %s",newfilename);
		

		}
		_findclose(handle);
	}


}

void FileManager::GetHashFromDeflated(char * inputfilename, char * hashin)
{
	CFile file_in;
	if(file_in.Open(inputfilename,CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone)==NULL)
	{
		return;
	}

	FileEntry record;
	int size;
	int len;

	file_in.Read(&size,sizeof(int));
	file_in.Read(&record,sizeof(FileEntry));
	if (strlen(record.m_hash) > 0)
	{
		len = sizeof(record.m_hash);
		memcpy(hashin,record.m_hash,sizeof(record.m_hash));
	}
	else 
	{
		strcpy(hashin,"notfound");
	}
}

void FileManager::GetHash(char * inputfilename, char * hashin)
{


	CFile file_in;
	if(file_in.Open(inputfilename,CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone)==NULL)
	{
		return;
	}

	unsigned char hash[20];

		// Read in first 300K
	unsigned char md5_buf[300*1024];
	unsigned int num_read=file_in.Read(md5_buf,sizeof(md5_buf));

	MD5Context md5;
	MD5Init(&md5);
	MD5Update(&md5,&md5_buf[0],num_read);
	MD5Final(hash,&md5);	// first 16 bytes of that hash
	
	// Stolen code, begin

		// Calculate the 4-byte small hash.
	unsigned int smallhash = 0xffffffff;
	unsigned int chunk_size=300*1024;
	unsigned char buf[300*1024];

	if(file_in.GetLength() > chunk_size)
	{
		size_t offset = 0x100000;	// 1 MB
		size_t lastpos = chunk_size;	// 300K
		size_t endlen;
		while(offset+2*chunk_size < file_in.GetLength()) 
		{
/*			
				if (fseek(fp, offset, SEEK_SET) < 0 || fread(filebuf, 1, chunk_size, fp) < chunk_size) 
				{
					free(filebuf);	
					return -1;
			    }
*/
			file_in.Seek(offset,CFile::begin);
			file_in.Read(buf,chunk_size);
	    
			smallhash = hashSmallHash(buf, chunk_size, smallhash);
			lastpos = offset+chunk_size;
			offset <<= 1;
		}

		endlen = (int)file_in.GetLength() - lastpos;
		if (endlen > chunk_size) 
		{
			endlen = chunk_size;
		}
/*
			if (fseek(fp, fdp->filesize - endlen, SEEK_SET) < 0 ||	fread(filebuf, 1, endlen, fp) < endlen) 
			{
				free(filebuf);
				return -1;
			}
*/			
		file_in.Seek(file_in.GetLength() - endlen,CFile::begin);
		file_in.Read(buf,(int)endlen);

		smallhash = hashSmallHash(buf, endlen, smallhash);
	}

	smallhash ^= file_in.GetLength();
	hash[16] = smallhash & 0xff;
	hash[17] = (smallhash >> 8) & 0xff;
	hash[18] = (smallhash >> 16) & 0xff;
	hash[19] = (smallhash >> 24) & 0xff;

// Stolen code - end

		// Print the hash out
	memset(buf,0,sizeof(buf));
	for(int i=0;i<sizeof(hash);i++)
	{
		sprintf((char *)&buf[i*2],"%02x",((int)hash[i])&0xFF);
	}

	memcpy(hashin,buf,40);
	file_in.Close();
	

}

void FileManager::ResetMap()
{
	ReadMapsFile();
	remove("swarmer_source_maps.dat");
}

int FileManager::GetEntryFromHash(char * hash, FileEntry * entry)
{
	ReadMapsFile();
	FileEntry tempentry;
	for (int i = 0; i< (int)v_old_file_list.size(); i++)
	{
		char maphash[41];
		tempentry = v_old_file_list[i];
		strcpy(maphash,v_old_file_list[i].m_hash);
		if ((strcmp(hash,maphash)) == 0)
		{
			*entry = v_old_file_list[i];
			return 1;
		}
	}

	return 0;
}

int FileManager::GetEntryFromHashAndRemove(char * hash, FileEntry * entry)
{

	FileEntry tempentry;
	vector <FileEntry>::iterator fileiter = v_old_file_list.begin();
	while (fileiter != v_old_file_list.end())
	{
		char maphash[41];
		strcpy(maphash,fileiter->m_hash);
		if ((strcmp(hash,maphash)) == 0)
		{
			*entry = *fileiter;
			v_old_file_list.erase(fileiter);
			return 1;
		}
		fileiter++;
	}

	return 0;
}


int FileManager::ExploreFolder(char * folder, vector<string>  *filenames)
{
	char buf[1024];
	strcpy(buf,folder);
	strcat(buf,"*.*");
	string tempname;
	int numfiles = 0;
	
	_finddata_t data;
	long handle=(long)_findfirst(buf,&data);

	if(handle==-1)
	{
		return (0);
	}

	if(data.attrib & _A_SUBDIR)
	{
		if((strcmp(data.name,".")!=0) && (strcmp(data.name,"..")!=0))
		{
			tempname = folder;
			tempname +=data.name;
			tempname += "/";

			numfiles += ExploreFolder((char *)tempname.c_str(),filenames);
			data.size;

		}
	}
	else
	{
		tempname = folder;
		tempname += data.name;

		filenames->push_back(tempname);
		numfiles++;
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

				numfiles += ExploreFolder((char * )tempname.c_str(),filenames);
				data.size;

			}
		}

		else
		{
			tempname = folder;
			tempname += data.name;

			filenames->push_back(tempname);
			numfiles++;
		}
	}
	_findclose(handle);
	return(numfiles);
}