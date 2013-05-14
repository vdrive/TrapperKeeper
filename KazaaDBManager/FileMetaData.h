#pragma once

class FileMetaData
{
public:

	virtual ~FileMetaData(void);

	string m_file_name;
	string m_folder;
	UINT m_file_size;
	byte m_hash[20];
	string m_title;
	string m_artist;
	string m_album;
	string m_keywords;
	string m_category;
	int m_quality;
	string m_comment;
	int m_integrity;

	bool mb_movie;

	FileMetaData& operator=(const FileMetaData &copy){
		m_file_name=copy.m_file_name;
		m_folder=copy.m_folder;
		m_file_size=copy.m_file_size;
		memcpy(m_hash,copy.m_hash,20);
		m_title=copy.m_title;
		m_artist=copy.m_artist;
		m_album=copy.m_album;
		m_keywords=copy.m_keywords;
		m_category=copy.m_category;
		m_quality=copy.m_quality;
		m_integrity=copy.m_integrity;
		m_comment=copy.m_comment;
		mb_movie=copy.mb_movie;	
		return *this;
	}

	FileMetaData(const FileMetaData& copy){
		*this=copy;
	}
	FileMetaData(){
		mb_movie=false;
		m_quality=-1;  //disabled by default
		m_integrity=-1; //disabled by default
		ZeroMemory(m_hash,sizeof(m_hash));  //zero the hash out, so we can see if its not getting set for some reason
	}
};
