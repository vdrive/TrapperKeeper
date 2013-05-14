#pragma once
#include "tasyncsocket.h"

class ConnectionManager;
class ListeningSocket :
	public TAsyncSocket
{
public:
	ListeningSocket(void);
	~ListeningSocket(void);

	void InitParent(ConnectionManager * parent, int socknum);

	void OnAccept(int error_code);


private:

	ConnectionManager *p_parent;
	int m_socknum;
};
