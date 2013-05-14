#pragma once
#include "stdafx.h"

string GetData(const char * url, const string referer);
int GetData(const char *url, unsigned short **buf, string referer);
int PostData(const char *url, unsigned short **buf, string referer, string post_info);
int GetHTTPData(const char *url, unsigned short **buf, string referer, bool b_post=false, string post_info="");
int UnZip(unsigned short ** dest, unsigned short * source, int len);
