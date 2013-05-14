#include "StdAfx.h"
#include "winmxmusicmodule.h"
#include "MediaManager.h"
#include "FrameHeader.h"
#include "ID3v1Tag.h"
#include "ID3v2Tag.h"
#include "imagehlp.h"
#include <io.h>

WinMXMusicModule::WinMXMusicModule(void)
{
}

WinMXMusicModule::~WinMXMusicModule(void)
{
}


void WinMXMusicModule::InitParent(MediaManager *parent)
{
	p_parent = parent;	
}

bool WinMXMusicModule::MakeMp3(MusicInfo info, char * inputfolder, char * outputfolder)
{

	CFile input_file;
	CFile output_file;

	char msg[1024];
	char outputfilename[256+1];
	char inputfilename[256+1];

	//Set up the outputfile
	strcpy(outputfilename, outputfolder);
	strcat(outputfilename,info.m_filename.c_str());

	//If the filename appears to be too long, cut it off and add the .mp3 tag so at least it works
	if (strlen(outputfilename) >= 255)
	{
		outputfilename[252] = '\0';
		strcat(outputfilename,".mp3");
	}
	sprintf(msg,"Making File %s",outputfilename);
	p_parent->DisplayInfo(msg);

	// Create the input filename with the folder plus input plus the track number
	strcpy(inputfilename,inputfolder);
	strcat(inputfilename,"Input");
	strcat(inputfilename,info.m_track.c_str());
	strcat(inputfilename,".mp3");

	//Try and open the input file
	if(input_file.Open(inputfilename,CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone)==FALSE)
	{
		DWORD Damnerror = ::GetLastError();
		sprintf(msg,"Failed to open input file %s",inputfilename);
		p_parent->DisplayInfo(msg);
		return false;
	}

	MakeSureDirectoryPathExists(outputfilename);
	// see if the file exists already
	_finddata_t data;
	intptr_t handle;
//	while (output_file.Open(outputfilename,CFile::modeRead) == TRUE)
	while ((handle = _findfirst(outputfilename, &data)) != -1)
	{
		_findclose(handle);
		//if it exists re-generate the filename and try again
		GenerateFilename(&info);
		strcpy(outputfilename, outputfolder);
		strcat(outputfilename,info.m_filename.c_str());
		//output_file.Close();
	}
	//Create the output file
	if(output_file.Open(outputfilename,CFile::modeCreate|CFile::modeWrite|CFile::modeNoTruncate|CFile::typeBinary|CFile::shareDenyWrite)==FALSE)
	{
		DWORD Damnerror = ::GetLastError();
		sprintf(msg,"Failed to create output file %s",outputfilename);
		p_parent->DisplayInfo(msg);
		return false;
	}

	ID3v2Tag id3v2_tag;
	// We don't like V2 tags so if you see it get rid of it
	if(id3v2_tag.ExtractTag(&input_file)==false)
	{
		// If there was no tag, rewind the file
		input_file.SeekToBegin();
	}

	//Get the frame Header
	FrameHeader frame_header;
	frame_header.ExtractHeader(&input_file);

	unsigned int bitrate=frame_header.ReturnBitrate();

	// Put back the frame header
	input_file.Seek(0-sizeof(FrameHeader),CFile::current);

	// We're gonna calculate the frame count based on the time and the bitrate
	unsigned int frame_count=0;

	// Need to be able to round up if necessary
	int seconds = 300;  // begin with a 5 minute song, will be resized later
	double dividend=1*1*bitrate*seconds;
	double divisor=frame_header.ReturnFrameSize()*8*1;
	double quotient=dividend/divisor;

	frame_count=(unsigned int)quotient;

	quotient-=(unsigned int)quotient;
	if(quotient>0.44444444444444444444444444444444)
	{
		frame_count++;
	}	

	// Add frames of silence...up to 3 seconds worth
	//unsigned int pre_silence_frame_count=0;

	dividend=1*1*bitrate*3;
	divisor=frame_header.ReturnFrameSize()*8*1*1;
	quotient=dividend/divisor;
	
	///pre_silence_frame_count=(unsigned int)quotient;

	quotient-=(unsigned int)quotient;
	if(quotient>0.44444444444444444444444444444444)
	{
//		pre_silence_frame_count++;
	}

	// Randomly pick up to 3 seconds worth of frames
	//pre_silence_frame_count=rand()%pre_silence_frame_count;

	bool is_first_frame_of_file=true;

	bool is_padded=false;
	/*
	for(unsigned int i=0;i<pre_silence_frame_count;i++)
	{
		frame_header.IsPadded(is_padded^=true);		// toggle the padded bit

		output_file.Write(&frame_header,sizeof(FrameHeader));

		unsigned char *buf=new unsigned char[frame_header.ReturnFrameSize()-sizeof(FrameHeader)];
		
		for(unsigned int j=0;j<frame_header.ReturnFrameSize()-sizeof(FrameHeader);j++)
		{
			buf[j]=rand()%10;	// 0-9 is ever more "more or less silence"
		}

		if(is_first_frame_of_file)
		{
			is_first_frame_of_file=false;
			unsigned int deadbeef=0xDEADBEEF;
			memcpy(buf,&deadbeef,sizeof(deadbeef));
		}
		
		output_file.Write(buf,frame_header.ReturnFrameSize()-sizeof(FrameHeader));
		delete [] buf;
	}
*/

	// Adjust for pre-silence frame count frames
//	frame_count-=pre_silence_frame_count;

	// Begin again
	input_file.SeekToBegin();
	// We don't like V2 tags so if you see it get rid of it
	if(id3v2_tag.ExtractTag(&input_file)==false)
	{
		// If there was no tag, rewind the file
		input_file.SeekToBegin();
	}
	// input_file.SeekToBegin();
	// Create the new file from the old one

	int randnoise = 0;
	int randseed = rand()%10;
	int filelen;
	switch (randseed)
	{	
		case 1:
		{
			randnoise = 1;
			filelen = 4103241;
			break;
		}
		case 2:
		{
			randnoise = 2;
			filelen = 3503241;
			break;
		}
		case 3:
		{
			randnoise = 3;
			filelen = 4603246;
			break;
		}
		case 4:
		{
			randnoise = 4;
			filelen = 5204241;
			break;
		}
		case 5:
		{
			randnoise = 5;
			filelen = 3196244;
			break;
		}
		default:
		{
			randnoise = 0;
			filelen = 4796294;
			break;
		}
	}

	for(int i=0;i<(int)frame_count;i++)
	{
		frame_header.ExtractHeader(&input_file);

		unsigned int frame_size=frame_header.ReturnFrameSize()-sizeof(FrameHeader);
	// Problem here FUCK
		if (frame_size > 2000000)
		{
			frame_size = 414;
		}
		unsigned char *buf=new unsigned char[frame_size];
		
		// If we've reached the end of the file, then rewind it or set it to silence
		if(input_file.Read(buf,frame_size)==NULL)
		{
			for(unsigned int j=0;j<frame_size;j++)
			{
				buf[j]=randnoise;	// 0-9 is "more or less silence"
			}
		}

		output_file.Write(&frame_header,sizeof(FrameHeader));
		output_file.Write(buf,frame_size);

		delete [] buf;
	}

	int modlen = 0;
	int flen = (int)output_file.GetLength();
	// If this is flagged as a single we assign it a specific filesize so that each file is > 200k apart
	if (m_single_flag)
	{

		double twenty_five_percent=((double)flen)/4.0;	// on a 4 MB file, this is 1 MB

		int delta=(int)(((-1.0)*twenty_five_percent)+((300*1024)*info.m_single_offset));

		output_file.SetLength(flen+delta);	// delta is -1 MB and up (in the positive direction) for a 4 MB file + up to 5K of noise

	}
	//If it isn't a single give it a random filesize from about 1 to 6 megs
	else
	{
			output_file.SetLength(filelen);

	}

	// add the length of the file plus the length of the tag to determine the final file size 
	flen = (int)(output_file.GetLength()+sizeof(ID3v1Tag));



	//add the inverse of the filesize mod 137 so the final size will be divisible by 137
	modlen = (flen%137);
	modlen = 137 - modlen;
	unsigned char *buffer=new unsigned char[modlen];
	for(int m=0;m<modlen;m++)
	{
		buffer[m]=rand()%10;	
	}
	output_file.Write(buffer,modlen);		

	// ID3v1 Tag
	ID3v1Tag tag;

	tag.Init();
	tag.SetAlbum((char *)info.m_album.c_str());
	tag.SetArtist((char *)info.m_artist.c_str());
	tag.SetGenre(atoi((char *)info.m_genre.c_str()));
	tag.SetYear((char *)info.m_year.c_str());
	tag.SetComment((char *)info.m_comments.c_str());

	tag.SetTitle((char *)info.m_title.c_str());

	output_file.Write(&tag,sizeof(ID3v1Tag));


	//Clean Up
	tag.Clear();

	delete [] buffer;

	input_file.Close();
	output_file.Close();

	// Add the file association 
	if (p_parent->AddFileAssociation(inputfilename, outputfilename) == true)
	{
		 if (m_single_flag == false)
		 {
			//file association worked so remove the input if it is not a single
			CFile::Remove(inputfilename);
		 }		
			return true;
	}
	return false;

}