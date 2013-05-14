//Logfile.cpp

#include "stdafx.h"
#include "Logfile.h"

//
//
//
Logfile::Logfile(/*int type,char *title,*/char *self)
{
	memset(m_path,0,sizeof(m_path));
	memset(m_filename,0,sizeof(m_filename));
	
	CTime time = CTime::GetCurrentTime();
	char year[16];
	char month[16];
	char day[16];
	int yr=time.GetYear();
	_itoa(yr, year, 10);

	int mnth=time.GetMonth();
	sprintf(month,"%02u",mnth);

	int dy=time.GetDay();
	sprintf(day,"%02u",dy);
/*
	// Create sub directories
	strcpy(m_path,"Logfiles");
	CreateDirectory(m_path,NULL);
	strcat(m_path,"\\");
	strcat(m_path,title);
	CreateDirectory(m_path,NULL);
		
	strcpy(m_filename,title);
*/
//	strcat(m_filename," - ");
	strcpy(m_filename,self);

	strcat(m_filename," - Poisoner Logfile - ");
/*
	switch(type)
	{
		case Logfile::LogfileTypeSpoof:
		{
			strcat(m_filename," - Spoofer Logfile - ");
			strcat(m_path,"\\Spoofer Logfiles");
			break;
		}
		case Logfile::LogfileTypeSupply:
		{
			strcat(m_filename," - Supply Logfile - ");
			strcat(m_path,"\\Supply Logfiles");
			break;
		}
		case Logfile::LogfileTypeDebug:
		{
			strcat(m_filename," - Debug Logfile - ");
			strcat(m_path,"\\Debug Logfiles");
			break;
		}
		case Logfile::LogfileTypeDist:
		{
			strcat(m_filename," - Dist Logfile - ");
			strcat(m_path,"\\Dist Logfiles");
			break;
		}
	}

	CreateDirectory(m_path,NULL);

	strcat(m_path,"\\");
*/

//	strcpy(m_filename,year);
	strcat(m_filename,year);
	strcat(m_filename,"-");
	strcat(m_filename,month);
	strcat(m_filename,"-");
	strcat(m_filename, day);
	strcat(m_filename,".txt");
}

//
//
//
void Logfile::Log(char *msg)
{
	strcat(m_path,m_filename);

	if(CStdioFile::Open(m_path,CFile::typeText|CFile::modeRead|CFile::shareDenyNone)==0)
	{
		CStdioFile::Open(m_path,CFile::typeText|CFile::modeCreate|CFile::modeWrite|CFile::shareDenyNone);
	}
	else
	{
		Close();
		CStdioFile::Open(m_path,CFile::typeText|CFile::modeWrite|CFile::shareDenyNone);
	}
	
	SeekToEnd();

	WriteString(msg);

	Close();
}