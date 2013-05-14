// DownloadingSocket.h

#ifndef DOWNLOADING_SOCKET_H
#define DOWNLOADING_SOCKET_H

#include "TAsyncSocket.h"
#include "GnutellaHTTPHeaders.h"

class Downloader;
class GnutellaFileDownloaderDlg;

class DownloadingSocket : public TAsyncSocket
{
public:
	DownloadingSocket();
	~DownloadingSocket();
	void InitParent(GnutellaFileDownloaderDlg *dlg);
	void OnConnect(int error_code);
	void OnSend(int error_code);
	bool OnReceive(int error_code);
	void OnClose(int error_code);

	CString GenerateGnutellaGETCommand(unsigned int start_range, unsigned int end_range);
	void SetConnectionInfo(CString ipaddress, unsigned int port, CString hash, unsigned int filesize);

	void SocketDataReceived(char *data,unsigned int len);
	void SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len);

	int parseHTTPHeader(char *data, int unsigned len);	//function to retrieve HTTP GET request
	void appendFile(char *data, unsigned int len);		//function to write out to a file
	bool checkSHA();									//function to verify SHA1 value of a file
	void Begin();										//begins the downloader

private:
	GnutellaFileDownloaderDlg *d_dlg;
	
	vector<GnutellaHTTPHeaders*> headers;					//store all the parameters and headers
	CFile dl_filename;
	CFileException fileException;
	unsigned int start_range;
	unsigned int end_range;
	unsigned int byte_size_to_receive;

	char* excess_data_buffer;
	char* header_info;

	unsigned int dl_port;
	unsigned int dl_filesize;
	CString dl_ipaddress;
	CString dl_hash;
	unsigned char *m_pBuf;
};

#endif // DOWNLOADING_SOCKET_H