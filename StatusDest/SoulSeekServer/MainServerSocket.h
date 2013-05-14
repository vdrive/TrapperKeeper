#pragma once
#include "tasyncsocket.h"
#include "SearchRequest.h"
#include <vector>
using namespace std;

class SoulSeekServerDll;

class MainServerSocket : public TAsyncSocket
{
public:
	MainServerSocket(void);
	~MainServerSocket(void);

	void InitParent(SoulSeekServerDll *parent);
	void OnConnect(int error_code);
	bool OnReceive(int error_code);
	void OnClose(int error_code);
	int Close();

	void RequestMoreParents();

	void SocketDataReceived(char *data,unsigned int len);
	void SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len);

	void ConnectToCachedParents(void);

	void GetClientAddress(SearchRequest* sr);

private:
	SoulSeekServerDll *p_parent;
	bool m_bGotHeader;
	vector<SearchRequest *> lookupUsers;
};
