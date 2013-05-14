#include "StdAfx.h"
#include "Hash20Byte.h"

//
//
//
Hash20Byte::Hash20Byte(void)
{
	ZeroMemory( m_aHash, 20 );
}

//
//
//
Hash20Byte::Hash20Byte(const unsigned char aHash[20])
{
	if( aHash == NULL )
		ZeroMemory( m_aHash, 20 );
	else
		memcpy( m_aHash, aHash, 20 );
}

//
//
//
Hash20Byte::Hash20Byte(const char aHash[20])
{
	if( aHash == NULL )
		ZeroMemory( m_aHash, 20 );
	else
		memcpy( m_aHash, aHash, 20 );
}

//
//
//
Hash20Byte::Hash20Byte(const Hash20Byte &rRight)
{
	memcpy( m_aHash, rRight.m_aHash, 20 );
}

//
//
//
Hash20Byte &Hash20Byte::operator=(const Hash20Byte &rRight)
{
	memcpy( m_aHash, rRight.m_aHash, 20 );

	return (*this);
}

//
//
//
Hash20Byte &Hash20Byte::operator=(const unsigned char aRight[20])
{
	if( aRight == NULL )
		ZeroMemory( m_aHash, 20 );
	else
		memcpy( m_aHash, aRight, 20 );

	return (*this);
}

//
//
//
Hash20Byte &Hash20Byte::operator=(const char aRight[20])
{
	if( aRight == NULL )
		ZeroMemory( m_aHash, 20 );
	else
		memcpy( m_aHash, aRight, 20 );

	return (*this);
}

//
//
//
bool Hash20Byte::operator==(const Hash20Byte &rRight) const
{
	return ( memcmp( m_aHash, rRight.m_aHash, 20 ) == 0 );
}

//
//
//
bool Hash20Byte::operator!=(const Hash20Byte &rRight) const
{
	return ( memcmp( m_aHash, rRight.m_aHash, 20 ) != 0 );
}

//
//
//
bool operator<(const Hash20Byte &rLeft, const Hash20Byte &rRight)
{
	return ( memcmp( rLeft.m_aHash, rRight.m_aHash, 20 ) < 0 );
}

//
//
//
bool operator>(const Hash20Byte &rLeft, const Hash20Byte &rRight)
{
	return ( memcmp( rLeft.m_aHash, rRight.m_aHash, 20 ) > 0 );
}

//
//
//
void Hash20Byte::FromHexString(const char *pData)
{
	if( pData == NULL )
	{
		ZeroHash();
	}
	else
	{
		for( int nDataIdx = 0, nHashIdx = 0; nHashIdx < 20; nHashIdx++, nDataIdx += 2 )
		{
			m_aHash[nHashIdx] = htoi( pData[nDataIdx], pData[nDataIdx+1] );
		}
	}
}

//
//
//
void Hash20Byte::FromEscapedHexString(const char *pData)
{
	if( pData == NULL )
	{
		ZeroHash();
	}
	else
	{
		for( int nDataIdx = 0, nHashIdx = 0; nDataIdx < 60 && nHashIdx < 20; )
		{
			if( pData[nDataIdx] == '%' )
			{
				m_aHash[nHashIdx] = htoi( pData[nDataIdx+1], pData[nDataIdx+2] );
				nDataIdx += 3;
			}
			else
			{
				m_aHash[nHashIdx] = pData[nDataIdx];
				nDataIdx += 1;
			}

			nHashIdx++;
		}
	}
}

//
//
//
const char *Hash20Byte::CopyPrintableToCharBuf(char *pBuf) const
{
	if( pBuf == NULL )
		return NULL;

	BYTE currentByte = 0;

	for( int i = 0, j = 0; i < 20; i++, j+=2 )
	{
		currentByte =  m_aHash[i];
		sprintf( pBuf+j, "%02x", currentByte );
	}

	return pBuf;
}

//
//
//
void Hash20Byte::ZeroHash(void)
{ 
	ZeroMemory( m_aHash, 20 );
}
