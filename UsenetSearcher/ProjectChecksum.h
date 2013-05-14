// ProjectChecksum.h
#ifndef PROJECT_CHECKSUM_H
#define PROJECT_CHECKSUM_H

class ProjectChecksum
{
public:
	// Public Member Functions
	ProjectChecksum();
	~ProjectChecksum();
	void Clear();

	int GetBufferLength();
	int WriteToBuffer(char *buf);
	int ReadFromBuffer(char *buf);	// returns buffer length
	
	void Checksum(char *buf,int buf_len);
	char *Checksum();

	bool operator ==(ProjectChecksum &checksum);
	bool operator !=(ProjectChecksum &checksum);

	// Public Data Members
	string m_project_name;

private:
	// Private Data Members
	char m_sha1[20];
};

#endif