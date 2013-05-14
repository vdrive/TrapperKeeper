// WSocketSendData.h

#ifndef W_SOCKET_SEND_DATA_H
#define W_SOCKET_SEND_DATA_H

class WSocketSendData
{
public:
	WSocketSendData(void *data,unsigned int data_len);
	~WSocketSendData();
	unsigned int GetDataLength();
	unsigned char *GetDataBuffer();

private:
	unsigned char *p_data;
	unsigned int m_data_len;
};

#endif // W_SOCKET_SEND_DATA_H