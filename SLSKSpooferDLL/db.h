////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                    //
//  The following is the database class. It is used to connect to a mysql server and perform the      //
//                                  necessary queries required.                                       //
//                                                                                                    //
//                                                           WRITTEN BY:                              //
//                                                              ALAIN DADAIAN                         //
//                                                                                                    //
////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "mysql.h"


class db
{
private:
	MYSQL my_connection;
    MYSQL_RES *res_ptr;
public:
	db();
	~db();
	void dbConnection(char*, char*, char*, char*); 
	void dbClose(void);
	int dbSQuery(char*);
	int dbOQuery(char*);
	MYSQL_RES * getResult(void);
	void clearResult(void);
};