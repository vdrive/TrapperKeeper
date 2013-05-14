// DownloadingSocket.cpp

#include "stdafx.h"
#include "DownloadingSocket.h"
#include "SHA1.h"
#include "GnutellaFileDownloaderDlg.h"

//
//
//

DownloadingSocket::DownloadingSocket()
{
	char* pszFileName = "c:\\leo_temp.dat";
	if ( !dl_filename.Open( pszFileName, CFile::modeCreate | CFile::modeReadWrite, &fileException ) )
	{
		TRACE( "Can't open file %s, error = %u\n", pszFileName, fileException.m_cause );
	}
}

DownloadingSocket::~DownloadingSocket()
{
	vector<GnutellaHTTPHeaders*>::iterator itVectorData;

	if(dl_filename.m_hFile!=CFile::hFileNull)
	{
		dl_filename.Close();
	}

	for (int j = 0; j < (int)headers.size(); j++)
	{
		delete headers[j];
	}

	for(itVectorData = headers.begin(); itVectorData != headers.end(); itVectorData++)
	{
		//delete itVectorData;
	}
	headers.clear();
	Close();
}

void DownloadingSocket::InitParent(GnutellaFileDownloaderDlg *dlg)
{
	d_dlg=dlg;
	
	//dl_port=6436;
	//dl_ipaddress="216.151.150.196";
	//dl_hash="TXMT67FOD65W2MNMIFERCOSFP6L7OZG7";
	//dl_filesize=68929;
	start_range = 0;
	end_range = 16384;			//16 kb inital asking chunk
}

void DownloadingSocket::Begin()
{	
	if(Create()==SOCKET_ERROR)
	{
		DWORD Errer=::GetLastError();
		char msg[1024];
		sprintf(msg, "Error creating socket port");
		AfxMessageBox(msg);
		return;
	}

	char msg[1024];
	sprintf(msg, "Created socket on port: %u", dl_port);
	d_dlg->Status(msg);

	//AfxMessageBox(dl_ipaddress.GetBuffer());
	//AfxMessageBox(itoa(dl_port, msg, 10));
	
	//Connect(dl_ipaddress.GetBuffer(), dl_port);
	Connect("216.151.150.196", 6346);
	//Connect(dl_ipaddress.GetBuffer(), (unsigned short)dl_port);
}

CString DownloadingSocket::GenerateGnutellaGETCommand(unsigned int start_range, unsigned int end_range)
{
	char* msg = new char[255];	//used for itoa function (converting an int to string)
	char* start = new char[255];	//used for itoa function (converting an int to string)
	char* end = new char[255];	//used for itoa function (converting an int to string)

	CString temp = "GET /uri-res/N2R?urn:sha1:" + dl_hash + " HTTP/1.1\r\n" \
	+ "User-Agent: Phex 2.6.4.89\r\n" + "Host:" + dl_ipaddress + ":" + itoa(dl_port, msg, 10) + "\r\n" \
	+ "Range: bytes=" + itoa(start_range, start, 10) + "-" + itoa(end_range, end, 10) + "\r\n" \
	+ "X-Queue: 0.1\r\n" + "Connection: Keep-Alive\r\n\r\n";

	delete[] msg;
	delete[] start;
	delete[] end;

	return temp;
}

void DownloadingSocket::SetConnectionInfo(CString ipaddress, unsigned int port, CString hash, unsigned int filesize)
{
	dl_port=port;
	dl_ipaddress=ipaddress;
	dl_hash=hash;
	dl_filesize=filesize;
}

void DownloadingSocket::OnSend(int error_code)
{

}

void DownloadingSocket::OnConnect(int error_code)
{
	CString buffer;
	char* msg = new char[1024];
	if (end_range <= dl_filesize)
	{
		buffer = GenerateGnutellaGETCommand(start_range, end_range);
	}
	else
	{
		end_range = dl_filesize;		//end the loop
		buffer = GenerateGnutellaGETCommand(start_range, (dl_filesize-1) );
	}
	byte_size_to_receive = end_range - start_range + 1;
	strcpy(msg, buffer);
	d_dlg->Status(msg);
	SendSocketData((unsigned char*)msg, buffer.GetLength());
	delete[] msg;
}

bool DownloadingSocket::OnReceive(int error_code)
{
	if(error_code!=0)
	{
		Close();
		return false;
	}

	if(m_receiving_socket_data || m_receiving_some_socket_data)
	{
		ContinueToReceiveSocketData();
		return false;
	}

	// ---

	// Your ReceiveSocketData() or ReceiveSomeSocketData() code goes here
	
	ReceiveSomeSocketData(4096);		//attempt to receive the 4 kilobyte header file

	// ---

	// Check for more data
	char buf[1];
	memset(buf,0,sizeof(buf));
	WSABUF wsabuf;
	wsabuf.buf=buf;
	wsabuf.len=1;

	DWORD num_read=0;
	DWORD flags=MSG_PEEK;
	int ret=WSARecv(m_hSocket,&wsabuf,1,&num_read,&flags,NULL,NULL);

	if(ret!=SOCKET_ERROR)
	{
		if(num_read!=0)
		{
			return true;	// continute to call OnReceive()
		}
	}

	return false;
}

void DownloadingSocket::OnClose(int error_code)
{
	// Do any additional cleanup here
	TAsyncSocket::OnClose(error_code);
}

int DownloadingSocket::parseHTTPHeader(char *data, int unsigned len)
{
	CString param_buf = "";		//stores a single parameter
	CString value_buf = "";		//stores a single value for a given parameter

	GnutellaHTTPHeaders* head;	//keep track of our params and headers

	char *buf = new char[512];

	unsigned int i = 0;
	int last_colon = 0;
	int last_rn = 0;
	bool cSplit = false;	//represents if the first instance of ':' was found in the string so we can ignore the rest
	bool rn = false;		//represents \r\n

	while(i <= len)			//while we are not at the end of the header represented by \r\n\r\n
	{
		rn = false;										//reset this value so you do not keep entering the rn cond below
		if(*(data + i) == '\r')							//if we have a return carriage
		{
			if(*(data + i + 1) == '\n')					//check to see if we have a newline following it
			{
				rn = true;								//yes we do so set rn newline to true
				cSplit = false;							//we have encountered a newline, so reset the ':' instance
				last_rn = i + 2;						//keep track of where the last rn was for memcpy below
			}
		}

		if(*(data + i) == ':')
		{
			if(!cSplit)									//if there is is not a previous instance of ':'
			{
				last_colon = i + 1;						//keep track of where the last ':' was located

				memcpy(buf, (data + last_rn), (i - last_rn));
				*(buf + i - last_rn) = '\0';			//force a nul terminator

				param_buf = buf;			
			}
			cSplit = true;								//we have now found the first instance of ':' in our headers.
		}

		if(rn || i == len)								//if we encountered a \r\n that means it is a start of a new parameter
		{
			memcpy(buf, (data + last_colon), (i-last_colon));
			*(buf + i-last_colon) = '\0';				//force a null terminator at the end of the copied buffer

			value_buf = buf;
			head = new GnutellaHTTPHeaders();
			head->SetValues(param_buf, value_buf);		//set out header values
			headers.push_back(head);					//add it to our stored list
			param_buf.Empty();
			value_buf.Empty();							//clean up our buffers to prepare for next strings
		}

		i++;											//increment the i to get next character
	}
	delete[] buf;
	return 0;
}

void DownloadingSocket::SocketDataReceived(char *data,unsigned int len)
{
	CString buffer;
	char* msg = new char[1024];

	//1.  Write the data you received
	appendFile(data, len);	

	//2.  Prepare the next chunk of data to receive
	start_range = end_range;
	start_range++;
	end_range = end_range + (16 * 1024 * 15);
	byte_size_to_receive = end_range - start_range + 1;

	//3.  Make certain you are not asking for more data than the filesize
	if (end_range <= dl_filesize )
	{
		buffer = GenerateGnutellaGETCommand(start_range, end_range);
	}
	else
	{
		end_range = dl_filesize;		//end the loop
		buffer = GenerateGnutellaGETCommand(start_range, dl_filesize);
		byte_size_to_receive = dl_filesize - start_range;
	}

	//5.  Resend the next chunck of data
	if(start_range < dl_filesize)
	{
		strcpy(msg, buffer);
		d_dlg->Status(msg);
		SendSocketData((unsigned char*)msg, buffer.GetLength());
	}
	else
	{
		d_dlg->Status("Download complete");
		dl_filename.Close();
		checkSHA();	
	}
	delete[] msg;
}

bool DownloadingSocket::checkSHA()
{
	SHA1 verify;
	char *sha = new char[20];
	ZeroMemory( sha, 20 );
	if(verify.HashFile("c:\\leo_temp.dat"))
	{
		verify.Final();
		verify.Base32Encode(sha);
		if(strcmp(dl_hash, sha)==NULL)
		{
			d_dlg->Status("Downloaded File Verified - SHA1 match");
			return true;
		}
		else
		{
			d_dlg->Status("Warning: Downloaded file does not match SHA1");
			return false;
		}
	}
	delete[] sha;
	return false;
}

void DownloadingSocket::appendFile(char *data, unsigned int len)
{
	if(dl_filename.m_hFile!=CFile::hFileNull)
	{
		dl_filename.Write(data, len);
	}
}

void DownloadingSocket::SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len)
{
	// Some data has been received
	char *pTerm = strstr( data, "\r\n\r\n" );	//  HTTP header check

	if( pTerm == NULL )
	{
		ReceiveSomeMoreSocketData(data, data_len, new_len, max_len);
		return;
	}

	unsigned int nHeaderLen = (unsigned int)(pTerm - data);
	const char *pDataStart = pTerm + 4;
	unsigned int nDataLen = data_len - ( nHeaderLen + 4 );

	char *pHeaderBuf = new char[nHeaderLen];
	memcpy( pHeaderBuf, data, nHeaderLen );

	parseHTTPHeader(pHeaderBuf, nHeaderLen);

	char *pDataBuf = new char[nDataLen];
	memcpy( pDataBuf, pDataStart, nDataLen );

	if(nDataLen >= dl_filesize)
	{
		//then we have already received enough information therefore: output to file
		end_range = dl_filesize;
		appendFile(pDataBuf, nDataLen);
	}
	else
	{
		//else we have NOT received enough information so receive more data
		appendFile(pDataBuf, nDataLen);
		ReceiveSocketData(byte_size_to_receive - nDataLen);
	}
	delete[] pHeaderBuf;
	delete[] pDataBuf;
}