#include "StdAfx.h"
#include "ByteBufferReader.h"

ByteBufferReader::ByteBufferReader(void)
: m_pData(NULL), m_size(0)
{
}

ByteBufferReader::ByteBufferReader(const CHAR *pData, size_t nLen)
: m_pData( (const BYTE *)pData ), m_size(nLen)
{
}

ByteBufferReader::ByteBufferReader(const BYTE *pData, size_t nLen)
: m_pData(pData), m_size(nLen)
{
}

ByteBufferReader::~ByteBufferReader(void)
{
	m_pData = NULL;
	m_size = 0;
}


// Resize the buffer to the given length, and set the m_pData to point to the given pData
void ByteBufferReader::SetBuffer(const BYTE *pData, size_t nLen)
{
	m_pData = pData;

	if( pData == NULL )
		m_size = 0;
	else
		m_size = nLen;
}


// Resize the buffer to the given length, and set the m_pData to point to the given pData
void ByteBufferReader::SetBuffer(const CHAR *pData, size_t nLen)
{
	m_pData = (const BYTE *)pData;

	if( pData == NULL )
		m_size = 0;
	else
		m_size = nLen;
}
