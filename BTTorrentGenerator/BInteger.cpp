#include "StdAfx.h"
#include "BInteger.h"

BInteger::BInteger(void)
{
	m_btype=BEncoding::btype::integer;
	m_value=0;
}

BInteger::~BInteger(void)
{
}

BInteger::BInteger(int regularInt)
{
	m_btype=BEncoding::btype::integer;
	m_value = abs(regularInt);
	signedValue = regularInt;
}

//
//
//
void BInteger::WriteEncodedData(CFile *pOutputFile)
{
	if( pOutputFile == NULL )
		return;

	char aBuf[ sizeof(int)*8 + 3 ];

	int nLen = sprintf( aBuf, "%c%u%c", 'i', (unsigned int)signedValue, 'e' );

	if( nLen < 0 )
	{
		TRACE( "BInteger::sprintf() to buffer error" );
	}
	else
	{
		pOutputFile->Write( aBuf, nLen );
	}
}
