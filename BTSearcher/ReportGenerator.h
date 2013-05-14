#pragma once

#include "mysql.h"

class ReportGenerator
{
public:
	ReportGenerator(CListCtrl * list);
	~ReportGenerator(void);

	bool GenerateReport(vector<string> v_labels);
	string GetLastDate(MYSQL * conn, string query);
	vector<string> GetData(MYSQL * conn, string query, string owner);
	void WriteFile(string owner, string source, string type, vector<string> * data);
	void WriteFile(string owner, string source, vector<string> * zero, vector<string> * good, vector<string> * all);
	void WriteWorksheetHeader(CFile * t_file, int row_count, string owner, string source, CTime now, string type);
	void WriteWorksheetClose(CFile * t_file);
	void WriteLine(CFile *file, string line);
	void Log(string buf);
	void Log(const char * buf);

	bool CreateDirectory(string dirname);
	bool CreateFile(string filename);
	bool OpenFile(string filename, CFile *t_file);
	void CloseFile(CFile *file);
	void WriteFile(string filename, unsigned short * data, int len);

protected:
	CListCtrl  * m_main_list_ctrl;
	CFile file;
};
