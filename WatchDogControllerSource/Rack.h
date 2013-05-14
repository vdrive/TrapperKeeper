#pragma once

class Rack
{
public:
	Rack();
	Rack(string rack_name);
	~Rack(void);
	string m_rack_name;
	CTime m_last_ping_time;
};
