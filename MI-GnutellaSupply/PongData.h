// PongData.h
#pragma once

class PongData
{
public:
	PongData();
	~PongData();

	// Data Reading Functions
	unsigned short int Port();
	unsigned int IP();
	string IPString();
	unsigned int SharedFiles();
	unsigned int SharedKB();

	// Data Writing Functions
	void Init();
	void Init(unsigned short int port,unsigned int ip,unsigned int shared_files,unsigned int shared_kb);
	void Init(unsigned short int port,char *ip,unsigned int shared_files,unsigned int shared_kb);

	void Port(unsigned short int port);
	void IP(unsigned int ip);
	void IP(char *ip);
	void SharedFiles(unsigned int shared_files);
	void SharedKB(unsigned int shared_kb);

private:
	unsigned char m_data[14];
};