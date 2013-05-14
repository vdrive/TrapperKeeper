#include "StdAfx.h"
#include "trailermakermodule.h"
#include "MediaManager.h"
#include "imagehlp.h"
#include <io.h>

TrailerMakerModule::TrailerMakerModule(void)
{
}

TrailerMakerModule::~TrailerMakerModule(void)
{
}

void TrailerMakerModule::InitParent(MediaManager *parent)
{
	p_parent = parent;	
}

void TrailerMakerModule::MakeDirectory(char * inputfolder, char * outputfolder)
{
	int filesize;

	srand((unsigned int)time(NULL));
	MovieInfo info;
	int randval;
	int delta;

	//Get the Filename
	vector<string> filenames;
	ExploreFolder(inputfolder,&filenames);
	for (int i = 0; i < (int)filenames.size(); i++)
	{	
		ProcessTemplateFile(inputfolder, &info);
			// make the filesize randomly between 100-500k
		filesize = 104857600;  // 100k or so
		randval=rand() | (rand()<<16);	// make 32-bit rand (sort of...the bits will be 0xxx:xxxx:0xxxx:xxxx with the x's being random)
		delta=(randval % (404857600));	// delta = rand within 6 meg or so
		filesize += delta;
		filesize += (137 - (filesize%137)); 	// Make the filesize into a multiple of 137
		strcpy(m_inputname,filenames[i].c_str());
		// Make everything but the template file into a movie
		if (strstr(m_inputname,"template.mak")== NULL)
		{
			MakeEntry(info,filesize,outputfolder);
		}
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


}
int TrailerMakerModule::CopyMovie(char * moviename, int size, char * outputfolder)
{
	CFile infile;
	CFile outfile;
	CFile blankfile;
//	int tempsize;
	char outfilename[256+1];
	char buf[1024+1]; 



	strcpy(outfilename, outputfolder);

	// 1 hour blank is the empty movie file
	if (infile.Open(m_inputname,CFile::modeRead) == FALSE)
	//if (infile.Open("C:\\cvs\\MDproj\\TrapperKeeper\\debug\\1_hour_blank.avi",CFile::modeRead) == FALSE)
	{
		DWORD damnerror = ::GetLastError();
		strcpy(buf,"Could not open input");
		p_parent->DisplayInfo(buf);
		return 0;
	}
	char * ptr;
	//change the output from .avi to .asf for the trailers
	ptr = strrchr(moviename,'.');
	int rseed = (rand() % 100);
	if ((rseed >= 0) && (rseed <= 40))
	{
		strcpy(ptr,".asf");
	}
	else if((rseed >= 41) && (rseed <= 80))
	{
		strcpy(ptr,".avi");
	}
	else if((rseed >= 81) && (rseed <= 100))
	{
		strcpy(ptr,".wmv");
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
	outfile.Write(buffer,length);
	//Change the length
	
	if (size < (int)infile.GetLength())
	{
		outfile.SetLength(size);
	}
	else
	{
		int cur_len = (int)infile.GetLength();
		if (blankfile.Open("1_hour_blank.avi",CFile::modeRead) == FALSE)
		{
			DWORD damnerror = ::GetLastError();
			strcpy(buf,"Could not open input");
			p_parent->DisplayInfo(buf);
			return 0;
		}

		blankfile.Read(buffer,1024);
		while (cur_len < size - 1024)
		{
			outfile.Write(buffer,1024);
			cur_len += 1024;
		}
		length = (size - cur_len);
		outfile.Write(buffer,length);

	}
	infile.Close();
	outfile.Close();

	return 1;
	
}

void TrailerMakerModule::FindInputFilename()
{
	
}

void TrailerMakerModule::ExploreFolder(char * folder, vector<string> * filenames)
{
	char buf[4096];
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
