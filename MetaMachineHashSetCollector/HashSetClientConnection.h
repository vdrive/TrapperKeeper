#pragma once
#include "Buffer2000.h"
#include "MetaSpooferTCP.h"
#include "..\tkcom\vector.h"
#include "..\tkcom\Timer.h"

class ClientConnection : public Object
{
protected:
	UINT m_tcp_handle;
	Timer m_created;  //an instance of when this was first created
	Buffer2000 m_buffer;
	enum CLIENTSTATE {CLIENT_START=1,CLIENT_HELLO,CLIENT_HELLO_WAIT,CLIENT_FILE_REQUEST,CLIENT_FILE_REQUEST_WAIT,CLIENT_FILE_STATUS,CLIENT_FILE_STATUS_WAIT,CLIENT_HASH_PARTS,CLIENT_HASH_PARTS_WAIT,CLIENT_SEND_PARTS,CLIENT_SEND_PARTS_WAIT};
	CLIENTSTATE m_state;
	bool mb_abort;
	string m_ip;
	byte m_desired_hash[16];
	UINT m_start1;
	UINT m_start2;
	UINT m_start3;
	UINT m_end1;
	UINT m_end2;
	UINT m_end3;
public:
	ClientConnection(UINT tcp_handle,const char* ip);
	~ClientConnection(void);
	void NewData(Buffer2000& packet);
protected:
	void CheckBuffer(void);
public:
	void Update(void);
	void RespondHello(void);
	void RespondHashParts(void);
	bool IsConnection(UINT tcp_handle);
	void Shutdown(void);
	bool IsOld(void);
private:
	void ClearDesiredHash(void);
public:
	void RespondFileStatus(void);
	void RespondFileRequest(void);
	void RespondSendParts(void);
};
