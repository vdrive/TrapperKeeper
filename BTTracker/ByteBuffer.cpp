/* 
** FILE: ByteBuffer.cpp
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
#include "ByteBuffer.h"

//
// Construct an empty buffer
//
ByteBuffer::ByteBuffer(void)
: m_pData(NULL), m_allocated(0), m_size(0), m_index(0)
{
}

//
// Construct a buffer of the given length, and copy the given array of bytes into it
//
ByteBuffer::ByteBuffer(const BYTE *pData, size_t nLen)
: m_allocated(nLen), m_size(nLen), m_index(0)
{
	if( pData == NULL || nLen == 0 )
	{
		m_pData = NULL;
	}
	else
	{
		m_pData = new BYTE[nLen];
		ASSERT( m_pData != NULL );
		memcpy( m_pData, pData, nLen );
	}
}

//
// Construct a buffer of the given length, and copy the given array of chars into it
//
ByteBuffer::ByteBuffer(const CHAR *pData, size_t nLen)
: m_allocated(nLen), m_size(nLen), m_index(0)
{
	if( pData == NULL || nLen == 0 )
	{
		m_pData = NULL;
	}
	else
	{
		m_pData = new BYTE[nLen];
		ASSERT( m_pData != NULL );
		memcpy( m_pData, pData, nLen );
	}
}

//
// Construct a buffer equal to the length of the given NULL-terminated array of chars, and copy the chars
// into it (does not include the NULL)
//
ByteBuffer::ByteBuffer(const CHAR *pData)
: m_index(0)
{
	if( pData == NULL )
	{
		m_pData = NULL;
		m_size = 0;
	}
	else
	{
		m_size = strlen( pData );

		m_pData = new BYTE[ m_size ];
		ASSERT( m_pData != NULL );
		memcpy( m_pData, pData, m_size );
	}
}

//
// Copy constructor
//
ByteBuffer::ByteBuffer(const ByteBuffer &rBuf)
{
	if( rBuf.m_pData == NULL )
	{
		m_pData = NULL;
		m_size = 0;
		m_index = 0;
	}
	else
	{
		m_pData = new BYTE[ rBuf.m_size ];
		ASSERT( m_pData != NULL );
		memcpy( m_pData, rBuf.m_pData, rBuf.m_size );

		m_size = rBuf.m_size;
		m_index = rBuf.m_index;
	}
}

//
// Construct a buffer of the given length, initialized to '0's
//
ByteBuffer::ByteBuffer(size_t nLen)
: m_size(nLen), m_index(0)
{
	if( nLen == 0 )
	{
		m_pData = NULL;
	}
	else
	{
		m_pData = new BYTE[nLen];
		ASSERT( m_pData != NULL );
		ZeroMemory( m_pData, nLen );
	}
}

//
// Destroy the byte buffer
//
ByteBuffer::~ByteBuffer(void)
{
	if( m_pData != NULL )
	{
		delete [] m_pData;
		m_pData = NULL;
	}
}

//
// Resize the buffer to the given length, and copy nLen bytes of the given byte array into it
//
void ByteBuffer::SetBuffer(const BYTE *pData, size_t nLen)
{
	if( pData == NULL || nLen == 0 )
		return;

	if( m_pData != NULL )
		delete [] m_pData;

	m_pData = new BYTE[nLen];
	ASSERT( m_pData != NULL );
	memcpy( m_pData, pData, nLen );

	m_index = 0;
	m_size = nLen;
}

//
// Resize the buffer to the given length, and copy nLen chars of the given char array into it
//
void ByteBuffer::SetBuffer(const CHAR *pData, size_t nLen)
{
	if( pData == NULL || nLen == 0 )
		return;

	if( m_pData != NULL )
		delete [] m_pData;

	m_pData = new BYTE[nLen];
	ASSERT( m_pData != NULL );
	memcpy( m_pData, pData, nLen );

	m_index = 0;
	m_size = nLen;
}

//
// Resize the buffer to the length of the given null-terminated char array, and copy the chars
// of the given char array into it until the null is reached.  does not copy the NULL
//
void ByteBuffer::SetBuffer(const CHAR *pData)
{
	if( pData == NULL )
		return;

	if( m_pData != NULL )
		delete [] m_pData;

	m_index = 0;
	m_size = strlen( pData );

	m_pData = new BYTE[ m_size ];
	ASSERT( m_pData != NULL );
	memcpy( m_pData, pData, m_size );
}

//
// Deletes the buffer, but does not destroy the object
//
void ByteBuffer::ResetBuffer(void)
{
	if( m_pData != NULL )
	{
		delete [] m_pData;
		m_pData = NULL;
	}

	m_index = 0;
	m_size = 0;
}

//
// Keep the buffer the same size, but set all of the bytes to 0x0
//
void ByteBuffer::ZeroOutBuffer(void)
{
	ZeroMemory( m_pData, m_size );
	m_index = 0;
}

//
// Resize the buffer to the given length.  if the new size is smaller then the old size,
// the old data is truncated.  if the new size is larger, the additional space is uninitialized
//
void ByteBuffer::ResizeBuffer(size_t nLen)
{
	// IF the new size is the current size
	if( nLen == m_size )
	{
		// return
		return;
	}
	// ELSE IF the new size is an empty buffer
	else if( nLen == 0 )
	{
		// reset the buffer
		ResetBuffer();
	}
	// ELSE IF the new size is smaller than the current buffer AND there is a buffer
	else if( nLen < m_size && m_pData != NULL )
	{
		BYTE *pTempData = new BYTE[nLen];
		ASSERT( pTempData != NULL );
		memcpy( pTempData, m_pData, nLen );

		delete [] m_pData;
		m_pData = pTempData;

		m_index = min( m_index, nLen );
		m_size = nLen;
	}
	// ELSE the new size is larger than the current buffer or there was no buffer for some reason
	else
	{
		BYTE *pTempData = new BYTE[nLen];
		ASSERT( pTempData != NULL );

		if( m_pData != NULL )
		{
			memcpy( pTempData, m_pData, m_size );
			delete [] m_pData;
		}

		m_pData = pTempData;
		m_size = nLen;
	}
}

//
// Set CHAR at given byte index
//
void ByteBuffer::SetChar(CHAR data, size_t index)
{
	if( index + sizeof(CHAR) > m_size )
	{
		ASSERT(false);
		return;
	}

	*((CHAR *)(m_pData + index)) = data;
}

//
// Set BYTE at given byte index
//
void ByteBuffer::SetByte(BYTE data, size_t index)
{
	if( index + sizeof(BYTE) > m_size )
	{
		ASSERT(false);
		return;
	}

	*((BYTE *)(m_pData + index)) = data;
}

//
// Set WORD at given byte index
//
void ByteBuffer::SetWord(WORD data, size_t index)
{
	if( index + sizeof(WORD) > m_size )
	{
		ASSERT(false);
		return;
	}

	*((WORD *)(m_pData + index)) = data;
}

//
// Set DWORD at given byte index
//
void ByteBuffer::SetDword(DWORD data, size_t index)
{
	if( index + sizeof(DWORD) > m_size )
	{
		ASSERT(false);
		return;
	}

	*((DWORD *)(m_pData + index)) = data;
}

//
// Set SHORT at given byte index
//
void ByteBuffer::SetShort(SHORT data, size_t index)
{
	if( index + sizeof(SHORT) > m_size )
	{
		ASSERT(false);
		return;
	}

	*((SHORT *)(m_pData + index)) = data;
}

//
// Set USHORT at given byte index
//
void ByteBuffer::SetUshort(USHORT data, size_t index)
{
	if( index + sizeof(USHORT) > m_size )
	{
		ASSERT(false);
		return;
	}

	*((USHORT *)(m_pData + index)) = data;
}

//
// Set INT at given byte index
//
void ByteBuffer::SetInt(INT data, size_t index)
{
	if( index + sizeof(INT) > m_size )
	{
		ASSERT(false);
		return;
	}

	*((INT *)(m_pData + index)) = data;
}

//
// Set UINT at given byte index
//
void ByteBuffer::SetUint(UINT data, size_t index)
{
	if( index + sizeof(UINT) > m_size )
	{
		ASSERT(false);
		return;
	}

	*((UINT *)(m_pData + index)) = data;
}

//
// Set LONG at given byte index
//
void ByteBuffer::SetLong(LONG data, size_t index)
{
	if( index + sizeof(LONG) > m_size )
	{
		ASSERT(false);
		return;
	}

	*((LONG *)(m_pData + index)) = data;
}

//
// Set const CHAR * at given byte index for nLen bytes
//
void ByteBuffer::SetCharPtr(const CHAR *pData, size_t nLen, size_t index)
{
	if( index + nLen > m_size || pData == NULL )
	{
		ASSERT(false);
		return;
	}

	memcpy( m_pData + index, pData, nLen );
}

//
// Set const BYTE * at given byte index for nLen bytes
//
void ByteBuffer::SetBytePtr(const BYTE *pData, size_t nLen, size_t index)
{
	if( index + nLen > m_size || pData == NULL )
	{
		ASSERT(false);
		return;
	}

	memcpy( m_pData + index, pData, nLen );
}

//
// Put CHAR at current byte index
//
void ByteBuffer::PutChar(CHAR data)
{
	if( m_index + sizeof(CHAR) > m_size )
		ResizeBuffer( m_index + sizeof(CHAR) );

	*((CHAR *)(m_pData + m_index)) = data;
	m_index += sizeof(CHAR);
}

//
// Put BYTE at current byte index
//
void ByteBuffer::PutByte(BYTE data)
{
	if( m_index + sizeof(BYTE) > m_size )
		ResizeBuffer( m_index + sizeof(BYTE) );

	*((BYTE *)(m_pData + m_index)) = data;
	m_index += sizeof(BYTE);
}

//
// Put WORD at current byte index
//
void ByteBuffer::PutWord(WORD data)
{
	if( m_index + sizeof(WORD) > m_size )
		ResizeBuffer( m_index + sizeof(WORD) );

	*((WORD *)(m_pData + m_index)) = data;
	m_index += sizeof(WORD);
}

//
// Put DWORD at current byte index
//
void ByteBuffer::PutDword(DWORD data)
{
	if( m_index + sizeof(DWORD) > m_size )
		ResizeBuffer( m_index + sizeof(DWORD) );

	*((DWORD *)(m_pData + m_index)) = data;
	m_index += sizeof(DWORD);
}

//
// Put SHORT at current byte index
//
void ByteBuffer::PutShort(SHORT data)
{
	if( m_index + sizeof(SHORT) > m_size )
		ResizeBuffer( m_index + sizeof(SHORT) );

	*((SHORT *)(m_pData + m_index)) = data;
	m_index += sizeof(SHORT);
}

//
// Put USHORT at current byte index
//
void ByteBuffer::PutUshort(USHORT data)
{
	if( m_index + sizeof(USHORT) > m_size )
		ResizeBuffer( m_index + sizeof(USHORT) );

	*((USHORT *)(m_pData + m_index)) = data;
	m_index += sizeof(USHORT);
}

//
// Put INT at current byte index
//
void ByteBuffer::PutInt(INT data)
{
	if( m_index + sizeof(INT) > m_size )
		ResizeBuffer( m_index + sizeof(INT) );

	*((INT *)(m_pData + m_index)) = data;
	m_index += sizeof(INT);
}

//
// Put UINT at current byte index
//
void ByteBuffer::PutUint(UINT data)
{
	if( m_index + sizeof(UINT) > m_size )
		ResizeBuffer( m_index + sizeof(UINT) );

	*((UINT *)(m_pData + m_index)) = data;
	m_index += sizeof(UINT);
}

//
// Put LONG at current byte index
//
void ByteBuffer::PutLong(LONG data)
{
	if( m_index + sizeof(LONG) > m_size )
		ResizeBuffer( m_index + sizeof(LONG) );

	*((LONG *)(m_pData + m_index)) = data;
	m_index += sizeof(LONG);
}

//
// Put const CHAR * with NULL termination at current byte index, does not put the NULL in the ByteBuffer
//
void ByteBuffer::PutCharPtr(const CHAR *pData)
{
	if( pData == NULL )
	{
		ASSERT(false);
		return;
	}

	size_t newStrLen = strlen( pData );

	if( m_index + newStrLen > m_size )
		ResizeBuffer( m_index + newStrLen );

	memcpy( m_pData + m_index, pData, newStrLen );
	m_index += newStrLen;
}

//
// Put const CHAR * at current byte index for nLen bytes
//
void ByteBuffer::PutCharPtr(const CHAR *pData, size_t nLen)
{
	if( pData == NULL || nLen == 0 )
	{
		ASSERT(false);
		return;
	}

	if( m_index + nLen > m_size )
		ResizeBuffer( m_index + nLen );

	memcpy( m_pData + m_index, pData, nLen );
	m_index += nLen;
}

//
// Put const BYTE * at current byte index for nLen bytes
//
void ByteBuffer::PutBytePtr(const BYTE *pData, size_t nLen)
{
	if( pData == NULL || nLen == 0 )
	{
		ASSERT(false);
		return;
	}

	if( m_index + nLen > m_size )
		ResizeBuffer( m_index + nLen );

	memcpy( m_pData + m_index, pData, nLen );
	m_index += nLen;
}

//
// Put ByteBuffer at current byte index.  if bDeleteAfter is true, the given ByteBuffer will be deleted after the put
//
void ByteBuffer::PutByteBuffer(ByteBuffer *pBuf, bool bDeleteAfter /*= false*/)
{
	if( pBuf == NULL )
	{
		ASSERT(false);
		return;
	}

	if( m_index + pBuf->m_size > m_size )
		ResizeBuffer( m_index + pBuf->m_size );

	memcpy( m_pData + m_index, pBuf->m_pData, pBuf->m_size );
	m_index += pBuf->m_size;

	if( bDeleteAfter )
		delete pBuf;
}

//
// drops n bytes off the front of the buffer and resizes the buffer
//
void ByteBuffer::DropFront(size_t nLen)
{
	if( nLen > m_size || nLen == 0 )
	{
		ASSERT(false);
		return;
	}

	if( nLen == m_size )
	{
		ResetBuffer();
	}
	else
	{
		BYTE *pTempData = new BYTE[ m_size - nLen ];
		ASSERT( pTempData != NULL );

		if( m_pData != NULL )
		{
			memcpy( pTempData, m_pData + nLen, m_size - nLen );
			delete [] m_pData;
		}

		m_pData = pTempData;

		m_index = max( m_index - nLen, 0 );
		m_size = m_size - nLen;
	}
}

//
// drops n bytes off the back of the buffer and resizes the buffer
//
void ByteBuffer::DropBack(size_t nLen)
{
	if( nLen > m_size || nLen == 0 )
	{
		ASSERT(false);
		return;
	}

	ResizeBuffer( m_size - nLen );
}

//
// set equal to operator
//
ByteBuffer &ByteBuffer::operator=(const ByteBuffer &rRight)
{
	SetBuffer( rRight.m_pData, rRight.m_size );
	return *this;
}

//
// IsEqualTo? operator
//
bool ByteBuffer::operator==(const ByteBuffer &rRight) const
{
	return ( m_size == rRight.m_size ) && ( memcmp( m_pData, rRight.m_pData, m_size ) == 0 );
}
