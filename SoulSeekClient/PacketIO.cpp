#include "StdAfx.h"
#include "packetio.h"
#include "WSocket.h"

#define MESSAGE_HEADER  4
#define INT_SIZE		4

PacketReader::PacketReader(char *data)
{
	packet = data;
	next_item = 0;
}

PacketReader::~PacketReader(void)
{
}

char *PacketReader::GetNextString()
{
	int sizeOfString = *(unsigned int *) (packet+next_item);
	next_item +=4;
	int temp_next_item = next_item;
	next_item +=sizeOfString;
	char* temp;
	temp = new char[sizeOfString+1];
	memcpy( temp, packet+temp_next_item, sizeOfString);
	temp[sizeOfString] = '\0';
	return temp;
}

unsigned int PacketReader::GetNextInt()
{
	int temp_next_item = next_item;
	next_item += 4;
	return *(unsigned int *) (packet + temp_next_item);
}

void PacketReader::GetNextIP(char * pRet)
{
	int temp_next_item = next_item;
	next_item += 4;

	in_addr ip;
	ip.S_un.S_addr = *(unsigned int *) (packet + temp_next_item);
	ip.S_un.S_addr = (ip.S_un.S_addr & 0xff000000) >> 24 | (ip.S_un.S_addr & 0x00ff0000) >> 8 |
		(ip.S_un.S_addr & 0x0000ff00) << 8 | (ip.S_un.S_addr & 0x000000ff) << 24;
	strcpy( pRet, inet_ntoa( ip ) );
}

BYTE PacketReader::GetNextByte()
{
	int temp_next_item = next_item;
	next_item += 1;
	//if( *(unsigned char *) (packet + temp_next_item) == 1 ) return true;
	//else return false;
	return *(BYTE *) (packet + temp_next_item);
}

// ===================================
//            Packet Writer
// ===================================
PacketWriter::PacketWriter(void)
{
	p_ip = NULL;
	m_port = 0;
	p_username = NULL;
	numOfBytes = MESSAGE_HEADER;
	tailPointer = MESSAGE_HEADER;
	packet = new unsigned char[numOfBytes];
	*(unsigned int *)packet = 0;  //its a empty packet
}
PacketWriter::PacketWriter(int size)
{
	p_ip = NULL;
	m_port = 0;
	p_username = NULL;
	//4 beginning bytes telling how large the size is
	numOfBytes = size + MESSAGE_HEADER;
	tailPointer = MESSAGE_HEADER;
	packet = new unsigned char[numOfBytes];
	*(unsigned int *)packet = size;
}

PacketWriter::~PacketWriter(void)
{
	if(packet != NULL)
	{
		delete[] packet;
		packet = NULL;
	}
	if(p_ip != NULL)
	{
		delete[] p_ip;
		p_ip = NULL;
	}
	if(p_username != NULL)
	{
		delete[] p_username;
		p_username = NULL;
	}
}

void PacketWriter::AppendSizeAndString(char* stringToWrite, bool resize)
{
	if(resize == true) Resize(numOfBytes + (int)strlen(stringToWrite) + INT_SIZE);
	*(unsigned int *)(packet+tailPointer) = (unsigned int)strlen(stringToWrite);
	tailPointer += INT_SIZE;
	memcpy( packet+tailPointer, stringToWrite, strlen(stringToWrite) );
	tailPointer += (int)strlen(stringToWrite);
}
void PacketWriter::AppendString(char* stringToWrite, bool resize)
{
	if(resize == true) Resize(numOfBytes + (int)strlen(stringToWrite));
	memcpy( packet+tailPointer, stringToWrite, strlen(stringToWrite) );
	tailPointer += (int)strlen(stringToWrite);
}

void PacketWriter::AppendData(char* stringToWrite, UINT size)
{
	memcpy( packet+tailPointer, stringToWrite, size );
	tailPointer += size;
}

//Insert string into packet on BODY INDEX (NOT PACKET INDEX)
void PacketWriter::InsertSizeAndString(char* stringToWrite, int index)
{
	*(unsigned int *)(packet+index+MESSAGE_HEADER) = (unsigned int)strlen(stringToWrite);
	memcpy( packet+index+INT_SIZE+MESSAGE_HEADER, stringToWrite, strlen(stringToWrite) );
}
	
void PacketWriter::AppendInt(unsigned int intToWrite, bool resize)
{
	if(resize == true) Resize(numOfBytes + INT_SIZE);
	*(unsigned int *)(packet+tailPointer) = (unsigned int)(intToWrite);
	tailPointer += 4;
}

void PacketWriter::AppendChar(char charToWrite, bool resize)
{
	if(resize == true) Resize(numOfBytes + 4);
	*(char *)(packet+tailPointer) = (char)(charToWrite);
	tailPointer += 4;
}

void PacketWriter::AppendByte(bool byteToWrite, bool resize)
{
	if(resize == true) Resize(numOfBytes + 1);
	*(bool *)(packet+tailPointer) = (bool)(byteToWrite);
	tailPointer += 1;
}

//Insert int into packet on BODY INDEX (NOT PACKET INDEX)
void PacketWriter::InsertInt(unsigned int intToWrite, int index)
{
	*(unsigned int *)(packet+index+MESSAGE_HEADER) = (unsigned int)(intToWrite);
}

void PacketWriter::Resize(int size)
{
	unsigned char* tempPacket = new unsigned char[size];
	memcpy(tempPacket, packet, numOfBytes);
	numOfBytes = size;  //update the class on how big the string is
	delete packet;
	packet = tempPacket;
	*(unsigned int *)packet = size;  //changes the header of the packet to show the appropriate size
}
	
unsigned char* PacketWriter::GetPacket(void)
{
	return packet;
}
	
int PacketWriter::GetPacketSize()
{
	return numOfBytes;
}

int PacketWriter::GetBodySize()
{
	return numOfBytes - MESSAGE_HEADER;
}