#pragma once

class EnumWindowData
{
public:
	EnumWindowData(void);
	~EnumWindowData(void);
	char m_source[32+1];
	int m_uploads;
	HWND m_hwnd;
};
