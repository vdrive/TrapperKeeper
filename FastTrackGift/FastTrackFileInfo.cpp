#include "StdAfx.h"
#include "fasttrackfileinfo.h"
#include "FTHash.h"
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
	m_has_created_unique_hash=false;
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

	m_project_id=0;
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
	m_type="Video";
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
	count+=m_project_name.GetLength()+1;

	count+=sizeof(m_project_id);
	count+=sizeof(m_filesize);
	count+=sizeof(m_bit_rate);
	count+=sizeof(m_track_number);
	count+=sizeof(m_duration);
	count+=sizeof(m_resolution1);
	count+=sizeof(m_resolution2);
	count+=sizeof(m_integrity);
	count+=sizeof(m_media_type);

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
	if(m_project_name.GetLength() != 0)
	{
		strcpy(ptr,m_project_name);
	}
	ptr+=m_project_name.GetLength()+1;

	memcpy(ptr,m_hash,sizeof(m_hash));
	ptr+= sizeof(m_hash);

	*((unsigned int *)ptr)=m_project_id;
	ptr+=sizeof(m_project_id);
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
	m_project_name = ptr;
	ptr+=m_project_name.GetLength()+1;
	memcpy(m_hash,ptr, sizeof(m_hash));
	ptr+=sizeof(m_hash);

	m_project_id=*((unsigned int *)ptr);
	ptr+=sizeof(unsigned int);
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

	return GetBufferLength();
}

//
//
//
PacketBuffer* FastTrackFileInfo::GetShareFile(bool same_hash)
{
	if(m_media_type == MEDIA_TYPE_AUDIO)
	{
		PacketBuffer* buf = new PacketBuffer();
		CreateShareForAudio(buf,same_hash);
		m_num_shares++;
		return buf;
	}
	if(m_media_type == MEDIA_TYPE_VIDEO)
	{
		PacketBuffer* buf = new PacketBuffer();
		CreateShareForVideo(buf,same_hash);
		m_num_shares++;
		return buf;
	}
	if(m_media_type == MEDIA_TYPE_SOFTWARE)
	{
		PacketBuffer* buf = new PacketBuffer();
		CreateShareForSoftware(buf,same_hash);
		m_num_shares++;
		return buf;
	}
	return NULL; //unknown type
}

//
//
//
PacketBuffer* FastTrackFileInfo::GetShareFileWithOptions(bool same_hash,vector<CString>& file_exts, int chance)
{
	if(m_media_type == MEDIA_TYPE_AUDIO)
	{
		PacketBuffer* buf = new PacketBuffer();
		CreateShareForAudioWithOptions(buf,same_hash,file_exts,chance);
		m_num_shares++;
		return buf;
	}
	if(m_media_type == MEDIA_TYPE_VIDEO)
	{
		PacketBuffer* buf = new PacketBuffer();
		CreateShareForVideo(buf,same_hash);
		m_num_shares++;
		return buf;
	}
	return NULL; //unknown type
}

//
//
//
void FastTrackFileInfo::CreateShareForAudio(PacketBuffer* buf, bool same_hash)
{
/*
	//kludge for testing D-12 my band
	if(m_project_id==1071 && m_track_number==1)
	{
		if(m_has_created_unique_hash==false)
		{
			m_has_created_unique_hash=true;
			//random info
			m_filesize = m_rng.GenerateWord32(2000000, 10000000); //random filesize between 2MB - 10 MB
			//make it disvisable by 137 if we want decoy instead swarm
			m_filesize = m_filesize - m_filesize%137;
			for(int j=0; j<FST_FTHASH_LEN-4; j++)
				m_hash[j] = rand()%256;

			UINT *pFileSize = (UINT *)&m_hash[FST_FTHASH_LEN-4];
			*pFileSize = m_filesize;
		}
	}
	else
	{
		//random info
		m_filesize = m_rng.GenerateWord32(2000000, 10000000); //random filesize between 2MB - 10 MB
		//make it disvisable by 137 if we want decoy instead swarm
		m_filesize = m_filesize - m_filesize%137;
		for(int j=0; j<FST_FTHASH_LEN-4; j++)
			m_hash[j] = rand()%256;

		UINT *pFileSize = (UINT *)&m_hash[FST_FTHASH_LEN-4];
		*pFileSize = m_rng.GenerateWord32(1073741824, 2147483648); //1 GB to 2 GB

	}
*/

	if(!same_hash)
	{
		//random info
		if( (rand()%2)==0 )
			m_filesize = m_rng.GenerateWord32(1048576, 3145728); //random filesize between 1MB - 3MB
		else
			m_filesize = m_rng.GenerateWord32(7340032, 12582912); //random filesize between 7MB - 12MB
		//make it disvisable by 137 if we want decoy instead swarm
		m_filesize = m_filesize - m_filesize%137;
		for(int j=0; j<FST_FTHASH_LEN; j++)
			m_hash[j] = rand()%256;

		//UINT *pFileSize = (UINT *)&m_hash[FST_FTHASH_LEN-4];
	//	*pFileSize = m_filesize;
		//*pFileSize = m_rng.GenerateWord32(1073741824, 2147483648); //1 GB to 2 GB
	}

	if(!same_hash)
		m_duration=m_rng.GenerateWord32(180, 300); //3:00 - 5:00
	else
	{
	
		if(m_filesize <= 3000000)
			m_duration = m_rng.GenerateWord32(120, 180); //2:00 - 3:00
		else if(m_filesize <= 4000000)
			m_duration = m_rng.GenerateWord32(180, 240); //3:00 - 4:00
		else if(m_filesize <= 5000000)
			m_duration = m_rng.GenerateWord32(240, 300); //4:00 - 5:00
		else if(m_filesize <= 6000000)
			m_duration = m_rng.GenerateWord32(300, 360); //5:00 - 6:00
		else if(m_filesize <= 7000000)
			m_duration = m_rng.GenerateWord32(360, 420); //6:00 - 7:00
		else if(m_filesize <= 8000000)
			m_duration = m_rng.GenerateWord32(420, 480); //7:00 - 8:00
		else if(m_filesize <= 9000000)
			m_duration = m_rng.GenerateWord32(480, 540); //8:00 - 9:00
		else if(m_filesize <= 10000000)
			m_duration = m_rng.GenerateWord32(540, 600); //9:00 - 10:00
	}
	


	buf->PutByte(0x00); //unknown
	buf->PutByte(MEDIA_TYPE_AUDIO); //media type
	buf->PutStr("\x00\x00",2);
	buf->PutUStr(m_hash,FST_FTHASH_LEN); //hash
	buf->PutDynInt(GetHashChecksum(m_hash)); //file_id
	buf->PutDynInt(m_filesize); //file size

	int bit_rate_choice = rand()%3;
	switch(bit_rate_choice)
	{
		case 0:
		{
			m_bit_rate=128;
			break;
		}
		case 1:
		{
			m_bit_rate=192;
			break;
		}
		case 2:
		{
			m_bit_rate=360;
			break;
		}
	}

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
void FastTrackFileInfo::CreateShareForAudioWithOptions(PacketBuffer* buf, bool same_hash,vector<CString>& file_exts, int chance)
{
	if(!same_hash)
	{
		//random info
		m_filesize = m_rng.GenerateWord32(2000000, 10000000); //random filesize between 2MB - 10 MB
		//make it disvisable by 137 if we want decoy instead swarm
		m_filesize = m_filesize - m_filesize%137;
		for(int j=0; j<FST_FTHASH_LEN-4; j++)
			m_hash[j] = rand()%256;

		UINT *pFileSize = (UINT *)&m_hash[FST_FTHASH_LEN-4];
	//	*pFileSize = m_filesize;
		*pFileSize = m_rng.GenerateWord32(1073741824, 2147483648); //1 GB to 2 GB
	}

	if(m_filesize <= 3000000)
		m_duration = m_rng.GenerateWord32(120, 180); //2:00 - 3:00
	else if(m_filesize <= 4000000)
		m_duration = m_rng.GenerateWord32(180, 240); //3:00 - 4:00
	else if(m_filesize <= 5000000)
		m_duration = m_rng.GenerateWord32(240, 300); //4:00 - 5:00
	else if(m_filesize <= 6000000)
		m_duration = m_rng.GenerateWord32(300, 360); //5:00 - 6:00
	else if(m_filesize <= 7000000)
		m_duration = m_rng.GenerateWord32(360, 420); //6:00 - 7:00
	else if(m_filesize <= 8000000)
		m_duration = m_rng.GenerateWord32(420, 480); //7:00 - 8:00
	else if(m_filesize <= 9000000)
		m_duration = m_rng.GenerateWord32(480, 540); //8:00 - 9:00
	else if(m_filesize <= 10000000)
		m_duration = m_rng.GenerateWord32(540, 600); //9:00 - 10:00
	


	buf->PutByte(0x00); //unknown
	buf->PutByte(MEDIA_TYPE_AUDIO); //media type
	buf->PutStr("\x00\x00",2);
	buf->PutUStr(m_hash,FST_FTHASH_LEN); //hash
	buf->PutDynInt(GetHashChecksum(m_hash)); //file_id
	buf->PutDynInt(m_filesize); //file size

	int bit_rate_choice = rand()%3;
	switch(bit_rate_choice)
	{
		case 0:
		{
			m_bit_rate=128;
			break;
		}
		case 1:
		{
			m_bit_rate=192;
			break;
		}
		case 2:
		{
			m_bit_rate=360;
			break;
		}
	}

	int number_of_tags = 0;
	PacketBuffer* tag_buf = new PacketBuffer();

	//replacing the extension
	CString new_filename = m_filename;
	int ret = rand()%chance;
	if(ret==0 && file_exts.size() > 0)
	{
		int index = rand()%file_exts.size();
		new_filename = ReplaceFileExtension(m_filename,file_exts[index]);
	}

	//filename
	if(new_filename.GetLength() > 0)
	{
		tag_buf->PutDynInt(FILE_TAG_FILENAME);	//tag type
		tag_buf->PutDynInt((UINT)new_filename.GetLength());	//tag length
		tag_buf->PutStr(new_filename,new_filename.GetLength()); //tag content
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
void FastTrackFileInfo::CreateShareForVideo(PacketBuffer* buf, bool same_hash)
{
	byte hash[FST_FTHASH_LEN];
#ifdef CLUMP_SPOOFING
	if(!same_hash)
	{
		//make it disvisable by 137 if we want decoy instead swarm
		m_filesize = m_filesize - m_filesize%137;

		//1 out of 2, we will use the same hash and filesize in order to make some clump
		//if(rand()%2 != 0)
		//{
		//	for(int j=0; j<FST_FTHASH_LEN-4; j++)
		//		hash[j] = rand()%256;

		//	UINT *pFileSize = (UINT *)&hash[FST_FTHASH_LEN-4];
		//	*pFileSize = m_rng.GenerateWord32(1073741824, 2147483648); //1 GB to 2 GB
		//}
		//else
		//{
			for(int j=0; j<FST_FTHASH_LEN; j++)
			{
				if(j < FST_FTHASH_LEN-1)
					m_hash[j] ^= m_hash[j+1];
			}
			//memcpy(hash,m_hash,FST_FTHASH_LEN);
		//}
	}
#else
	if(!same_hash)
	{
		//random info
		m_filesize = m_rng.GenerateWord32(100000000, 700000000); //random filesize between 100MB - 700 MB
		//make it disvisable by 137 if we want decoy instead swarm
		m_filesize = m_filesize - m_filesize%137;
		for(int j=0; j<FST_FTHASH_LEN-4; j++)
			m_hash[j] = rand()%256;

		memcpy(hash,m_hash,FST_FTHASH_LEN);
		UINT *pFileSize = (UINT *)&m_hash[FST_FTHASH_LEN-4];
	//	*pFileSize = m_filesize;
		*pFileSize = m_rng.GenerateWord32(1073741824, 2147483648); //1 GB to 2 GB
	}
#endif



	if(m_filesize <= 200000000)
		m_duration = m_rng.GenerateWord32(10*60, 20*60);
	else if(m_filesize <= 300000000)
		m_duration = m_rng.GenerateWord32(20*60, 30*60);
	else if(m_filesize <= 400000000)
		m_duration = m_rng.GenerateWord32(30*60, 40*60);
	else if(m_filesize <= 500000000)
		m_duration = m_rng.GenerateWord32(40*60, 50*60);
	else if(m_filesize <= 600000000)
		m_duration = m_rng.GenerateWord32(50*60, 60*60);
	else if(m_filesize <= 700000000)
		m_duration = m_rng.GenerateWord32(60*60, 70*60);


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

	//resolution
	int rol = rand()%3;
	if(rol==0 || rol == 1)
		m_resolution1 = 352;
	else
		m_resolution1 = 640;
	if(m_resolution1 == 640)
		m_resolution2 = 320;
	else
	{
		rol = rand()%2;
		if(rol==0)
			m_resolution2 = 240;
		else
			m_resolution2 = 288;
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
CString FastTrackFileInfo::ReplaceFileExtension(CString& filename, CString& new_extension)
{
	CString new_filename = filename;
	int index = new_filename.ReverseFind('.');
	index++;
	if(index > 1)
	{
		new_filename.Delete(index,new_filename.GetLength()-index);
	}
	new_filename += new_extension;
	return new_filename;
}

//
//
//
//
//
//
void FastTrackFileInfo::CreateShareForSoftware(PacketBuffer* buf, bool same_hash)
{
	byte hash[FST_FTHASH_LEN];
	memcpy(hash,m_hash,FST_FTHASH_LEN);

#ifdef CLUMP_SPOOFING
	if(!same_hash)
	{
		//make it disvisable by 137 if we want decoy instead swarm
		m_filesize = m_filesize - m_filesize%137;

		//1 out of 2, we will use the same hash and filesize in order to make some clump
		if(rand()%2 != 0)
		{
			for(int j=0; j<FST_FTHASH_LEN-4; j++)
				hash[j] = rand()%256;

			UINT *pFileSize = (UINT *)&hash[FST_FTHASH_LEN-4];
			*pFileSize = m_rng.GenerateWord32(1073741824, 2147483648); //1 GB to 2 GB
		}
		else
		{
			for(int j=0; j<FST_FTHASH_LEN; j++)
			{
				if(j < FST_FTHASH_LEN-1)
					hash[j] ^= m_hash[j+1];
			}
		}
	}
#else
	if(!same_hash)
	{
		//random info
		m_filesize = m_rng.GenerateWord32(50000000, 400000000); //random filesize between 50MB - 400MB
		//make it disvisable by 137 if we want decoy instead swarm
		m_filesize = m_filesize - m_filesize%137;
		for(int j=0; j<FST_FTHASH_LEN-4; j++)
			hash[j] = rand()%256;

		UINT *pFileSize = (UINT *)&hash[FST_FTHASH_LEN-4];
	//	*pFileSize = m_filesize;
		*pFileSize = m_rng.GenerateWord32(1073741824, 2147483648); //1 GB to 2 GB
	}
#endif

	buf->PutByte(0x00); //unknown
	buf->PutByte(MEDIA_TYPE_SOFTWARE); //media type
	buf->PutStr("\x00\x00",2);
	buf->PutUStr(hash,FST_FTHASH_LEN); //hash
	buf->PutDynInt(GetHashChecksum(hash)); //file_id
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