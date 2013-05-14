#include "StdAfx.h"
#include ".\hashmanager.h"

HashManager::HashManager(void)
{
	p_hash_iter=p_movie_hash_iter=NULL;
}

HashManager::~HashManager(void)
{
}

//
//
//
bool HashManager::GetHashAndIP(byte* hash, vector<IPAndUserHash>& IPs, UINT& filesize, bool spoof)
{
	if(hs_file_hashes.size()>0)
	{
		if(p_hash_iter == hs_file_hashes.end())
			p_hash_iter = hs_file_hashes.begin();
		if(!spoof)
			memcpy(hash,p_hash_iter->m_hash,16);
		else
			memcpy(hash,p_hash_iter->m_spoof_hash,16);
		IPs = p_hash_iter->v_server_IPs;
		filesize = p_hash_iter->m_filesize;
		p_hash_iter++;
		return true;
	}else
		return false;
}

bool HashManager::GetMovieHashAndIP(byte* hash, vector<IPAndUserHash>& IPs, UINT& filesize, bool spoof)
{
	if(hs_movie_file_hashes.size()>0)
	{
		if(p_movie_hash_iter == hs_movie_file_hashes.end())
			p_movie_hash_iter = hs_movie_file_hashes.begin();
		if(!spoof)
			memcpy(hash,p_movie_hash_iter->m_hash,16);
		else
			memcpy(hash,p_movie_hash_iter->m_spoof_hash,16);
		IPs = p_movie_hash_iter->v_server_IPs;
		filesize = p_movie_hash_iter->m_filesize;
		p_movie_hash_iter++;
		return true;
	}else
		return false;
}

/*
void HashManager::ReadInSpoofIPs()
{
	hash_set<HashAndIPs>::iterator hash_iter=hs_file_hashes.begin();
	while(hash_iter!=hs_file_hashes.end())
	{
		IPAndUserHash ip_hash;
		ip_hash.SetIP("67.15.8.72");
		ip_hash.m_port=6346;
		hash_iter->v_server_spoofed_IPs.push_back(ip_hash);
		hash_iter++;
	}
	hash_iter=hs_movie_file_hashes.begin();
	while(hash_iter!=hs_movie_file_hashes.end())
	{
		IPAndUserHash ip_hash;
		ip_hash.SetIP("67.15.8.72");
		ip_hash.m_port=6346;
		hash_iter->v_server_spoofed_IPs.push_back(ip_hash);
		hash_iter++;
	}
}
*/
//
//
//
void HashManager::ReadInHashesFromDB()
{
	if(OpenSupplyConnection("38.119.64.101","onsystems","ebertsux37","overnet_data"))
	{
		MYSQL_RES *res=NULL;
		MYSQL_ROW row;
        CString query="select * from decoy_hashes";
		int ret = mysql_query(p_supply_conn, query);
			
		if(mysql_error(p_supply_conn)[0] != '\0')
		{
			CStdioFile file;
			if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
			{
				file.SeekToEnd();

				CString log;
				log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
				log += " ReadInHashesFromDB(...) ";
				log += query;
				log += "\n";
				log += "Error: ";
				log += mysql_error(p_supply_conn);
				log += "\n\n";

				file.WriteString(log);
				file.Close();
			}
			else
			{
				DWORD error = GetLastError();
				//MessageBox(NULL, "File Open Error: SQL_Process_Error_Log.txt", "File Open Error", MB_OK);
			}

			TRACE(mysql_error(p_supply_conn));
			TRACE("\n");
			TRACE(query);
			TRACE("\n");
		}
		else
		{
			res=mysql_store_result(p_supply_conn);	// allocates memory
			if(res != NULL)
			{
				UINT num_row = (UINT)mysql_num_rows(res);
				if(num_row > 0)
				{
					CString ip;
					hs_file_hashes.clear();
					while ((row = mysql_fetch_row(res)))
					{
						if(row[0]!=NULL)
						{
							HashAndIPs hash;
							IPAndUserHash ip_hash;
							ip_hash.SetIP(row[0]);
							byte hash_byte[16];
							DecodeBase16(hash_byte, row[1]);
							hash.SetHash(hash_byte);
							sscanf(row[2],"%u", &hash.m_filesize);
							pair<hash_set<HashAndIPs>::iterator, bool > pr;
							if(hash.m_filesize >= 100000000)
								pr = hs_movie_file_hashes.insert(hash);
							else
								pr = hs_file_hashes.insert(hash);
							pr.first->InsertServerIP(ip_hash);
						}
					}
				}
			}
			mysql_free_result(res);	// free memory
			//ReadInSpoofIPs();
		}
		CloseSupplyConnection();
		p_hash_iter = hs_file_hashes.begin();
		p_movie_hash_iter = hs_movie_file_hashes.begin();
	}
}

//
//
//
bool HashManager::OpenSupplyConnection(CString ip, CString login, CString pass, CString db)
{
	p_supply_conn = mysql_init(NULL);

	if (!mysql_real_connect(p_supply_conn, ip, login, pass, db, 0, NULL, 0))
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " DBInterface::OpenSupplyConnection(...) ";
			log += "Failed to connect to database: Error: ";
			log += mysql_error(p_supply_conn);
			log += "\n\n";
			file.WriteString(log);
			file.Close();
		}
		CloseSupplyConnection();
		return false;
	}
	return true;
}

//
//
//
void HashManager::CloseSupplyConnection()
{
	mysql_close(p_supply_conn);
}

//
//
//
UINT HashManager::GetNumHashes()
{
	
	return hs_file_hashes.size()+hs_movie_file_hashes.size();
}

//
//
//
bool HashManager::DecodeBase16(byte* hash, const char* hash_str)
{
	byte *buf;
	int len=0;
	memset(hash,0,len);
	
	if (!(buf = HexDecode (hash_str, &len)))
		return false;

	if (len < 16)
	{
		free (buf);
		return false;
	}
	
	memcpy(hash, buf, 16);
	free (buf);
	return true;
}

//
//
//
/* caller frees returned string */
unsigned char * HashManager::HexDecode (const char *data, int *dst_len)
{
	static const char hex_string[] = "0123456789abcdefABCDEF";
	char *dst, *h;
	int i;
	unsigned char hi, lo;

	if (!data)
		return NULL;

	if (! (dst = (char*)malloc (strlen (data) / 2 + 1)))
		return NULL;

	for(i=0; *data && data[1]; i++, data += 2)
	{
		/* high nibble */
		if( (h = strchr (hex_string, data[0])) == NULL)
		{
			free (dst);
			return NULL;
		}
		hi = (h - hex_string) > 0x0F ? (h - hex_string - 6) : (h - hex_string);

		/* low nibble */
		if ( (h = strchr (hex_string, data[1])) == NULL)
		{
			free (dst);
			return NULL;
		}
		lo = (h - hex_string) > 0x0F ? (h - hex_string - 6) : (h - hex_string);

		dst[i] = (hi << 4) | lo;
	}

	if (dst_len)
		*dst_len = i;

	return (unsigned char *)dst;
}

//
//
//
void HashManager::ResetHashPtr()
{
	if(hs_file_hashes.size())
		p_hash_iter = hs_file_hashes.begin();
	if(hs_movie_file_hashes.size())
		p_movie_hash_iter = hs_movie_file_hashes.begin();
}