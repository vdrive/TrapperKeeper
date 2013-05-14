// GWebCache.h
#pragma once

#include "GWebCacheSocket.h"

class ConnectionManager;

class GWebCache
{
public:
	GWebCache();
	~GWebCache();
	void InitParent(ConnectionManager *manager);

	bool ConnectToWebCache();

	void ReportURLs(vector<string> &urls);
	void ReportHosts(vector<string> &hosts);

	unsigned int NumURLs();

	void CheckForTimeout();
	void ReportBadURL(const char* url);

private:
	ConnectionManager *p_manager;
	GWebCacheSocket m_socket;

	vector<string> v_web_cache_urls;

	CTime m_connect_time;

	// Private Member Functions
	void ReadWebCacheURLsFromFile();
	void WriteWebCacheURLsToFile();
	void GWebCache::AddDefaultGWebCache();

};