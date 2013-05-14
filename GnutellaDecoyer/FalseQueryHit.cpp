// FalseQueryHit.cpp

#include "stdafx.h"
#include "FalseQueryHit.h"

//
//
//
FalseQueryHit::FalseQueryHit()
{
	m_swarm_trailer=NULL;
	Clear();
}

//
//
//
FalseQueryHit::~FalseQueryHit()
{
	v_results.clear();
	if(m_swarm_trailer!=NULL)
	{
		delete [] m_swarm_trailer;
		m_swarm_trailer=NULL;
	}
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
	if(m_swarm_trailer!=NULL)
	{
		delete [] m_swarm_trailer;
		m_swarm_trailer=NULL;
	}
	m_swarm_trailer_len=0;
}

//
//
//
void FalseQueryHit::InitTrailer()
{
	memset(m_trailer,0,sizeof(m_trailer));
	//memcpy(m_trailer,"MRPH",4);	// say we are Morpheus
	memcpy(m_trailer,"LIME",4);	// say we are Limewire
	
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
//Must have filled out m_xml with xml string before calling this function
void FalseQueryHit::InitSwarmTrailer(const char* artist, const char* album, const char* title, const char* comment, int bitrate,int track)
{
	CString xml="{}<?xml version=\"1.0\"?><audios xsi:noNamespaceSchemaLocation=\"http://www.limewire.com/schemas/audio.xsd\"><audio title=\"";
	xml+=title;
	xml+="\" artist=\"";
	xml+=artist;
	xml+="\" album=\"";
	xml+=album;
	xml+="\" comments=\"";
	xml+=comment;
	if(track>-1)
	{
		xml+="\" track=\"";
		xml.AppendFormat("%d",track);
	}
	xml+="\" bitrate=\"";
	xml.AppendFormat("%d",bitrate);
	xml+="\" index=\"0\" ></audio></audios>";

	if(m_swarm_trailer!=NULL)
		delete [] m_swarm_trailer;
	m_swarm_trailer_len = 15+xml.GetLength()+1;
	m_swarm_trailer = new char[m_swarm_trailer_len];
	memset(m_swarm_trailer,0,m_swarm_trailer_len);
	//memcpy(m_trailer,"MRPH",4);	// say we are Morpheus
	memcpy(m_swarm_trailer,"LIME",4);	// say we are Limewire
	
	m_swarm_trailer[4]=0x04;	// open data size (no private data)

	// flags
	m_swarm_trailer[5]=0x3C; //00111100 - GGEP is meaningful, upload speed flag is meaningful, have uploaded flag is meaningful, busy flag is meaningful, NOT behind firewall

	// flags2
	m_swarm_trailer[6]=0x39;	// 00111001 - GGEP enabled,upload speed was entered by user and NOT calculated, have uploaded, NOT busy, firewalled flag is meaningful

	//xml length +1
	unsigned short int *xml_len=(unsigned short int*)&m_swarm_trailer[7];
	*xml_len= (unsigned short int)xml.GetLength() + 1;

	//limewire private vendor flag, chat supported
	m_swarm_trailer[9]=0x01;

	//GGEP magic prefix
	m_swarm_trailer[10]=0xC3;
	//GGEP flag
	m_swarm_trailer[11]=0x82;
	//GGEP ID
	m_swarm_trailer[12]=0x42;
	m_swarm_trailer[13]=0x48;
	m_swarm_trailer[14]=0x40;

	memcpy(&m_swarm_trailer[15],(const char*)xml,xml.GetLength());
	m_swarm_trailer[m_swarm_trailer_len-1]=0;
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
	if(m_swarm_trailer_len > 0)
		len += m_swarm_trailer_len;
	else
	{
		len+=sizeof(m_trailer);
	}
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

	if(m_swarm_trailer_len==0)
	{
		memcpy(ptr,m_trailer,sizeof(m_trailer));
		ptr+=sizeof(m_trailer);
	}
	else
	{
		memcpy(ptr,m_swarm_trailer,m_swarm_trailer_len);
		ptr+=m_swarm_trailer_len;
	}

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

		///* making the filesize to be dividable by 137 so that we can identify the spoofs  *///
			*size = (*size / 137) * 137;


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