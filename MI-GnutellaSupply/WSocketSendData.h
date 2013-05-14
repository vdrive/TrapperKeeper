// WSocketSendData.h
#pragma once

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