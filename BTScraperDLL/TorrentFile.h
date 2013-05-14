#pragma once


#include <string>
#include "sha1.h"
#include <vector>
#include "BtStructs.h"
#include "..\BTScraperDll\PeerList.h"
#include "TorrentFileInfo.h"


class TorrentFile
{
public:
	TorrentFile(void);
	TorrentFile(const TorrentFile & tf);
	TorrentFile(void* data, unsigned int data_length);
	~TorrentFile(void);
	
	TorrentFile & operator=(const TorrentFile & tf);
	bool operator==(const TorrentFile &tf);

	int GetPeerCount();
	bool IsValid();

	void ReadInTorrentFile(char * filename);  // parses a local file
	bool ParseTorrentFile(unsigned short * buf, int size);  // parses the torrent in buf. 
	string GetURL(); // the announce url
	unsigned short * GetInfoHash(); // the info hash
	string GetInfoHashString(); // the info hash as an escaped string

	string GetName();
	int GetPieceLength(); // piece length
	int GetNumPieces(); // number of pieces
	unsigned short * GetEd2k();
	string GetNameUTF8();
	string GetPublisher();
	string GetPublisherURL();
	string GetPublisherURL_UTF8();
	string GetPublisherUTF8();
	unsigned short * GetSHA1();
	string GetCreatedBy();
	long GetCreationDate();
	string GetEncoding();
	vector<CTorrentFileInfo*> GetFileInfo();

	vector<unsigned short*> GetPieces(); // in order vector of the pieces;
	void SetPeers(PeerList*peers); // sets the peers 
	PeerList * GetPeers(); // gets the associated peer list
	int GetLength(); 
	vector<string> GetAnnounceList(); // vector of the announce urls.  will always return at least one

	unsigned int GetBufferSize(); // how big is this object?
	void GetBuffer(char *buffer);

protected:
	char * GetBuffer();
    
private:
	string ParseNext(unsigned short * buf, int * pos);
	void DecodeFileInfo(unsigned short * buf, int * pos);
	void CreateInfoHash();
	vector<unsigned short*> DecodePieces(unsigned short * buf, int *pos); // custom for bit torrents
	int DecodeHash(unsigned short * buf, int * pos, unsigned short * dest); // returns size of buf

private: // members
	PeerList * m_peers;
	char m_url[256];
	int m_creationdate;
	int m_length;
	char m_name[256];
	int m_piecelength;
	int m_numpieces;
	unsigned short * m_data;
	int info_len;
	int info_start;
	string m_info_hash_string;
	unsigned short m_info_hash[20];
	int m_size;
	vector<string> v_announce_list; // announce list
	vector<unsigned short*> v_pieces;

	vector<string> v_files; // files
	vector<string> v_path; // path, should correspond to file
	bool b_is_ed2k;

	unsigned short m_ed2k_hash[16]; 
	string m_name_utf8; 
	string m_publisher;
	string m_publisher_url;
	string m_publisher_url_utf8;
	string m_publisher_utf8;
	unsigned short m_sha1[20];
	string m_created_by;
	string m_encoding;
	int m_num_files;
	vector<CTorrentFileInfo*> v_file_info;

	bool b_is_valid;
	
};
