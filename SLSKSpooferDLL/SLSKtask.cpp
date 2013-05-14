

#include "stdafx.h"
#include "SLSKtask.h"

SLSKtask::SLSKtask(void)
{
	m_port = 0;
	m_state = 0;
	m_token = 0;
	m_user_name = "";
	m_album = "";
	m_artist = "";
	//m_project_id = 0;
	m_ip = "";
}

/*SLSKtask::SLSKtask(CString username, CString connection_ip, buffer Buffer, int Port)
{
	un = username;
	ip = connection_ip;
	buf = Buffer;
	port = Port;
}
*/


SLSKtask::~SLSKtask()
{
	//tracks.clear();
}

/*void SLSKtask::setUN(CString username)
{
	un = username;
}

void SLSKtask::setBuf(buffer &Buffer)
{
	buf.SetBuffer( Buffer.GetCharPtr(), Buffer.Size() );
}
*/
void SLSKtask::setIP(CString connection_ip)
{
	m_ip = connection_ip;
}

CString SLSKtask::getIP(void)
{
	return m_ip;
}

/*CString SLSKtask::getUN(void)
{
	return un;
}

buffer SLSKtask::getBuf(void)
{
	return buf;
}


void SLSKtask::setPort(int p)
{
	port = p;
}

int SLSKtask::getPort(void)
{
	return port;
}
*/