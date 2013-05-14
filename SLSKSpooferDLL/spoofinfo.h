#pragma once

struct spoof
{
public:
	int project_id;
	int track_number;
	int size;
	int bitrate;
};

class spoofinfo
{
public:
	vector<spoof> Spoofs;
	spoofinfo();
	~spoofinfo();
	void getSpoofInfo(void);
};