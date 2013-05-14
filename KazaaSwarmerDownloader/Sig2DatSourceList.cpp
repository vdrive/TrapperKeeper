#include "StdAfx.h"
#include "sig2datsourcelist.h"

Sig2DatSourceList::Sig2DatSourceList(void)
{

	memset(m_srcRemoteFileName,0,256+1);
	m_srcDownloadURL = '\0';
	m_srcFileID= 0x00000000;
	m_srcNodeInetNum = 0xE0000000;
	m_srcNodeTcpPort = 0x00000000;
	memset(m_srcName,0,69);
	m_srcKbps = 0x00000000;
	m_srcKbpsLastModified= 0x00000000;
	m_srcUnknown0 = 0x0000001;
	m_srcUnknown1= 0x00000000;
	m_srcGroup= 0x00000000;
	m_srcRetry= 0x00000000;
	m_srcUnknown2= 0x00000000;
	m_srcUnknown3 = 0;
	m_srcUnknown4 = 1;
}

Sig2DatSourceList::~Sig2DatSourceList(void)
{
}

int Sig2DatSourceList::GetSize()
{
	int size = (sizeof(DWORD) * 10) 
		+ (sizeof(BYTE) * 91) 
		+ strlen(m_srcRemoteFileName) 
		+ 1; // strlen(m_srcDownloadURL)

		return size;


}
