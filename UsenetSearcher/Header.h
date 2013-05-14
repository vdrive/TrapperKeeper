#pragma once

class Header
{
public:
	Header(void);
	~Header(void);

	char m_message_id[256];
	char m_from[64];
	char m_group[512];
	char m_subject[256];
	char m_subject_no_parts[256];
	char m_post_date[64];
	char m_project[256];
	char m_nntp_posting_host[256];
	char m_nntp_posting_host_ip[32];
	char m_client_posting_ip[32];

	int m_lines;
	int m_total_parts;

	CTime m_counter_post_date;

	// Clears all the attributes of the message
	void Clear();
};
