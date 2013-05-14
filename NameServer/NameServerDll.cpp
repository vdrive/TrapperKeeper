#include "StdAfx.h"
#include "nameserverdll.h"
#include "DllInterface.h"
#include "mysql.h"	// for database access (also need libmysql.lib)

NameServerDll::NameServerDll(void)
{
}

NameServerDll::~NameServerDll(void)
{
}
//
//
//
void NameServerDll::DllInitialize()
{
	m_dlg.Create(IDD_NAME_SERVER_DIALOG,CWnd::GetDesktopWindow());
	//moved by Ivan
	parser.ReadXMLFile();	//reads the xml file
	m_dlg.GetParser(parser);  //gives the parser to NameServerDlg class
}

//
//
//
void NameServerDll::DllUnInitialize()
{

}

//
//
//
void NameServerDll::DllStart()
{
	//modified by Ivan: moved it to DllInitialize() function so that when other dlls request IPs
	//in their DllStart() function, the NameServer will have the IP list ready already.
	//parser.ReadXMLFile();	//reads the xml file
}

//
//
//
void NameServerDll::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}

//when the dll receives data is comes here
bool NameServerDll::ReceivedDllData(AppID from_app_id, void* input_data, void* output_data)
{
	//parse tag and run query
	char* temp;
	temp=(char*)input_data;
	vector<CString> v_names;
	vector<string> temp_vector;
	if(strcmp(temp,"GETALLNAMES")==0)
	{
		temp_vector.clear();
		v_names=parser.GetTags();
		for(UINT i=0;i<v_names.size();i++)
		{
			temp_vector.push_back((LPCSTR)v_names[i]);
		}
		vector<string> &names = *(vector<string>*)output_data;
		names=temp_vector;
		return true;

	}
	//get ip info
	else if(strcmp(temp,"GETIPINFO")==0)
	{
		temp_vector.clear();
		vector<string> ip_dest;
		vector<string> ip_source;
		vector<RackInfo> rack_data;		//vector to be sent
		RackInfo temp_rack_data;	//a single rack and it's properties

		ip_dest=parser.GetIPDestVector();		//pass all the destination ip addresses
		ip_source=parser.GetIPSourceVector();	//pass all the source ip addresses
		//connect to DB
		MYSQL *conn;
		MYSQL_RES *res_set;
		MYSQL_ROW row;
		conn=mysql_init(NULL);
		//try connecting to the database
		if(!mysql_real_connect(conn,"address","username","password","database",0,NULL,0))
		{
			::AfxMessageBox(mysql_error(conn));		//display error in message box
			return false;	//return because of error
		}
		//start of query
		CString query="select IP,cabinet,switch,network,port,rackname from ips where ";
		//makes query that has all ip ranges read in from the file
		for(UINT x=0;x<(UINT)ip_dest.size();x++)
		{
			if(x==(UINT)ip_source.size()-1)
				query+="(internal_ip between inet_aton('"+(CString)ip_source[x].c_str()+"') and inet_aton('"+(CString)ip_dest[x].c_str()+"'))";
			else
				query+="(internal_ip between inet_aton('"+(CString)ip_source[x].c_str()+"') and inet_aton('"+(CString)ip_dest[x].c_str()+"')) or ";
		}
		TRACE("\n%s", query);
		if(mysql_query(conn,query)!=0)
		{
			::AfxMessageBox(mysql_error(conn));
			return false;	//return because of error
		}
		res_set=mysql_store_result(conn);

		unsigned int num_rows=(unsigned int)mysql_num_rows(res_set);
		//read all the data from each row
		while((row=mysql_fetch_row(res_set))!=NULL)
		{
			temp_rack_data.ip=row[0];
			temp_rack_data.cabinet=row[1];
			temp_rack_data.cabinet_switch=row[2];
			temp_rack_data.network=row[3];
			temp_rack_data.port=atoi(row[4]);
			temp_rack_data.rackname=row[5];
			rack_data.push_back(temp_rack_data);	//push all the data into a vector
		}
		vector<RackInfo> &racks = *(vector<RackInfo>*)output_data;
		racks = rack_data;	//set all data retrieved
		return true;		//returned successful
	}
	vector<string> vec;
	vec.clear();	//clear vector
	vec=parser.GetVector(temp);	//get the list of ips that correspond to temp(tag)
	
	vector<string> &ips = *(vector<string>*)output_data;
	ips = vec;
	return true;
}