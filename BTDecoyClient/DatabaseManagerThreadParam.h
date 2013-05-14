#pragma once

class DatabaseManagerThreadParam
{
public:

	DatabaseManagerThreadParam(void)
		: m_hwnd(NULL), m_bComplete(false), m_nVer(0)
	{
	}

	~DatabaseManagerThreadParam(void)
	{
	}

public:
	HWND			m_hwnd;
	bool			m_bComplete;
	unsigned int	m_nVer;
};
