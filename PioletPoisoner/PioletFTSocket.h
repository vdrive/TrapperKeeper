#pragma once
#include "TEventSocket.h"

/* session states */
typedef enum
{
	SessNew,
	SessConnected,
	SessFilesizeSent,
	SessStartPositionReceived,
	SessMD5Sent,
	SessGoReceived,
	SessDisconnected
} FTSessionStatus;

class ConnectionSockets;
class PioletFTSocket :	public TEventSocket
{
public:
	PioletFTSocket(void);
	~PioletFTSocket(void);
	void InitParent(ConnectionSockets* parent);
	//int Connect(char* ip, int port,  UINT file_length, const char* md5);
//	int CreateSocket(char* ip, int port,  UINT file_length, const char* md5);
	bool AcceptConnection(SOCKET hSocket,char* ip, int port,  UINT file_length, const char* md5);
	//void OnConnect(int error_code);
	void OnReceive(int error_code);
	void OnClose(int error_code);
	int Close();
	void OnSend(int error_code);
	void CloseIdle();

	//void OnAccept(int error_code);

private:
	// Private Data Members
	ConnectionSockets* p_parent;
	FTSessionStatus m_status;
	CString m_remote_ip;
	int m_local_port;
	UINT m_file_length;
	string m_md5;
	UINT m_starting_at;

	CTime m_last_time_i_sent_stuff;
	unsigned int m_file_len;
	unsigned int m_file_num_sent;
	unsigned int m_rand_offset;
	unsigned char m_buf[4096];
	unsigned int m_buf_offset;

	void ResetData();
	void DataSent(unsigned int len);
	void SendFrames();


};
