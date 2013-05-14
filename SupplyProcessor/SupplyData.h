#pragma once
#include "..\OvernetSpoofer\PacketBuffer.h"

class SupplyData
{
public:
	SupplyData(void);
	SupplyData(const SupplyData &);
	~SupplyData(void);

	SupplyData & operator=(const SupplyData &);

	//void GetInsertQuery(CString& query); // insert statement for one item
	void GetValueQuery(CString& query);  // get partial sql statment for insert.. 
	CString SQLFormat(const char* str); // get escaped string to insert

	int GetBufferLength();
	int WriteToBuffer(char *buf);
	int ReadFromBuffer(char *buf);	// returns buffer length
	void CreateOvernetPacket();
	CString GetFileFormat(CString filename);
	void Clear();


	string m_filename;  // mandatory
	UINT m_filesize;	// mandatory
	unsigned int m_project_id; // mandatory
	string m_network_name; // mandatory

	int m_track;
	int m_bitrate;
	int m_media_len;

	string m_hash; // mandatory

	string m_ip;
	int m_port;
	int m_count;

	string m_file_type; // VIDEO, AUDIO, SOFTWARE

	string m_artist;
	string m_title;
	string m_album;
	string m_codec;

	PacketBuffer* p_overnet_supply_packet;
	CString m_official_title;
	CString m_official_artist;
};
