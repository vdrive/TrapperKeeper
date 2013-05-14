// ID3v2TagHeader.cpp

#include "stdafx.h"
#include "ID3v2TagHeader.h"

//
//
//
ID3v2TagHeader::ID3v2TagHeader()
{
	Init();
}

//
//
//
void ID3v2TagHeader::Clear()
{
//	memset(this,0,sizeof(ID3v2TagHeader));
	memset(m_tag,0,sizeof(m_tag));	// "ID3"
	memset(m_version,0,sizeof(m_version));
	memset(m_flags,0,sizeof(m_flags));
	memset(m_size,0,sizeof(m_size));
}

//
//
//
void ID3v2TagHeader::Init()
{
	Clear();
	
	memcpy(m_tag,"ID3",3);
	m_version[0]=0x03;
	m_version[1]=0x00;
	
	// the flags and size are good at 0
}

//
//
//
bool ID3v2TagHeader::ExtractTagHeader(CFile *file)
{
	file->SeekToBegin();

	file->Read(this,sizeof(ID3v2TagHeader));

	if(IsValid())
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
string ID3v2TagHeader::ReturnVersion()
{
	char buf[1024+1];
	sprintf(buf,"%u.%u",m_version[0],m_version[1]);

	string ret=buf;
	return ret;
}

//
//
//
bool ID3v2TagHeader::IsValid()
{
	if(memcmp(m_tag,"ID3",sizeof(m_tag))==0)
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
bool ID3v2TagHeader::IsUnsynchronizationUsed()
{
	if(m_flags[0] & 0x80)
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
bool ID3v2TagHeader::IsExtendedHeaderPresent()
{
	if(m_flags[0] & 0x40)
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
bool ID3v2TagHeader::IsTagExperimental()
{
	if(m_flags[0] & 0x20)
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
bool ID3v2TagHeader::IsFooterPresent()
{
	if(m_flags[0] & 0x10)
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
unsigned int ID3v2TagHeader::GetBodyLengthValue()
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

	size4>>=0;
	size3>>=1;
	size2>>=2;
	size1>>=3;
	
	length=size4+size3+size2+size1;		
	
	return length;
}

//
//
//
string ID3v2TagHeader::GetBodyLength()
{
	unsigned int length=GetBodyLengthValue();

	char buf[1024+1];
	_itoa(length,buf,10);

	string ret=buf;
	return ret;
}

//
//
//
unsigned int ID3v2TagHeader::GetTagLengthValue()
{
	unsigned int length=GetBodyLengthValue();

	// header length + body length + footer length if present
	if(IsFooterPresent())
	{
		return sizeof(ID3v2TagHeader)+length+10;
	}
	else
	{
		return sizeof(ID3v2TagHeader)+length;
	}
}

//
//
//
string ID3v2TagHeader::GetTagLength()
{
	unsigned int length=GetTagLengthValue();
	
	char buf[1024+1];
	_itoa(length,buf,10);

	string ret=buf;
	return ret;
}

//
//
//
void ID3v2TagHeader::SetTagSize(unsigned int size)
{
	// Size of Tag Body minus the header
	unsigned int size1,size2,size3,size4;
	size1=size;
	size2=size;
	size3=size;
	size4=size;

	size1&=0x0000007F;	// 7 bit numbers (synch-safe)
	size2&=0x00003F80;	
	size3&=0x001FC000;
	size4&=0x0FE00000;

	size1<<=0;
	size2<<=1;
	size3<<=2;
	size4<<=3;

	m_size[3]=(size1>>0) & 0x0FF;
	m_size[2]=(size2>>8) & 0x0FF;
	m_size[1]=(size3>>16) & 0x0FF;
	m_size[0]=(size4>>24) & 0x0FF;
}