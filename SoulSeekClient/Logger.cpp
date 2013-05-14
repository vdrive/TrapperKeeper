#include "logger.h"

Logger::Logger(void)
{
}

Logger::~Logger(void)
{
}

void Logger::WriteToTextFile(string s)
{
	CFile errorLog;
	CFileException fileException;

	if ( !errorLog.Open( "C:\\SoulSeekLogPM.txt", CFile::modeCreate |   
			CFile::modeNoTruncate | CFile::modeWrite | CFile::shareDenyWrite, &fileException ) )
	{
		TRACE( "Can't open file %s, error = %u\n",
			"C:\\SoulSeekLogPM.txt", fileException.m_cause );
	}

	errorLog.SeekToEnd();
	char buf[512];
	sprintf(buf,"%s",s.c_str());
	errorLog.Write(buf, (UINT)strlen(buf));

	errorLog.Close();
}
