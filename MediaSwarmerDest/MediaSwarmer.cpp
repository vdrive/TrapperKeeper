#include "StdAfx.h"
#include "mediaswarmer.h"
#include "imagehlp.h"
#include "FrameHeader.h"
#include <io.h>

MediaSwarmer::MediaSwarmer(void)
{
}

MediaSwarmer::~MediaSwarmer(void)
{
}

void MediaSwarmer::DllInitialize()
{
	// Create the dialog
	m_dlg.Create(IDD_MEDIASWARMER_DEST_DLG,CWnd::GetDesktopWindow());

}
void MediaSwarmer::DllUnInitialize()
{

}


void MediaSwarmer::DllStart()
{
	// Register the Syncher to start synching
	m_syncher.Register(this,"Swarmer");

}

void  MediaSwarmer::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);

}

void MediaSwarmer::DataReceived(char *source_name, void *data, int data_length)
{
	//received remote data from the Interface
}

bool MediaSwarmer::DeflateFile(char * filepath)
{
	CFile inputfile;
	CFile outputfile;
	CString filename;
	char outputfilename[256+1];
	int inputfilesize;

	// Open the input
	if (inputfile.Open(filepath,CFile::modeRead|CFile::typeBinary|CFile::modeNoTruncate|CFile::shareDenyNone) == 0)
	{
		DWORD damnerror = ::GetLastError();
		return false;
	}

	// Create the output file name from the input file name
	filename = inputfile.GetFileName();	
	strcpy(outputfilename,"C:\\DeflatedFiles\\deflated_");
	strcat(outputfilename,filename);

	// Open the file to write to
	MakeSureDirectoryPathExists(outputfilename);
	if (outputfile.Open(outputfilename,CFile::modeWrite|CFile::typeBinary|CFile::modeNoTruncate|CFile::modeCreate|CFile::shareDenyWrite) == 0 )
	{
		DWORD damnerror = ::GetLastError();
		return false;
	}

	inputfilesize = (int)inputfile.GetLength();

	char * buf300k;
	buf300k = new char[300*1024];

	// Read the first 300k
	inputfile.Read(&buf300k,(300*1024));
	outputfile.Write(buf300k,(300*1024));

	int pos = 0;
	int lastpos = 0 ; // last good position
	int numread = 0;
	int specialcase = 0;

	// save space in the loop to do up to a terabyte 
	// why? because files will never be that large and it will break out of the loop at the end
	for (int i = 0; i <30; i ++)
	{	
		// Try to seek to the 2^ith megabyte
		try
		{
			(pos = (int)inputfile.Seek(2^i *1024 *1024,CFile::begin));
		}
		catch(CFileException *ex)
		{
			ex->Delete();
			break;
		}
		i++;
		// save the last good position for calculations later
		lastpos = pos;
		//Write the file

		// if the filesize falls between the range of 2^i +300k and 2^i +600k there will be overlap
		if (inputfile.GetLength() - 300*1024 < lastpos + 300*1024)
		{
			specialcase = 2;
			break;
		}
		numread = inputfile.Read(&buf300k,(300*1024));
		if (numread == 300*1024)
		{
			outputfile.Write(buf300k,(300*1024));
		}
		else
		{
			specialcase = 1;
		}
	}
	// Seek to the beginning of the last 300k
	pos = (int)inputfile.Seek(-300*1024,CFile::end);

	// Here will be the logic to take care of cases where the last 300 k falls inside a written block

	switch (specialcase)
	{
		// normalcase
		case 0:
		{
			inputfile.Read(&buf300k,(300*1024));
			outputfile.Write(buf300k,(300*1024));
			break;
		}
		// the filesize was within 2^i + 300k
		// Finish same as case 1 since we decided not to write the partial buffer
		case 1:
		{
			inputfile.Read(&buf300k,(300*1024));
			outputfile.Write(buf300k,(300*1024));
			break;
		}
		// the filesize was between 2^i + 300k and 2^i + 600k
		case 2:
		{
			int numread = 0;
			char * bigbuf;
			bigbuf = new char[2*300*1024];
			
			numread = inputfile.Read(&bigbuf,(2*300*1024));
			outputfile.Write(bigbuf,numread);

			delete [] bigbuf;
			break;
		}	
	}

	delete [] buf300k;
	return true;
}

bool MediaSwarmer::InflateFile(char * filepath, char * outputfiledir)
{
	CFile inputfile;
	CFile outputfile;
	CString filename;
	char outputfilename[256+1];
	int inputfilesize;


	if (inputfile.Open(filepath,CFile::modeRead|CFile::typeBinary|CFile::modeNoTruncate|CFile::shareDenyNone) == 0)
	{
		DWORD damnerror = ::GetLastError();
		return false;
	}

	filename = inputfile.GetFileName();
	char * ptr = strstr(filename,"_");
	ptr++;
	
	strcpy(outputfilename,outputfiledir);
	strcat(outputfilename,ptr);

	if (outputfile.Open(outputfilename,CFile::modeWrite|CFile::typeBinary|CFile::modeNoTruncate|CFile::modeCreate|CFile::shareDenyWrite) == 0 )
	{
		inputfile.Close();
		DWORD damnerror = ::GetLastError();
		return false;
	}

	inputfilesize = (int)inputfile.GetLength();

	
	int numchunks;
	int leftover;

	numchunks = inputfilesize/(300 * 1024);
	leftover = inputfilesize%(300 * 1024);


	//create the first good chunk 
	char * buf300k;
	buf300k = new char[300*1024];

	// Read the first 300k
	inputfile.Read(&buf300k,(300*1024));
	outputfile.Write(buf300k,(300*1024));

	// first bad chunk is 700k
	unsigned int chunk_len = (700 * 1024);
	int copied_size = 0;

	FillInChunk(&outputfile,chunk_len);

	// Loop through the rest of the chunks -1 since the first one is done
	for (int i = 0; i < numchunks -1; i ++)
	{
		// Read the next 300k
		inputfile.Read(&buf300k,(300*1024));
		outputfile.Write(buf300k,(300*1024));
		//calculate the next chunk
		chunk_len = (2^i *1024 *1024 - (300 *1024));
		FillInChunk(&outputfile,chunk_len);
	}
	
	if (leftover != 0)
	{
		inputfile.Read(&buf300k,leftover);
		outputfile.Write(buf300k,leftover);
	}

	delete [] buf300k;
	inputfile.Close();
	outputfile.Close();
	return true;
}

bool MediaSwarmer::FillInChunk(CFile * outfile, unsigned int chunk_size)
{

// Build a frame to copy
	FrameHeader hdr(32,44.1);
	int frame_len=hdr.ReturnFrameSize();
	frame_len += sizeof(FrameHeader);
	char *frame_buf=new char[frame_len];

	memset(frame_buf,0,frame_len);
	unsigned int frame_offset=0;
	memcpy(&frame_buf[frame_offset],&hdr,sizeof(FrameHeader));
	frame_offset+=sizeof(FrameHeader);

	// Copy the crap into the frame
	for(int i=0;i<(int)(frame_len-sizeof(FrameHeader));i++)
	{
		frame_buf[frame_offset+i]=(i+rand()%2)%256;	
	}

	unsigned int chunk_offset = 0;
	while (chunk_offset < chunk_size)
	{
		if (chunk_offset + frame_len < chunk_size)
		{
			outfile->Write(frame_buf,frame_len);
			chunk_offset += frame_len;
		}
		else
		{
			break;
		}
	}

	char * endbuf;
	endbuf = new char[chunk_size - chunk_offset];
	for(unsigned int i=0;i<(chunk_size - chunk_offset);i++)
	{
		endbuf[i]=rand()%256;
	}
	outfile->Write(endbuf,chunk_size - chunk_offset);

	return true;
}

void MediaSwarmer::ReadDestDir()
{
	vector<string> filenames;
	// Folder will change to C:\\Syncher\\Rcv\\Swarmer
	char * filedir = "C:\\DeflatedFiles\\";
	ExploreFolder(filedir,filenames);

	for (int i = 0; i< (int)filenames.size(); i++)
	{
		InflateFile((char *)filenames[i].c_str(), "C:\\FastTrack Shared\\Swarmer\\");
	}

}

void MediaSwarmer::ExploreFolder(char * folder, vector<string> filenames)
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

		filenames.push_back(tempname);
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

			filenames.push_back(tempname);
		}
	}
}

void MediaSwarmer::ReadMapsFile()
{
	CFile list;
	FileAssociation fa;

	v_mapped_files.clear();

	//  read each line of the file associations and place it onto the  vector.
	if( list.Open("swarmermaps.dat", CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone) == TRUE )
	{
		while(list.Read(&fa, sizeof(fa)) != 0)
		{
			v_mapped_files.push_back(fa);

		}
		list.Close();
	}


}

bool MediaSwarmer::AddFileAssociation(char * filename, char * assocname)
{
	// Create a file association for the two files and save it to the maps file
	FileAssociation fa;
	strcpy(fa.m_path, filename);
	strcpy(fa.m_assoc_path, assocname);
	// if you were able to save the file association to the maps file
	if(fa.Save() == true)
	{
		/*
		char buf[1024+1];
		sprintf(buf,"Created File %s from file %s",assocname,filename);
		DisplayInfo(buf);
		*/
		return true;
	}
	return false;
}



void MediaSwarmer::CheckMapsForDeletion(vector<TKSyncherMap::TKFile> &file_list)
{
	vector<FileAssociation> files_for_deletion; //List of Files to delete
	int found = 0;
	ReadMapsFile();
	//Step through each entry in the maps file
	for (int i = 0; i < (int) v_mapped_files.size(); i ++)
	{
		//Compare each maps entry to the Syncher Maps
		for (int j = 0; j < (int)file_list.size(); j++)
		{
			
			if (stricmp(v_mapped_files[i].m_path, file_list[j].m_name.c_str()) == 0)
			{
				// we found it
				found = 1;
				break;
			}


		}
		if (found == 0)
		{
			// we didn't find it so it is supposed to be deleted
			files_for_deletion.push_back(v_mapped_files[i]);
		}
		// reset found flag
		found = 0;
	}
	// Delete all of the files that are supposed to be deleted
	if (files_for_deletion.size() > 0)
	{
		DeleteMappedFileList(files_for_deletion);
	}
}


bool MediaSwarmer::DeleteMappedFileList(vector<FileAssociation> vfa)
{

		for (int j = 0; j < (int)vfa.size(); j ++)
		{	
			//Remove both the input and associated files to catch any templates with no association
			if (remove(vfa[j].m_assoc_path) == 0)
			{
				remove(vfa[j].m_path);
				
					// If we were able to remove the association
					// Take the File Association off of the current maps list
					RemoveFileAssociationFromList(vfa[j]);
			}

		}
		// Fully re-write the maps file with the current maps list
		RewriteMapsFile();
		return true;
}

void MediaSwarmer::RemoveFileAssociationFromList(FileAssociation fa)
{
	vector<FileAssociation>::iterator fa_iter = v_mapped_files.begin();
	while (fa_iter != v_mapped_files.end())
	{
		if ((strcmp(fa.m_assoc_path,fa_iter->m_assoc_path) == 0)&&(strcmp(fa.m_path,fa_iter->m_path) == 0))
		{
			v_mapped_files.erase(fa_iter);
			break;
		}
		fa_iter ++;
	}
}

void MediaSwarmer::RewriteMapsFile()
{
	// Delete the old maps file
	remove("maps.dat");
	//go through the vector of current maps and write it out to the file
	for (int i = 0; i< (int)v_mapped_files.size(); i ++)
	{
		v_mapped_files[i].Save();
	}
}