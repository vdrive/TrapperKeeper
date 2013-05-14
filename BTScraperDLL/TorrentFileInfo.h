#pragma once

#include "stdafx.h"

class CTorrentFileInfo
{
public:
	CTorrentFileInfo(void);
	CTorrentFileInfo(const CTorrentFileInfo & info);
	~CTorrentFileInfo(void);
	CTorrentFileInfo & operator=(const CTorrentFileInfo & info);

	int GetLength();
	unsigned short * GetEd2k();
	string GetMd5sum();
	string GetPath();
	string GetPathUTF8();
	unsigned short * GetSha1();

	void SetLength(int len);
	void SetMd5sum(string sum);
	void SetPath(string path);
	void SetPathUTF8(string path);
	


private:
	int m_length;
	unsigned short m_ed2k[16];
	string m_md5sum;
	string m_path;
	string m_path_utf8;
	unsigned short m_sha1[20];
};
