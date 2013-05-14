// PushData.h
#pragma once

class PushData
{
public:
	PushData();
	~PushData();

	// Data Reading Functions
	GUID Guid();
	unsigned int FileIndex();
	unsigned int IP();
	char *IPString();
	unsigned short int Port();

	// Data Writing Functions
	void Init();
	void Init(GUID guid,unsigned int file_index,unsigned int ip,unsigned short int port);
	void Init(GUID guid,unsigned int file_index,char *ip,unsigned short int port);

	void Guid(GUID guid);
	void FileIndex(unsigned int file_index);
	void IP(unsigned int ip);
	void IP(char *ip);
	void Port(unsigned short int port);

private:
	unsigned char m_data[26];
};