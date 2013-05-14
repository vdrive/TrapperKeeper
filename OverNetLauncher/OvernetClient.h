#pragma once

class OvernetClient
{
public:
	OvernetClient(void);
	~OvernetClient(void);

	int m_id;
	HINSTANCE m_hInst;
	HWND m_hWnd;
};
