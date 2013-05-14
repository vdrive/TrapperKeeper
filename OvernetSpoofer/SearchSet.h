#pragma once
#include "OvernetPeer.h"
#include "MetaData.h"
#include "IPAndUserHash.h"
class SearchSet
{
public:
	SearchSet(void);
	~SearchSet(void);
	void Clear();
	bool IsHashCloseEnough(const byte hash);
	//bool IsKeywordHashCloseEnough(const byte hash);
	//bool IsFileHash(const byte* hash);
	//bool IsKeywordHash(const byte* hash);
	//void SetFileHash(byte* hash);
	//void SetFileHash(byte* hash,const char* server_ip);
	//void SetKeywordHash(byte* hash);
	bool operator < (const SearchSet&  other)const;
	bool operator ==(SearchSet &other);
	void SetHash(byte* hash);
	//void InsertServerIP(const char* ip);

	hash_set<OvernetPeer,OvernetPeerHash> hs_peer_hashes;
	//byte m_keyword_hash[16];
	byte m_hash[16];
	unsigned int m_checksum;
	bool m_is_file_hash;

	hash_set<MetaData,MetaDataHash> hs_meta_data; //storing each file's meta data with the same keyword/file hash
	//vector<IPAndUserHash> v_server_IPs;
};
class SearchSetHash : public hash_compare <SearchSet>
{
public:
	using hash_compare<SearchSet>::operator();
	size_t operator()(const SearchSet& key)const
	{
		return key.m_checksum;
	}
};