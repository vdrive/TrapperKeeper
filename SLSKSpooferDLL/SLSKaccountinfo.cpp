

#include "stdafx.h"
#include "SLSKaccountInfo.h"
#include "SLSKSpooferDlg.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>


SLSKaccountInfo::SLSKaccountInfo()
{
	
	//setAccountInfo_Database();
}


void SLSKaccountInfo::Init()
{
    srand((unsigned)time(NULL));
	dbip = "38.119.66.38";
	dbun = "root";
	dbpass = "sumyungguy37";
	dbname = "slsk";
	files = 5000;
	folders = 300;
	version = 152;
}

SLSKaccountInfo::~SLSKaccountInfo()
{
	//sm_instance=NULL;
}


char* SLSKaccountInfo::getPass()
{
	return (char*)(LPCTSTR)pass;
}

char* SLSKaccountInfo::getUN()
{
	return (char*)(LPCTSTR)un;
}

int SLSKaccountInfo::getPort()
{
	return port;
}

void SLSKaccountInfo::setPort(int p)
{
	port = p;
}

int SLSKaccountInfo::getVersion()
{
	return version;
}

void SLSKaccountInfo::setAccountInfo(void)
{
	/*char* query;

	//database shit
	myDB.dbConnection((char*)(LPCTSTR)dbip, (char*)(LPCTSTR)dbun, (char*)(LPCTSTR)dbpass, (char*)(LPCTSTR)dbname);
	
	query = "Select user_name, pass, port from account_info where locked = '0' limit 1;";
	int temp = myDB.dbSQuery(query);

	if (temp >= 0)
	{
	MYSQL_RES *res_p = myDB.getResult();
 //   res_ptr = myDB.getResult();
	MYSQL_ROW row = mysql_fetch_row(res_p);
	un = (char*)row[0];
	pass = (char*)row[1];
	port = atoi(row[2]);
	//CString s;
	myDB.clearResult();
	CString newquery = "Update account_info set locked = '1' where user_name = '";
	newquery.Append(un);
	newquery.Append("';");
	myDB.dbOQuery((char*)(LPCTSTR)newquery);
//	myDB.dbClose();
	}
	myDB.dbClose();*/
}

void SLSKaccountInfo::getUserAccounts()
{
	/*	char* query;

	//database shit
	myDB.dbConnection((char*)(LPCTSTR)dbip, (char*)(LPCTSTR)dbun, (char*)(LPCTSTR)dbpass, (char*)(LPCTSTR)dbname);
	
	query = "Select user_name, pass, port from account_info where locked = '0' limit 10;";
	int temp = myDB.dbSQuery(query);

	if (temp >= 0)
	{
	MYSQL_RES *res_p = myDB.getResult();
 //   res_ptr = myDB.getResult();
	MYSQL_ROW row; 
	while(row = mysql_fetch_row(res_p))
	{
	un = (char*)row[0];
	pass = (char*)row[1];
	port = atoi(row[2]);
	account a;
	a.un = un;
	a.pass = pass;
//	CSLSKSpooferDlg::GetInstance()->accounts.push_back(a);
	}
	//CString s;
	
	myDB.clearResult();
	//for (int i = 0; i < CSLSKSpooferDlg::GetInstance()->accounts.size(); i++)
	{

	CString newquery = "Update account_info set locked = '1' where user_name = '";
	newquery.Append(CSLSKSpooferDlg::GetInstance()->accounts[i].un);
	newquery.Append("';");
	myDB.dbOQuery((char*)(LPCTSTR)newquery);
	}
//	myDB.dbClose();
	}
	myDB.dbClose();*/
}

int SLSKaccountInfo::checkAccountInfo(void)
{
/*	myDB.dbConnection((char*)(LPCTSTR)dbip, (char*)(LPCTSTR)dbun, (char*)(LPCTSTR)dbpass, (char*)(LPCTSTR)dbname);
	int size = myDB.dbSQuery("Select count(*) from account_info;");

	MYSQL_RES *res_ptr;
	res_ptr = myDB.getResult();
	MYSQL_ROW row;
	row = mysql_fetch_row(res_ptr);
	//CSLSKSpooferDlg::GetInstance()->status_edit_box.SetWindowText(row[0]);

	int temp = atoi(row[0]);
	myDB.clearResult();
	myDB.dbClose();


	if (temp > 0)
    return temp;
	*/
	return 0;
}

int SLSKaccountInfo::checkDeadAccounts(void)
{
/*	myDB.dbConnection((char*)(LPCTSTR)dbip, (char*)(LPCTSTR)dbun, (char*)(LPCTSTR)dbpass, (char*)(LPCTSTR)dbname);
	int num = myDB.dbSQuery("Select count(*) from account_info where dead = '1';");
	if (num >= 0)
	{
		MYSQL_RES *res_ptr;
		res_ptr = myDB.getResult();
		MYSQL_ROW row;
		row = mysql_fetch_row(res_ptr);
		int temp = atoi(row[0]);

		myDB.clearResult();
		myDB.dbClose();
		return temp;
	}

	myDB.clearResult();
	myDB.dbClose();*/
	return -1;
}


void SLSKaccountInfo::TerminateAccount()
{
/*	myDB.dbConnection((char*)(LPCTSTR)dbip, (char*)(LPCTSTR)dbun, (char*)(LPCTSTR)dbpass, (char*)(LPCTSTR)dbname);
	char* query = "Update account_info set locked = '0' and dead = '1' where un = ';";
	strcat(query, un);
	strcat(query, "';");
	myDB.dbOQuery(query);
	myDB.dbClose();

	setAccountInfo;

	CSLSKSpooferDlg::GetInstance()->un_edit_box.SetWindowText((char*)(LPCTSTR)un);
	CSLSKSpooferDlg::GetInstance()->pass_edit_box.SetWindowText((char*)(LPCTSTR)pass);
	CSLSKSpooferDlg::GetInstance()->port_edit_box.SetWindowText((LPCTSTR)port);

	//controller.resetAccountInfo(un, pass, port);*/
}

void SLSKaccountInfo::createNewAccountInfo(void)
{
/*	//set un and pass in database when the usernames are corrupted
	fillName();
	
	int size;

	if (checkDeadAccounts() != -1)
		if (checkDeadAccounts() != 0 && checkDeadAccounts() != -1)
		{
			myDB.dbConnection((char*)(LPCTSTR)dbip, (char*)(LPCTSTR)dbun, (char*)(LPCTSTR)dbpass, (char*)(LPCTSTR)dbname);
			size = myDB.dbOQuery("Delete from account_info where dead = '1';");
			myDB.dbClose();
			if (size == -1)
				size = 0;
		}
		else if (checkAccountInfo() < 100)
		{
			myDB.dbConnection((char*)(LPCTSTR)dbip, (char*)(LPCTSTR)dbun, (char*)(LPCTSTR)dbpass, (char*)(LPCTSTR)dbname);
				size = myDB.dbOQuery("Select * from account_info");
				myDB.dbClose();
				if (size == -1)
					size = 0;
		}
		else
			size = 100;

	myDB.dbConnection((char*)(LPCTSTR)dbip, (char*)(LPCTSTR)dbun, (char*)(LPCTSTR)dbpass, (char*)(LPCTSTR)dbname);

	int numOfRecreate = 100 - size;

	CString s;

	char ctemp[1];
	CString password, username;
	int temp;

	for (int i = 0; i < numOfRecreate; i++)
	{
	username = names[rand() % 100];
	username.AppendFormat("%d", rand() % 10);
	username.AppendFormat("%d", rand() % 10); 
	username.AppendFormat("%d", rand() % 10);
	username.AppendFormat("%d", rand() % 10);
  
	password.Empty();
	for (int j = 0; j < 8; j++)
		{
			temp = random(48, 122);

			while (temp >= 58 && temp <= 64 || temp >= 91 && temp <= 96)
			   temp = random(48, 122);

			password.AppendChar((char)temp);//strcat(password, ctemp);
		}

		CString query = "Insert into account_info values('";
		query.Append(username);
		query.Append("', '");
		query.Append(password);
		query.Append("', '2234', '0', '0');");
		myDB.dbOQuery((char*)(LPCTSTR)query);
	}

	myDB.dbClose();*/
 }

void SLSKaccountInfo::freeAccountInfo(void)
{
/*	//free the un and pass
	myDB.dbConnection((char*)(LPCTSTR)dbip, (char*)(LPCTSTR)dbun, (char*)(LPCTSTR)dbpass, (char*)(LPCTSTR)dbname);
	for (int i = 0; i < CSLSKSpooferDlg::GetInstance()->accounts.size(); i++)
	{

	CString query = "Update account_info set locked = '0' where user_name = '";
	query.Append(CSLSKSpooferDlg::GetInstance()->accounts[i].un);
	query.Append("';");
	myDB.dbOQuery((char*)(LPCTSTR)query);
	}
	myDB.dbClose();
	un = "";
	pass = "";
	port = 0;*/
}

int SLSKaccountInfo::getFiles(void)
{
	return files;
}

int SLSKaccountInfo::getFolders(void)
{
	return folders;
}

int SLSKaccountInfo::random(int lowest, int highest)
{
	double range = (highest - lowest) + 1.0;
    return lowest + (int) (range * rand() / RAND_MAX + 1.0);
}

void SLSKaccountInfo::fillName(void)
{
	names[0]="Jacob";
	names[1]="Emily";
	names[2]="Michael";
	names[3]="Emma";
	names[4]="Joshua";
	names[5]="Madison";
	names[6]="Matthew";
	names[7]="Hannah";
	names[8]="Andrew";
	names[9]="Olivia";
	names[10]="Joseph";
	names[11]="Abigail";
	names[12]="Ethan";
	names[13]="Alexis";
	names[14]="Daniel";
	names[15]="Ashley";
	names[16]="Christopher";
	names[17]="Elizabeth";
	names[18]="Anthony";
	names[19]="Samantha";
	names[20]="William";
	names[21]="Isabella";
	names[22]="Ryan";
	names[23]="Sarah";
	names[24]="Nicholas";
	names[25]="Grace";
	names[26]="David";
	names[27]="Alyssa";
	names[26]="David";
	names[28]="Tyler";
	names[29]="Lauren";
	names[30]="Alexander";
	names[31]="Kayla";
	names[32]="John";
	names[33]="Brianna";
	names[34]="James";
	names[35]="Jessica";
	names[36]="Dylan";
	names[37]="Taylor";
	names[38]="Zachary";
	names[39]="Sophia";
	names[40]="Brandon";
	names[41]="Anna";
	names[42]="Jonathan";
	names[43]="Victoria";
	names[44]="Samuel";
	names[45]="Natalie";
	names[46]="Christian";
	names[47]="Chloe";
	names[48]="Benjamin";
	names[49]="Sydney";
	names[50]="Justin";
	names[51]="Hailey";
	names[52]="Nathan";
	names[53]="Jasmine";
	names[54]="Jose";
	names[55]="Rachel";
	names[56]="Logan";
	names[57]="Morgan";
	names[58]="Gabriel";
	names[59]="Megan";
	names[60]="Kevin";
	names[61]="Jennifer";
	names[62]="Noah";
	names[63]="Kaitlyn";
	names[64]="Austin";
	names[65]="Julia";
	names[66]="Caleb";
	names[67]="Haley";
	names[68]="Robert";
	names[69]="Mia";
	names[70]="Thomas";
	names[71]="Katherine";
	names[72]="Elijah";
	names[73]="Destiny";
	names[74]="Jordan";
	names[75]="Alexandra";
	names[76]="Aidan";
	names[77]="Nicole";
	names[78]="Cameron";
	names[79]="Maria";
	names[80]="Hunter";
	names[81]="Ava";
	names[82]="Jason";
	names[83]="Savannah";
	names[84]="Angel";
	names[85]="Brooke";
	names[86]="Connor";
	names[87]="Ella";
	names[88]="Evan";
	names[89]="Allison";
	names[90]="Jack";
	names[91]="MacKenzie";
	names[92]="Luke";
	names[93]="Paige";
	names[94]="Isaac";
	names[95]="Stephanie";
	names[96]="Aaron";
	names[97]="Jordan";
	names[98]="Isaiah";
	names[99]="Kylie";
}

