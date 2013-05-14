// MovieMakerModule.cpp : implementation file
//

#include "stdafx.h"
#include "MovieMakerModule.h"
#include "giFThash.h"
#include "giFTmd5.h"
#include <time.h>
#include <stdlib.h>
#include "imagehlp.h"
#include "mediamanager.h"
#include <io.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// MovieMakerModule

MovieMakerModule::MovieMakerModule()
{
	
}

MovieMakerModule::~MovieMakerModule()
{
}




void MovieMakerModule::InitParent(MediaManager *parent)
{
	p_parent = parent;	
}

//
//
//
// Take all of the movie info and create the KaZaa database entry
// Old style that edited all the DB's directly
/*
void MovieMakerModule::MakeEntry(MovieInfo info, int filesize, char * outputfolder)
{

	int numkazaa;
	vector<Record> tags;
	RecordHeader header;
	CFile dbfile;
	CFile tempfile;
	CFile delfile;
	CFile tempdb;
	int whichdata; // for determininge which file to use

	char displaydata[1024+1];


	// Kill all of the Kazaa so you can write to the database
	//numkazaa = p_parent->GetNumKazaas(); used to get numkazaa from kazaa launcher but ivan changed
	numkazaa = 99;  // this will copy to all directories possible
	sprintf(displaydata,"The total number of Kazaa is: %i",numkazaa);
	p_parent->DisplayInfo(displaydata);
	p_parent->KillAllKazaa();

	//Begin Setting fixed header fields
	header.Ksig = 0x6C33336C;
	header.unknown = 0;
	header.isShared = 1;
	header.lastshared = 0;
	header.filedate = 0;
	// Set the dependant fields
	header.filesize = filesize;

	char moviename[256+1];

	strcpy(moviename,info.m_filename.c_str());
	MakeSureDirectoryPathExists(outputfolder);
	CopyMovie(moviename, filesize, outputfolder);
//	header.filename = new char[strlen(moviename)+1];
//	header.filedirectory = new char[strlen(outputfolder)+1];

	strcpy(header.filename, info.m_filename.c_str());
	strcpy(header.filedirectory,outputfolder);
	char * ptr = strrchr(header.filedirectory,'\\');
	*ptr = '\0';

	//set the length of the header, add the tag lengths to this later
	header.recordlength = ((sizeof(int) * 5)+(sizeof(bool))+(strlen(moviename)+1)+(strlen(header.filedirectory)+1));

//	newrec.filedate = time.Format(


	// Begin creating all the tags as needed and place them on the vector
	char entry[256+1];
	Record temprec;



	//Length in seconds tag = 5
	int len = info.m_length;
	if (len == 0)
	{
		len = 3600 + rand()%1800;
	}

	len = len + rand()%120;

	temprec.Insert(5,len);
	tags.push_back(temprec);
	temprec.Clear();

	//Artist Tag = 6
	if (info.m_artist.size() != 0)
	{
		strcpy(entry,info.m_artist.c_str());
		temprec.Insert(6,entry);
		tags.push_back(temprec);
		temprec.Clear();
	}

	//Language tag = 10
	temprec.Insert(10,"English");
	tags.push_back(temprec);
	temprec.Clear();

	//Keywords Tag = 12
	if (info.m_keywords.size() != 0)
	{
		strcpy(entry,info.m_keywords.c_str());
		temprec.Insert(12,entry);
		tags.push_back(temprec);
		temprec.Clear();
	}

	//Resolution tag = 13
	temprec.Insert(13,320,240);
	tags.push_back(temprec);
	temprec.Clear();

	//Category Tag = 14
	if (info.m_category.size() != 0)
	{
		strcpy(entry,info.m_category.c_str());
		temprec.Insert(14,entry);
		tags.push_back(temprec);
		temprec.Clear();
	}

	// Type tag = 18
	temprec.Insert(18,"Movie");
	tags.push_back(temprec);
	temprec.Clear();

	//Description Tag = 26
	if (info.m_description.size() != 0)
	{
		strcpy(entry,info.m_description.c_str());
		temprec.Insert(26,entry);
		tags.push_back(temprec);
		temprec.Clear();
	}

	// Codec tag = 28
	temprec.Insert(28,"divx");
	tags.push_back(temprec);
	temprec.Clear();

	//Hash Tag = 3
	char fullpath[256+1];
	strcpy(fullpath, outputfolder);
	strcat(fullpath, header.filename);	
	CalculateHash(fullpath,entry);
	temprec.Insert(3,entry);
	tags.push_back(temprec);
	temprec.Clear();

	// Go through the vector to determine the final size and count
	header.recordcount = 0;
	vector<Record>::iterator record_iter = tags.begin();
	while (record_iter != tags.end())
	{
		header.recordlength += record_iter->GetSize();
		header.recordlength += (sizeof(int) * 2);
		header.recordcount++;
		record_iter++;
	}
	
	// Start Writing Everything
	int padding;

	_finddata_t data;
	long handle=(long)_findfirst("C:\\OnSystems0\\Kazaa Lite K++\\db\\data.tmp",&data);

	// if the tempfile is there get rid of it
	if (handle != -1)
	{
		CFile::Remove("C:\\OnSystems0\\Kazaa Lite K++\\db\\data.tmp");
	}
	// Figure out which DB file we want to use
	MakeSureDirectoryPathExists("C:\\OnSystems0\\Kazaa Lite K++\\db\\");
	if (header.recordlength < 256)
	{	
		try
		{
			CFile::Rename("C:\\OnSystems0\\Kazaa Lite K++\\db\\data256.dbb","C:\\OnSystems0\\Kazaa Lite K++\\db\\data.tmp");
		}
		catch(CFileException* Ex)
		{
			Ex->Delete();
		}
		//padding for the 256 recordlengh is 256 + 8 = 264 // I lied it's 256
		padding = 256 - header.recordlength;
		whichdata = 256;
		if(dbfile.Open("C:\\OnSystems0\\Kazaa Lite K++\\db\\data256.dbb",CFile::modeReadWrite|CFile::modeNoTruncate|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite)==FALSE)
		{
			strcpy(displaydata,"Failed to Open C:\\OnSystems0\\Kazaa Lite K++\\db\\data256.dbb");
			p_parent->DisplayInfo(displaydata);	
			DWORD damnerror = ::GetLastError();
			//p_parent->RelaunchAllKazaa();
			return;
		}
	}

	else 
	{
		try
		{
			CFile::Rename("C:\\OnSystems0\\Kazaa Lite K++\\db\\data1024.dbb","C:\\OnSystems0\\Kazaa Lite K++\\db\\data.tmp");
		}
		catch(CFileException* Ex)
		{
			Ex->Delete();
		}
		//padding for the 1024 recordlengh is 1024 + 8 = 1032// I lied it's 1024
		padding = 1024 - header.recordlength;
		whichdata = 1024;
		if(dbfile.Open("C:\\OnSystems0\\Kazaa Lite K++\\db\\data1024.dbb",CFile::modeReadWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeBinary|CFile::shareDenyWrite)==FALSE)
		{
			strcpy(displaydata,"Failed to Open C:\\OnSystems0\\Kazaa Lite K++\\db\\data1024.dbb");
			p_parent->DisplayInfo(displaydata);	
			DWORD damnerror = ::GetLastError();
			//p_parent->RelaunchAllKazaa();
			return;
		}

	
	}
	MakeSureDirectoryPathExists("C:\\OnSystemsTemp\\");
	handle=(long)_findfirst("C:\\OnSystemsTemp\\datatemp.dbb",&data);

	// if the file exists remove it so we start fresh
	if (handle != -1)
	{
		CFile::Remove("C:\\OnSystemsTemp\\datatemp.dbb");
	}

	_findclose(handle);
	if(tempdb.Open("C:\\OnSystemsTemp\\datatemp.dbb",CFile::modeReadWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeBinary|CFile::shareDenyWrite)==FALSE)
	{
		DWORD damnerror = ::GetLastError();
		//p_parent->RelaunchAllKazaa();
		return;
	}
	dbfile.SeekToBegin();

	//Write Header
	header.WriteHeader(&dbfile);
	header.WriteHeader(&tempdb);

	// Write tags
	record_iter = tags.begin();
	while (record_iter != tags.end())
	{
		record_iter->WriteToFile(&dbfile);
		record_iter->WriteToFile(&tempdb);
		record_iter++;
	}

	// pad the rest of the space reserved with 0's
	char c = 00;
	for (int i = 0; i<padding; i++)
	{
		dbfile.Write(&c,1);
		tempdb.Write(&c,1);
	}

	//Data.tmp holds the original data so copy it back, but it is possible to be blank
	if (tempfile.Open("C:\\OnSystems0\\Kazaa Lite K++\\db\\data.tmp",CFile::modeReadWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeBinary|CFile::shareDenyWrite)==FALSE)
	{
		//p_parent->RelaunchAllKazaa();
		//return;
	}
	
//	ULONGLONG tempfilesize = tempfile.GetLength();
	char  buffer [1024 +1];
	int length;
	//Read the file 1k at a time and write to dest
	while ((length = tempfile.Read(buffer,1024))==1024)
	{
		dbfile.Write(buffer, length);
		tempdb.Write(buffer, length);
	}
		dbfile.Write(buffer, length);
		tempdb.Write(buffer, length);

	tempfile.Close();


	//Copy DB to all directories.
	
	char dbfilename[32+1];
	strcpy(dbfilename,dbfile.GetFileName());
	
	//Clean Up
	dbfile.Close();
	tempdb.Close();

	// copy to all of the database folders
	//CopyDatabase(dbfilename,numkazaa);

	CopyDatabase(dbfilename,numkazaa,"Kazaa");
	CopyDatabase(dbfilename,numkazaa,"Kazaa Lite K++\\db");

	// Resume all of the Kazaa Programs
	//p_parent->RelaunchAllKazaa();


}
*/

void MovieMakerModule::MakeEntry(MovieInfo info, int filesize, char * outputfolder)
{

	vector<Record> tags;
	RecordHeader header;
	CFile dbfile;
	CFile tempfile;
	CFile delfile;
	int whichdata; // for determininge which file to use

	char displaydata[1024+1];


	// Kill all of the Kazaa so you can write to the database
	//numkazaa = p_parent->GetNumKazaas(); used to get numkazaa from kazaa launcher but ivan changed
	p_parent->KillAllKazaa();

	//Begin Setting fixed header fields
	header.Ksig = 0x6C33336C;
	header.unknown = 0;
	header.isShared = 1;
	header.lastshared = 0;
	header.filedate = 0;
	// Set the dependant fields
	header.filesize = filesize;

	char moviename[256+1];

	strcpy(moviename,info.m_filename.c_str());
	MakeSureDirectoryPathExists(outputfolder);
	CopyMovie(moviename, filesize, outputfolder);
//	header.filename = new char[strlen(moviename)+1];
//	header.filedirectory = new char[strlen(outputfolder)+1];

	strcpy(header.filename, info.m_filename.c_str());
	strcpy(header.filedirectory,outputfolder);
	char * ptr = strrchr(header.filedirectory,'\\');
	*ptr = '\0';

	//set the length of the header, add the tag lengths to this later
	header.recordlength = ((sizeof(int) * 5)+(sizeof(bool))+(strlen(moviename)+1)+(strlen(header.filedirectory)+1));

//	newrec.filedate = time.Format(


	// Begin creating all the tags as needed and place them on the vector
	char entry[256+1];
	Record temprec;



	//Length in seconds tag = 5
	int len = info.m_length;
	if (len == 0)
	{
		len = 3600 + rand()%1800;
	}

	len = len + rand()%120;

	temprec.Insert(5,len);
	tags.push_back(temprec);
	temprec.Clear();

	//Artist Tag = 6
	if (info.m_artist.size() != 0)
	{
		strcpy(entry,info.m_artist.c_str());
		temprec.Insert(6,entry);
		tags.push_back(temprec);
		temprec.Clear();
	}

	//Language tag = 10
	temprec.Insert(10,"English");
	tags.push_back(temprec);
	temprec.Clear();

	//Keywords Tag = 12
	if (info.m_keywords.size() != 0)
	{
		strcpy(entry,info.m_keywords.c_str());
		temprec.Insert(12,entry);
		tags.push_back(temprec);
		temprec.Clear();
	}

	//Resolution tag = 13
	temprec.Insert(13,320,240);
	tags.push_back(temprec);
	temprec.Clear();

	//Category Tag = 14
	if (info.m_category.size() != 0)
	{
		strcpy(entry,info.m_category.c_str());
		temprec.Insert(14,entry);
		tags.push_back(temprec);
		temprec.Clear();
	}

	// Type tag = 18
	temprec.Insert(18,"Movie");
	tags.push_back(temprec);
	temprec.Clear();

	//Description Tag = 26
	if (info.m_description.size() != 0)
	{
		strcpy(entry,info.m_description.c_str());
		temprec.Insert(26,entry);
		tags.push_back(temprec);
		temprec.Clear();
	}

	// Codec tag = 28
	temprec.Insert(28,"divx");
	tags.push_back(temprec);
	temprec.Clear();

	//Hash Tag = 3
	char fullpath[256+1];
	strcpy(fullpath, outputfolder);
	strcat(fullpath, header.filename);	
	CalculateHash(fullpath,entry);
	temprec.Insert(3,entry);
	tags.push_back(temprec);
	temprec.Clear();

	// Go through the vector to determine the final size and count
	header.recordcount = 0;
	vector<Record>::iterator record_iter = tags.begin();
	while (record_iter != tags.end())
	{
		header.recordlength += record_iter->GetSize();
		header.recordlength += (sizeof(int) * 2);
		header.recordcount++;
		record_iter++;
	}
	
	// Start Writing Everything
	int padding;

	_finddata_t data;
	long handle=(long)_findfirst("C:\\OnSystems0\\Kazaa Lite K++\\db\\data.tmp",&data);

	// if the tempfile is there get rid of it
	if (handle != -1)
	{
		CFile::Remove("C:\\OnSystems0\\Kazaa Lite K++\\db\\data.tmp");
	}
	// Figure out which DB file we want to use
	MakeSureDirectoryPathExists("C:\\OnSystemsTemp\\data256.dbb");
	if (header.recordlength < 256)
	{	
		//padding for the 256 recordlengh is 256 + 8 = 264 // I lied it's 256
		padding = 256 - header.recordlength;
		whichdata = 256;
		if(dbfile.Open("C:\\OnSystemsTemp\\data256.dbb",CFile::modeReadWrite|CFile::modeNoTruncate|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite)==FALSE)
		{
			strcpy(displaydata,"Failed to Open C:\\OnSystemsTemp\\data256.dbb");
			p_parent->DisplayInfo(displaydata);	
			DWORD damnerror = ::GetLastError();
			//p_parent->RelaunchAllKazaa();
			return;
		}
	}

	else 
	{
		//padding for the 1024 recordlengh is 1024 + 8 = 1032// I lied it's 1024
		padding = 1024 - header.recordlength;
		whichdata = 1024;
		if(dbfile.Open("C:\\OnSystemsTemp\\data1024.dbb",CFile::modeReadWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeBinary|CFile::shareDenyWrite)==FALSE)
		{
			strcpy(displaydata,"Failed to Open C:\\OnSystemsTemp\\data1024.dbb");
			p_parent->DisplayInfo(displaydata);	
			DWORD damnerror = ::GetLastError();
			//p_parent->RelaunchAllKazaa();
			return;
		}

	
	}

	dbfile.SeekToEnd();

	//Write Header
	header.WriteHeader(&dbfile);

	// Write tags
	record_iter = tags.begin();
	while (record_iter != tags.end())
	{
		record_iter->WriteToFile(&dbfile);
		record_iter++;
	}

	// pad the rest of the space reserved with 0's
	char c = 00;
	for (int i = 0; i<padding; i++)
	{
		dbfile.Write(&c,1);
	}
	
	//Clean Up
	dbfile.Close();

}


//
//
//
// Copies the database to however many folders are specified
bool MovieMakerModule::CopyDatabase(char * filename, int numfolders, char * dirname)
{	
	char dirbase[256+1];
	char dirfinal[256+1];
	char original[256+1];
	char num[2+1];
	char buf[1024+1];
	CFile tempfile, dbfile;	
	ULONGLONG tempfilesize;


	strcpy(original,"C:\\OnSystemstemp\\datatemp.dbb");
	//strcat(original,filename);
	strcpy(dirbase,"C:\\OnSystems");
	
	//Debugging
	sprintf(buf,"Copying %s to %i folders",original,numfolders);
	p_parent->DisplayInfo(buf);
	// Kill Kazaa Again just in case one was started up before you got to this point.
	p_parent->KillAllKazaa();
	// Dbfile is the original file to be copied
	if (dbfile.Open(original,CFile::modeRead|CFile::typeBinary|CFile::shareDenyWrite) == FALSE)
	{
		sprintf(buf,"Could not open %s bailing out",original);
		p_parent->DisplayInfo(buf);

		DWORD damnerror = ::GetLastError();
		return 0;
	}		

//	char * buffer = new char[dbfile.GetLength()];
	char buffer[1024+1];
	tempfilesize = dbfile.GetLength();
	// Generate the directory names from 1 to one less than the input number
	// If there are 30 kazaas the directories span 0-29.
	for (int i = 1; i <=numfolders; i++)
	{
		// start with the base onsystems0
		strcpy(dirfinal,dirbase);
		_itoa(i,num,10);
		// Add the number of the folder
		strcat(dirfinal,num);
		strcat(dirfinal,"\\");
		strcat(dirfinal,dirname);
		strcat(dirfinal,"\\");
		strcat(dirfinal,filename);
		//p_parent->KillAllKazaa();
		if (tempfile.Open(dirfinal,CFile::modeRead) == TRUE)
		{
			tempfile.Close();
			CFile::Remove(dirfinal);
		}	
		else
		{
			sprintf(buf,"Could not remove %s, it may not exist",dirfinal);
			p_parent->DisplayInfo(buf);
		}

		MakeSureDirectoryPathExists(dirfinal);
		if (tempfile.Open(dirfinal,CFile::modeReadWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeBinary|CFile::shareDenyWrite)==FALSE)
		{
			sprintf(buf,"Could not Open %s , it may be in use", dirfinal);
			p_parent->DisplayInfo(buf);
			continue;
		}

		else
		{
			dbfile.SeekToBegin();
			int length;
		//Read the file 1k at a time and write to dest
			while ((length = dbfile.Read(buffer,1024))==1024)
			{
			//sprintf(buf,"Copying %i to %s",length,dirfinal);
			//p_parent->DisplayInfo(buf);
				tempfile.Write(buffer, length);
			}
			tempfile.Write(buffer, length);
			tempfile.Close();

			sprintf(buf,"Copied to %s ",dirfinal);
			p_parent->DisplayInfo(buf);
		}
	}

	dbfile.Close();
	return 1;

}

//
//
//
int MovieMakerModule::CopyMovie(char * moviename, int size, char * outputfolder)
{
	CFile infile;
	CFile outfile;
//	int tempsize;
	char outfilename[256+1];
	char buf[1024+1]; 


	strcpy(outfilename, outputfolder);

	// 1 hour blank is the empty movie file
	if (infile.Open("1_hour_blank.avi",CFile::modeRead) == FALSE)
	//if (infile.Open("C:\\cvs\\MDproj\\TrapperKeeper\\debug\\1_hour_blank.avi",CFile::modeRead) == FALSE)
	{
		DWORD damnerror = ::GetLastError();
		strcpy(buf,"Could not open 1_hour_blank.avi");
		p_parent->DisplayInfo(buf);
		return 0;
	}
	strcat(outfilename,moviename);
	MakeSureDirectoryPathExists(outfilename);
	if (outfile.Open(outfilename,CFile::modeWrite|CFile::typeBinary|CFile::modeCreate) == FALSE)
	{
		sprintf(buf,"Could not write %s",outfile);
		p_parent->DisplayInfo(buf);
		DWORD damnerror = ::GetLastError();
		return 0;
	}
	char * buffer[1024+1];
	//Copy the File Exactly
	int length;
	//Read the file 1k at a time and write to dest
	while ((length = infile.Read(buffer,1024))==1024)
	{
		outfile.Write(buffer, length);
	}
	//Change the length
	outfile.SetLength(size);


	infile.Close();
	outfile.Close();

	return 1;
	
}


//
//
//


void MovieMakerModule::CalculateHash(char * outputfilename, char * hashin)
{
		CFile outfile;

		unsigned char hash[20];

		// Read in first 300K
		if(outfile.Open(outputfilename,CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone)==NULL)
		{
			return;
		}

		unsigned char md5_buf[300*1024];
		unsigned int num_read=outfile.Read(md5_buf,sizeof(md5_buf));

		MD5Context md5;
		MD5Init(&md5);
		MD5Update(&md5,&md5_buf[0],num_read);
		MD5Final(hash,&md5);	// first 16 bytes of that hash

		// Calculate the 4-byte small hash.
	    unsigned int smallhash = 0xffffffff;
		unsigned int chunk_size=300*1024;
		unsigned char md5buf[300*1024];

		if(outfile.GetLength() > chunk_size)
		{
			size_t offset = 0x100000;	// 1 MB
			size_t lastpos = chunk_size;	// 300K
			size_t endlen;
			while(offset+2*chunk_size < outfile.GetLength()) 
			{

				outfile.Seek(offset,CFile::begin);
				outfile.Read(md5buf,chunk_size);
	    
				smallhash = hashSmallHash(md5buf, chunk_size, smallhash);
				lastpos = offset+chunk_size;
				offset <<= 1;
			}

			endlen = (size_t)outfile.GetLength() - lastpos;
			if (endlen > chunk_size) 
			{
				endlen = chunk_size;
			}
			
			outfile.Seek(outfile.GetLength() - endlen,CFile::begin);
			outfile.Read(md5buf,(UINT)endlen);

			smallhash = hashSmallHash(md5buf, endlen, smallhash);
		}

		smallhash ^= outfile.GetLength();
		hash[16] = smallhash & 0xff;
		hash[17] = (smallhash >> 8) & 0xff;
		hash[18] = (smallhash >> 16) & 0xff;
		hash[19] = (smallhash >> 24) & 0xff;

		char temphash[20+1];
		for (int i = 0; i <20; i++)
		{
			temphash[i] = hash[i];
		}
		temphash[20] = '\0';
		memcpy(hashin,temphash,20+1);
		int test = (int)strlen(hashin);
}


//
//
//

// Processing the template file also sets the info for the final movie
bool MovieMakerModule::ProcessTemplateFile(char * filepath, MovieInfo * info)
{
	CStdioFile templatefile;
	char buf[4096+1];
	string template_filename;

	template_filename = filepath;
	template_filename += "template.mak";

	if (templatefile.Open(template_filename.c_str(),CFile::typeText|CFile::modeRead|CFile::shareDenyWrite)==FALSE)
	{
		return false;
	}

	//loop to read in and process each line.
	while(templatefile.ReadString(buf,sizeof(buf)-1)!=FALSE)
	{
		ProcessTemplate(buf, info);
	}
	GenerateFilename(info);

	// Pick a random description to tag onto the end of the must have part of the description
	int randdesc = rand()%(int)v_descriptions.size();
	info->m_description += v_descriptions[randdesc];

	int randartist = rand()%(int)v_artists.size();
	info->m_artist = v_artists[randartist];

	// Clear all of the vectors to prepare them to be used again
	v_descriptions.clear();;
	v_tags.clear();
	v_types.clear();
	v_artists.clear();

	return true;
}
//
//
//

// Add any mixing schemes we want to generate filenames using the data from the template
void MovieMakerModule::GenerateFilename(MovieInfo * info)
{
	int randnum;
	int rand2;
	string tempstring;
	char buf[256+1];

	randnum = rand()%6;
	// mix and match various naming schemes
	switch (randnum)
	{
		// normal tag, movie, type
		case 0:
		{
			rand2 = rand()%(int)v_tags.size();
			tempstring = v_tags[rand2];
			tempstring += " ";
			tempstring += info->m_filename;
			tempstring += " ";
			rand2 = rand()%(int)v_types.size();
			tempstring += v_types[rand2];
			break;
		}
		// normal movie, tag, type
		case 1:
		{
			tempstring = info->m_filename;
			tempstring += " ";
			rand2 = rand()%(int)v_tags.size();
			tempstring += v_tags[rand2];
			tempstring += " ";
			rand2 = rand()%(int)v_types.size();
			tempstring += v_types[rand2];
			break;
		}
		//lower tag, movie, type
		case 2:
		{
			rand2 = rand()%(int)v_tags.size();
			strcpy(buf, v_tags[rand2].c_str());
			strlwr(buf);
			tempstring = buf;
			tempstring += " ";
			strcpy(buf, info->m_filename.c_str());
			strlwr(buf);
			tempstring += buf;
			tempstring += " ";
			rand2 = rand()%(int)v_types.size();
			strcpy(buf, v_types[rand2].c_str());
			strlwr(buf);
			tempstring += buf;
			break;
		}
		// lower movie, tag, type
		case 3:
		{
			strcpy(buf, info->m_filename.c_str());
			strlwr(buf);
			tempstring = buf;
			tempstring += " ";
			rand2 = rand()%(int)v_tags.size();
			strcpy(buf, v_tags[rand2].c_str());
			strlwr(buf);
			tempstring += buf;
			tempstring += " ";
			rand2 = rand()%(int)v_types.size();
			strcpy(buf, v_types[rand2].c_str());
			strlwr(buf);
			tempstring += buf;
			break;
		}
		//upper tag, movie, type
		case 4:
		{
			rand2 = rand()%(int)v_tags.size();
			strcpy(buf, v_tags[rand2].c_str());
			strupr(buf);
			tempstring = buf;
			tempstring += " ";
			strcpy(buf, info->m_filename.c_str());
			strupr(buf);
			tempstring += buf;
			tempstring += " ";
			rand2 = rand()%(int)v_types.size();
			strcpy(buf, v_types[rand2].c_str());
			strupr(buf);
			tempstring += buf;
			break;
		}
		// lower movie, tag, type
		case 5:
		{
			strcpy(buf, info->m_filename.c_str());
			strlwr(buf);
			tempstring = buf;
			tempstring += " ";
			rand2 = rand()%(int)v_tags.size();
			strcpy(buf, v_tags[rand2].c_str());
			strlwr(buf);
			tempstring += buf;
			tempstring += " ";
			rand2 = rand()%(int)v_types.size();
			strcpy(buf, v_types[rand2].c_str());
			strlwr(buf);
			tempstring += buf;
			break;
		}
		default :
		{
			tempstring = "blank";
			break;
		}
	}
	// Add avi to the end of the final movie name and put it in the movie info
	tempstring += ".avi";
	info->m_filename = tempstring;
}

//
//
//

bool MovieMakerModule::ProcessTemplate(char * buf, MovieInfo * info)
{

	CString bufstring;
	string tempstring;
	// check each line for the template tag and put the data into the correct spot
	// Fill the vectors for the randomized parts first
	if (strstr(_strlwr(buf),"<description>"))
	{
		bufstring = buf + strlen("<description>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		tempstring = bufstring;
		v_descriptions.push_back(tempstring);
		return 1;
	}
	else if (strstr(_strlwr(buf),"<tag>"))
	{
		bufstring = buf + strlen("<tag>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		tempstring = bufstring;
		v_tags.push_back(tempstring);
		return 1;
	}
	else if (strstr(_strlwr(buf),"<type>"))
	{
		bufstring = buf + strlen("<type>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		tempstring = bufstring;
		v_types.push_back(tempstring);
		return 1;
	}
	// set the individualized template parts, should only be one of each of these tags
	else if (strstr(_strlwr(buf),"<filename>"))
	{
		bufstring = buf + strlen("<filename>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		info->m_filename = bufstring;
		return 1;
	}
	else if (strstr(_strlwr(buf),"<artist>"))
	{
		bufstring = buf + strlen("<artist>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		tempstring = bufstring;
		v_artists.push_back(tempstring);
		return 1;
	}
	else if (strstr(_strlwr(buf),"<category>"))
	{
		bufstring = buf + strlen("<category>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		info->m_category = bufstring;
		return 1;
	}
	else if (strstr(_strlwr(buf),"<keywords>"))
	{
		bufstring = buf + strlen("<keywords>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		info->m_keywords = bufstring;
		return 1;
	}
	// Must have words are words that will be in the description of every copy of the movie
	else if (strstr(_strlwr(buf),"<musthave>"))
	{
		bufstring = buf + strlen("<musthave>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		info->m_description = bufstring;
		return 1;
	}
	else if (strstr(_strlwr(buf),"<length>"))
	{
		bufstring = buf + strlen("<length>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		int templen = atoi(bufstring);
		info->m_length = templen;
		return 1;
	}
	// if the line didn't contain a template 
	return 0;
}


// Make Directory takes an input and output folder and creates the template found 
// in the input folder and copies the blank movie to the output folder
void MovieMakerModule::MakeDirectory(char * inputfolder, char * outputfolder)
{
	int filesize;

	srand((unsigned int)time(NULL));
	MovieInfo info;
	ProcessTemplateFile(inputfolder, &info);
	// make the filesize randomly between 200-600k
	filesize = 204857600;  // 200k or so
	int randval=rand() | (rand()<<16);	// make 32-bit rand (sort of...the bits will be 0xxx:xxxx:0xxxx:xxxx with the x's being random)
	int delta=(randval % (404857600));	// delta = rand within 6 meg or so
	filesize += delta;
	filesize += (137 - (filesize%137)); 	// Make the filesize into a multiple of 137
	MakeEntry(info,filesize,outputfolder);

	// Prepare the file association
	char inputfa[256+1];
	char outputfa[256+1];
	strcpy(inputfa,inputfolder);
	strcat(inputfa,"template.mak");
	strcpy(outputfa,outputfolder);
	strcat(outputfa,info.m_filename.c_str());
	// Create the file association
	if(p_parent->AddFileAssociation(inputfa, outputfa) == false)
	{
		//We couldn't save the file association sadly, so delete the output
		//CFile::Remove(outputfa);
	}


}





