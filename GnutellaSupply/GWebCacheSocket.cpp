// GWebCacheSocket.cpp

#include "stdafx.h"
#include "GWebCacheSocket.h"
#include "GWebCache.h"
#include "URLComponents.h"

//
//
//
GWebCacheSocket::GWebCacheSocket()
{
	m_receiving_content=false;
}

//
//
//
GWebCacheSocket::~GWebCacheSocket()
{
}

//
//
//
void GWebCacheSocket::InitParent(GWebCache *parent)
{
	p_parent=parent;
}

//
//
//
void GWebCacheSocket::SocketDataSent(unsigned int len)
{
}

//
//
//
void GWebCacheSocket::SocketDataReceived(char *data,unsigned int len)
{
}

//
//
//
void GWebCacheSocket::SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len)
{
	int i=3;

	char *ptr=strstr(data,"\r\n\r\n");	// look for end of header

	// If we haven't received the header, receive more data
	if(ptr==NULL)
	{
		// Check to see if we're read in the 4K...if so, then close the socket...this is too big of a buffer
		if(data_len==4096)
		{
			Close();	
		}
		else	// else we have not filled the buffer yet, so receive some more data
		{
			// If there was data waiting there, try to receive some more, else wait for next OnReceive
			if(new_len>0)
			{
				ReceiveSomeMoreSocketData(data,data_len,max_len);
			}
			else
			{
				m_receiving_some_socket_data=true;
			}
		}
	}
	else	// else we have the complete header
	{
		// Check to see that the response is 200 OK
		if(strstr(data,"200 OK")==NULL && !m_receiving_content)
		{
			p_parent->ReportBadURL(m_url.c_str());
			Close();
		}
		else
		{
			// Look past the header, to get the data
			if(!m_receiving_content)
				ptr+=strlen("\r\n\r\n");
			else
				ptr=data;

			// Check to see if this was a hostfile or urlfile request
			if(strstr(m_url.c_str(),"?hostfile=1")!=NULL)
			{
				// Extract all of the hosts
				vector<string> hosts;
				char buf[4096+1];
				bool content_received=false;			
				while(*ptr!=NULL)
				{
					content_received=true;
					// Look for an ascii number
					while((*ptr!=NULL) && ((*ptr < '0') || (*ptr > '9')))
					{
						ptr++;
					}

					// Check for end of data
					if(*ptr=='\0')
					{
						break;
					}

					strncpy(buf,ptr,4096);

					// Clear trailing whitespace
					if(strchr(buf,'\n')!=NULL)
					{
						*strchr(buf,'\n')='\0';
					}
					if(strchr(buf,'\r')!=NULL)
					{
						*strchr(buf,'\r')='\0';
					}
					if(strchr(buf,' ')!=NULL)
					{
						*strchr(buf,' ')='\0';
					}

					CString cstring=buf;
					cstring.TrimLeft();
					cstring.TrimRight();
					int index=cstring.Find(":");
					if(index > 0)
					{
						string str=cstring;
						hosts.push_back(str);
					}

					ptr+=strlen(buf);
				}
				if(!content_received)
				{
					m_receiving_content=true;
					ReceiveSomeSocketData(4096);
				}
				else
				{
					Close();
					// Now we have a vector of ips
					p_parent->ReportHosts(hosts);
				}
			}
			else if(strstr(m_url.c_str(),"?urlfile=1")!=NULL)
			{
				// Extract all of the urls
				vector<string> urls;
				char buf[4096+1];
				bool content_received=false;

				while(*ptr!='\0')
				{
					content_received=true;
					// Look for http://
					while((*ptr!=NULL) && (strnicmp(ptr,"http://",strlen("http://"))!=0))
					{
						ptr++;
					}
/*
					// Look for an ascii 'h' or 'H'
					while((*ptr!=NULL) && (*ptr!='h') && (*ptr!='H'))
					{
						ptr++;
					}
*/
					// Check for end of data
					if(*ptr=='\0')
					{
						break;
					}
					
					strncpy(buf,ptr,4096);

					// Clear trailing whitespace
					if(strchr(buf,'\n')!=NULL)
					{
						*strchr(buf,'\n')='\0';
					}
					if(strchr(buf,'\r')!=NULL)
					{
						*strchr(buf,'\r')='\0';
					}
					if(strchr(buf,' ')!=NULL)
					{
						*strchr(buf,' ')='\0';
					}

					CString cstring=buf;
					cstring.TrimLeft();
					cstring.TrimRight();
					string str=cstring;
					if(cstring.Find("<")==-1 && cstring.Find("|")==-1 && cstring.Find("\"")==-1 && cstring.Find(",")==-1
						&& cstring.Find(",")==-1 && cstring.Find("%")==-1 && cstring.Find("?")==-1)
					{
						int index = cstring.ReverseFind('.');
						if(index != -1 && index != cstring.GetLength()-1)
							urls.push_back(str);
					}

					ptr+=strlen(buf);
				}

				if(!content_received)
				{
					m_receiving_content=true;
					ReceiveSomeSocketData(4096);
				}
				else
				{
					Close();
					// Now we have a vector of urls
					p_parent->ReportURLs(urls);
				}
			}
			else	// who knows what this was
			{
				Close();
			}
		}
	}
}

//
//
//
void GWebCacheSocket::OnConnect(int error_code)
{
	if(error_code!=0)
	{
		Close();
		p_parent->ReportBadURL(m_url.c_str());
		return;
	}

	char buf[8192];

	// Extract the pertinent info from this url
	DWORD flags=ICU_DECODE|ICU_ESCAPE;
	URLComponents uc;
	InternetCrackUrl(m_url.c_str(),0,flags,(URL_COMPONENTS *)&uc);

	strcpy(buf,"GET ");
	strcat(buf,uc.m_data.lpszUrlPath);
	strcat(buf,uc.m_data.lpszExtraInfo);
	strcat(buf," HTTP/1.1\r\n");

	strcat(buf,"Accept: */*\r\n");
	strcat(buf,"Accept-Language: en-us\r\n");
	strcat(buf,"---------------: ----- -------\r\n");
	strcat(buf,"User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; .NET CLR 1.1.4322; .NET CLR 1.0.3705)\r\n");
	strcat(buf,"Host: ");
	strcat(buf,uc.m_data.lpszHostName);
	strcat(buf,"\r\n");
	strcat(buf,"Connection: Keep-Alive\r\n");
	strcat(buf,"\r\n");

	SendSocketData(buf,(UINT)strlen(buf));
}

//
//
//
void GWebCacheSocket::OnReceive(int error_code)
{
	if(error_code!=0)
	{
		Close();
		return;
	}

	ReceiveSomeSocketData(4096);
}

//
//
//
void GWebCacheSocket::GetURLFile(char *url)
{
	m_url=url;
	m_url+="?urlfile=1";

	// Extract the pertinent info from this url
	DWORD flags=ICU_DECODE|ICU_ESCAPE;
	URLComponents uc;
	BOOL ret = InternetCrackUrl(m_url.c_str(),0,flags,(URL_COMPONENTS *)&uc);
	
	Create();
	Connect(uc.m_data.lpszHostName,uc.m_data.nPort);
}

//
//
//
void GWebCacheSocket::GetHostFile(char *url)
{
	m_url=url;
	m_url+="?hostfile=1";

	// Extract the pertinent info from this url
	DWORD flags=ICU_DECODE|ICU_ESCAPE;
	URLComponents uc;
	BOOL ret = InternetCrackUrl(m_url.c_str(),0,flags,(URL_COMPONENTS *)&uc);
	
	Create();
	Connect(uc.m_data.lpszHostName,uc.m_data.nPort);
}

//
//
//
int GWebCacheSocket::Close()
{
	m_receiving_content=false;
	return WAsyncSocket::Close();
}