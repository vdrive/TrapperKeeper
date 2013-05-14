#pragma once

class TrackerConnector
{
public:
	TrackerConnector(void);
	~TrackerConnector(void);

	int RegisterWithTracker(char * trackerurl,unsigned short **body, string referer);
	int UnZip(unsigned short ** dest, unsigned short * source, int len);
};
