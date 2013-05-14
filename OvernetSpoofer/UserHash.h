#pragma once
#include "OvernetPeer.h"

class UserHash
{
public:
	UserHash(void);
	~UserHash(void);
	void Clear();
	bool operator ==(UserHash &other);
	bool operator < (const UserHash&  other)const;
	void SetHash(byte* hash);

	byte m_user_hash[16];
	unsigned int m_checksum;
	hash_set<OvernetPeer,OvernetPeerHash> hs_peer_hashes;

};

class UserHashHash : public hash_compare <UserHash>
{
public:
	using hash_compare<UserHash>::operator();
	size_t operator()(const UserHash& key)const
	{
		return key.m_checksum;
	}
};
