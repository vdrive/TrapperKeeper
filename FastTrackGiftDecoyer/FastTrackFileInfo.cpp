#include "StdAfx.h"
#include "fasttrackfileinfo.h"
#include "FTHash.h"
#include "md5.h"	// for MD5 stuff
#include "giFThash.h"	// for hashSmallHash stuff
#include <math.h>	// for sqrt

FastTrackFileInfo::FastTrackFileInfo(void)
{
	m_rng.Reseed(true,32);
	Clear();
}

//
//
//
FastTrackFileInfo::~FastTrackFileInfo(void)
{
}

//
//
//
void FastTrackFileInfo::Clear()
{
	//m_has_created_unique_hash=false;
	m_filename.Empty();
	memset(&m_hash, 0, FST_FTHASH_LEN);
	m_title.Empty();
	m_artist.Empty();
	m_album.Empty();
	m_keyword.Empty();
	m_description.Empty();
	m_category.Empty();
	//m_type.Empty();
//	m_language.Empty();

	//m_project_id=0;
	m_filesize=0;
    //m_year=0;
	m_bit_rate=0;
	m_track_number=0;
	m_duration=0;
	m_resolution1=0;
	m_resolution2=0;
//	m_integrity=0;

	m_media_type=MEDIA_TYPE_UNKNOWN;
	m_num_shares=0;

	m_language="en";
	m_integrity=4;
	m_type="Audio";
	m_path.Empty();
	m_codec.Empty();
}

//
//
//
int FastTrackFileInfo::GetBufferLength()
{
	int count = 0;
	count+= m_filename.GetLength()+1;
	count+=sizeof(m_hash);
	count+=m_title.GetLength()+1;
	count+=m_artist.GetLength()+1;
	count+=m_album.GetLength()+1;
	count+=m_keyword.GetLength()+1;
	count+=m_description.GetLength()+1;
	count+=m_category.GetLength()+1;
	count+=m_type.GetLength()+1;
	count+=m_language.GetLength()+1;
	//count+=m_project_name.GetLength()+1;

	//count+=sizeof(m_project_id);
	count+=sizeof(m_filesize);
	count+=sizeof(m_bit_rate);
	count+=sizeof(m_track_number);
	count+=sizeof(m_duration);
	count+=sizeof(m_resolution1);
	count+=sizeof(m_resolution2);
	count+=sizeof(m_integrity);
	count+=sizeof(m_media_type);
	count+=m_path.GetLength()+1;
	count+=m_codec.GetLength()+1;
	return count;
}

	
///
//
//
int FastTrackFileInfo::WriteToBuffer(char *buf)
{
	// Clear buffer
	memset(buf,0,GetBufferLength());

	char *ptr=buf;

	if(m_filename.GetLength() != 0)
	{
		strcpy(ptr,m_filename);
	}
	ptr+=m_filename.GetLength()+1;
	if(m_title.GetLength() != 0)
	{
		strcpy(ptr,m_title);
	}
	ptr+=m_title.GetLength()+1;
	if(m_artist.GetLength() != 0)
	{
		strcpy(ptr,m_artist);
	}
	ptr+=m_artist.GetLength()+1;
	if(m_album.GetLength() != 0)
	{
		strcpy(ptr,m_album);
	}
	ptr+=m_album.GetLength()+1;
	if(m_keyword.GetLength() != 0)
	{
		strcpy(ptr,m_keyword);
	}
	ptr+=m_keyword.GetLength()+1;
	if(m_description.GetLength() != 0)
	{
		strcpy(ptr,m_description);
	}
	ptr+=m_description.GetLength()+1;
	if(m_category.GetLength() != 0)
	{
		strcpy(ptr,m_category);
	}
	ptr+=m_category.GetLength()+1;
	if(m_type.GetLength() != 0)
	{
		strcpy(ptr,m_type);
	}
	ptr+=m_type.GetLength()+1;
	if(m_language.GetLength() != 0)
	{
		strcpy(ptr,m_language);
	}
	ptr+=m_language.GetLength()+1;
	/*
	if(m_project_name.GetLength() != 0)
	{
		strcpy(ptr,m_project_name);
	}
	ptr+=m_project_name.GetLength()+1;
	*/
	memcpy(ptr,m_hash,sizeof(m_hash));
	ptr+= sizeof(m_hash);
/*
	*((unsigned int *)ptr)=m_project_id;
	ptr+=sizeof(m_project_id);
*/
	*((unsigned int *)ptr)=m_filesize;
	ptr+=sizeof(m_filesize);
	*((unsigned short *)ptr)=m_bit_rate;
	ptr+=sizeof(m_bit_rate);
	*((unsigned short *)ptr)=m_track_number;
	ptr+=sizeof(m_track_number);
	*((unsigned short *)ptr)=m_duration;
	ptr+=sizeof(m_duration);
	*((unsigned short *)ptr)=m_resolution1;
	ptr+=sizeof(m_resolution1);
	*((unsigned short *)ptr)=m_resolution2;
	ptr+=sizeof(m_resolution2);
	*((unsigned short *)ptr)=m_integrity;
	ptr+=sizeof(m_integrity);
	*((FSTMediaType *)ptr)=m_media_type;
	ptr+=sizeof(m_media_type);

	if(m_path.GetLength() != 0)
	{
		strcpy(ptr,m_path);
	}
	ptr+=m_path.GetLength()+1;
	if(m_codec.GetLength() != 0)
	{
		strcpy(ptr,m_codec);
	}
	ptr+=m_codec.GetLength()+1;
	return GetBufferLength();
}

	
//
//
//
int FastTrackFileInfo::ReadFromBuffer(char *buf)
{
	Clear();
	char *ptr=buf;
	
	m_filename = ptr;
	ptr+=m_filename.GetLength()+1;
	m_title = ptr;
	ptr+=m_title.GetLength()+1;
	m_artist = ptr;
	ptr+=m_artist.GetLength()+1;
	m_album = ptr;
	ptr+=m_album.GetLength()+1;
	m_keyword = ptr;
	ptr+=m_keyword.GetLength()+1;
	m_description = ptr;
	ptr+=m_description.GetLength()+1;
	m_category = ptr;
	ptr+=m_category.GetLength()+1;
	m_type = ptr;
	ptr+=m_type.GetLength()+1;
	m_language = ptr;
	ptr+=m_language.GetLength()+1;
//	m_project_name = ptr;
//	ptr+=m_project_name.GetLength()+1;
	memcpy(m_hash,ptr, sizeof(m_hash));
	ptr+=sizeof(m_hash);

//	m_project_id=*((unsigned int *)ptr);
//	ptr+=sizeof(unsigned int);
	m_filesize=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);
	m_bit_rate=*((unsigned short *)ptr);
	ptr+=sizeof(unsigned short);
	m_track_number=*((unsigned short *)ptr);
	ptr+=sizeof(unsigned short);
	m_duration=*((unsigned short *)ptr);
	ptr+=sizeof(unsigned short);
	m_resolution1=*((unsigned short *)ptr);
	ptr+=sizeof(unsigned short);
	m_resolution2=*((unsigned short *)ptr);
	ptr+=sizeof(unsigned short);
	m_integrity=*((unsigned short *)ptr);
	ptr+=sizeof(unsigned short);

	m_media_type=*((FSTMediaType *)ptr);
	ptr+=sizeof(FSTMediaType);

	m_path = ptr;
	ptr+=m_path.GetLength()+1;
	m_codec = ptr;
	ptr+=m_codec.GetLength()+1;

	return GetBufferLength();
}

//
//
//
PacketBuffer* FastTrackFileInfo::GetShareFile(/*bool same_hash*/)
{
	if(m_media_type == MEDIA_TYPE_AUDIO)
	{
		PacketBuffer* buf = new PacketBuffer();
		CreateShareForAudio(buf/*,same_hash*/);
		m_num_shares++;
		return buf;
	}
	if(m_media_type == MEDIA_TYPE_VIDEO)
	{
		PacketBuffer* buf = new PacketBuffer();
		CreateShareForVideo(buf/*,same_hash*/);
		m_num_shares++;
		return buf;
	}
	if(m_media_type == MEDIA_TYPE_SOFTWARE)
	{
		PacketBuffer* buf = new PacketBuffer();
		CreateShareForSoftware(buf/*,same_hash*/);
		m_num_shares++;
		return buf;
	}
	return NULL; //unknown type
}

//
//
//
void FastTrackFileInfo::CreateShareForAudio(PacketBuffer* buf/*, bool same_hash*/)
{
	buf->PutByte(0x00); //unknown
	buf->PutByte(MEDIA_TYPE_AUDIO); //media type
	buf->PutStr("\x00\x00",2);
	buf->PutUStr(m_hash,FST_FTHASH_LEN); //hash
	buf->PutDynInt(GetHashChecksum(m_hash)); //file_id
	buf->PutDynInt(m_filesize); //file size

	int number_of_tags = 0;
	PacketBuffer* tag_buf = new PacketBuffer();

	//filename
	if(m_filename.GetLength() > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_FILENAME);	//tag type
		tag_buf->PutDynInt((UINT)m_filename.GetLength());	//tag length
		tag_buf->PutStr(m_filename,m_filename.GetLength()); //tag content
		number_of_tags++;
	}

	//artist
	if(m_artist.GetLength() > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_ARTIST);
		tag_buf->PutDynInt((UINT)m_artist.GetLength());
		tag_buf->PutStr(m_artist,m_artist.GetLength());
		number_of_tags++;
	}

	//album
	if(m_album.GetLength() > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_ALBUM);
		tag_buf->PutDynInt((UINT)m_album.GetLength());
		tag_buf->PutStr(m_album,m_album.GetLength());
		number_of_tags++;
	}

	//title
	if(m_title.GetLength() > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_TITLE);	//tag type
		tag_buf->PutDynInt((UINT)m_title.GetLength());	//tag length
		tag_buf->PutStr(m_title,m_title.GetLength()); //tag content
		number_of_tags++;
	}

	//keyword
	if(m_keyword.GetLength() > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_KEYWORDS);	//tag type
		tag_buf->PutDynInt((UINT)m_keyword.GetLength());	//tag length
		tag_buf->PutStr(m_keyword,m_keyword.GetLength()); //tag content
		number_of_tags++;
	}

	//description
	if(m_description.GetLength() > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_COMMENT);	//tag type
		tag_buf->PutDynInt((UINT)m_description.GetLength());	//tag length
		tag_buf->PutStr(m_description,m_description.GetLength()); //tag content
		number_of_tags++;
	}

	//category
	if(m_category.GetLength() > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_CATEGORY);	//tag type
		tag_buf->PutDynInt((UINT)m_category.GetLength());	//tag length
		tag_buf->PutStr(m_category,m_category.GetLength()); //tag content
		number_of_tags++;
	}

	//duration
	if(m_duration > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_TIME);	//tag type
		PacketBuffer* temp_buf = new PacketBuffer();
		temp_buf->PutDynInt(m_duration);
		tag_buf->PutDynInt(temp_buf->Size());
		temp_buf->Rewind();
		tag_buf->Append(*temp_buf);
		delete temp_buf;
		number_of_tags++;
	}

	//year
	/*
	tag_buf->PutDynInt(FILE_TAG_YEAR);	//tag type
	temp_buf = new PacketBuffer();
	temp_buf->PutDynInt(m_year);
	tag_buf->PutDynInt(temp_buf->Size());
	temp_buf->Rewind();
	tag_buf->Append(*temp_buf);
	delete temp_buf;
	*/

	//bitrate
	if(m_bit_rate > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_QUALITY);	//tag type
		PacketBuffer* temp_buf = new PacketBuffer();
		temp_buf->PutDynInt(m_bit_rate);
		tag_buf->PutDynInt(temp_buf->Size());
		temp_buf->Rewind();
		tag_buf->Append(*temp_buf);
		delete temp_buf;
		number_of_tags++;
	}

	//rating
	if(rand()%5==0) // one fifth chance to show Integrity
	{
		if(m_integrity > 0)
		{
			tag_buf->PutDynInt(FILE_TAG_RATING);	//tag type
			PacketBuffer* temp_buf = new PacketBuffer();
			temp_buf->PutDynInt(m_integrity);
			tag_buf->PutDynInt(temp_buf->Size());
			temp_buf->Rewind();
			tag_buf->Append(*temp_buf);
			delete temp_buf;		
			number_of_tags++;
		}
	}

	buf->PutDynInt(number_of_tags);	//number of tag
	tag_buf->Rewind();
	buf->Append(*tag_buf);
	delete tag_buf;
}

//
//
//
void FastTrackFileInfo::CreateShareForVideo(PacketBuffer* buf/*, bool same_hash*/)
{


	buf->PutByte(0x00); //unknown
	buf->PutByte(MEDIA_TYPE_VIDEO); //media type
	buf->PutStr("\x00\x00",2);
	buf->PutUStr(m_hash,FST_FTHASH_LEN); //hash
	buf->PutDynInt(GetHashChecksum(m_hash)); //file_id
	buf->PutDynInt(m_filesize); //file size

	int number_of_tags = 0;
	PacketBuffer* tag_buf = new PacketBuffer();

	//filename
	if(m_filename.GetLength() > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_FILENAME);	//tag type
		tag_buf->PutDynInt((UINT)m_filename.GetLength());	//tag length
		tag_buf->PutStr(m_filename,m_filename.GetLength()); //tag content
		number_of_tags++;
	}

	//artist
	if(m_artist.GetLength() > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_ARTIST);
		tag_buf->PutDynInt((UINT)m_artist.GetLength());
		tag_buf->PutStr(m_artist,m_artist.GetLength());
		number_of_tags++;
	}

	//type
	if(m_type.GetLength() > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_TYPE);	//tag type
		tag_buf->PutDynInt((UINT)m_type.GetLength());	//tag length
		tag_buf->PutStr(m_type,m_type.GetLength()); //tag content
		number_of_tags++;
	}

	//title
	if(m_title.GetLength() > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_TITLE);	//tag type
		tag_buf->PutDynInt((UINT)m_title.GetLength());	//tag length
		tag_buf->PutStr(m_title,m_title.GetLength()); //tag content
		number_of_tags++;
	}

	//keyword
	if(m_keyword.GetLength() > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_KEYWORDS);	//tag type
		tag_buf->PutDynInt((UINT)m_keyword.GetLength());	//tag length
		tag_buf->PutStr(m_keyword,m_keyword.GetLength()); //tag content
		number_of_tags++;
	}

	//description
	if(m_description.GetLength() > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_COMMENT);	//tag type
		tag_buf->PutDynInt((UINT)m_description.GetLength());	//tag length
		tag_buf->PutStr(m_description,m_description.GetLength()); //tag content
		number_of_tags++;
	}

	//category
	if(m_category.GetLength() > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_CATEGORY);	//tag type
		tag_buf->PutDynInt((UINT)m_category.GetLength());	//tag length
		tag_buf->PutStr(m_category,m_category.GetLength()); //tag content
		number_of_tags++;
	}
	//Lauguage
	if(m_language.GetLength() > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_LANGUAGE);	//tag type
		tag_buf->PutDynInt((UINT)m_language.GetLength());	//tag length
		tag_buf->PutStr(m_language,m_language.GetLength()); //tag content
		number_of_tags++;
	}

	//duration
	if(m_duration > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_TIME);	//tag type
		PacketBuffer* temp_buf = new PacketBuffer();
		temp_buf->PutDynInt(m_duration);
		tag_buf->PutDynInt(temp_buf->Size());
		temp_buf->Rewind();
		tag_buf->Append(*temp_buf);
		delete temp_buf;
		number_of_tags++;
	}
	
	if(m_codec.GetLength() > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_CODEC);	//tag type
		tag_buf->PutDynInt((UINT)m_codec.GetLength());	//tag length
		tag_buf->PutStr(m_codec,m_codec.GetLength()); //tag content
		number_of_tags++;
	}

	if(m_resolution1 > 0 && m_resolution2 > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_RESOLUTION);	//tag type
		PacketBuffer* temp_buf = new PacketBuffer();
		temp_buf->PutDynInt(m_resolution1);
		PacketBuffer* temp_buf2 = new PacketBuffer();
		temp_buf2->PutDynInt(m_resolution2);
		tag_buf->PutDynInt(temp_buf->Size()+temp_buf2->Size());
		temp_buf->Rewind();
		tag_buf->Append(*temp_buf);
		temp_buf2->Rewind();
		tag_buf->Append(*temp_buf2);
		delete temp_buf;
		delete temp_buf2;
		number_of_tags++;
	}

	//rating
	if(rand()%5==0) // one fifth chance to show Integrity
	{
		if(m_integrity > 0)
		{
			tag_buf->PutDynInt(FILE_TAG_RATING);	//tag type
			PacketBuffer* temp_buf = new PacketBuffer();
			temp_buf->PutDynInt(m_integrity);
			tag_buf->PutDynInt(temp_buf->Size());
			temp_buf->Rewind();
			tag_buf->Append(*temp_buf);
			delete temp_buf;		
			number_of_tags++;
		}
	}


	buf->PutDynInt(number_of_tags);	//number of tag
	tag_buf->Rewind();
	buf->Append(*tag_buf);
	delete tag_buf;
}

//
//
//
unsigned short FastTrackFileInfo::GetHashChecksum (unsigned char *hash)
{
	unsigned short sum = 0;
	int i;

	unsigned char* hash_ptr = hash;
	/* calculate 2 byte checksum used in the URL from 20 byte fthash */
	for (i = 0; i < FST_FTHASH_LEN; i++)
	{
		sum = checksumtable[(*hash_ptr)^(sum >> 8)] ^ (sum << 8);
		hash_ptr++;
	}

	return (sum & 0x3fff);
}

//
//
//
void FastTrackFileInfo::SetHash(const char* hash)
{
	FTHash fthash;
	fthash.HashDecode16(hash);
	memset(&m_hash,0,sizeof(m_hash));
	memcpy(m_hash,fthash.m_data,sizeof(m_hash));
}


//
//
//
void FastTrackFileInfo::CreateShareForSoftware(PacketBuffer* buf/*, bool same_hash*/)
{

	buf->PutByte(0x00); //unknown
	buf->PutByte(MEDIA_TYPE_SOFTWARE); //media type
	buf->PutStr("\x00\x00",2);
	buf->PutUStr(m_hash,FST_FTHASH_LEN); //hash
	buf->PutDynInt(GetHashChecksum(m_hash)); //file_id
	buf->PutDynInt(m_filesize); //file size

	int number_of_tags = 0;
	PacketBuffer* tag_buf = new PacketBuffer();

	//filename
	if(m_filename.GetLength() > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_FILENAME);	//tag type
		tag_buf->PutDynInt((UINT)m_filename.GetLength());	//tag length
		tag_buf->PutStr(m_filename,m_filename.GetLength()); //tag content
		number_of_tags++;
	}

	//artist
	if(m_artist.GetLength() > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_ARTIST);
		tag_buf->PutDynInt((UINT)m_artist.GetLength());
		tag_buf->PutStr(m_artist,m_artist.GetLength());
		number_of_tags++;
	}

	//title
	if(m_title.GetLength() > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_TITLE);	//tag type
		tag_buf->PutDynInt((UINT)m_title.GetLength());	//tag length
		tag_buf->PutStr(m_title,m_title.GetLength()); //tag content
		number_of_tags++;
	}

	//keyword
	if(m_keyword.GetLength() > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_KEYWORDS);	//tag type
		tag_buf->PutDynInt((UINT)m_keyword.GetLength());	//tag length
		tag_buf->PutStr(m_keyword,m_keyword.GetLength()); //tag content
		number_of_tags++;
	}

	//description
	if(m_description.GetLength() > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_COMMENT);	//tag type
		tag_buf->PutDynInt((UINT)m_description.GetLength());	//tag length
		tag_buf->PutStr(m_description,m_description.GetLength()); //tag content
		number_of_tags++;
	}

	//category
	if(m_category.GetLength() > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_CATEGORY);	//tag type
		tag_buf->PutDynInt((UINT)m_category.GetLength());	//tag length
		tag_buf->PutStr(m_category,m_category.GetLength()); //tag content
		number_of_tags++;
	}


	//rating
	if(rand()%5==0) // one fifth chance to show Integrity
	{
		if(m_integrity > 0)
		{
			tag_buf->PutDynInt(FILE_TAG_RATING);	//tag type
			PacketBuffer* temp_buf = new PacketBuffer();
			temp_buf->PutDynInt(m_integrity);
			tag_buf->PutDynInt(temp_buf->Size());
			temp_buf->Rewind();
			tag_buf->Append(*temp_buf);
			delete temp_buf;		
			number_of_tags++;
		}
	}

	buf->PutDynInt(number_of_tags);	//number of tag
	tag_buf->Rewind();
	buf->Append(*tag_buf);
	delete tag_buf;
}

//
//
//
bool FastTrackFileInfo::CalculateHash()
{
	CFile file_in;
	CString filename = m_path + m_filename;
	if(file_in.Open(filename,CFile::modeRead|CFile::typeBinary|CFile::shareDenyNone)==NULL)
		return false;

	// Read in first 300K
	unsigned char md5_buf[300*1024];
	unsigned int num_read=file_in.Read(md5_buf,sizeof(md5_buf));

	MD5Context md5;
	MD5Init(&md5);
	MD5Update(&md5,&md5_buf[0],num_read);
	MD5Final(m_hash,&md5);	// first 16 bytes of that hash

	// Calculate the 4-byte small hash.
	unsigned int smallhash = 0xffffffff;
	unsigned int chunk_size=300*1024;
	unsigned char buf[300*1024];

	if(file_in.GetLength() > chunk_size)
	{
		size_t offset = 0x100000;	// 1 MB
		size_t lastpos = chunk_size;	// 300K
		size_t endlen;
		while(offset+2*chunk_size < file_in.GetLength()) 
		{
			file_in.Seek(offset,CFile::begin);
			file_in.Read(buf,chunk_size);

			smallhash = hashSmallHash(buf, chunk_size, smallhash);
			lastpos = offset+chunk_size;
			offset <<= 1;
		}

		endlen = file_in.GetLength() - lastpos;
		if (endlen > chunk_size) 
		{
			endlen = chunk_size;
		}
		file_in.Seek(file_in.GetLength() - endlen,CFile::begin);
		file_in.Read(buf,endlen);

		smallhash = hashSmallHash(buf, endlen, smallhash);
	}

	smallhash ^= file_in.GetLength();
	m_hash[16] = smallhash & 0xff;
	m_hash[17] = (smallhash >> 8) & 0xff;
	m_hash[18] = (smallhash >> 16) & 0xff;
	m_hash[19] = (smallhash >> 24) & 0xff;

	file_in.Close();
	return true;
}

//
//
//
void FastTrackFileInfo::SetVideoPresetOptions()
{
	m_type="Video";
	m_media_type = MEDIA_TYPE_VIDEO;
	m_resolution1=352;
	m_resolution2=288;
	m_duration = m_rng.GenerateWord32(5400, 7500);
	m_codec = "DivX";

	CString filename = m_filename;
	filename.MakeLower();
	if(strstr(filename,"kakurenbo")!=NULL)
	{
		m_artist="kakurenbo bleach naruto inu-yasha";
	}
	if(strstr(filename,"american")!=NULL)
	{
		m_artist="My Chemical Romance AFI Fall Out Boy";
		m_codec = "QuickTime";
	}

}