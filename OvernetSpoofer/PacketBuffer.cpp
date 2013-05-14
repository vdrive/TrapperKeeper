#include "StdAfx.h"
#include "packetbuffer.h"

//
//
//
PacketBuffer::PacketBuffer(void)
{
	p_data = p_read_ptr = NULL;
	m_used = m_allocated = 0;
}

//
//
//
PacketBuffer::~PacketBuffer(void)
{
	if(p_data != NULL)
		free(p_data);
}

//
//
//
// appends everything from append->read_ptr to packet
void PacketBuffer::Append(PacketBuffer& append)
{
	Write(append.p_read_ptr, append.Remaining());
}

//
//
//
// rewinds read_ptr to data
void PacketBuffer::Rewind()
{
	p_read_ptr = p_data;
}

//
//
//
// removes everything from before read_ptr from packet
void PacketBuffer::Truncate()
{
	size_t remaining = Remaining();
	size_t i;
	unsigned char *src = p_read_ptr;
	unsigned char *dst = p_data;

	for(i=0;i<remaining;++i,++src,++dst)
		*dst = *src;

	p_read_ptr = p_data;
	m_used = (int)remaining;
}

//
//
// returns size of entire packet
UINT PacketBuffer::Size()
{
	return m_used;
}

//
//
//
// returns size of remaining data
UINT PacketBuffer::Remaining()
{
	return (UINT)(m_used - (p_read_ptr - p_data));
}

//
//
//
/* make sure this packet is large enough to hold len bytes */
int PacketBuffer::Resize(size_t len)
{
	unsigned char *new_mem;
	size_t new_alloc, read_offset;


	/* realloc (0) == free */
	if(len == 0)
	{ 
		delete [] p_data;
		p_data = p_read_ptr = NULL;
		m_used = m_allocated = 0;
		return TRUE;
	}

	/* the buffer we have allocated is already large enough */
	if(m_allocated >= len)
		return TRUE;

	/* determine an appropriate resize length */
	new_alloc = m_allocated;
	while(new_alloc < len)
		new_alloc += 32;

	read_offset = p_read_ptr - p_data;

	/* gracefully fail if we are unable to resize the buffer */
	if(!(new_mem = (unsigned char*)realloc(p_data,new_alloc)))
		return FALSE;

	/* modify the packet structure to reflect this resize */
	p_data = new_mem;
	p_read_ptr = p_data + read_offset;
	m_allocated = (UINT)new_alloc;

	return TRUE;
}

//
//
//
// append data to packet, resizing if necessary, moving to next position
int PacketBuffer::Write(void* data, size_t size)
{
	if (!Resize(m_used + size))
		return FALSE;

	memcpy(p_data + m_used, data, size);
	m_used += (UINT)size;

	return TRUE;
}

// read data from packet, moving to next position
int PacketBuffer::Read(void* data, size_t size)
{
	if(Remaining() < size)
		return FALSE;

	memcpy(data, p_read_ptr, size);
	p_read_ptr += size;

	return TRUE;
}

//
//
// append uint8 to packet
void PacketBuffer::PutByte(byte data)
{
	Write(&data, sizeof(byte));
}

//
//
// append uint16 to packet
void PacketBuffer::PutUShort(unsigned short data)
{
	Write(&data, sizeof(unsigned short));
}

//
//
// append uint32 to packet
void PacketBuffer::PutUInt(unsigned int data)
{
	Write(&data, sizeof(unsigned int));
}

//
//
//
// append string of length len to packet
void PacketBuffer::PutUStr(const unsigned char *str, size_t len)
{
	Write((void*)str, len);
}

//
//
//
// append string of length len to packet
void PacketBuffer::PutStr(const char *str, size_t len)
{
	Write((void*)str, len);
}

//
//
// append FastTrack dynamic int to packet
void PacketBuffer::PutDynInt(unsigned int data)
{
	unsigned char buf[6];
	int len, i;
	unsigned int value = data;

	for (len = 1; value > 127; value >>= 7)
		len++;

	for (i = len-1; i >= 0; i--)
	{
		buf[i] = 0x80 | (unsigned char)(data & 0x7f);
		data >>= 7;
	}

	/* remove high bit from last byte */
	buf[len-1] &= 0x7f; 

	Write(buf, len);
}

//
//
// returns uint8 and moves read_ptr
byte PacketBuffer::GetByte()
{
	byte ret;
	Read(&ret, sizeof(byte));
	return ret;
}

//
//
// returns uint16 and moves read_ptr
unsigned short PacketBuffer::GetUShort()
{
	unsigned short ret;
	Read(&ret, sizeof(unsigned short));
	return ret;
}

//
//
// returns uint32 and moves read_ptr
unsigned int PacketBuffer::GetUInt()
{
	unsigned int ret;
	Read(&ret, sizeof(unsigned int));
	return ret;
}

//
//
// returns string and moves read_ptr, caller frees returned string
unsigned char* PacketBuffer::GetUStr(size_t len)
{
	if(len > MAX_STRING_SIZE)
		return NULL;

	unsigned char *ret = new unsigned char[len];
	if (!Read(ret, len))
	{
		delete [] ret;
		return NULL;
	}
	return ret;
}

//
//
// returns zero terminated string and moves read_ptr, caller frees returned string
char* PacketBuffer::GetStr(size_t len)
{
	if(len > MAX_STRING_SIZE)
		return NULL;

	char *ret = new char[len+1];
	if (!Read(ret, len))
	{
		delete [] ret;
		return NULL;
	}
	ret[len] = 0;
	return ret;
}

// reads FastTrack dynamic int and moves read_ptr
unsigned int PacketBuffer::GetDynInt()
{
	unsigned int ret = 0;
	byte curr;

    do
	{
		if(Read(&curr, 1) == FALSE)
			return 0;
		ret <<= 7;
		ret |= (curr & 0x7f);
    } while(curr & 0x80);

    return ret;
}

// counts the number of bytes from read_ptr until termbyte is reached
// returns -1 if termbyte doesn't occur in packet
int PacketBuffer::Strlen(byte termbyte)
{
	unsigned char *p = p_read_ptr;
	int remaining = Remaining();
	int i = 0;

	for(i=0; i < remaining; i++, p++)
		if(*p == termbyte)
			return i;

	return -1;
}