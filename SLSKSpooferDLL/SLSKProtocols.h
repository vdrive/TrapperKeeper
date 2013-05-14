

#pragma once
#include "buffer.h"
#include "db.h"

class SLSKProtocols
{
private:
	db myDB;
	char* dip;
	char* dun;
	char* dpass;
	char* ddb;
public:
	SLSKProtocols(void);
	~SLSKProtocols(void);
  void reply(buffer);
	void login(void);
};