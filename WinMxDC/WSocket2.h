// WSocket.h

#ifndef W_SOCKET2_H
#define W_SOCKET2_H

#include "winsock2.h"	// also add ws2_32.lib to project library includes

#include <vector>
using namespace std;

#include "WSocketSendData.h"

class WSocket2
{
public:
	WSocket2();
	~WSocket2();

	static int Startup();
	static void Cleanup();
	static int GetLastError();

	int Create(unsigned int port);
	int Connect(char *host,unsigned int port);
	int Listen();
	bool Attach(SOCKET hSocket);
	SOCKET Detach();

//	int Send(char *buf,int len);
//	int Receive(char *buf,int len);
	virtual int Close();
	bool IsSocket();

	// Public Functions for sending data, both synchronously and asynchronously
	int SendData(void *buf,unsigned int buf_len);
	void OnSend(int error_code);

	// Functions for receiving data, both synchronously and asynchronously
	int ReceiveData(unsigned int buf_len);
	int ContinueToReceiveData();

protected:
	SOCKET m_hSocket;
	int m_socket_type;

	// Variables for sending data, both synchronously and asynchronously
	bool m_sending_data;
	unsigned int m_num_sent;
	unsigned int m_send_data_buf_len;
	unsigned char *p_send_data_buf;
	vector<WSocketSendData *> v_send_data_buffers;

	// Variables for receiving data, both synchronously and asynchronously
	bool m_receiving_data;
	unsigned int m_num_read;
	unsigned int m_read_data_buf_len;
	unsigned char *p_read_data_buf;

	// Protected Functions for sending data, both synchronously and asynchronously
	virtual void DataSent(unsigned int len);

	// Protected Functions for receiving data, both synchronously and asynchronously
	virtual void DataReceived(unsigned int len);

	// Protected Functions for receiving data, both synchronously and asynchronously
	virtual int ReceiveBuffer();

	int Receive(void *buf,unsigned int buf_len);

	SOCKET Accept();

private:
	// Private Functions for sending data, both synchronously and asynchronously
	int SendNextDataBuffer();
	int SendQueuedDataBuffers();
	int Send(void *buf,unsigned int buf_len);

};

#endif // W_SOCKET_H
