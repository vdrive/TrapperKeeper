#pragma once
#include "..\tkcom\threadedobject.h"
#include "..\tkcom\vector.h"

class FileRequestLogger : public ThreadedObject
{
	class FalseDecoyRequest : public Object
	{
	public:
		string m_project;
		string m_ip;
		string m_file_name;
		FalseDecoyRequest(const char* project,const char *ip,const char* file_name){
			m_project=project;
			m_ip=ip;
			m_file_name=file_name;
		}
	};
protected:
	Vector mv_false_decoy_requests;
public:
	FileRequestLogger(void);
	~FileRequestLogger(void);
	UINT Run();
	void LogFalseDecoyRequest(const char* project, const char* ip, const char* file_name);
};
