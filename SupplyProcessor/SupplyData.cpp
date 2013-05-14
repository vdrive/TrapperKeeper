#include "StdAfx.h"
#include "supplydata.h"

SupplyData::SupplyData(void)
{
	p_overnet_supply_packet=NULL;
	Clear();
}

SupplyData::SupplyData(const SupplyData &source) 
{
	p_overnet_supply_packet=NULL;
	Clear();
	operator=(source);
}

SupplyData::~SupplyData(void)
{
	if(p_overnet_supply_packet!=NULL)
		delete p_overnet_supply_packet;
}

SupplyData & SupplyData::operator=(const SupplyData &source)
{
	Clear();
	m_filename = source.m_filename;
	m_filesize = source.m_filesize;
	m_project_id = source.m_project_id;
	m_network_name = source.m_network_name;

	m_track = source.m_track;
	m_bitrate = source.m_bitrate;
	m_media_len = source.m_media_len;

	m_hash = source.m_hash;

	m_ip = source.m_ip;
	m_port = source.m_port;
	m_count = source.m_count;

	m_file_type = source.m_file_type;
	m_artist=source.m_artist;
	m_title=source.m_title;
	m_album=source.m_album;
	m_codec=source.m_codec;
	if(source.p_overnet_supply_packet!=NULL)
	{
		if(p_overnet_supply_packet!=NULL)
			delete p_overnet_supply_packet;
		p_overnet_supply_packet = new PacketBuffer();
		p_overnet_supply_packet->Append(*(source.p_overnet_supply_packet));
	}
	m_official_title=source.m_official_title;
	m_official_artist=source.m_official_artist;
	return *this;
}

/*
void SupplyData::GetInsertQuery(CString& query)
{
	query="insert ignore into supply_test.raw_supply values ";
	GetValueQuery(query);
}
*/

void SupplyData::GetValueQuery(CString& query)
{

	query.AppendFormat( 
		"('%s', %u, '%u', %d, %d, %d, '%s', inet_aton('%s'), %d, %d, '%s', curdate(), '%s', '%s', '%s', '%s', '%s')",
		SQLFormat(m_filename.c_str()), m_filesize, m_project_id,
		m_track, m_bitrate, m_media_len, 
		m_hash.c_str(), m_ip.c_str(), m_port, m_count, 
		SQLFormat(m_file_type.c_str()), SQLFormat(m_network_name.c_str()), 
		SQLFormat(m_artist.c_str()), SQLFormat(m_title.c_str()), SQLFormat(m_album.c_str()), SQLFormat(m_codec.c_str())
		);

}

CString SupplyData::SQLFormat(const char* in) 
{
	CString temp = in;
	temp.Replace("\\", "\\\\");
	temp.Replace("'", "\\'");
	temp.Replace("\"", "\\\"");
	return temp;
}

//
//
//
int SupplyData::GetBufferLength()
{
	int len = 0;

	len += (int)m_filename.length()+1;  
	len += sizeof(m_filesize);
	len += sizeof(m_project_id);
	len += (int)m_network_name.length()+1;

	len += sizeof(m_track);
	len += sizeof(m_bitrate);
	len += sizeof(m_media_len);

	len += (int)m_hash.length()+1; 

	len += (int)m_ip.length()+1;
	len += sizeof(m_port);
	len += sizeof(m_count);

	len += (int)m_file_type.length()+1;
	len += (int)m_artist.length()+1;
	len += (int)m_title.length()+1;
	len += (int)m_album.length()+1;
	len += (int)m_codec.length()+1;


	return len;
}

	
//
//
//
int SupplyData::WriteToBuffer(char *buf)
{
	// Clear the buffer
	memset(buf,0,GetBufferLength());
	
	char *ptr=buf;

	strcpy(ptr,m_filename.c_str());
	ptr+=m_filename.size()+1;	
	*((unsigned int *)ptr)=m_filesize;
	ptr+=sizeof(m_filesize);
	*((unsigned int*)ptr)=m_project_id;
	ptr+=sizeof(m_project_id);
	strcpy(ptr,m_network_name.c_str());
	ptr+=m_network_name.size()+1;	// +1 for NULL

	*((unsigned int *)ptr)=m_track;
	ptr+=sizeof(m_track);
	*((unsigned int *)ptr)=m_bitrate;
	ptr+=sizeof(m_bitrate);
	*((unsigned int *)ptr)=m_media_len;
	ptr+=sizeof(m_media_len);

	strcpy(ptr,m_hash.c_str());
	ptr+=m_hash.size()+1;	// +1 for NULL

	strcpy(ptr, m_ip.c_str());
	ptr+=m_ip.size()+1;	// +1 for NULL
	*((unsigned int *)ptr)=m_port;
	ptr+=sizeof(m_port);
	*((unsigned int *)ptr)=m_count;
	ptr+=sizeof(m_count);

	strcpy(ptr,m_file_type.c_str());
	ptr+=m_file_type.size()+1;	// +1 for NULL

	strcpy(ptr,m_artist.c_str());
	ptr+=m_artist.size()+1;	// +1 for NULL
	strcpy(ptr,m_title.c_str());
	ptr+=m_title.size()+1;	// +1 for NULL
	strcpy(ptr,m_album.c_str());
	ptr+=m_album.size()+1;	// +1 for NULL
	strcpy(ptr,m_codec.c_str());
	ptr+=m_codec.size()+1;	// +1 for NULL

	return GetBufferLength();
}
	
//
//
//
int SupplyData::ReadFromBuffer(char *buf)
{
	char *ptr=buf;

	m_filename = ptr;
	ptr+=strlen(ptr)+1;
	m_filesize=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);
	m_project_id = *((unsigned int*)ptr);
	ptr+=sizeof(unsigned int);
	m_network_name = ptr;
	ptr+=strlen(ptr)+1;

	m_track=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);
	m_bitrate=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);
	m_media_len=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	m_hash = ptr;
	ptr+=strlen(ptr)+1;

	m_ip= ptr;
	ptr+=strlen(ptr)+1;
	m_port=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);
	m_count=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);

	m_file_type = ptr;
	ptr+=strlen(ptr)+1;

	m_artist = ptr;
	ptr+=strlen(ptr)+1;
	m_title = ptr;
	ptr+=strlen(ptr)+1;
	m_album = ptr;
	ptr+=strlen(ptr)+1;
	m_codec = ptr;
	ptr+=strlen(ptr)+1;

	return GetBufferLength();
}

//
//
//
void SupplyData::CreateOvernetPacket()
{
	int tag_count=0;
    PacketBuffer next_buf;
	//filename
	next_buf.PutByte(0x02);
	next_buf.PutUShort(1);
	next_buf.PutByte(0x01);
	next_buf.PutUShort(m_filename.length());
	next_buf.PutStr(m_filename.c_str(),m_filename.length());
	tag_count++;

	//filesize
	//make a divisable to 137
	UINT new_filesize = m_filesize - (m_filesize%137);
	next_buf.PutByte(0x03);
	next_buf.PutUShort(1);
	next_buf.PutByte(0x02);
	next_buf.PutUInt(new_filesize);
	tag_count++;

	//file type audio
	if(stricmp(m_file_type.c_str(),"audio")==0)
	{
		next_buf.PutByte(0x02);
		next_buf.PutUShort(1);
		next_buf.PutByte(0x03);
		next_buf.PutUShort(5); // "Audio" length
		next_buf.PutStr("Audio",5);
		tag_count++;

		//Artist
		if(m_artist.length()>0)
		{
			next_buf.PutByte(0x02);
			next_buf.PutUShort(6);
			next_buf.PutStr("Artist",6);
			next_buf.PutUShort(m_artist.length());
			next_buf.PutStr(m_artist.c_str(),m_artist.length());
			tag_count++;
		}
		//Album
		if(m_album.length()>0)
		{
			next_buf.PutByte(0x02);
			next_buf.PutUShort(5);
			next_buf.PutStr("Album",5);
			next_buf.PutUShort(m_album.length());
			next_buf.PutStr(m_album.c_str(),m_album.length());
			tag_count++;
		}

		//Title
		if(m_title.length()>0)
		{
			next_buf.PutByte(0x02);
			next_buf.PutUShort(5);
			next_buf.PutStr("Title",5);
			next_buf.PutUShort(m_title.length());
			next_buf.PutStr(m_title.c_str(),m_title.length());
			tag_count++;
		}

		//bitrate
		if(m_bitrate!=0)
		{
			next_buf.PutByte(0x03);
			next_buf.PutUShort(7);
			next_buf.PutStr("bitrate",7);
			next_buf.PutUInt(m_bitrate);
			tag_count++;
		}
		//file format
		CString format=GetFileFormat(m_filename.c_str());
		if(format.GetLength()>0)
		{
			next_buf.PutByte(0x02);
			next_buf.PutUShort(1);
			next_buf.PutByte(0x04);
			next_buf.PutUShort(format.GetLength());
			next_buf.PutStr(format,format.GetLength());
			tag_count++;
		}
		//Length
		if(m_media_len>0)
		{
			CString str_length;
			if( m_media_len >= 3600)
				str_length.Format("%.dh %.dm %.ds", (int)((float)m_media_len/3600), (int)((m_media_len%3600)/60), (int)((m_media_len%3600)%60));
			else
				str_length.Format("%.2d:%.2d",(int)((float)m_media_len/60), (int)(m_media_len%60));

			next_buf.PutByte(0x02);
			next_buf.PutUShort(6);
			next_buf.PutStr("length",6);
			next_buf.PutUShort(str_length.GetLength());
			next_buf.PutStr(str_length,str_length.GetLength());
			tag_count++;
		}
	}
	else if(stricmp(m_file_type.c_str(),"video")==0)
	{
		next_buf.PutByte(0x02);
		next_buf.PutUShort(1);
		next_buf.PutByte(0x03);
		next_buf.PutUShort(5); // "Video" length
		next_buf.PutStr("Video",5);
		tag_count++;
		
		//Codec
		if(m_codec.length()>0)
		{
			next_buf.PutByte(0x02);
			next_buf.PutUShort(5);
			next_buf.PutStr("codec",5);
			next_buf.PutUShort(m_codec.length());
			next_buf.PutStr(m_codec.c_str(),m_codec.length());
			tag_count++;
		}
		//Length
		if(m_media_len>0)
		{
			CString str_length;
			if( m_media_len >= 3600)
				str_length.Format("%.dh %.dm %.ds", (int)((float)m_media_len/3600), (int)((m_media_len%3600)/60), (int)((m_media_len%3600)%60));
			else
				str_length.Format("%.2d:%.2d",(int)((float)m_media_len/60), (int)(m_media_len%60));

			next_buf.PutByte(0x02);
			next_buf.PutUShort(6);
			next_buf.PutStr("length",6);
			next_buf.PutUShort(str_length.GetLength());
			next_buf.PutStr(str_length,str_length.GetLength());
			tag_count++;
		}
		//file format
		CString format=GetFileFormat(m_filename.c_str());
		if(format.GetLength()>0)
		{
			next_buf.PutByte(0x02);
			next_buf.PutUShort(1);
			next_buf.PutByte(0x04);
			next_buf.PutUShort(format.GetLength());
			next_buf.PutStr(format,format.GetLength());
			tag_count++;
		}
	}
	else if( stricmp(m_file_type.c_str(),"software")==0)
	{
		next_buf.PutByte(0x02);
		next_buf.PutUShort(1);
		next_buf.PutByte(0x03);
		next_buf.PutUShort(3); // "Pro" length
		next_buf.PutStr("Pro",3);
		tag_count++;
	}

	//Avaiability
	next_buf.PutByte(0x03);
	next_buf.PutUShort(1);
	next_buf.PutByte(0x15);
	int rand1 = rand()%3; //randomly pick an availability number
	switch (rand1)
	{
		case 0: // one third will be either 254 or 255
		{
			if( (rand()%2)==0 )
				next_buf.PutUInt(255);
			else
				next_buf.PutUInt(254);
			break;
		}
		case 1:
		case 2: //two third will be 245 - 253
		{
			int rand2 = rand()%9;
			switch (rand2)
			{
				case 0:
				{
					next_buf.PutUInt(253);
					break;
				}
				case 1:
				{
					next_buf.PutUInt(252);
					break;
				}
				case 2:
				{
					next_buf.PutUInt(251);
					break;
				}
				case 3:
				{
					next_buf.PutUInt(250);
					break;
				}
				case 4:
				{
					next_buf.PutUInt(249);
					break;
				}
				case 5:
				{
					next_buf.PutUInt(248);
					break;
				}
				case 6:
				{
					next_buf.PutUInt(247);
					break;
				}
				case 7:
				{
					next_buf.PutUInt(246);
					break;
				}
				case 8:
				{
					next_buf.PutUInt(245);
					break;
				}
			}
			break;
		}
	}
	tag_count++;

	if(p_overnet_supply_packet!=NULL)
		delete p_overnet_supply_packet;
	p_overnet_supply_packet = new PacketBuffer();

	p_overnet_supply_packet->PutUInt(tag_count);	//meta tag list size
	p_overnet_supply_packet->Append(next_buf);
}

//
//
//
CString SupplyData::GetFileFormat(CString filename)
{
	CString format;
	int index = filename.ReverseFind('.');
	if(index > -1)
	{
		filename.Delete(0,index+1);
		format=filename;
	}
	return format;
}

//
//
//
void SupplyData::Clear()
{
	m_track = 0;
	m_count = 1;
	m_bitrate = 0;
	m_media_len = 0;
	m_ip = "0";
	m_port = 0;
	m_file_type = "OTHER";
	m_network_name = "unknown";
	m_official_title.Empty();
	m_official_artist.Empty();
	if(p_overnet_supply_packet!=NULL)
		delete p_overnet_supply_packet;
	p_overnet_supply_packet=NULL;
}