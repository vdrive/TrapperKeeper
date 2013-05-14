// TSocketSendData.h

#ifndef TSOCKET_SEND_DATA_H
#define TSOCKET_SEND_DATA_H

class TSocketSendData
{
public:
	TSocketSendData(void *data,unsigned int data_len);
	~TSocketSendData();
	unsigned int GetDataLength();
	unsigned char *GetDataBuffer();

protected:
	unsigned char *p_data;
	unsigned int m_data_len;
};

#endif // WSOCKET_SEND_DATA_H