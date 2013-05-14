#include "StdAfx.h"
#include ".\inflationsocket.h"
#include "inflationsocketmanager.h"

InflationSocket::InflationSocket(void)
{

	m_creation_time = CTime::GetCurrentTime();
}

InflationSocket::InflationSocket(string announce_url,string info_hash, string peer_id, int port, bool is_seed)
{

	m_info_hash = info_hash;
	m_announce_url = announce_url;
	m_peer_id = peer_id;
	m_port = port;
	m_is_seed = is_seed;
	m_is_first_connect = false;

	testleak = "the socket leaks";

	m_creation_time = CTime::GetCurrentTime();

}


InflationSocket::~InflationSocket(void)
{
	

}

void InflationSocket::SetFirstConnect()
{

	m_is_first_connect = true;
}

void InflationSocket::InitParent(InflationSocketManager * parent)
{

	p_parent = parent;

}

void InflationSocket::OnConnect(int error_code)
{

	if(error_code!=0)
	{
		//p_parent->TorrentFailure(m_info_hash,m_announce_url,m_peer_id,this);
		p_parent->TorrentFailure(this);
		return;
	}
/*
	char traceme[256];
	sprintf(traceme,"connected to %s \r\n",m_peer_id.c_str());
	TRACE0(traceme);
*/
	char* message = GetPeerMessage();
	int size = (int)strlen(message);
	SendSocketData((unsigned char *)message,size);
	delete [] message;

}

int InflationSocket::Connect(char * host, unsigned short port)
{
	return TAsyncSocket::Connect(host,port);
}

void InflationSocket::OnReceive(int error_code)
{

	ReceiveSomeSocketData(4096);


}

void InflationSocket::SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len)
{

	char * buf = new char[data_len+1];
	memcpy(buf,data,data_len);
	buf[data_len] = '\0';
	char * ptr = data;
	char returncode[4];
	int returnint = 0;
	ptr += 9;   // Skip the HTTP/1.? 
	memcpy(returncode,ptr,3);
	returncode[3] = '\0';
	returnint = atoi(returncode);

 	if (returnint == 200)
	{
		//p_parent->TorrentSuccess(m_info_hash, m_announce_url,m_peer_id,this);
		p_parent->TorrentSuccess(this);
	}
	else
	{
		//p_parent->TorrentFailure(m_info_hash, m_announce_url,m_peer_id,this);
		p_parent->TorrentFailure(this);
	}

	delete [] buf;

}

void InflationSocket::OnClose(int error_code)
{
	TAsyncSocket::OnClose(error_code);

}

string InflationSocket::ReturnPeerId()
{
	return m_peer_id;

}



void InflationSocket::InitConnection(string announce_url, string info_hash, string peer_id, int port, bool is_seed, string key_id)
{

	m_info_hash = info_hash;
	m_announce_url = announce_url;
	m_peer_id = peer_id;
	m_port = port;
	m_is_seed = is_seed;
	m_key_id = key_id;
	m_is_first_connect = false;

}

char * InflationSocket::GetKeyID() 
{
	int key_len = 8;
	char * id = new char [key_len+1];
	strcpy(id, "");
	for(int i = 0; i < key_len; i++) {
		int pos = (int) ( rand() % 62);
		id[i] = chars[pos];
	}
	id[key_len] = 0;
	return id;
}

//char * InflationSocket::GetPeerMessage(string announce_url, string info_hash, string peer_id, int port, bool is_seed)
char * InflationSocket::GetPeerMessage()
{

	char url[256+1] = "";
	char temp[2];
	string temphash = m_info_hash;
	char * trackerURL = new char[2048];

	strcpy(temp, " ");
	for(int i = 0; i < (int)strlen(m_announce_url.c_str()); i++) 
	{	
		temp[0] = m_announce_url[i];
		if(temp[0] != ' ') 
		{
			strcat(url, temp);
		}
	}

	int urlsize = (int)strlen(url);
	char * tempurl = new char[urlsize+1];
	strcpy(tempurl,url);
	//char * host = new char[urlsize+1];
	//char * newurl = new char[urlsize+1];
	char host[256+1];
	char newurl[256+1];

	int port = 80;

	ParseHttpString(tempurl,host,newurl,&port);

/*	
	char *ptr, *ptrbegin;
	ptr = strstr(tempurl,"http://");
	if (ptr != NULL)
	{
		ptr +=7;
		ptrbegin = ptr;
		ptr = strchr(ptr,'/');
	}
	else // there is no http so begin at the beginning
	{
		ptr = tempurl;
		ptrbegin = ptr;
		ptr = strchr(host,'/');
	}
	*/

	// HTTP Protocol, get anything after the host info
	strcpy(trackerURL, "GET ");
	strcat(trackerURL,newurl);
//	delete [] newurl;
//	strcat(trackerURL,ptr);



	// Generate the request message
	char cport[5+1];
	//char * hash = new char[80+1];
	_itoa(m_port,cport,10);
	char * hash = ConvertToHash(temphash);
	strcat(trackerURL, "?info_hash=");
	strcat(trackerURL, hash);
	delete [] hash;
	strcat(trackerURL, "&peer_id=");
	strcat(trackerURL, m_peer_id.c_str());
	strcat(trackerURL, "&port=");
	strcat(trackerURL, cport);
	if (m_is_seed == FALSE)
	{
		int tempint;
		char intchar[12+1];
		strcat(trackerURL, "&uploaded=");
		tempint = rand()%20000000;  // up to 20 megs uploaded
		_itoa(tempint,intchar,10);
		strcat(trackerURL, intchar);
		strcat(trackerURL, "&downloaded=0&left=");
		tempint = rand()%20000000;  // and up to 20 megs left
		_itoa(tempint,intchar,10);
		strcat(trackerURL, intchar);
		//strcat(trackerURL,"&left=0&event=completed");
		if(m_is_first_connect)
		{
			strcat(trackerURL,"&key=");
			strcat(trackerURL,m_key_id.c_str());
			strcat(trackerURL, "&compact=1&event=started");
		}
		else
		{
			strcat(trackerURL,"&key=");
			strcat(trackerURL,m_key_id.c_str());
			strcat(trackerURL, "&compact=1");
		}
	}
	else
	{
		
		if(m_is_first_connect)
		{
			//strcat(trackerURL,"&uploaded=0&downloaded=0&left=14840&event=started&numwant=160&compact=1");
			strcat(trackerURL,"&uploaded=0&downloaded=0&left=0&event=completed&numwant=160&compact=1");
			strcat(trackerURL,"&key=");
			strcat(trackerURL,m_key_id.c_str());
		}
		
		else
		
		{
			strcat(trackerURL,"&uploaded=0&downloaded=0&left=0&event=completed&numwant=160&compact=1");
			//strcat(trackerURL,"&uploaded=0&downloaded=14840&left=0&event=completed&numwant=160&compact=1");
			strcat(trackerURL,"&key=");
			strcat(trackerURL,m_key_id.c_str());
		}
	}

	

	// get message finished
	// HTTP Protocol
	strcat(trackerURL," HTTP/1.1\r\nUser-Agent: Azureus 2.2.0.2\r\nConnection: close\r\nAccept-Encoding: gzip\r\nHost: ");
	strcat(trackerURL,host);
//	delete [] host;
	strcat(trackerURL,"\r\nAccept: text/html, image/gif, image/jpeg, *; q=.2, */*; q=.2\r\nContent-type: application/x-www-form-urlencoded\r\n");
	strcat(trackerURL,"\r\n\0");

//	delete [] key_id;
	//delete [] hash;
	delete [] tempurl;  
	//delete [] newurl;
	//delete [] host;

	return trackerURL;
}

char * InflationSocket::ConvertToHash(string hash)
{

	string returnstring;

	for (int i = 0; i < (int)hash.length(); i ++)
	{
		if ( i%2 == 0)
		{
			returnstring += '%';
		}
		returnstring += hash[i];
	}
	char * retchar = new char[returnstring.size()+1];
	strcpy(retchar,returnstring.c_str());
	return retchar;
}


void InflationSocket::ParseHttpString(char * input, char * host, char * url, int * port)
{

	int tempport;
	char *ptr, *ptrbegin, *portptr;
	//skip the http
	ptr = strstr(input,"http://");
	if (ptr != NULL)
	{
		ptr +=7;
	}
	else
	{
		ptr = strstr(input,"udp://");
		if (ptr != NULL)
		{
			ptr +=6;
		}
	}

	if (ptr != NULL)
	{
		ptrbegin = ptr;
	}
	else
	{
		ptrbegin = input;
	}


	ptr = strchr(ptrbegin,':');
	if (ptr != NULL)
	{
		portptr = strchr(ptr,'/');
		*portptr = '\0';
		tempport = atoi(ptr+1);
		*portptr = '/';
		// Get the host as well to be used later
		*ptr = '\0';  // Break String
		strcpy(host,ptrbegin);
		*ptr = ':';
		ptr = portptr;
	}
	else
	{
		ptr = strchr(ptrbegin,'/');
		*ptr = '\0';  // Break String
		strcpy(host,ptrbegin);
		*ptr = '/';
		tempport = 80;
	}

	strcpy(url,ptr);
	*port = tempport;

}

bool InflationSocket::IsTimedout()
{

	CTimeSpan diff = CTime::GetCurrentTime()-m_creation_time;
	int change = (int)diff.GetTotalMinutes();
	if (change >= 10)
	{
		return true;
	}
	return false;

}


