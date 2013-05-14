#pragma once


#include <string>
#include "sha1.h"
#include <vector>
#include "BtStructs.h"
#include "PeerList.h"


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
	string GetName();
	string GetInfoHashString(); // the info hash as an escaped string
	int GetPieceLength(); // piece length
	int GetNumPieces(); // number of pieces
	void SetPeers(PeerList*peers); // sets the peers 
	PeerList * GetPeers(); // gets the associated peer list
	int GetLength(); 
	vector<string> GetAnnounceList(); // vector of the announce urls.  will alwasy return at least one

	unsigned int GetSize(); // how big is this object?
	void GetBuffer(char *buffer);

protected:
	char * GetBuffer();
    
private:
	string ParseNext(unsigned short * buf, int * pos);
	void CreateInfoHash();

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

	bool b_is_valid;
	
};
