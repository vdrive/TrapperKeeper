#include "StdAfx.h"
#include "dbwriter.h"
#include "RecordHeader.h"
#include "Record.h"
#include <io.h>
#include "imagehlp.h"
#include "giFThash.h"
#include "giFTmd5.h"


DbWriter::DbWriter(void)
{
}

DbWriter::~DbWriter(void)
{
}

void DbWriter::WriteDbFile(vector<FileMetaData> files)
{

	for (int i=0;i<(int)files.size(); i++)
	{
		WriteKnownFile(files[i]);

	}
}

void DbWriter::RetrieveEntry(byte * hash)
{

	CFile file;
	FileMetaData mdata;

}
void DbWriter::WriteMovieFile(FileMetaData metadata, RecordHeader header)
{
	vector<Record> tags;
	ReadInMovieMetadata();
	
	//Find out which metadata to use
	{
		for(int i =0; i< (int)v_movie_entries.size(); i++)
		{
			if(strcmp(header.filename,v_movie_entries[i].m_file_name.c_str())==0)
			{
				//memcpy(&metadata,&v_movie_entries[i],sizeof(FileMetaData));
				metadata = v_movie_entries[i];
				break;
			}
		}
	}

	// Create all the tags
	Record temprec;
	char entry[4096+1];

	// tag 3 = hash
	memcpy(entry,metadata.m_hash,20);
	entry[20] = '\0';
	temprec.Insert(3,entry);
	tags.push_back(temprec);
	temprec.Clear();

//Length in seconds tag = 5
	int len = 3600 + rand()%1800;
	len = len + rand()%120;
	temprec.Insert(5,len);
	tags.push_back(temprec);
	temprec.Clear();

	//Artist Tag = 6
	
	if (metadata.m_artist.size() != 0)
	{
		strcpy(entry,metadata.m_artist.c_str());
		temprec.Insert(6,entry);
		tags.push_back(temprec);
		temprec.Clear();
	}

	//Language tag = 10
	temprec.Insert(10,"English");
	tags.push_back(temprec);
	temprec.Clear();

	//Keywords Tag = 12
	if (metadata.m_keywords.size() != 0)
	{
		strcpy(entry,metadata.m_keywords.c_str());
		temprec.Insert(12,entry);
		tags.push_back(temprec);
		temprec.Clear();
	}

	//Resolution tag = 13
	temprec.Insert(13,320,240);
	tags.push_back(temprec);
	temprec.Clear();

	//Category Tag = 14
	if (metadata.m_category.size() != 0)
	{
		strcpy(entry,metadata.m_category.c_str());
		temprec.Insert(14,entry);
		tags.push_back(temprec);
		temprec.Clear();
	}

	// Type tag = 18
	temprec.Insert(18,"Movie");
	tags.push_back(temprec);
	temprec.Clear();

	//Description Tag = 26
	if (metadata.m_comment.size() != 0)
	{
		strcpy(entry,metadata.m_comment.c_str());
		temprec.Insert(26,entry);
		tags.push_back(temprec);
		temprec.Clear();
	}

	// Codec tag = 28
	temprec.Insert(28,"divx");
	tags.push_back(temprec);
	temprec.Clear();


	//get the final size
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
	CFile dbfile;

	// Figure out which DB file we want to use
	MakeSureDirectoryPathExists("C:\\OnSystems0\\Kazaa Lite K++\\db\\");

	if (header.recordlength < 256)
	{	
		padding = 256 - header.recordlength;
		if(dbfile.Open("C:\\OnSystems0\\Kazaa Lite K++\\db\\data256.dbb",CFile::modeReadWrite|CFile::modeNoTruncate|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite)==FALSE)
		{
			DWORD damnerror = ::GetLastError();
			return;
		}
	}

	else if (header.recordlength < 1024)
	{

		//padding for the 1024 recordlengh is 1024 
		padding = 1024 - header.recordlength;
		if(dbfile.Open("C:\\OnSystems0\\Kazaa Lite K++\\db\\data1024.dbb",CFile::modeReadWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeBinary|CFile::shareDenyWrite)==FALSE)
		{
			DWORD damnerror = ::GetLastError();
			return;
		}

	
	}
	else
	{

		//padding for the 4096 recordlengh 
		padding = 4096 - header.recordlength;
		if(dbfile.Open("C:\\OnSystems0\\Kazaa Lite K++\\db\\data4096.dbb",CFile::modeReadWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeBinary|CFile::shareDenyWrite)==FALSE)
		{

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

}


void DbWriter::WriteKnownFile(FileMetaData metadata)
{
	RecordHeader header;
	vector<Record> tags;
	//Init the header
	header.Ksig = 0x6C33336C;
	header.unknown = 0;
	header.isShared = 1;
	header.lastshared = 0;
	header.filedate = 0;

	strcpy(header.filename,metadata.m_file_name.c_str());
	strcpy(header.filedirectory,metadata.m_folder.c_str());
	header.filesize = metadata.m_file_size;

	header.recordlength = ((sizeof(int) *5) + sizeof(bool)) + (strlen(header.filename) +1) + (strlen(header.filedirectory) +1);

	if (metadata.mb_movie == true)
	{
		WriteMovieFile(metadata,header);
		return;
	}

	// Create all the tags
	Record temprec;
	char entry[4096+1];

	// tag 3 = hash
	memcpy(entry,metadata.m_hash,20);
	entry[20] = '\0';
	temprec.Insert(3,entry);
	tags.push_back(temprec);
	temprec.Clear();

	// tag 4 = title
	strcpy(entry,metadata.m_title.c_str());
	temprec.Insert(4,entry);
	tags.push_back(temprec);
	temprec.Clear();

	// tag 6 = Artist
	strcpy(entry,metadata.m_artist.c_str());
	temprec.Insert(6,entry);
	tags.push_back(temprec);
	temprec.Clear();

	// tag 8 = Album
	strcpy(entry,metadata.m_album.c_str());
	temprec.Insert(8,entry);
	tags.push_back(temprec);
	temprec.Clear();

	// tag 12 = Keywords
	strcpy(entry,metadata.m_keywords.c_str());
	temprec.Insert(12,entry);
	tags.push_back(temprec);
	temprec.Clear();

	// tag 14 = Catagory
	strcpy(entry,metadata.m_category.c_str());
	temprec.Insert(14,entry);
	tags.push_back(temprec);
	temprec.Clear();

	// tag 21 quality
	int intentry = metadata.m_quality;
	temprec.Insert(21,intentry);
	tags.push_back(temprec);
	temprec.Clear();
	

	// tag 26 = Comment
	strcpy(entry,metadata.m_comment.c_str());
	temprec.Insert(26,entry);
	tags.push_back(temprec);
	temprec.Clear();

	// tag 53 integrity
	intentry = rand()%3;
	temprec.Insert(53,intentry);
	tags.push_back(temprec);
	temprec.Clear();

	//get the final size
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
	CFile dbfile;

	// Figure out which DB file we want to use
	MakeSureDirectoryPathExists("C:\\OnSystems0\\Kazaa Lite K++\\db\\");

	if (header.recordlength < 256)
	{	
		padding = 256 - header.recordlength;
		if(dbfile.Open("C:\\OnSystems0\\Kazaa Lite K++\\db\\data256.dbb",CFile::modeReadWrite|CFile::modeNoTruncate|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite)==FALSE)
		{
			DWORD damnerror = ::GetLastError();
			return;
		}
	}

	else if (header.recordlength < 1024)
	{

		//padding for the 1024 recordlengh is 1024 
		padding = 1024 - header.recordlength;
		if(dbfile.Open("C:\\OnSystems0\\Kazaa Lite K++\\db\\data1024.dbb",CFile::modeReadWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeBinary|CFile::shareDenyWrite)==FALSE)
		{
			DWORD damnerror = ::GetLastError();
			return;
		}

	
	}
/*
	else if (header.recordlength < 2048)
	{

		//padding for the 2048 recordlengh 
		padding = 2048 - header.recordlength;
		if(dbfile.Open("C:\\OnSystems0\\Kazaa Lite K++\\db\\data2048.dbb",CFile::modeReadWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeBinary|CFile::shareDenyWrite)==FALSE)
		{

			DWORD damnerror = ::GetLastError();
			//p_parent->RelaunchAllKazaa();
			return;
		}

	
	}
*/
	else
	{

		//padding for the 4096 recordlengh 
		padding = 4096 - header.recordlength;
		if(dbfile.Open("C:\\OnSystems0\\Kazaa Lite K++\\db\\data4096.dbb",CFile::modeReadWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeBinary|CFile::shareDenyWrite)==FALSE)
		{

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
}

void DbWriter::TestWriting()
{

	srand((unsigned int)time(NULL));

	RecordHeader header;
	FileMetaData metadata;

	vector<FileMetaData> datavector;


	vector<string> filenames;
	ExploreFolder("C:\\FastTrack Shared\\",&filenames);
	_finddata_t data;
	long handle;

	//Begin Setting fixed header fields
	for (int i = 0; i< (int)filenames.size(); i++)

	{
		handle=(long)_findfirst(filenames[i].c_str(),&data);
		char fullpath[256];
		strcpy(fullpath,filenames[i].c_str());

		char hashin[20+1];

		CalculateHash(fullpath,hashin);
		memcpy(metadata.m_hash,hashin,20);

		char *ptr;
		ptr = strrchr(fullpath,'\\');
		ptr++;
		metadata.m_file_name = ptr;
		ptr --;
		*ptr = '\0';
		metadata.m_folder = fullpath;
		metadata.m_file_size = data.size;

		metadata.m_artist = "BLAHBLAH";
		metadata.m_album = "BADALBUM";
//		metadata.m_artist ="daad askldufh alf uoe lasuoef hl liausehf laiu liuseiy al if47 8aldu fkli7 laijkhd liyua laiuef liauhf o9a 8ailsud iuylhga eoioail hfldiu ao9ie7f oli3 alkjd flkia7e oai fliasu fo87a iseiu fkaw7yig oasid fkaeyg7afk8i lka s7ifg 47oas d aifue7 oa87sw fo387 as8e dfgo8374 gao8f ge87 blahblah abba askldufh alf uoe lasuoef hl liausehf laiu liuseiy al if47 8aldu fkli7 laijkhd liyua laiuef liauhf o9a 8ailsud iuylhga eoioail hfldiu ao9ie7f oli3 alkjd";
//		metadata.m_album ="eaae askldufh alf uoe lasuoef hl liausehf laiu liuseiy al if47 8aldu fkli7 laijkhd liyua laiuef liauhf o9a 8ailsud iuylhga eoioail hfldiu ao9ie7f oli3 alkjd flkia7e oai fliasu fo87a iseiu fkaw7yig oasid fkaeyg7afk8i lka s7ifg 47oas d aifue7 oa87sw fo387 as8e dfgo8374 gao8f ge87 blahblah abba askldufh alf uoe lasuoef hl liausehf laiu liuseiy al if47 8aldu fkli7 laijkhd liyua laiuef liauhf o9a 8ailsud iuylhga eoioail hfldiu ao9ie7f oli3 alkjd";

		metadata.m_category = "Soft Rock";

		metadata.m_comment = "baab askldufh alf uoe lasuoef hl liausehf laiu liuseiy al if47 8aldu fkli7 laijkhd liyua laiuef liauhf o9a 8ailsud iuylhga eoioail hfldiu ao9ie7f oli3 alkjd flkia7e oai fliasu fo87a iseiu fkaw7yig oasid fkaeyg7afk8i lka s7ifg 47oas d aifue7 oa87sw fo387 as8e dfgo8374 gao8f ge87 blahblah abba askldufh alf uoe lasuoef hl liausehf laiu liuseiy al if47 8aldu fkli7 laijkhd liyua laiuef liauhf o9a 8ailsud iuylhga eoioail hfldiu ao9ie7f oli3 alkjd 1234567890 1234567890";
		metadata.m_keywords = "abba askldufh alf uoe lasuoef hl liausehf laiu liuseiy al if47 8aldu fkli7 laijkhd liyua laiuef liauhf o9a 8ailsud iuylhga eoioail hfldiu ao9ie7f oli3 alkjd flkia7e oai fliasu fo87a iseiu fkaw7yig oasid fkaeyg7afk8i lka s7ifg 47oas d aifue7 oa87sw fo387 as8e dfgo8374 gao8f ge87 blahblah abba askldufh alf uoe lasuoef hl liausehf laiu liuseiy al if47 8aldu fkli7 laijkhd liyua laiuef liauhf o9a 8ailsud iuylhga eoioail hfldiu ao9ie7f oli3 alkjd";

//		metadata.m_keywords = "baab askldufh";
/*
		char keywordses[4096+1];

		strcpy(keywordses,"baab askldufh alf uoe lasuoef hl liausehf laiu liuseiy al if47 8aldu fkli7 laijkhd liyua laiuef liauhf o9a 8ailsud iuylhga eoioail hfldiu ao9ie7f oli3 alkjd flkia7e oai fliasu fo87a iseiu fkaw7yig oasid fkaeyg7afk8i lka s7ifg 47oas d aifue7 oa87sw fo387 as8e dfgo8374 gao8f ge87 3 bleh 123456789012345678901234567890 123456789012345678901234567890 123456789012345678901234567890 123456789012345678901234567890 123456789012345678901234567890 oh12345678901234567890 baab  hfldiu ao9ie7f oli3 alkjd flkia7e oai fliasu fo87a");
		strcat(keywordses,"askldufh alf uoe lasuoef hl liausehf laiu liuseiy al if47 8aldu fkli7 laijkhd liyua laiuef liauhf o9a 8ailsud iuylhga eoioail hfldiu ao9ie7f oli3 alkjd flkia7e oai fliasu fo87a iseiu fkaw7yig oasid fkaeyg7afk8i lka s7ifg 47oas d aifue7 oa87 baab askldufh alf uoe lasuoef hl liausehf laiu liuseiy al if47 8aldu fkli7 laijkhd liyua laiuef liauhf o9a 8ailsud iuylhga eoioail hfldiu ao9ie7f oli3 alkjd flkia7e oai fliasu fo87a iseiu fkaw7yig oasid fkaeyg7afk8i lka s7ifg 47oas d aifue7 oa87sw fo387 as8e dfgo8374 gao8f ge87 3 bleh 123456789012345678901234567890 123456789012345678901234567890 123456789012345678901234567890 123456789012345678901234567890 123456789012345678901234567890 oh12345678901234567890 baab askldufh alf uoe lasuoef hl liausehf laiu liuseiy al if47 8aldu fkli7 laijkhd liyua laiuef liauhf o9a 8ailsud iuylhga eoioail hfldiu ao9ie7f oli3 alkjd flkia7e oai fliasu fo87a iseiu fkaw7yig oasid fkaeyg7afk8i lka s7ifg 47oas d aifue7 oa87 baab askldufh alf uoe lasuoef hl liausehf laiu liuseiy al if47 8aldu fkli7 laijkhd liyua laiuef liauhf o9a 8ailsud iuylhga eoioail hfldiu ao9ie7f oli3 alkjd flkia7e oai fliasu fo87a iseiu fkaw7yig oasid fkaeyg7afk8i lka s7ifg 47oas d aifue7 oa87sw fo387 as8e dfgo8374 gao8f ge87 3 bleh 123456789012345678901234567890 123456789012345678901234567890 123456789012345678901234567890 123456789012345678901234567890 123456789012345678901234567890 oh12345678901234567890 baab askldufh alf uoe lasuoef hl liausehf laiu liuseiy al if47 8aldu fkli7 laijkhd liyua laiuef liauhf o9a 8ailsud iuylhga eoioail");
		metadata.m_keywords = keywordses;
*/
		metadata.m_quality = 128;
		metadata.m_title = "The Song Name";
		metadata.mb_movie = true;


		datavector.push_back(metadata);
		
	}

	WriteDbFile(datavector);

}

void DbWriter::ExploreFolder(char * folder, vector<string>  *filenames)
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
			tempname += "\\";

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
				tempname += "\\";

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

void DbWriter::CalculateHash(char * outputfilename, char * hashin)
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
}


void DbWriter::ReadInMovieMetadata()
{
	FileMetaData data;
	CStdioFile dbfile;

	int end = 0;
	v_movie_entries.clear();

	if(dbfile.Open("C:\\OnSystemsTemp\\data1024.dbb",CFile::modeRead|CFile::typeBinary|CFile::shareDenyWrite)==TRUE)
	{
		while (end != 1)
		{
			end = ReadEntry(&dbfile,&data,1024);
			if (end !=1)
			{
				v_movie_entries.push_back(data);
			}
		}
		dbfile.Close();
	}
	else
	{
		DWORD damnerror = ::GetLastError();
	}


	if(dbfile.Open("C:\\OnSystemsTemp\\data256.dbb",CFile::modeRead|CFile::typeBinary|CFile::shareDenyWrite)==TRUE)
	{
		while (end != 1)
		{
			end = ReadEntry(&dbfile,&data,256);
			if (end !=1)
			{
				v_movie_entries.push_back(data);
			}
		}
		dbfile.Close();
	}
	else
	{
		DWORD damnerror = ::GetLastError();
	}

}

int DbWriter::ReadEntry(CStdioFile * dbfile, FileMetaData * mdata, int entrysize)
{

	int Ksig;
	int recordlength;
	char  filename[256+1];
	char  filedirectory[256+1];
	int size;
	int filedate;
	int lastshared;
	int unknown;
	bool isShared;
	int recordcount;	
	char trash[2048+1];

	char hash[20+1];

	// count of where we are in the entry
	int actualcount = 0;
	int i;
	actualcount += dbfile->Read(&Ksig,sizeof(int));
	//If there is nothing we're at the end of the file
	//Will also check for l33l here later
	if (actualcount == 0)
	{
		return 1;
	}
	// Find out what the record length is
	actualcount += dbfile->Read(&recordlength,sizeof(int));
	i = 0;
	// Get the file name
	while (actualcount += dbfile->Read(&filename[i],1))
	{
		if (filename[i] == '\0')
			break;
		i++;
	}
	mdata->m_file_name = filename;
	i = 0;
	while (actualcount += dbfile->Read(&filedirectory[i],1))
	{
		if (filedirectory[i] == '\0')
			break;
		i++;
	}
	mdata->m_folder = filedirectory;

	int recordlengthtest = ((sizeof(int) * 6)+(sizeof(bool))+(strlen(filename)+1)+(strlen(filedirectory)+1));
	// Read in each of the header fields.
	actualcount += dbfile->Read(&size,sizeof(DWORD));
	mdata->m_file_size = size;
	actualcount += dbfile->Read(&filedate,sizeof(DWORD));
	actualcount += dbfile->Read(&lastshared,sizeof(DWORD));
	actualcount += dbfile->Read(&unknown,sizeof(DWORD));
	actualcount += dbfile->Read(&isShared,sizeof(bool));
	actualcount += dbfile->Read(&recordcount,sizeof(DWORD));

	int tag;
	char entry[256];
	int tagsize;
	int other;
	int other2;
	int j = 0;
	for (i = 0; i<recordcount; i++)
	{
		j = 0;
		// Read in the tag number.
		actualcount += dbfile->Read(&tag,sizeof(DWORD));
		actualcount += dbfile->Read(&tagsize,sizeof(DWORD));
		//if it is an int tag
		if ((tag == 1)||(tag == 5)||(tag == 17)||(tag == 21)||(tag == 53))  
		{
			actualcount += dbfile->Read(&other,sizeof(int));	
		}
		else if (tag == 13) // resolution tag 2x int
		{
			actualcount += dbfile->Read(&other,sizeof(int));
			actualcount += dbfile->Read(&other2,sizeof(int));
		}
		// If the tag is 3 which is the hash tag read in the hash.
		else if (tag == 3)
		{
			j = 0;
			while ((j <= 19))
			{
				actualcount += dbfile->Read(&hash[j],1);
				j++;
			}
			hash[20] = '\0';

			memcpy(mdata->m_hash,hash,20);
			break;
			
			
		}
		else while (actualcount += dbfile->Read(&entry[j],1)) //string tag
		{
			if (entry[j] == '\0')
				break;
			j++;
		}
		recordlengthtest += tagsize;
		recordlengthtest += (sizeof(int)*2);

		switch (tag)
		{
			// 4 is Title
			case (4):
			{
				mdata->m_title = entry;
				break;
			}
			case(6):
			{
				mdata->m_artist = entry;
				break;
			}
			case(12):
			{
				mdata->m_keywords = entry;
				break;
			}
			case(26):
			{
				mdata->m_comment = entry;
				break;
			}

		}
	}
	// Read the padding
	// I seem to have a little too much padding by 8, will check that out later
	dbfile->Read(trash,(entrysize)-actualcount + 8);

	return 0;
}

void DbWriter::CopyDatabases()
{	
	static const int default_len=1024;
	char dirbase[default_len+1];
	char dirfinal[default_len+1];
	char data256[default_len+1];
	char data1024[default_len+1];
	char data4096[default_len+1];
	char dirname[default_len+1];
	char num[default_len+1];

	char outputfilename[default_len+1];
	
	long handle256,handle1024,handle4096;

	//prepare inputs
	strcpy(dirbase,"C:\\OnSystems");
	strcpy(dirname,"Kazaa Lite K++\\db");
	strcpy(data256,"C:\\OnSystems0\\Kazaa Lite K++\\db\\data256.dbb");
	strcpy(data1024,"C:\\OnSystems0\\Kazaa Lite K++\\db\\data1024.dbb");
	strcpy(data4096,"C:\\OnSystems0\\Kazaa Lite K++\\db\\data4096.dbb");



	_finddata_t data;
	handle256=(long)_findfirst(data256,&data);
	handle1024=(long)_findfirst(data1024,&data);
	handle4096=(long)_findfirst(data4096,&data);

	// Generate the directory names from 1 to 99
	for (int i = 1; i <=99; i++)
	{
		// start with the base onsystems0
		strcpy(dirfinal,dirbase);
		_itoa(i,num,10);
		// Add the number of the folder
		strcat(dirfinal,num);
		strcat(dirfinal,"\\");
		strcat(dirfinal,dirname);
		strcat(dirfinal,"\\");
	
		if (handle256 != -1)
		{
			strcpy(outputfilename,dirfinal);
			strcat(outputfilename,"data256.dbb");
			CopyDatFile(data256,outputfilename);
		}

		if (handle1024 != -1)
		{
			strcpy(outputfilename,dirfinal);
			strcat(outputfilename,"data1024.dbb");
			CopyDatFile(data1024,outputfilename);
		}

		if (handle4096 != -1)
		{
			strcpy(outputfilename,dirfinal);
			strcat(outputfilename,"data4096.dbb");
			CopyDatFile(data4096,outputfilename);
		}

	}

	if (handle256 != -1)
	{
		_findclose(handle256);
	}

	if (handle1024 != -1)
	{
		_findclose(handle1024);
	}

	if (handle4096 != -1)
	{
		_findclose(handle4096);
	}
}

void DbWriter::ClearDatabases()
{

	char dirbase[256+1];
	char dirfinal[256+1];
	char data256[256+1];
	char data1024[256+1];
	char data4096[256+1];
	char dirname[256+1];
	char num[2+1];

	//prepare inputs
	strcpy(dirbase,"C:\\OnSystems");
	strcpy(dirname,"Kazaa Lite K++\\db");
	strcpy(data256,"data256.dbb");
	strcpy(data1024,"data1024.dbb");
	strcpy(data4096,"data4096.dbb");


	// Generate the directory names from 1 to 99
	for (int i = 0; i <=99; i++)
	{
		// start with the base onsystems0
		strcpy(dirfinal,dirbase);
		_itoa(i,num,10);
		// Add the number of the folder
		sprintf(dirfinal,"%s%s\\%s\\%s",dirbase,num,dirname,data256);
		remove(dirfinal);
		sprintf(dirfinal,"%s%s\\%s\\%s",dirbase,num,dirname,data1024);
		remove(dirfinal);
		sprintf(dirfinal,"%s%s\\%s\\%s",dirbase,num,dirname,data4096);
		remove(dirfinal);
	}

}

bool DbWriter::CopyDatFile(char * infilename, char * outfilename)
{

	CFile infile, outfile;

	if (infile.Open(infilename,CFile::modeRead|CFile::typeBinary|CFile::shareDenyWrite) == FALSE)
	{
		DWORD damnerror = ::GetLastError();
		return false;
	}		

	_finddata_t data;
	long handle=(long)_findfirst(outfilename,&data);
	if (handle != -1)
	{
		_findclose(handle);
		remove(outfilename);

	}



	MakeSureDirectoryPathExists(outfilename);
	if (outfile.Open(outfilename,CFile::modeReadWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeBinary|CFile::shareDenyWrite)==FALSE)
	{
		DWORD damnerror = ::GetLastError();
		return false;
	}
	char buffer[1024+1];
	infile.SeekToBegin();
	int length;
	//Read the file 1k at a time and write to dest
	while ((length = infile.Read(buffer,1024))==1024)
	{
		outfile.Write(buffer, length);
	}
	outfile.Write(buffer, length);
	outfile.Close();
	infile.Close();

	return true;
}
