/* 
** FILE: buffer.cpp
**
** ABSTRACT:
**  None
**
** AUTHOR:
**	Dylan Douglas
**
** CREATION DATE:
**	07/03/2004
**
** NOTES:
**	None
*/

#include "StdAfx.h"
#include "buffer.h"

// Construct an empty "0 length" buffer
buffer::buffer(void)
: m_size(0), m_index(0)
{
	m_data = new BYTE[1];
	ASSERT( m_data != NULL );
}

// Construct a buffer of the given length, and copy the given array of bytes into it
buffer::buffer(const BYTE *pData, size_t len)
: m_index(0)
{
	m_data = new BYTE[len];
	ASSERT( m_data != NULL );
	memcpy( m_data, pData, len );

	m_size = len;
}

// Construct a buffer of the given length, and copy the given array of chars into it
buffer::buffer(const CHAR *pData, size_t len)
: m_index(0)
{
	m_data = new BYTE[len];
	ASSERT( m_data != NULL );
	memcpy( m_data, pData, len );

	m_size = len;
}

// Construct a buffer equal to the length of the given null-terminated array of chars,
// and copy the chars into it
buffer::buffer(const CHAR *pData)
: m_index(0)
{
	m_data = new BYTE[ strlen( pData )];
	ASSERT( m_data != NULL );
	memcpy( m_data, pData, strlen( pData ) + 1 );

	m_size = strlen( pData );
}


//


// Construct a buffer of the given length, initialized to '0's
buffer::buffer(size_t len)
: m_index(0)
{
	m_data = new BYTE[len];
	ASSERT( m_data != NULL );
	memset( m_data, 0, len );

	m_size = len;
}

// Destroy the byte buffer
buffer::~buffer(void)
{
	delete [] m_data;
}


// Resize the buffer to the given length, and copy len bytes of the given byte array into it
void buffer::SetBuffer(const BYTE *pData, size_t len)
{
	delete [] m_data;

	m_data = new BYTE[len];
	ASSERT( m_data != NULL );
	memcpy( m_data, pData, len );

	m_index = 0;
	m_size = len;
}

// Resize the buffer to the given length, and copy len chars of the given char array into it
void buffer::SetBuffer(const CHAR *pData, size_t len)
{
	delete [] m_data;

	m_data = new BYTE[len];
	ASSERT( m_data != NULL );
	memcpy( m_data, pData, len );

	m_index = 0;
	m_size = len;
}

// Resize the buffer to the length of the given null-terminated char array, and copy the chars
// of the given char array into it until the null is reached
void buffer::SetBuffer(const CHAR *pData)
{
	m_data = new BYTE[ strlen( pData )];
	ASSERT( m_data != NULL );
	memcpy( m_data, pData, strlen( pData ) + 1 );

	m_size = strlen( pData );
}

// Keep the buffer the same size, but set all of the bytes to '0'
void buffer::ClearBuffer(void)
{
	memset( m_data, 0, m_size );

	m_index = 0;
}

// Resize the buffer to the given length.  if the new size is smaller then the old size,
// the old data is truncated.  if the new size is larger, the additional space is uninitialized
void buffer::ResizeBuffer(size_t len)
{
	ASSERT( len >= 0 );

	if( len == 0 )
	{
		BYTE *pTempData = new BYTE[1];
		ASSERT( pTempData != NULL );
		pTempData[0] = 0;

		delete [] m_data;
		m_data = pTempData;

		m_index = 0;
		m_size = 0;
	}
	else if( len != m_size )
	{
		BYTE *pTempData = new BYTE[len];
		ASSERT( pTempData != NULL );
		memcpy( pTempData, m_data, min( m_size, len ) );

		delete [] m_data;
		m_data = pTempData;

		m_index = min( m_index, len );
		m_size = len;
	}
}


// Get Functions
// Get Functions
// Get Functions
CHAR buffer::GetChar(size_t index)
{
	ASSERT( index + sizeof(CHAR) <= m_size );
	return (CHAR)m_data[index];
}

BYTE buffer::GetByte(size_t index)
{
	ASSERT( index + sizeof(BYTE) <= m_size );
	return m_data[index];
}

WORD buffer::GetWord(size_t index)
{
	ASSERT( index + sizeof(WORD) <= m_size );
	return *((WORD *)&(m_data[index]));
}

DWORD buffer::GetDword(size_t index)
{
	ASSERT( index + sizeof(DWORD) <= m_size );

	DWORD ret = *( (DWORD *) (&(m_data[index])) );

	BYTE temp[4];
	for( int i = 0; i < sizeof(DWORD); i++ )
		temp[i] = m_data[index+i];
	DWORD ret2 = *( (DWORD *)temp );

	DWORD ret3 = ( (DWORD *) (&(m_data[index])) )[0];

	return ret;
}

SHORT buffer::GetShort(size_t index)
{
	ASSERT( index + sizeof(SHORT) <= m_size );
	return *((SHORT *)&(m_data[index]));
}

USHORT buffer::GetUshort(size_t index)
{
	ASSERT( index + sizeof(USHORT) <= m_size );

	USHORT ret = *((USHORT *)&(m_data[index]));
	BYTE retByte = m_data[index];


	return *((USHORT *)&(m_data[index]));
}

INT buffer::GetInt(size_t index)
{
	ASSERT( index + sizeof(INT) <= m_size );
	return *((INT *)&(m_data[index]));
}

UINT buffer::GetUint(size_t index)
{
	ASSERT( index + sizeof(UINT) <= m_size );
	return *((UINT *)&(m_data[index]));
}

LONG buffer::GetLong(size_t index)
{
	ASSERT( index + sizeof(LONG) <= m_size );
	return *((LONG *)&(m_data[index]));
}

const CHAR *buffer::GetCharPtr(size_t index)
{
	ASSERT( index < m_size );
	return (const CHAR *)(m_data + index);
}

const UCHAR *buffer::GetUcharPtr(size_t index)
{
	ASSERT( index < m_size );
	return m_data + index;
}

const BYTE *buffer::GetBytePtr(size_t index)
{
	ASSERT( index < m_size );
	return m_data + index;
}

CHAR *buffer::GetMutableCharPtr(size_t index)
{
	ASSERT( index < m_size );
	return (CHAR *)(m_data + index);
}

UCHAR *buffer::GetMutableUcharPtr(size_t index)
{
	ASSERT( index < m_size );
	return m_data + index;
}

BYTE *buffer::GetMutableBytePtr(size_t index)
{
	ASSERT( index < m_size );
	return m_data + index;
}

// Set Functions
// Set Functions
// Set Functions
void buffer::SetChar(CHAR data, size_t index)
{
	ASSERT( index + sizeof(CHAR) <= m_size );
	*((CHAR *)&(m_data[index])) = data;
}

void buffer::SetByte(BYTE data, size_t index)
{
	ASSERT( index + sizeof(BYTE) <= m_size );
	*((BYTE *)&(m_data[index])) = data;
}

void buffer::SetWord(WORD data, size_t index)
{
	ASSERT( index + sizeof(WORD) <= m_size );
	*((WORD *)&(m_data[index])) = data;
}

void buffer::SetDword(DWORD data, size_t index)
{
	ASSERT( index + sizeof(DWORD) <= m_size );
	*((DWORD *)&(m_data[index])) = data;
}

void buffer::SetShort(SHORT data, size_t index)
{
	ASSERT( index + sizeof(SHORT) <= m_size );
	*((SHORT *)&(m_data[index])) = data;
}

void buffer::SetUshort(USHORT data, size_t index)
{
	ASSERT( index + sizeof(USHORT) <= m_size );
	*((USHORT *)&(m_data[index])) = data;
}

void buffer::SetInt(INT data, size_t index)
{
	ASSERT( index + sizeof(INT) <= m_size );
	*((INT *)&(m_data[index])) = data;
}

void buffer::SetUint(UINT data, size_t index)
{
	ASSERT( index + sizeof(UINT) <= m_size );
	*((UINT *)&(m_data[index])) = data;
}

void buffer::SetLong(LONG data, size_t index)
{
	ASSERT( index + sizeof(LONG) <= m_size );
	*((LONG *)&(m_data[index])) = data;
}

void buffer::SetCharPtr(const CHAR *pData, size_t len, size_t index)
{
	ASSERT( index + len <= m_size );
	memcpy( (CHAR *)&(m_data[index]), pData, len );
}

void buffer::SetBytePtr(const BYTE *pData, size_t len, size_t index)
{
	ASSERT( index + len <= m_size );
	memcpy( (BYTE *)&(m_data[index]), pData, len );
}



// Put Functions
// Put Functions
// Put Functions
void buffer::PutChar(CHAR data)
{
	if( m_index + sizeof(CHAR) > m_size )
		ResizeBuffer( m_index + sizeof(CHAR) );
	ASSERT( m_index + sizeof(CHAR) <= m_size );
	*((CHAR *)&(m_data[m_index])) = data;
	m_index += sizeof(CHAR);
}

void buffer::PutByte(BYTE data)
{
	if( m_index + sizeof(BYTE) > m_size )
		ResizeBuffer( m_index + sizeof(BYTE) );
	ASSERT( m_index + sizeof(BYTE) <= m_size );
	*((BYTE *)&(m_data[m_index])) = data;
	m_index += sizeof(BYTE);
}

void buffer::PutWord(WORD data)
{
	if( m_index + sizeof(WORD) > m_size )
		ResizeBuffer( m_index + sizeof(WORD) );
	ASSERT( m_index + sizeof(WORD) <= m_size );
	*((WORD *)&(m_data[m_index])) = data;
	m_index += sizeof(WORD);
}

void buffer::PutDword(DWORD data)
{
	if( m_index + sizeof(DWORD) > m_size )
		ResizeBuffer( m_index + sizeof(DWORD) );
	ASSERT( m_index + sizeof(DWORD) <= m_size );
	*((DWORD *)&(m_data[m_index])) = data;
	m_index += sizeof(DWORD);
}

void buffer::PutShort(SHORT data)
{
	if( m_index + sizeof(SHORT) > m_size )
		ResizeBuffer( m_index + sizeof(SHORT) );
	ASSERT( m_index + sizeof(SHORT) <= m_size );
	*((SHORT *)&(m_data[m_index])) = data;
	m_index += sizeof(SHORT);
}

void buffer::PutUshort(USHORT data)
{
	if( m_index + sizeof(USHORT) > m_size )
		ResizeBuffer( m_index + sizeof(USHORT) );
	ASSERT( m_index + sizeof(USHORT) <= m_size );
	*((USHORT *)&(m_data[m_index])) = data;
	m_index += sizeof(USHORT);
}

void buffer::PutInt(INT data)
{
	if( m_index + sizeof(INT) > m_size )
		ResizeBuffer( m_index + sizeof(INT) );
	ASSERT( m_index + sizeof(INT) <= m_size );
	*((INT *)&(m_data[m_index])) = data;
	m_index += sizeof(INT);
}

void buffer::PutUint(UINT data)
{
	if( m_index + sizeof(UINT) > m_size )
		ResizeBuffer( m_index + sizeof(UINT) );
	ASSERT( m_index + sizeof(UINT) <= m_size );
	*((UINT *)&(m_data[m_index])) = data;
	m_index += sizeof(UINT);
}

void buffer::PutLong(LONG data)
{
	if( m_index + sizeof(LONG) > m_size )
		ResizeBuffer( m_index + sizeof(LONG) );
	ASSERT( m_index + sizeof(LONG) <= m_size );
	*((LONG *)&(m_data[m_index])) = data;
	m_index += sizeof(LONG);
}

void buffer::PutCharPtr(const CHAR *pData, size_t len)
{
	if( m_index + len > m_size )
		ResizeBuffer( m_index + len );
	ASSERT( m_index + len <= m_size );
	memcpy( (CHAR *)&(m_data[m_index]), pData, len );
	m_index += len;
}

void buffer::PutBytePtr(const BYTE *pData, size_t len)
{
	if( m_index + len > m_size )
		ResizeBuffer( m_index + len );
	ASSERT( m_index + len <= m_size );
	memcpy( (BYTE *)&(m_data[m_index]), pData, len );
	m_index += len;
}

void buffer::PutByteBuffer(buffer *pBuf, bool bDeleteAfter)
{
	if( m_index + pBuf->Size() > m_size )
		ResizeBuffer( m_index + pBuf->Size() );
	ASSERT( m_index + pBuf->Size() <= m_size );
	memcpy( (BYTE *)&(m_data[m_index]), pBuf->GetBytePtr(), pBuf->Size() );
	m_index += pBuf->Size();

	if( bDeleteAfter )
		delete pBuf;
}


// drops n bytes off the front of the buffer and resizes the buffer
void buffer::DropFront(size_t len)
{
	ASSERT( len >= 0 && len <= m_size );

	if( len == 0 )
	{
		return;
	}
	else if( len == m_size )
	{
		BYTE *pTempData = new BYTE[1];
		ASSERT( pTempData != NULL );
		pTempData[0] = 0;

		delete [] m_data;
		m_data = pTempData;

		m_index = 0;
		m_size = 0;
	}
	else
	{
		BYTE *pTempData = new BYTE[ m_size - len ];
		ASSERT( pTempData != NULL );
		memcpy( pTempData, m_data + len, m_size - len );

		delete [] m_data;
		m_data = pTempData;

		m_index = max( m_index - len, 0 );
		m_size = m_size - len;
	}
}

// drops n bytes off the back of the buffer and resizes the buffer
void buffer::DropBack(size_t len)
{
	ASSERT( len >= 0 && len <= m_size );

	if( len == 0 )
	{
		return;
	}
	else
	{
		ResizeBuffer( m_size - len );
	}
}

// set equal to operator
buffer &buffer::operator=( const buffer &right )
{
	this->SetBuffer( right.m_data, right.m_size );
	return *this;
}

// IsEqualTo? operator
bool buffer::operator==( const buffer &right )
{
	if( this->m_size != right.m_size )
		return false;

	return ( strncmp( (const char *)(this->m_data), (const char *)(right.m_data), this->m_size ) == 0 );
}
