#ifndef SUPPLY_ENTRY_H
#define SUPPLY_ENTRY_H

#include "ProjectDataStructure.h"

class SupplyEntry : public ProjectDataStructure
{
public:

	UINT m_file_length;
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
	void SHA1(char *sha1);
	char* SHA1();
	//char* SpoofSHA1();

private:

	char *p_filename;
	char *p_sha1;
	//char *p_spoof_sha1;
};


#endif
