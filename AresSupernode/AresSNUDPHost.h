#pragma once
#include "..\AresProtector\AresHost.h"
#include "..\tkcom\vector.h"


class AresSNUDPHost : public AresHost
{
public:
	AresSNUDPHost(unsigned int ip,unsigned short port);

	AresSNUDPHost(const char* ip,unsigned short port);

	CTime m_last_response;
	CTime m_create_time;
	CTime m_last_search;
	CTime m_last_1e_ping;
	CTime m_last_1e_receive;
	bool mb_accepted;
	//unsigned short m_tmp_word;
	bool mb_alive;
	bool mb_saveable;
	Vector mv_processor_job;
	~AresSNUDPHost(void);
};
