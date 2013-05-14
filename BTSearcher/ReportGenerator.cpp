#include "StdAfx.h"
#include "reportgenerator.h"
#include "SQLInterface.h"

ReportGenerator::ReportGenerator(CListCtrl * list)
{
	m_main_list_ctrl = list;
}

ReportGenerator::~ReportGenerator(void)
{
}

bool ReportGenerator::GenerateReport(vector<string> v_labels)
{
	SQLInterface sql;
	MYSQL conn;
	char buf[1024];
	sql.OpenDB(&conn, "206.161.141.35", "onsystems", "ebertsux37", "bittorrent_data");
	vector<string> v_source;
	v_source.push_back("TorrentSpy");
	v_source.push_back("IsoHunt");
	v_source.push_back("piratebay");
	v_source.push_back("MiniNova");
	v_source.push_back("TorrentBox");
	v_source.push_back("TorrentPortal");
	v_source.push_back("Torrentreactor");

	const char * ignore_list = "2244, 468, 2325, 2242, 3221, 2459, 2474";
	for(unsigned int i = 0; i < v_labels.size(); i++) {
		for(int j = 0; j < (int)v_source.size(); j++) {
			string source = v_source[j];

			sprintf(buf, "select max(last_seen) from supply_data where owner like '%s%%' and source = '%s'", v_labels[i].c_str(), source.c_str());
			string last_seen = GetLastDate(&conn, string(buf));

			sprintf(buf, "select project_id, title, url, search_string, seed, size, files, date_found, last_seen, times_seen, owner from supply_data where owner like '%s%%' and source = '%s' and last_seen = '%s' order by search_string", v_labels[i].c_str(), source.c_str(), last_seen.c_str()); 
			vector<string> all = GetData(&conn, string(buf), v_labels[i]);

			sprintf(buf, "select  project_id, title, url, search_string, seed, size, files, date_found, last_seen, times_seen, owner from supply_data where owner like '%s%%' and source = '%s' and last_seen = '%s' and seed in (0, -1) and project_id not in (%s) order by search_string", v_labels[i].c_str(), source.c_str(), last_seen.c_str(), ignore_list); // 24
			vector<string> zero = GetData(&conn, string(buf), v_labels[i]);

			sprintf(buf, "select  project_id, title, url, search_string, seed, size, files, date_found, last_seen, times_seen, owner from supply_data where owner like '%s%%' and source = '%s' and last_seen = '%s' and seed not in (0, -1) and project_id not in (%s) order by search_string", v_labels[i].c_str(), source.c_str(), last_seen.c_str(), ignore_list);
			vector<string> good = GetData(&conn, string(buf), v_labels[i]);

			WriteFile(v_labels[i], source, &zero, &good, &all);
		}
	}
	sql.CloseDB(&conn);
	return true;
}

void ReportGenerator::WriteFile(string owner, string source, vector<string> * zero, vector<string> * good, vector<string> * all)
{
	CTime now=CTime::GetCurrentTime();
	CFile t_file;
	string base_dir = "C:\\Documents and Settings\\onsystems\\My Documents\\";
	char msg[128];

	if(owner.find("Universal Music") == 0) owner = "UMG";
	else if(owner.find("Universal Pictures") == 0) owner = "UP";

	sprintf(msg,"%s%s_%s-%04u-%02u-%02u.xml",base_dir.c_str(), owner.c_str(), source.c_str(),now.GetYear(),now.GetMonth(),now.GetDay());
	Log(string("writting file: ") + owner +string(" ") + source);
	string filename = string(msg);

	CreateFile(filename);
	if(!OpenFile(filename, &t_file)) {
		return;
	}	
	
	t_file.SeekToBegin();
	t_file.SetLength(0);
	TRACE("Writting %s\n", filename.c_str());

	WriteLine(&t_file, "<?xml version=\"1.0\"?>");
	WriteLine(&t_file, "<?mso-application progid=\"Excel.Sheet\"?>");
	WriteLine(&t_file, "<Workbook xmlns=\"urn:schemas-microsoft-com:office:spreadsheet\"");
 	WriteLine(&t_file, " xmlns:o=\"urn:schemas-microsoft-com:office:office\"");
 	WriteLine(&t_file, " xmlns:x=\"urn:schemas-microsoft-com:office:excel\"");
 	WriteLine(&t_file, " xmlns:ss=\"urn:schemas-microsoft-com:office:spreadsheet\"");
 	WriteLine(&t_file, " xmlns:html=\"http://www.w3.org/TR/REC-html40\">");
 	WriteLine(&t_file, " <DocumentProperties xmlns=\"urn:schemas-microsoft-com:office:office\">");
  	WriteLine(&t_file, "  <LastAuthor> </LastAuthor>");
	CString temp = now.FormatGmt("  <Created>%Y-%m-%dT%H:%M:%SZ</Created>");
	WriteLine(&t_file, temp.GetBuffer());
	temp = now.FormatGmt("  <LastSaved>%Y-%m-%dT%H:%M:%SZ</LastSaved>");
	WriteLine(&t_file, temp.GetBuffer());
  	WriteLine(&t_file, "  <Version>11.6360</Version>");
 	WriteLine(&t_file, " </DocumentProperties>");
 	WriteLine(&t_file, " <ExcelWorkbook xmlns=\"urn:schemas-microsoft-com:office:excel\">");
  	WriteLine(&t_file, "  <WindowHeight>10125</WindowHeight>");
  	WriteLine(&t_file, "  <WindowWidth>16815</WindowWidth>");
  	WriteLine(&t_file, "  <WindowTopX>1440</WindowTopX>");
  	WriteLine(&t_file, "  <WindowTopY>405</WindowTopY>");
  	WriteLine(&t_file, "  <ProtectStructure>False</ProtectStructure>");
  	WriteLine(&t_file, "  <ProtectWindows>False</ProtectWindows>");
 	WriteLine(&t_file, " </ExcelWorkbook>");
 	WriteLine(&t_file, " <Styles>");
 	WriteLine(&t_file, "  <Style ss:ID=\"Default\" ss:Name=\"Normal\">");
 	WriteLine(&t_file, "   <Alignment ss:Vertical=\"Bottom\"/>");
 	WriteLine(&t_file, "   <Borders/>");
 	WriteLine(&t_file, "   <Font/>");
 	WriteLine(&t_file, "   <Interior/>");
 	WriteLine(&t_file, "   <NumberFormat/>");
 	WriteLine(&t_file, "   <Protection/>");
 	WriteLine(&t_file, "  </Style>");
 	WriteLine(&t_file, "  <Style ss:ID=\"s21\">");
 	WriteLine(&t_file, "   <NumberFormat ss:Format=\"Short Date\"/>");
 	WriteLine(&t_file, "  </Style>");
 	WriteLine(&t_file, " </Styles>");

	WriteWorksheetHeader(&t_file, (int)zero->size()+1, owner, source, now, "zero");

	WriteLine(t_file, "   <Row>");
 	WriteLine(t_file, "    <Cell><Data ss:Type=\"String\">project_id</Data></Cell>");
 	WriteLine(t_file, "    <Cell><Data ss:Type=\"String\">title</Data></Cell>");
 	WriteLine(t_file, "    <Cell><Data ss:Type=\"String\">url</Data></Cell>");
 	WriteLine(t_file, "    <Cell><Data ss:Type=\"String\">search_string</Data></Cell>");
 	WriteLine(t_file, "    <Cell><Data ss:Type=\"String\">seed</Data></Cell>");
 	WriteLine(t_file, "    <Cell><Data ss:Type=\"String\">size</Data></Cell>");
 	WriteLine(t_file, "    <Cell><Data ss:Type=\"String\">files</Data></Cell>");
 	WriteLine(t_file, "    <Cell><Data ss:Type=\"String\">date_found</Data></Cell>");
 	WriteLine(t_file, "    <Cell><Data ss:Type=\"String\">last_seen</Data></Cell>");
 	WriteLine(t_file, "    <Cell><Data ss:Type=\"String\">times_seen</Data></Cell>");
 	WriteLine(t_file, "    <Cell><Data ss:Type=\"String\">owner</Data></Cell>");
 	WriteLine(t_file, "   </Row>");

	for(unsigned int i = 0; i < zero->size(); i++) {
		WriteLine(&t_file, "   <Row>");
		string line = (*zero)[i]; 
		t_file.Write(line.c_str(), (UINT)line.length());
		WriteLine(&t_file, "   </Row>");
	}

	WriteWorksheetClose(&t_file);

	WriteWorksheetHeader(&t_file, (int)good->size()+1, owner, source, now, "good");
	
	for(unsigned int i = 0; i < good->size(); i++) {
		WriteLine(&t_file, "   <Row>");
		string line = (*good)[i]; 
		t_file.Write(line.c_str(), (UINT)line.length());
		WriteLine(&t_file, "   </Row>");
	}

	WriteWorksheetClose(&t_file);

	WriteWorksheetHeader(&t_file, (int)all->size()+1, owner, source, now, "all");
	
	for(unsigned int i = 0; i < all->size(); i++) {
		WriteLine(&t_file, "   <Row>");
		string line = (*all)[i]; 
		t_file.Write(line.c_str(), (UINT)line.length());
		WriteLine(&t_file, "   </Row>");
	}

	WriteWorksheetClose(&t_file);

	TRACE("ReportGenerator::WriteFile Done\n");
	CloseFile(&t_file);
}

void ReportGenerator::WriteWorksheetClose(CFile * t_file)
{
	WriteLine(t_file, "  </Table>");
	WriteLine(t_file, "  <WorksheetOptions xmlns=\"urn:schemas-microsoft-com:office:excel\">");
	WriteLine(t_file, "   <Selected/>");
	WriteLine(t_file, "   <ProtectObjects>False</ProtectObjects>");
	WriteLine(t_file, "   <ProtectScenarios>False</ProtectScenarios>");
	WriteLine(t_file, "  </WorksheetOptions>");
	WriteLine(t_file, " </Worksheet>");
}

void ReportGenerator::WriteWorksheetHeader(CFile * t_file, int row_count, string owner, string source, CTime now, string type)
{
	char temp[128];
	if(owner.length() + source.length() > 14) {
		source = source.substr(0, 14-owner.length());
	}
	sprintf(temp, " <Worksheet ss:Name=\"%s_%s_%d-%d-%d_%s\">", 
		owner.c_str(), source.c_str(), now.GetYear(), now.GetMonth(), now.GetDay(), type.c_str());
	WriteLine(t_file, temp);
	sprintf(temp, "  <Table ss:ExpandedColumnCount=\"11\" ss:ExpandedRowCount=\"%d\" x:FullColumns=\"1\"", row_count);
	WriteLine(t_file, temp);
 	WriteLine(t_file, "   x:FullRows=\"1\">");
 	WriteLine(t_file, "   <Column ss:Index=\"2\" ss:AutoFitWidth=\"0\" ss:Width=\"237.75\"/>");
 	WriteLine(t_file, "   <Column ss:AutoFitWidth=\"0\" ss:Width=\"99\"/>");
 	WriteLine(t_file, "   <Column ss:Width=\"81.75\"/>");
 	WriteLine(t_file, "   <Column ss:Width=\"26.25\"/>");
 	WriteLine(t_file, "   <Column ss:Width=\"40.5\"/>");
 	WriteLine(t_file, "   <Column ss:Width=\"22.5\"/>");
 	WriteLine(t_file, "   <Column ss:Width=\"53.25\" ss:Span=\"1\"/>");
 	WriteLine(t_file, "   <Column ss:Index=\"10\" ss:Width=\"55.5\"/>");
 	WriteLine(t_file, "   <Column ss:Width=\"74.25\"/>");


}

void ReportGenerator::WriteLine(CFile *t_file, string line)
{
	line += "\r\n";
	t_file->Write(line.c_str(), (UINT)line.length());
}

string ReportGenerator::GetLastDate(MYSQL * conn, string query) {
	MYSQL_RES *res;
	MYSQL_ROW row;
	SQLInterface sql;
	res = sql.Query(conn, query.c_str());

	string date = "2005-01-01";

	if(res != NULL)
	{
		//int col_count =  (int) mysql_num_fields(res);
		int num_rows = (int)mysql_num_rows(res);
		if(num_rows > 0)
		{
			row = mysql_fetch_row(res);
			if(row[0] != NULL) date = string(row[0]);
			TRACE("DATE: %s\n", date.c_str());
		}
		mysql_free_result(res);			// free's mem (::)
	} 
	return date;
}

vector<string> ReportGenerator::GetData(MYSQL * conn, string query, string owner) {
	MYSQL_RES *res;
	MYSQL_ROW row;
	SQLInterface sql;

	int owner_col = 10;
	int title_col = 1;
	
	vector<string> data;

	Log(string("Getting Data for ")+owner);
	TRACE("query: %s\n", query.c_str());

	res = sql.Query(conn, query);
	if(res != NULL)
	{
		int col_count =  (int) mysql_num_fields(res);
		int num_rows = (int)mysql_num_rows(res);
		if(num_rows > 0)
		{
			while ((row = mysql_fetch_row(res)))
			{	
				string line = "";
				for(int i = 0; i < col_count; i++) 
				{
					if(i == owner_col) {
						line += "    <Cell><Data ss:Type=\"String\">" + 
							owner + string("</Data></Cell>\r\n");
					}
					else if(i==0||i==4||i==6||i==9){ // numbers
						line += "    <Cell><Data ss:Type=\"Number\">";
						line += string(row[i]);
						line += "</Data></Cell>\r\n";
					} else if(i==7||i==8) { //dates
						line += "    <Cell ss:StyleID=\"s21\"><Data ss:Type=\"DateTime\">";
						line += string(row[i]) + string("T00:00:00.000");;
						line += "</Data></Cell>\r\n";
					} else { // strings
						CString temp = row[i];
						temp.Replace("&", "&amp;");
						temp.Replace(">", "&gt;");
						temp.Replace("\"", "&quot;");
						temp.Replace("å", "?");
						temp.Replace("í", "?");
						//if(i==1) temp = "test";
						line += "    <Cell><Data ss:Type=\"String\">" + 
							string(temp.GetBuffer()) + string("</Data></Cell>\r\n");

					}
				}
				data.push_back(line);
				//TRACE(line.c_str());
			}
		}
		mysql_free_result(res);			// free's mem (::)
	} 
	return data;
}


bool ReportGenerator::CreateDirectory(string dirname)
{
	return (::CreateDirectory(dirname.c_str(), NULL) != 0);
}

bool ReportGenerator::CreateFile(string filename)
{
	CFile t_file;
	CFileStatus status;
	if( CFile::GetStatus( filename.c_str(), status ) ) {
		TRACE("%s already exits\n", filename.c_str());
		return false;
	} else {
		if(t_file.Open(filename.c_str(),CFile::modeCreate|CFile::typeBinary|CFile::shareDenyNone)==FALSE)
		{
			DWORD damnerror = ::GetLastError();
			return false;
		}
		t_file.Close();
		return true;
	}

}

bool ReportGenerator::OpenFile(string filename, CFile *t_file)
{

	if(t_file->Open(filename.c_str(),CFile::modeWrite|CFile::typeBinary|CFile::shareDenyNone)==FALSE)
	{
		DWORD damnerror = ::GetLastError();
		return false;
	}
	return true;
}

void ReportGenerator::CloseFile(CFile *file)
{
	file->Flush();
	file->Close();
}

void ReportGenerator::WriteFile(string owner, string source, string type, vector<string> * data)
{
	CTime now=CTime::GetCurrentTime();
	CFile t_file;
	string base_dir = "C:\\Documents and Settings\\onsystems\\My Documents\\";
	char msg[128];

	if(owner.find("Universal Music") == 0) owner = "UMG";

	sprintf(msg,"%s%s_%s_%04u-%02u-%02u_%s.txt",base_dir.c_str(), owner.c_str(), source.c_str(), now.GetYear(),now.GetMonth(),now.GetDay(),type.c_str());
	Log(string("writting file: ") + owner + string(" ") + source + string(" ") + type);
	string filename = string(msg);

	CreateFile(filename);
	if(!OpenFile(filename, &t_file)) {
		return;
	}	
	
	t_file.SeekToBegin();
	TRACE("Writting %s\n", filename.c_str());


	for(unsigned int i = 0; i < data->size(); i++) {
		string line = (*data)[i]; 
		t_file.Write(line.c_str(), (UINT)line.length());
	}
	TRACE("ReportGenerator::WriteFile Done\n");
	CloseFile(&t_file);
}


void ReportGenerator::Log(string buf)
{
	Log(buf.c_str());
}

void ReportGenerator::Log(const char * buf)
{
	char msg[4096];
	if(strlen(buf) > 4005) {
		memcpy(msg, buf, 4005);
		msg[4005] = '\0';
		Log(msg);
		return;
	}
	CTime now=CTime::GetCurrentTime();
	sprintf(msg,"%04u-%02u-%02u %02u:%02u:%02u - ",now.GetYear(),now.GetMonth(),now.GetDay(),now.GetHour(),now.GetMinute(),now.GetSecond());
	strcat(msg, buf);

	int count = m_main_list_ctrl->GetItemCount();
	if(count > 1000) m_main_list_ctrl->DeleteItem(1000);

	m_main_list_ctrl->InsertItem(0, msg);
}
