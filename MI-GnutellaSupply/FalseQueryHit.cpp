// FalseQueryHit.cpp

#include "stdafx.h"
#include "FalseQueryHit.h"

//
//
//
FalseQueryHit::FalseQueryHit()
{
	Clear();
}

//
//
//
FalseQueryHit::~FalseQueryHit()
{
}

//
//
//
void FalseQueryHit::Clear()
{
	m_hdr.Clear();
	v_results.clear();
	memset(m_trailer,0,sizeof(m_trailer));
	memset(&m_guid,0,sizeof(GUID));
}

//
//
//
void FalseQueryHit::InitTrailer()
{
	memset(m_trailer,0,sizeof(m_trailer));
	
	memcpy(m_trailer,"MRPH",4);	// say we are Morpheus
	
	m_trailer[4]=0x02;	// open data size (no private data)

	// flags
	m_trailer[5]=0x1C;	// 00011100 - upload speed flag is meaningful, have uploaded flag is meaningful, busy flag is meaningful, NOT behind firewall

	// flags2
	m_trailer[6]=0x09;	// 00011001 - upload speed was entered by user and NOT calculated, have uploaded, NOT busy, firewalled flag is meaningful

	// Private data to identify us with...409
	unsigned short int *private_data=(unsigned short int *)&m_trailer[7];
	*private_data=409;
}

//
//
//
void FalseQueryHit::InitGUID()
{
	// Create GUID and make it look like a new gnutella GUID
	CoCreateGuid(&m_guid);
	unsigned char *ptr=(unsigned char *)&m_guid;
	ptr[8]=0xFF;
	ptr[15]=0x00;;
}

//
//
//
int FalseQueryHit::GetBufferLength()
{
	UINT i;
	int len=0;

	len+=sizeof(m_hdr);
	for(i=0;i<v_results.size();i++)
	{
		len+=v_results[i].GetBufferLength();
	}
	len+=sizeof(m_trailer);
	len+=sizeof(GUID);

	return len;
}

//
//
//
void FalseQueryHit::WriteToBuffer(char *buf)
{
	UINT i;

	char *ptr=buf;

	memset(ptr,0,GetBufferLength());

	memcpy(ptr,&m_hdr,sizeof(m_hdr));
	ptr+=sizeof(m_hdr);
	
	for(i=0;i<v_results.size();i++)
	{
		v_results[i].WriteToBuffer(ptr);
		ptr+=v_results[i].GetBufferLength();
	}

	memcpy(ptr,m_trailer,sizeof(m_trailer));
	ptr+=sizeof(m_trailer);

	memcpy(ptr,&m_guid,sizeof(GUID));
}

//
// Tweak the size and sha so that it doesn't clump with the real stuff (for spoofs only)
//
void FalseQueryHit::Tweak()
{
	UINT i,j;

	// Init the sha characters array
	char sha_chars[32];	// A-Z 2-7
	for(i=0;i<sizeof(sha_chars);i++)
	{
		if(i<26)
		{
			sha_chars[i]=i+'A';
		}
		else
		{
			sha_chars[i]=i-26+'2';
		}
	}


	for(i=0;i<v_results.size();i++)
	{
		// Tweaked Size = (Size-10%)+(rand() % 20%)
		unsigned int *size=&v_results[i].m_file_size;

		unsigned int ten_percent=*size/10;
		unsigned int twenty_percent=*size/5;

		if(twenty_percent>0)	// avoid divide by 0
		{
			*size=(*size-ten_percent)+(rand() % twenty_percent);
		}
		else
		{
			*size+=rand()%(1024+1);	// add between 0 and 1 K bytes if 20% of the size is 0
		}

		// See if the info contains a sha
		if(strstr(v_results[i].m_info.c_str(),"urn:sha1:")!=NULL)
		{
			// Tweak the sha with randonly selected sha characters

			char *info=new char[v_results[i].m_info.size()+1];
			strcpy(info,v_results[i].m_info.c_str());

			char *ptr=strstr(info,"urn:sha1:");
			ptr+=strlen("urn:sha1:");

			// generate a whole new sha
			for(j=0;j<32;j++)
			{
				// Just in case
				if(*ptr==NULL)
				{
					break;
				}

				*ptr=sha_chars[rand() % sizeof(sha_chars)];
				ptr++;
			}

			v_results[i].m_info=info;
			delete [] info;
		}
	}
}