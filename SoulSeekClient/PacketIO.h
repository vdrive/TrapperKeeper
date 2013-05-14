#pragma once

// ===========================================
//            PACKET READER OBJECT
// ===========================================
class PacketReader
{
public:
	PacketReader(char* data);
	~PacketReader(void);
	char *GetNextString();
	unsigned int GetNextInt();
	void GetNextIP(char * pRet);
	BYTE GetNextByte();

private:
	char* packet;   // The char string/array
	int next_item;  // Pointer to the location of the next item to read in the packet
};

// ===========================================
//            PACKET WRITER OBJECT
// ===========================================
// Used to write packets designed for SOULSEEK
// Packets are a 4 byte header specifying the size of the body
// Insert functions start from the body's index ie 0 index = 4
// Body size maybe set either in the constructor or dynamically (which is slower)
class PacketWriter
{
public:
	PacketWriter(void);                                         // Constructor for unknow packet size
	PacketWriter(int size);                                     // Constructor for know packet size is know
	~PacketWriter(void);                                        // Destructor
	void Resize(int size);                                      // Resizes the char string
	void AppendSizeAndString(char* stringToWrite, bool resize); // Append int at the end of the string
	void AppendString(char* stringToWrite, bool resize);
	void AppendData(char* stringToWrite, UINT size);
	void InsertSizeAndString(char* stringToWrite, int index);   // Insert string into specified location
	void AppendInt(unsigned int intToWrite, bool resize);                // Append int at the end of the string
	void AppendByte(bool byteToWrite, bool resize);             // Append int at the end of the string
	void InsertInt(unsigned int intToWrite, int index);                  // Insert int into specified location
	void AppendChar(char charToWrite, bool resize);
	unsigned char* GetPacket(void);                             // Returns the packet to be sent
	int GetPacketSize();                                        // Returns total packet size
	int GetBodySize();                                          // Returns just the body size
	char* p_ip;
	int m_port;
	char* p_username;

private:
	int numOfBytes;         // How big is the char string/array
	int tailPointer;        // Next free space to write value to
	unsigned char* packet;  // The char string/array
};