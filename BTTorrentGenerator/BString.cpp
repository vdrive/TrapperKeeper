#include "StdAfx.h"
#include "BString.h"

//
//
//
BString::BString(void)
{
	m_btype=BEncoding::btype::string;

	m_len=0;
	p_buf=NULL;
}

BString::BString(std::string str)
{
	m_btype = BEncoding::btype::string;
	m_len = (unsigned int) str.length();
	p_buf = new UCHAR[m_len+1];
	memcpy( p_buf, str.c_str(), m_len+1 );
}

//
//
//
BString::~BString(void)
{
	if(p_buf!=NULL)
	{
		delete [] p_buf;
	}
}

void BString::SetString(std::string str)
{
	if (p_buf != NULL)
	{
		delete [] p_buf;
	}
	m_len = (unsigned int) str.length();
	p_buf = new unsigned char[m_len+1];
	memcpy(p_buf,str.c_str(),m_len+1);
}

void BString::WriteEncodedData(CFile *pOutputFile)
{
	if( pOutputFile == NULL )
		return;

	char aBuf[ (sizeof(int)*8) + 2 ];

	int nLen = sprintf( aBuf, "%d%c", m_len, ':' );

	if(nLen < 0)
	{
		TRACE( "BString::sprintf() to buffer error" );
	}
	else
	{
		pOutputFile->Write( aBuf, nLen );
		pOutputFile->Write( p_buf, m_len );
	}
}


/*
//
//
//
BString::BString(const BString &str)
{
	m_len=str.m_len;
	p_buf=new unsigned char[m_len+1];	// +1 just so that any strings will be null-terminated for debugging
	memset(p_buf,0,m_len+1);
	memcpy(p_buf,str.p_buf,m_len);
}

BString &BString::operator=(const BString &str)
{
	if(p_buf!=NULL)
	{
		delete [] p_buf;
	}

	m_len=str.m_len;
	p_buf=new unsigned char[m_len+1];	// +1 just so that any strings will be null-terminated for debugging
	memset(p_buf,0,m_len+1);
	memcpy(p_buf,str.p_buf,m_len);

	return *this;
}
*/
// Reading

//
//
//
unsigned int BString::Len()
{
	return m_len;
}

//
//
//
unsigned char *BString::Buf()
{
	return p_buf;
}

//
//
//
/* void BString::ReadSelf(CFile *file)
{
	if(p_buf!=NULL)
	{
		delete [] p_buf;
	}

	unsigned char buf[1024];
	memset(buf,0,sizeof(buf));
	unsigned char *ptr=buf;
	
	while(file->Read(ptr,1)>0)
	{
		if(*ptr != ':')
		{
			ptr++;
		}
		else	// ':'
		{
			*ptr='\0';
			break;
		}
	}
	
	unsigned int len=0;
	sscanf((char *)buf,"%u",&len);

	m_len=len;
	p_buf=new unsigned char[len+1];	// +1 just so that any strings will be null-terminated for debugging
	memset(p_buf,0,m_len+1);

	file->Read(p_buf,m_len);
}
*/