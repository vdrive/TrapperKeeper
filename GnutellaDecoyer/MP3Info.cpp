#include "StdAfx.h"
#include "mp3info.h"
#include "FrameHeader.h"
#include "ID3v2TagHeader.h"

MP3Info::MP3Info(const char* filename)
{
	m_bitrate = m_duration = 0;
	CFile file;

	if(file.Open(filename,CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone)!=NULL)
	{
		ID3v2TagHeader id3v2_tag_header;

		// Read in the ID3v2 tag header if there is one
		id3v2_tag_header.Clear();
		id3v2_tag_header.ExtractTag(filename);
		
		// If there is a ID3v2 tag, seek to the right position
		if(id3v2_tag_header.IsValid())
		{
			unsigned int tag_length=id3v2_tag_header.GetTagLengthValue();
			file.Seek(tag_length, CFile::SeekPosition::begin);
		}

		unsigned char buf[100*1024];	// must be divisible into 300K and 1MB
		memset(&buf, 0, sizeof(buf));

		FrameHeader frame_hdr;
		// Extract frame header from file
		UINT datastart = file.GetPosition();
		if(frame_hdr.ExtractHeader(&file)==-1)
		{
			//doing nothing for now
			/*
			// This may be the start of a goofy file...where there is a non-standard tag header that is 128 bytes
			// Read in the rest of the 128 bytes if we are at the beginning, and see if we can find a frame
			if(file->GetPosition()==sizeof(FrameHeader))
			{
				file->SeekToBegin();
				file->Read(buf,128);	// try to find frame header by skipping goofy tag

				if(frame_hdr.ExtractHeader(file)==-1)
				{
					//MessageBox("Error reading FrameHeader in F---MaintainingFrames().  Not an mp3?","Error");
					//try byte by byte method
					return IsSwarmByteByByte(filename,file);
				}
			}
			else
			{
				// Look for synch
				file->SeekToBegin();

				// Read in 4096 bytes, looking for synch
				file->Read(buf,4096);
				int synch_index=-1;
				for(int i=0;i<4096-1;i++)
				{
					if((buf[i]==0xFF)&&((buf[i+1]==0xFB)||(buf[i+1]==0xFA)))	// !protected bit set / not set
					{
						synch_index=i;
						break;
					}
				}

				bool failed=true;
				if(synch_index!=-1)
				{
					file->SeekToBegin();
					file->Read(buf,synch_index);

					if(frame_hdr.ExtractHeader(file)!=-1)
					{
						failed=false;
					}
				}

				if(failed)
				{
					//MessageBox("Error reading FrameHeader in F---MaintainingFrames().  Corrupt mp3?","Error");
					//try byte by byte method
					return IsSwarmByteByByte(filename,file);
				}
			}
			*/
		}
		unsigned int frame_size=frame_hdr.ReturnFrameSize();
		m_bitrate = frame_hdr.ReturnBitrate() /1000; //KBps
		if(frame_size!=0)
		{
			UINT frames =(file.GetLength()-datastart)/(frame_size);
			m_duration = (int)((float)(frame_size*frames)/
						(float)(m_bitrate*125)+0.5);
		}
		file.Close();
	}
}

//
//
//
MP3Info::~MP3Info(void)
{

}

//
//
//
int MP3Info::GetBitRate(void)
{
	return m_bitrate;
}

//
//
//
int MP3Info::GetDuration(void)
{
	return m_duration;
}