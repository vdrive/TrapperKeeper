#pragma once
#include "OvernetPeer.h"

class UserGroupHashes
{
public:
	UserGroupHashes(byte key);
	~UserGroupHashes(void);

	byte m_key;
	hash_set<OvernetPeer,OvernetPeerHash> hs_peers;
};
