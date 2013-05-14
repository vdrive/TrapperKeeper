#pragma once

#include "FileInfo.h"
#include "ProjectDataStructure.h"

class TorrentFileData : public ProjectDataStructure
{
public:
	vector<FileInfo> v_files;
	string m_name;
	unsigned int m_piece_length;
	vector<string> v_piece_hashes;
	string m_announce_URL;
	vector<string> v_announce_list;
	unsigned int m_creation_date;
	string m_comment;
	string m_created_by;

	string m_hash;
	unsigned int m_torrent_id;

public:
	TorrentFileData();
	~TorrentFileData(void);
	virtual void Clear();						// call this in the constructors of the derived classes
	virtual int GetBufferLength();
	virtual int WriteToBuffer(char *buf);
	virtual int ReadFromBuffer(char *buf);	// returns buffer length
};
