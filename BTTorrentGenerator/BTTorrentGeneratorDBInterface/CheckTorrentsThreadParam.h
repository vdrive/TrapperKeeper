#pragma once
#include <vector>
using namespace std;

class CheckTorrentsThreadParam
{
public:

	CheckTorrentsThreadParam(void)
	{
	}

	~CheckTorrentsThreadParam(void)
	{
	}

public:
	HWND			m_hwnd;
	CString			m_path;
	vector<CString> m_vTorrentFiles;
};
