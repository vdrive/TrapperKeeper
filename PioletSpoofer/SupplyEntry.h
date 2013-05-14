#ifndef SUPPLY_ENTRY_H
#define SUPPLY_ENTRY_H

#include "ProjectDataStructure.h"

class SupplyEntry : public ProjectDataStructure
{
public:

	UINT m_file_length;
	int m_bit_rate;
	int m_sampling_rate;
	int m_stereo;
	int m_song_length;
	int m_track;
	int m_popularity;

	SupplyEntry();
	~SupplyEntry();
	SupplyEntry(const SupplyEntry &entry);
	SupplyEntry& operator=(const SupplyEntry &entry);

	void Clear();						// call this in the constructors of the derived classes
	int GetBufferLength();				// returns buffer length
	int WriteToBuffer(char *buf);
	int ReadFromBuffer(char *buf);	

	void Filename(char *filename);
	char *Filename();
	void Nickname(char *nickname);
	char* Nickname();
	void MD5(char *md5);
	char* MD5();

private:

	char *p_filename;
	char *p_nickname;
	char *p_md5;
};


#endif
