// BlubsterHeader.h

#ifndef BLUBSTER_HEADER_H
#define BLUBSTER_HEADER_H

class BlubsterHeader
{
public:
	BlubsterHeader();
	~BlubsterHeader();

	static unsigned int EndianSwitch(unsigned int val);
	static unsigned short int EndianSwitch(unsigned short int val);

	// Reading
	unsigned short int Checksum();		// 4 bytes (2 of the bytes are always 0)
	unsigned int Counter();				// 4 bytes
	unsigned int SourceIP();			// 4 bytes
	unsigned int DestIP();				// 4 bytes
	unsigned char TTL();				// 1 byte
	unsigned char Op();					// 1 byte
	unsigned short int DataLength();	// 2 bytes

	// Writing
	void Checksum(unsigned short int checksum);
	void Counter(unsigned int counter);
	void SourceIP(unsigned int ip);
	void DestIP(unsigned int ip);
	void TTL(unsigned char ttl);
	void Op(unsigned char op);
	void DataLength(unsigned short int len);

	unsigned char m_data[20];
};

#endif // BLUBSTER_HEADER_H