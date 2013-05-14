#include "StdAfx.h"
#include "header.h"

Header::Header(void)
{
	Clear();
}

Header::~Header(void)
{
}

void Header::Clear()
{
	memset(&m_message_id, 0, sizeof(m_message_id));
	memset(m_from, 0, sizeof(m_from));
	memset(m_group, 0, sizeof(m_group));
	memset(m_subject, 0, sizeof(m_subject));
	memset(m_subject_no_parts, 0, sizeof(m_subject_no_parts));
	memset(m_post_date, 0, sizeof(m_post_date));
	memset(m_project, 0, sizeof(m_project));
	memset(m_nntp_posting_host, 0, sizeof(m_nntp_posting_host));
	memset(m_nntp_posting_host_ip, 0, sizeof(m_nntp_posting_host_ip));
	memset(m_client_posting_ip, 0, sizeof(m_client_posting_ip));

	m_lines = 0;
	m_total_parts = 0;
	m_counter_post_date = NULL;
}