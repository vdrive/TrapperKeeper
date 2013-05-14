#pragma once

class DCClient
{
public:
	DCClient(void);
	~DCClient(void);

	int m_sharesize;	//amount of files sharing
	char * m_nick;
	int m_speed;	//internet connection
	char * m_interests;
	char * m_email;		//email address


};
