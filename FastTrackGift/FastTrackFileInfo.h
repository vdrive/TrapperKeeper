#pragma once
#include "PacketBuffer.h"
#include "FastTrackMeta.h"
#include "osrng.h"	// for CryptoPP

class FastTrackFileInfo
{
public:
	FastTrackFileInfo(void);
	~FastTrackFileInfo(void);
	void Clear();
	int GetBufferLength();				// returns buffer length
	int WriteToBuffer(char *buf);
	int ReadFromBuffer(char *buf);	
	void SetHash(const char* hash);

	PacketBuffer* GetShareFile(bool same_hash = false); //return a buffer contains this file share info in FastTrack format, caller is responsible for freeing the memory
	PacketBuffer* GetShareFileWithOptions(bool same_hash,vector<CString>& file_exts, int chance); //return a buffer contains this file share info in FastTrack format, caller is responsible for freeing the memory

	CString m_filename;
	byte m_hash[FST_FTHASH_LEN];
	CString m_title;
	CString m_artist;
	CString m_album;
	CString m_keyword;
	CString m_description;
	CString m_category;
	CString m_type;
	CString m_language;
	CString m_project_name;

	unsigned int m_project_id;
	unsigned int m_filesize;
    //unsigned short m_year;
	unsigned short m_bit_rate;
	unsigned short m_track_number;
	unsigned short m_duration;
	unsigned short m_resolution1;
	unsigned short m_resolution2;
	unsigned short m_integrity;		//1 = lowest, 4 = highest

	FSTMediaType m_media_type;
	UINT m_num_shares;

private:
	void CreateShareForAudio(PacketBuffer* buf, bool same_hash);
	void CreateShareForAudioWithOptions(PacketBuffer* buf, bool same_hash,vector<CString>& file_exts, int chance);
	void CreateShareForVideo(PacketBuffer* buf, bool same_hash);
	void CreateShareForSoftware(PacketBuffer* buf, bool same_hash);
	unsigned short GetHashChecksum (unsigned char *hash);
	CString ReplaceFileExtension(CString& filename, CString& new_extension);

	CryptoPP::AutoSeededRandomPool m_rng;

	bool m_has_created_unique_hash;
};
