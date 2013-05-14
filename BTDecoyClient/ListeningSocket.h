#pragma once
#include "TEventSocket.h"

class ListeningSocket :
	virtual public TEventSocket
{
// enum
public:
	enum ListenerStatus { OPEN, CLOSED };

// implementation
public:
	ListeningSocket(HWND hwnd);
	~ListeningSocket(void);

	void OnAccept(int errorcode);

	HANDLE Open(void);
	int Close(void);

private:
	inline void SetStatusOpen(void) { m_eStatus = ListenerStatus::OPEN; }
	inline void SetStatusClosed(void) { m_eStatus = ListenerStatus::CLOSED; }

private:
	HWND			m_hwnd;
	ListenerStatus	m_eStatus;
};
