// WSocket.h
#pragma once

#include "winsock2.h"	// also add ws2_32.lib to project library includes

#include <vector>
using namespace std;

#include "WSocketSendData.h"

class WSocket
{
public:
	WSocket();
	~WSocket();

	static int Startup();
	static void Cleanup();
	static int GetLastError();

	bool IsSocket();

	virtual int Create(unsigned int port);
	virtual int Close();

	int Connect(char *host,unsigned int port);

	void OnSend(int error_code);

	int SendSocketData(void *buf,unsigned int buf_len);
	int Listen();
	bool Attach(SOCKET hSocket);
	SOCKET Detach();

protected:
	//
	// Protected Data Members
	//
	
	SOCKET m_hSocket;
	int m_socket_type;	// SOCK_STREAM or SOCK_DGRAM

	// Data Members for sending data
	bool m_sending_socket_data;
	vector<WSocketSendData *> v_send_data_buffers;
	
	// Data Members for receiving data
	bool m_receiving_socket_data;	
	bool m_receiving_some_socket_data;

	//
	// Protected Member Functions
	//

//	int SendSocketData(void *buf,unsigned int buf_len);
	SOCKET Accept();
	int ReceiveSocketData(unsigned int len);
	int ReceiveSomeSocketData(unsigned int max_len);
	int ReceiveSomeMoreSocketData(char *data,unsigned int len,unsigned int max_len);

	int ContinueToReceiveSocketData();
	int Receive(void *buf,unsigned int buf_len);

	virtual void SocketDataSent(unsigned int len);
	virtual void SocketDataReceived(char *data,unsigned int len);
	virtual void SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len);

private:
	//
	// Private Member Functions
	//
	int Send(void *buf,unsigned int buf_len);

	int SendNextDataBuffer();
	int SendQueuedDataBuffers();

	int ReceiveBuffer();

	void ResetSendDataMembers();
	void ResetReceiveDataMembers();

	//
	// Private Data Members
	//

	// Data Members for sending data
	unsigned int m_num_sent;
	unsigned int m_send_data_buf_len;
	char *p_send_data_buf;
	
	// Data Members for receiving data
	unsigned int m_num_read;
	unsigned int m_read_data_buf_len;
	char *p_read_data_buf;
};