////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                    //
//  The following is the database class. It is used to connect to a mysql server and perform the      //
//                                  necessary queries required.                                       //
//                                                                                                    //
//                                                           WRITTEN BY:                              //
//                                                              ALAIN DADAIAN                         //
//                                                                                                    //
////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "db.h"
#include "SLSKSpooferDlg.h"


//------------------------------------------------------------------------------------------------------
// The following is the constructor for the database class.
//------------------------------------------------------------------------------------------------------
db::db()
{
	res_ptr = NULL;
	mysql_init(&my_connection); 
}

//------------------------------------------------------------------------------------------------------
// The following is the deconstructor for the database class.
//------------------------------------------------------------------------------------------------------
db::~db()
{
	dbClose();
}

//------------------------------------------------------------------------------------------------------
// The following is the connection function for the database. It takes four parameters ip, login, pass,
// and the database name all in the type char*.
//------------------------------------------------------------------------------------------------------
void db::dbConnection(char* ip, char* login, char* pass, char* db)
{

  if (!mysql_real_connect(&my_connection, ip, login, pass, db, 0, NULL, 0))
	{
	//	::MessageBox(NULL, "DIDNT CONNECT TO THE DATABASE!", "DATABASE CONNECTION ERROE", MB_OK);//CSLSKSpooferDlg::GetInstance()->status_edit_box.SetWindowText("Did not connect to the database!");
	}

}

//------------------------------------------------------------------------------------------------------
// The following closes the database connection.
//------------------------------------------------------------------------------------------------------
void db::dbClose()
{
	mysql_close(&my_connection);
}

//------------------------------------------------------------------------------------------------------
// The following is the function that performs every task but the select query. It returns the number
// of rows effected by the sql statment in the form of an int.
//------------------------------------------------------------------------------------------------------
int db::dbOQuery(char* query)
{
	int res, temp;

	res = mysql_query(&my_connection, query);
  if (res == 0) 
  {
	  temp = (int)mysql_affected_rows(&my_connection);
		return (int)mysql_affected_rows(&my_connection);
  }
  else 
		return -1;
}

//------------------------------------------------------------------------------------------------------
// The following is the function that performs the select query. It returns the number of rows returned
// by the select query. It also stores the information in a mysql_res * that can later be retrieved 
// by the getResult function.
//------------------------------------------------------------------------------------------------------
int db::dbSQuery(char* query)
{

	int res;

	res = mysql_query(&my_connection, query);

	if (res == 0)
	{
		res_ptr = mysql_store_result(&my_connection);
	//	int temp = (int)mysql_num_rows(res_ptr);
	//	CSLSKSpooferDlg::GetInstance()->status_edit_box.SetWindowText((LPCTSTR)temp);
		return res;

	}
	else
		return -1;
}


//------------------------------------------------------------------------------------------------------
// The following gets the results of your select query. It returns a mysql_res *.
//------------------------------------------------------------------------------------------------------
MYSQL_RES* db::getResult()
{
	return res_ptr;
}

//------------------------------------------------------------------------------------------------------
// The following frees the pointer with all the information that was attained by the select query.
//------------------------------------------------------------------------------------------------------
void db::clearResult()
{
	mysql_free_result(res_ptr);
 }
