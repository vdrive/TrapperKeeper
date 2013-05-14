// DatFileEntry.h

#ifndef DAT_FILE_ENTRY_H
#define DAT_FILE_ENTRY_H

#include <string>
using namespace std;

class DatFileEntry
{
public:
	DatFileEntry();
	void Clear();

	// Proto
	unsigned int m_internal_ip;
	// External IP
	unsigned int m_port;
	string m_user_name;
	string m_title;
	string m_file_name;
	unsigned int m_file_size;
	CTime m_time_stamp;
	unsigned int m_length_in_seconds;
	unsigned int m_resolution_x;
	unsigned int m_resolution_y;
	// Company
	// Version of Software
};

#endif // DAT_FILE_ENTRY_H