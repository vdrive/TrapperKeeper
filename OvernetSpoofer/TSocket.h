// TSocket.h

#ifndef TSOCKET_H
#define TSOCKET_H

#include "WSocket.h"
#include "TSocketSendData.h"

#include <vector>
using namespace std;

class TSocket : virtual public WSocket
{
public:
	// --- Public Member Functions
	TSocket();
	~TSocket();

	virtual int Close();

	virtual void OnSend(int error_code);
	int SendSocketData(unsigned char *buf,unsigned int buf_len);

protected:
	// --- Protected Data Members
	// Data Members for sending data
	bool m_sending_socket_data;
	vector<TSocketSendData *> v_send_data_buffers;

	// Data Members for receiving data
	bool m_receiving_socket_data;	
	bool m_receiving_some_socket_data;

	// --- Protected Member Functions
	int ReceiveSocketData(unsigned int len);
	int ReceiveSomeSocketData(unsigned int max_len);
	int ReceiveSomeMoreSocketData(char *data,unsigned int len,unsigned int max_len);

	int ContinueToReceiveSocketData();

	virtual void SocketDataSent(unsigned int len);
	virtual void SocketDataReceived(char *data,unsigned int len);
	virtual void SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len);

	int Receive(void *buf,unsigned int buf_len);

private:
	// --- Private Member Functions
	int Send(void *buf,unsigned int buf_len);

	int SendNextDataBuffer();
	int SendQueuedDataBuffers();

	int ReceiveBuffer();

	void ResetSendDataMembers();
	void ResetReceiveDataMembers();

	// --- Private Data Members
	// Data Members for sending data
	unsigned int m_num_sent;
	unsigned int m_send_data_buf_len;
	char *p_send_data_buf;
	
	// Data Members for receiving data
	unsigned int m_num_read;
	unsigned int m_read_data_buf_len;
	char *p_read_data_buf;
};

#endif // TSOCKET_H