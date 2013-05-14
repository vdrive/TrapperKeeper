#pragma once

class MP3Info
{
public:
	MP3Info(const char* filename);
	~MP3Info(void);
	int GetBitRate(void);
	int GetDuration(void);
private:
	int m_bitrate;
	int m_duration;
};
