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
const char *Hash20Byte::ToPrintableCharPtr(char *pBuf) const
{
	if( pBuf == NULL )
		return NULL;

	for( int i = 0, j = 0; i < 20; i++, j+=2 )
		sprintf( pBuf+j, "%02x", m_aHash[i] );

	return pBuf;
}

//
//
//
void Hash20Byte::ZeroHash(void)
{
	ZeroMemory( m_aHash, 20 );
}

//
//
//
bool Hash20Byte::IsZero(void) const
{
	return ( m_aHash[0] == 0 && m_aHash[1] == 0 && m_aHash[2] == 0 && m_aHash[3] == 0 && m_aHash[4] == 0 &&
		m_aHash[5] == 0 && m_aHash[6] == 0 && m_aHash[7] == 0 && m_aHash[8] == 0 && m_aHash[9] == 0 &&
		m_aHash[10] == 0 && m_aHash[11] == 0 && m_aHash[12] == 0 && m_aHash[13] == 0 && m_aHash[14] == 0 &&
		m_aHash[15] == 0 && m_aHash[16] == 0 && m_aHash[17] == 0 && m_aHash[18] == 0 && m_aHash[19] == 0 );
}
