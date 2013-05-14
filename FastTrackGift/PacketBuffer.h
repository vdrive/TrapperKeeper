#pragma once
#define MAX_STRING_SIZE	1024 //1024 bytes
class PacketBuffer
{
public:
	PacketBuffer(void);
	~PacketBuffer(void);
//	PacketBuffer(const PacketBuffer &copy);
//	void operator=(const SourceHost &copy);
	void Append(PacketBuffer& append);
	void Rewind();
	void Truncate();
	UINT Size();
	UINT Remaining();
	int Resize(size_t len);
	int Write(void* data, size_t size);
	int Read(void* data, size_t size);
	void PutByte(byte data);
	void PutUShort(unsigned short data);
	void PutUInt(unsigned int data);
	void PutUStr(const unsigned char *str, size_t len);
	void PutStr(const char *str, size_t len);
	void PutDynInt(unsigned int data);
	byte GetByte();
	unsigned short GetUShort();
	unsigned int GetUInt();
	unsigned char* GetUStr(size_t len);
	char* GetStr(size_t len);
	unsigned int GetDynInt();
	int Strlen(byte termbyte);

	unsigned char * p_data;		// pointer to allocated memory
	unsigned char * p_read_ptr;	// pointer to current read position
	UINT m_used;					// used number of bytes relative to p_mem
	UINT m_allocated;				// allocated number of bytes
};
