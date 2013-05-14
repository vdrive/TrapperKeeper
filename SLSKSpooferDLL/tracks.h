#pragma once

struct track
{
public:
	int project_id;
	int track_number;
	CString track_name;
};

class tracks
{
public:
	vector<track> Tracks;
	tracks();
	~tracks();
	void getTracks(void);
};