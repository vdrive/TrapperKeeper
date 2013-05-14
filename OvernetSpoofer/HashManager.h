#pragma once
#include "mysql.h"
#include "HashAndIPs.h"
#include "IPAndUserHash.h"


class HashManager
{
public:
	HashManager(void);
	~HashManager(void);
	bool GetHashAndIP(byte* hash, vector<IPAndUserHash>& IPs, UINT& filesize,bool spoof=true);
	bool GetMovieHashAndIP(byte* hash, vector<IPAndUserHash>& IPs, UINT& filesize,bool spoof=true);
	void ReadInHashesFromDB();
	UINT GetNumHashes();
	void ResetHashPtr();

private:
	hash_set<HashAndIPs,HashAndIPsHash> hs_file_hashes;
	hash_set<HashAndIPs,HashAndIPsHash> hs_movie_file_hashes;
	MYSQL *p_supply_conn;
	hash_set<HashAndIPs>::const_iterator p_hash_iter;
	hash_set<HashAndIPs>::const_iterator p_movie_hash_iter;
	vector<IPAndUserHash> v_spoof_ips;

	bool OpenSupplyConnection(CString ip, CString login, CString pass, CString db);
	void CloseSupplyConnection();
	bool DecodeBase16(byte* hash, const char* hash_str);
	unsigned char * HexDecode (const char *data, int *dst_len);
	//void ReadInSpoofIPs();
};
