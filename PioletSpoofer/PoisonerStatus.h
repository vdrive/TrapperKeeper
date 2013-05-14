#pragma once

class PoisonerStatus
{
public:
	PoisonerStatus(void);
	~PoisonerStatus(void);
	bool operator == (const PoisonerStatus& dc_status) const;
	bool operator == (UINT ip) const;

	CTime m_last_init_response_time;
	UINT m_ip;
	char m_ip_str[16];
};
