// MovieMakerModule.cpp : implementation file
//

#include "stdafx.h"
#include "MovieMakerModule.h"
#include "giFThash.h"
#include "giFTmd5.h"
#include <time.h>
#include <stdlib.h>
#include "imagehlp.h"

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


void MovieMakerModule::DllInitialize()
{
	m_dlg.Create(IDD_MOVIE_MAKER_DLG,CWnd::GetDesktopWindow());
	srand(time(NULL));
}
void MovieMakerModule::DllUnInitialize()
{

}


void MovieMakerModule::DllStart()
{

}

void  MovieMakerModule::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
}

/*
void MovieMakerModule::InitParent(CMovieMakerDlg *parent)
{
	p_parent = parent;	
}
*/

// Make Entry Function will be totally rewritten, takes the input and converts it to a Kazaa header
void MovieMakerModule::MakeEntry(MovieInfo info, int filesize)
{

	vector<Record> tags;
	RecordHeader header;
	CFile dbfile;
	CFile tempfile;
	CFile delfile;
	int whichdata; // for determininge which file to use


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
	CopyMovie(moviename, filesize);
	header.filename = new char[strlen(moviename)+1];
	header.filedirectory = new char[strlen("C:\\FastTrack Shared\\Movies")+1];

	strcpy(header.filename, info.m_filename.c_str());
	strcpy(header.filedirectory,"C:\\FastTrack Shared\\Movies");

	//set the length of the header, add the tag lengths to this later
	header.recordlength = ((sizeof(int) * 5)+(sizeof(bool))+(strlen(moviename)+1)+(strlen(header.filedirectory)+1));

//	newrec.filedate = time.Format(


	// Begin creating all the tags as needed and place them on the vector
	Record temprec;
	char entry[256+1];

	//Hash Tag = 3
	char fullpath[256+1];
	strcpy(fullpath, "C:\\FastTrack Shared\\Movies\\");
	strcat(fullpath, header.filename);	
	CalculateHash(fullpath,entry);
	temprec.Insert(3,entry);
	tags.push_back(temprec);
	temprec.Clear();

	//Length in seconds tag = 5
	int len = info.m_length;
	if (len == 0)
	{
		len = 3600 + rand()%1800;
	}
	temprec.Insert(18,len);
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
	temprec.Insert(10,"Movie");
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


	// Go through the vector to determine the final size and count
	header.recordcount = 0;
	vector<Record>::iterator record_iter = tags.begin();
	while (record_iter != tags.end())
	{
		header.recordlength += record_iter->GetSize();
		header.recordcount++;
		record_iter++;
	}
	
	// Start Writing Everything
	int padding;
	if (delfile.Open("data.tmp",CFile::modeRead) == TRUE)
	{
		delfile.Close();
		CFile::Remove("data.tmp");
	}
	// Figure out which DB file we want to use
	if (header.recordlength < 256)
	{
		CFile::Rename("data256.dbb","data.tmp");
		//padding for the 256 recordlengh is 256 + 8 = 264
		padding = 264 - header.recordlength;
		whichdata = 256;
		if(dbfile.Open("data256.dbb",CFile::modeReadWrite|CFile::modeNoTruncate|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite)==FALSE)
		{
			DWORD damnerror = ::GetLastError();
			return;
		}
	}

	else 
	{
		CFile::Rename("data1024.dbb","data.tmp");
		//padding for the 256 recordlengh is 1024 + 8 = 1032
		padding = 1032 - header.recordlength;
		whichdata = 1024;
		if(dbfile.Open("data1024.dbb",CFile::modeReadWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeBinary|CFile::shareDenyWrite)==FALSE)
		{
			DWORD damnerror = ::GetLastError();
			return;
		}

	
	}
	dbfile.SeekToBegin();

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

	//Data.tmp holds the original data so copy it back
	if (tempfile.Open("data.tmp",CFile::modeReadWrite|CFile::modeNoTruncate|CFile::typeBinary|CFile::shareDenyWrite)==FALSE)
	{
		return;
	}
	
//	ULONGLONG tempfilesize = tempfile.GetLength();
	char  buffer [1024 +1];
	int length;
	//Read the file 1k at a time and write to dest
	while ((length = tempfile.Read(buffer,1024))==1024);
	{
		dbfile.Write(buffer, length);
	}
	dbfile.Write(buffer,length);
	

	tempfile.Close();
	CFile::Remove("data.tmp");

	//Copy DB to all directories.
	
	char dbfilename[32+1];
	strcpy(dbfilename,dbfile.GetFileName());
	
	CopyDatabase(dbfilename,12);

	//Clean Up
	dbfile.Close();

}
//
//
//
// Copies the database to however many folders are specified
bool MovieMakerModule::CopyDatabase(char * filename, int numfolders)
{	
	char dirbase[256];
	char dirfinal[256];
	char original[256];
	char num[2];
	CFile tempfile, dbfile;	
	ULONGLONG tempfilesize;


	strcpy(original,"C:\\onsystems\\Kazaa\\");
	strcat(original,filename);
	strcpy(dirbase,"C:\\onsystems");

	if (dbfile.Open(original,CFile::modeRead) == FALSE)
	{
		dbfile.Close();
		return 0;
	}		

//	char * buffer = new char[dbfile.GetLength()];
	char buffer[1024+1];
	tempfilesize = dbfile.GetLength();
	// Generate the directory names from 1 to the input number
	for (int i = 1; i <=numfolders; i++)
	{
		// start with the base onsystems
		strcpy(dirfinal,dirbase);
		_itoa(i,num,10);
		// Add the number of the folder
		strcat(dirfinal,num);
		strcat(dirfinal,"\\Kazaa\\");
		strcat(dirfinal,filename);
		if (tempfile.Open(dirfinal,CFile::modeRead) == TRUE)
		{
			tempfile.Close();
			CFile::Remove(dirfinal);
		}		

		MakeSureDirectoryPathExists(dirfinal);
		if (tempfile.Open(dirfinal,CFile::modeReadWrite|CFile::modeCreate|CFile::modeNoTruncate|CFile::typeBinary|CFile::shareDenyWrite)==FALSE)
		{
			return 0;
		}


		dbfile.SeekToBegin();
		int length;
		//Read the file 1k at a time and write to dest
		while ((length = tempfile.Read(buffer,1024))==1024);
		{
			dbfile.Write(buffer, length);
		}
		dbfile.Write(buffer,length);
		tempfile.Close();


	}

	dbfile.Close();
	return 1;

}

//
//
//
int MovieMakerModule::CopyMovie(char * moviename, int size)
{
	CFile infile;
	CFile outfile;
//	int tempsize;
	char outfilename[256+1];

	strcpy(outfilename, "C:\\FastTrack Shared\\Movies\\");

	// 1 hour blank is the empty movie file
	if (infile.Open("1 hour blank.avi",CFile::modeRead) == FALSE)
	{
		return 0;
	}
	strcat(outfilename,moviename);
	if (outfile.Open(outfilename,CFile::modeWrite|CFile::typeBinary|CFile::modeCreate) == FALSE)
	{
		DWORD damnerror = ::GetLastError();
		return 0;
	}
	char * buffer[1024+1];
	//Copy the File Exactly
	int length;
	//Read the file 1k at a time and write to dest
	while ((length = infile.Read(buffer,1024))==1024);
	{
		outfile.Write(buffer, length);
	}
	outfile.Write(buffer,length);
	//Change the length
	outfile.SetLength(size);


	infile.Close();
	outfile.Close();

	return size;
	
}


//
//
//


void MovieMakerModule::ReadEntry()
{

	//Not needed for the program just used in debugging to test the entry format

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
	int actualcount = 0;

	CStdioFile dbfile;

	if (dbfile.Open("data256.dbb",CFile::modeReadWrite|CFile::modeNoTruncate|CFile::typeBinary|CFile::shareDenyWrite)==FALSE)
	{
		DWORD damnerror = ::GetLastError();
		return;
	}

	int i;

	actualcount += dbfile.Read(&Ksig,sizeof(int));
	actualcount += dbfile.Read(&recordlength,sizeof(int));
	i = 0;
	while (actualcount += dbfile.Read(&filename[i],1))
	{
		if (filename[i] == '\0')
			break;
		i++;
	}
	i = 0;
	while (actualcount += dbfile.Read(&filedirectory[i],1))
	{
		if (filedirectory[i] == '\0')
			break;
		i++;
	}

	int recordlengthtest = ((sizeof(int) * 6)+(sizeof(bool))+(strlen(filename)+1)+(strlen(filedirectory)+1));

	actualcount += dbfile.Read(&size,sizeof(int));
	actualcount += dbfile.Read(&filedate,sizeof(int));
	actualcount += dbfile.Read(&lastshared,sizeof(int));
	actualcount += dbfile.Read(&unknown,sizeof(int));
	actualcount += dbfile.Read(&isShared,sizeof(bool));
	actualcount += dbfile.Read(&recordcount,sizeof(int));

	int tag;
	char entry[256];
	int tagsize;
	int other;
	int other2;
	int j = 0;
	for (i = 0; i<recordcount; i++)
	{
		j = 0;
		actualcount += dbfile.Read(&tag,sizeof(int));
		actualcount += dbfile.Read(&tagsize,sizeof(int));
		if ((tag == 1)||(tag == 5)||(tag == 17)||(tag == 21))  //if it is an int tag
		{
			actualcount += dbfile.Read(&other,sizeof(int));	
		}
		else if (tag == 13) // resolution tag 2x int
		{
			actualcount += dbfile.Read(&other,sizeof(int));
			actualcount += dbfile.Read(&other2,sizeof(int));
		}
		else while (actualcount += dbfile.Read(&entry[j],1)) //string tag
		{
			if (entry[j] == '\0')
				break;
			j++;
		}
		recordlengthtest += tagsize;
		recordlengthtest += (sizeof(int)*2);
	}
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

// Stolen code, begin

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
/*			
				if (fseek(fp, offset, SEEK_SET) < 0 || fread(filebuf, 1, chunk_size, fp) < chunk_size) 
				{
					free(filebuf);	
					return -1;
			    }
*/
				outfile.Seek(offset,CFile::begin);
				outfile.Read(md5buf,chunk_size);
	    
				smallhash = hashSmallHash(md5buf, chunk_size, smallhash);
				lastpos = offset+chunk_size;
				offset <<= 1;
			}

			endlen = outfile.GetLength() - lastpos;
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
			outfile.Seek(outfile.GetLength() - endlen,CFile::begin);
			outfile.Read(md5buf,endlen);

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
		strcpy(hashin,temphash);
		int test = strlen(hashin);
}
// Stolen code - end

//
//
//

// Processing the template file also sets the info for the final movie
bool MovieMakerModule::ProcessTemplateFile(char * filename, MovieInfo * info)
{
	CStdioFile templatefile;
	char buf[4096+1];

	if (templatefile.Open(filename,CFile::typeText|CFile::modeRead|CFile::shareDenyWrite)==FALSE)
	{
		return 0;
	}

	//loop to read in and process each line.
	while(templatefile.ReadString(buf,sizeof(buf)-1)!=FALSE)
	{
		ProcessTemplate(buf, info);
	}
	GenerateFilename(info);

	int randdesc = rand()%v_descriptions.size();
	info->m_description += v_descriptions[randdesc];

	v_descriptions.clear();;
	v_tags.clear();
	v_types.clear();

	return 1;
}
//
//
//

// Add any mixing schemes we want to generate filenames using the data from the template
void MovieMakerModule::GenerateFilename(MovieInfo * info)
{
	int randnum;
	string tempstring;
	char buf[256+1];

	randnum = rand()%6;
	// mix and match various naming schemes
	switch (randnum)
	{
		// normal tag, movie, type
		case 0:
		{
			randnum = rand()%v_tags.size();
			tempstring = v_tags[randnum];
			tempstring += info->m_filename;
			randnum = rand()%v_types.size();
			tempstring += v_types[randnum];
			info->m_filename = tempstring;
			break;
		}
		// normal movie, tag, type
		case 1:
		{
			tempstring = info->m_filename;
			randnum = rand()%v_tags.size();
			tempstring += v_tags[randnum];
			randnum = rand()%v_types.size();
			tempstring += v_types[randnum];
			info->m_filename = tempstring;
			break;
		}
		//lower tag, movie, type
		case 2:
		{
			randnum = rand()%v_tags.size();
			strcpy(buf, v_tags[randnum].c_str());
			strlwr(buf);
			tempstring = buf;
			strcpy(buf, info->m_filename.c_str());
			strlwr(buf);
			tempstring += buf;
			randnum = rand()%v_types.size();
			strcpy(buf, v_types[randnum].c_str());
			strlwr(buf);
			tempstring += buf;
			info->m_filename = tempstring;
			break;
		}
		// lower movie, tag, type
		case 3:
		{
			strcpy(buf, info->m_filename.c_str());
			strlwr(buf);
			tempstring = buf;
			randnum = rand()%v_tags.size();
			strcpy(buf, v_tags[randnum].c_str());
			strlwr(buf);
			tempstring += buf;
			randnum = rand()%v_types.size();
			strcpy(buf, v_types[randnum].c_str());
			strlwr(buf);
			tempstring += buf;
			info->m_filename = tempstring;
			break;
		}
		//upper tag, movie, type
		case 4:
		{
			randnum = rand()%v_tags.size();
			strcpy(buf, v_tags[randnum].c_str());
			strupr(buf);
			tempstring = buf;
			strcpy(buf, info->m_filename.c_str());
			strupr(buf);
			tempstring += buf;
			randnum = rand()%v_types.size();
			strcpy(buf, v_types[randnum].c_str());
			strupr(buf);
			tempstring += buf;
			info->m_filename = tempstring;
			break;
		}
		// lower movie, tag, type
		case 5:
		{
			strcpy(buf, info->m_filename.c_str());
			strupr(buf);
			tempstring = buf;
			randnum = rand()%v_tags.size();
			strcpy(buf, v_tags[randnum].c_str());
			strupr(buf);
			tempstring += buf;
			randnum = rand()%v_types.size();
			strcpy(buf, v_types[randnum].c_str());
			strupr(buf);
			tempstring += buf;
			info->m_filename = tempstring;
			break;
		}
	}
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
	if (strstr(buf,"<DESCRIPTION>"))
	{
		bufstring = buf + strlen("<DESCRIPTION>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		tempstring = bufstring;
		v_descriptions.push_back(tempstring);
		return 1;
	}
	else if (strstr(buf,"<TAG>"))
	{
		bufstring = buf + strlen("<TAG>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		tempstring = bufstring;
		v_tags.push_back(tempstring);
		return 1;
	}
	else if (strstr(buf,"<TYPE>"))
	{
		bufstring = buf + strlen("<TYPE>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		tempstring = bufstring;
		v_types.push_back(tempstring);
		return 1;
	}
	// set the individualized template parts, should only be one of each of these tags
	else if (strstr(buf,"<FILENAME>"))
	{
		bufstring = buf + strlen("<FILENAME>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		info->m_filename = bufstring;
		return 1;
	}
	else if (strstr(buf,"<ARTIST>"))
	{
		bufstring = buf + strlen("<ARTIST>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		info->m_artist = bufstring;
		return 1;
	}
	else if (strstr(buf,"<CATEGORY>"))
	{
		bufstring = buf + strlen("<CATEGORY>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		info->m_category = bufstring;
		return 1;
	}
	else if (strstr(buf,"<KEYWORDS>"))
	{
		bufstring = buf + strlen("<KEYWORDS>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		info->m_keywords = bufstring;
		return 1;
	}
	// Must have words are words that will be in the description of every copy of the movie
	else if (strstr(buf,"<MUSTHAVE>"))
	{
		bufstring = buf + strlen("<MUSTHAVE>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		info->m_description = bufstring;
		return 1;
	}
	else if (strstr(buf,"<LENGTH>"))
	{
		bufstring = buf + strlen("<LENGTH>");
		bufstring.TrimLeft();
		bufstring.TrimRight();
		int templen = atoi(bufstring);
		info->m_length = templen;
		return 1;
	}
	// if the line didn't contain a template 
	return 0;
}





