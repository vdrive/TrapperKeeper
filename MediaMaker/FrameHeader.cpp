// FrameHeader.cpp

#include "stdafx.h"
#include "FrameHeader.h"

//
//
//
FrameHeader::FrameHeader()
{
	Clear();
	Init();

}

//
//
//
FrameHeader::FrameHeader(unsigned int bitrate)
{
	Clear();
	Init();

	if(bitrate==160)
	{
		m_data[2]&=0x0F;	// clear it 
		m_data[2]|=0xA0;	// reset it
	}
	else if(bitrate==192)
	{
		m_data[2]&=0x0F;	// clear it
		m_data[2]|=0xB0;	// reset it
	}
}

//
//
//
void FrameHeader::Clear()
{
//	memset(this,0,sizeof(FrameHeader));
	memset(m_data,0,sizeof(m_data));
}

//
//
//
void FrameHeader::Init()
{
	// Set synch				1111 1111 : 111x xxxx :: xxxx xxxx : xxxx xxxx
	m_data[0]|=0xFF;
	m_data[1]|=0xE0;

	// Set MPEG-1				xxxx xxxx : xxx1 1xxx :: xxxx xxxx : xxxx xxxx
	m_data[1]|=0x18;

	// Set Layer III			xxxx xxxx : xxxx x01x :: xxxx xxxx : xxxx xxxx
	m_data[1]|=0x02;

	// Set Not Protected 		xxxx xxxx : xxxx xxx1 :: xxxx xxxx : xxxx xxxx
	m_data[1]|=0x01;

	// Set 128kbps				xxxx xxxx : xxxx xxxx :: 1001 xxxx : xxxx xxxx
	m_data[2]|=0x90;

	// Set 44.1 kHz				xxxx xxxx : xxxx xxxx :: xxxx 00xx : xxxx xxxx
	m_data[2]|=0x00;

	// Set not padded			xxxx xxxx : xxxx xxxx :: xxxx xx0x : xxxx xxxx
	m_data[2]|=0x00;

	// Set not private			xxxx xxxx : xxxx xxxx :: xxxx xxx0 : xxxx xxxx
	m_data[2]|=0x00;		
/*
	// Set Stereo				xxxx xxxx : xxxx xxxx :: xxxx xxxx : 00xx xxxx
	m_data[3]|=0x00;
*/
	// Set Mono 				xxxx xxxx : xxxx xxxx :: xxxx xxxx : 00xx xxxx
	m_data[3]|=0x11;


	// Set No Mode Extensions	xxxx xxxx : xxxx xxxx :: xxxx xxxx : xx00 xxxx
	m_data[3]|=0x00;

	// Set Not Copyrighted		xxxx xxxx : xxxx xxxx :: xxxx xxxx : xxxx 0xxx
	m_data[3]|=0x00;

	// Set Copy of Orig Media	xxxx xxxx : xxxx xxxx :: xxxx xxxx : xxxx x0xx
	m_data[3]|=0x00;

	// Set No Emphasis			xxxx xxxx : xxxx xxxx :: xxxx xxxx : xxxx xx00
	m_data[3]|=0x00;
}

//
// 1 - read in header, 0 - it was the start of an ID3v1 tag, -1 - error
//
int FrameHeader::ExtractHeader(CFile *file)
{
	// Keep reading in 0x00's
	unsigned char c;
	while(file->Read(&c,1)==1)
	{
		if(c!=0x00)
		{
			file->Seek(-1,CFile::current);	// back up when it is not a 0x00
			break;
		}
	}

	if(int size = file->Read(this,sizeof(FrameHeader))!=sizeof(FrameHeader))
	{
		int headersize = sizeof(FrameHeader);
		DWORD damnerror = ::GetLastError();
		return -1;
	}

	// Check to see if it is valid
	if(IsValid())
	{
		return 1;
	}
	else
	{
		// Check to see if it is the start of an ID3v1 tag
		if(IsID3v1Tag())
		{
			return 0;
		}
		else	// error
		{
			return -1;
		}
	}
}

//
//
//
bool FrameHeader::SetHeader(unsigned char *buf)
{
	memcpy(m_data,buf,sizeof(m_data));
	return IsValid();
}

//
//
//
bool FrameHeader::IsValid()
{
	if(IsSynched() && IsMPEG1() && IsLayerIII())
	{
		return true;
	}
	else
	{
		return false;
	}
}

//
//
//
bool FrameHeader::IsID3v1Tag()
{
	if(memcmp(m_data,"TAG",3)==0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//
//
//
bool FrameHeader::IsSynched()
{
	// Check 11 sync bits
	if((m_data[0] & 0xFF) == 0xFF)
	{
		if((m_data[1] & 0xE0) == 0xE0)
		{
			return true;
		}
	}

	return false;
}

//
//
//
bool FrameHeader::IsMPEG1()
{
	if((m_data[1] & 0x18) == 0x18)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//
//
//
bool FrameHeader::IsLayerIII()
{
	if((m_data[1] & 0x06) == 0x02)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//
//
//
bool FrameHeader::IsProtected()
{
	if((m_data[1] & 0x01) == 0x01)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//
// Return value is bps
//
unsigned int FrameHeader::ReturnBitrate()
{
	// Valid for MPEG 1 Layer III only

	unsigned int ret=0;
	switch(m_data[2] & 0xF0)
	{
		case 0x00:
		{
			ret=0;	// free
			break;
		}
		case 0x10:
		{
			ret=32;
			break;
		}
		case 0x20:
		{
			ret=40;
			break;
		}
		case 0x30:
		{
			ret=48;
			break;
		}
		case 0x40:
		{
			ret=56;
			break;
		}
		case 0x50:
		{
			ret=64;
			break;
		}
		case 0x60:
		{
			ret=80;
			break;
		}
		case 0x70:
		{
			ret=96;
			break;
		}
		case 0x80:
		{
			ret=112;
			break;
		}
		case 0x90:
		{
			ret=128;
			break;
		}
		case 0xA0:
		{
			ret=160;
			break;
		}
		case 0xB0:
		{
			ret=192;
			break;
		}
		case 0xC0:
		{
			ret=224;
			break;
		}
		case 0xD0:
		{
			ret=256;
			break;
		}
		case 0xE0:
		{
			ret=320;
			break;
		}
		case 0xF0:
		{
			ret=0;	// bad
			break;
		}
	}

	return ret*1000;	// bps
}

//
// Return value is in Hz
//
unsigned int FrameHeader::ReturnSamplingRate()
{
	// Valid for MPEG 1 only

	unsigned int ret=0;
	switch(m_data[2] & 0x0C)
	{
		case 0x00:
		{
			ret=(int)(44.1*1000);
			break;
		}
		case 0x04:
		{
			ret=48*1000;
			break;
		}
		case 0x08:
		{
			ret=32*1000;
			break;
		}
		case 0x0C:
		{
			ret=0;	// reserved
			break;
		}
	}


	//Kludge to default to a nonzero value when the header is corrupt
	//should no longer be necessary
	if (ret == 0)
	{
		ret = 32*1000;
	}

	return ret;	
}

//
//
//
bool FrameHeader::IsPadded()
{
	if((m_data[2] & 0x02) == 0x02)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//
//
//
void FrameHeader::IsPadded(bool is_padded)
{
	// Clear the bit
	m_data[2] &= 0xFD;
	
	// Set it if padded
	if(is_padded)
	{
		m_data[2] |= 0x02;
	}
}

//
//
//
unsigned int FrameHeader::ReturnFrameSize()
{
	unsigned int bitrate=ReturnBitrate();
	unsigned int sample_rate=ReturnSamplingRate();
	unsigned int padding=0;
	if(IsPadded())
	{
		padding=1;
	}

	unsigned int size=((144 * bitrate) / sample_rate) + padding;

	// if the size is invalid set it to default 128000/44100
	if (size == 0)
	{
		size = 418;
	}
	return size;
}