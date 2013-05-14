#pragma once

class Sig2DatSourceList
{
public:
	Sig2DatSourceList(void);
	~Sig2DatSourceList(void);

	char m_srcRemoteFileName[256+1];
	char m_srcDownloadURL[256+1];
	DWORD m_srcFileID;
	BYTE m_srcContentHash[20];
	DWORD m_srcFileSize;
	DWORD m_srcNodeInetNum;
	DWORD m_srcNodeTcpPort;
	BYTE m_srcName[69];
	DWORD m_srcKbps;
	DWORD m_srcKbpsLastModified;
	DWORD m_srcUnknown1;
	DWORD m_srcGroup;
	DWORD m_srcRetry;
	DWORD m_srcUnknown2;
	BYTE m_srcUnknown3;
	BYTE m_srcUnknown4;
}
