// ID3v2TagFrameHeader.cpp

#include "stdafx.h"
#include "ID3v2TagFrameHeader.h"

//
//
//
ID3v2TagFrameHeader::ID3v2TagFrameHeader()
{
	Clear();
}

//
//
//
void ID3v2TagFrameHeader::Clear()
{
//	memset(this,0,sizeof(ID3v2TagFrameHeader));
	memset(m_frame_id,0,sizeof(m_frame_id));
	memset(m_size,0,sizeof(m_size));
	memset(m_flags,0,sizeof(m_flags));
}

//
//
//
void ID3v2TagFrameHeader::ExtractFrameHeaderPointer(unsigned char *ptr)
{
	memcpy(this,ptr,sizeof(ID3v2TagFrameHeader));
}

//
//
//
unsigned char *ID3v2TagFrameHeader::ReturnFrameIDPointer()
{
	return &m_frame_id[0];
}

//
//
//
unsigned int ID3v2TagFrameHeader::ReturnFrameSize()
{
	// Size of Tag Body and the whole Tag
	unsigned int length,size1,size2,size3,size4;
	size1=m_size[3]+(m_size[2]<<8)+(m_size[1]<<16)+(m_size[0]<<24);
	size2=size1;
	size3=size1;
	size4=size1;
		
	size1&=0xFF000000;
	size2&=0x00FF0000;
	size3&=0x0000FF00;
	size4&=0x000000FF;
/*
	size4>>=0;
	size3>>=1;
	size2>>=2;
	size1>>=3;
*/	
	length=size4+size3+size2+size1;		
	
	return length;
}

//
//
//
void ID3v2TagFrameHeader::SetFrameID(char *id)
{
	memcpy(m_frame_id,id,sizeof(m_frame_id));
}

//
//
//
void ID3v2TagFrameHeader::SetFrameSize(unsigned int size)
{
	// Size of Tag Body and the whole Tag
	unsigned int size1,size2,size3,size4;
	size1=size;
	size2=size;
	size3=size;
	size4=size;
/*
	size1&=0x0000007F;	// 7 bit numbers (synch-safe)
	size2&=0x00003F80;	
	size3&=0x001FC000;
	size4&=0x0FE00000;

	size1<<=0;
	size2<<=1;
	size3<<=2;
	size4<<=3;
*/
	size1&=0x000000FF;	
	size2&=0x0000FF00;	
	size3&=0x00FF0000;
	size4&=0xFF000000;

	m_size[3]=(size1>>0) & 0x0FF;
	m_size[2]=(size2>>8) & 0x0FF;
	m_size[1]=(size3>>16) & 0x0FF;
	m_size[0]=(size4>>24) & 0x0FF;
}
