

#include "stdafx.h"
#include "SLSKController.h"
//#include "SLSKaccountInfo.h"

#include "SLSKToDoList.h"
#include "SLSKSpooferDlg.h"
#include "mysql.h"
#include "SupernodeHost.h"
#include "fstream"




SLSKController::SLSKController(void)
{
	m_dlg = NULL;
	m_server_started = false;
	//SLSKaccountInfo::sm_instance = new SLSKaccountInfo;
//	connection_manager.InitAccountInfo(account_info);
	
}

void SLSKController::InitDialog(CSLSKSpooferDlg *pDlg)
{
	m_dlg = pDlg;
}

SLSKController::~SLSKController(void)
{
	m_dlg = NULL;
	stop();
}

/*void SLSKController::DllInitialize()
{
}

void SLSKController::DllUnInitialize()
{
	WSocket::Cleanup();
	m_connection_manager.KillModules();
	m_dlg.DestroyWindow();
}
void SLSKController::DllStart()
{
	m_dlg.Create(IDD_SLSKSPOOFER_DIALOG,CWnd::GetDesktopWindow());
	m_dlg.InitParent(this);
	m_connection_mananger.InitParent(this);

	m_dlg.SetTimer(1,1000,0);
	m_dlg.SetTimer(2,60*1000,0);
	m_dlg.SetTimer(3,5*60*1000,0);
	m_dlg.SetTimer(6,5*1000,0);
	m_dlg.SetTimer(5,30*1000,0);
}

void SLSKController::DllShowGUI()
{
	m_dlg.ShowWindow(SW_NORMAL);
	m_dlg.BringWindowToTop();
}*/

void SLSKController::StartSpoofing(void)
{

	//TRACE("BEGINING START SPOOFING!\n");
	m_connection_manager.InitParent(this);
	
	m_connection_manager.AddModule(1);
	
	m_connection_manager.AddModule(2);
	
	m_connection_manager.AddModule(3);
	

	SupernodeHost h;
	for (int i = 0; i < 2; i++)
	{
		h.SetIP("38.115.131.131");
		h.setport(2240);
	
		m_connection_manager.server_ips.push_back(h);
	}



//	TRACE("CONNECTING TO DATABASE!\n");
		/*	db mydb;
		//	TRACE("1\n");
			mydb.dbConnection("localhost", "onsystems", "sumyungguy37", "slsk");
		//	TRACE("2\n");
			mydb.dbSQuery("Select ip, port from parent_ips;");
		//	TRACE("3\n");
			MYSQL_RES *res_ptr;
			//TRACE("4\n");
		    res_ptr = mydb.getResult();
			//TRACE("5\n");
			MYSQL_ROW row;
			SupernodeHost nh;
		//	row = mysql_fetch_row(res_ptr)
			//CString s;
			//TRACE("6\n");

			while ( row = mysql_fetch_row(res_ptr))
			{
				//TRACE ("IN WHILE LOOP!\n");
				//if (counter == 100)
				//	break;
				if(row[0] != NULL && row[1] != NULL)
					{
				nh.SetIP(row[0]);
				nh.setport(atoi(row[1]));

				m_connection_manager.parent_ips.push_back(nh);
//				counter++;
			//	row = mysql_fetch_row(res_ptr)
					}

					//TRACE("END WHILE LOOP!\n");
			}
			mydb.clearResult();
			//mydb.dbOQuery("delete from parent_ips where ip like '%';");
			mydb.dbClose();*/
	//connection_manager.ConnectNewHosts(CSLSKSpooferDlg::GetInstance()->todo.hosts);*/

	m_server_started = true;
	m_dlg->SetTimer(5,30*1000,0);
//	TRACE("END START SPOOF!\n");
}

void SLSKController::start(void)
{
	//m_dlg.InitParent(this);
	//WSocket::Startup();
//	initAccountInfo();

	
	
	//h.SetIP("200.65.75.196");
	//h.setport(2234);

	//CSLSKSpooferDlg::GetInstance()->todo.hosts.push_back(h);


	

//	int counter = 1;
			/*db mydb;
			mydb.dbConnection("localhost", "root", "sumyungguy37", "slsk");
			mydb.dbSQuery("Select ip, port from parent_ips;");
			MYSQL_RES *res_ptr;
		    res_ptr = mydb.getResult();
			MYSQL_ROW row;
			SupernodeHost nh;
		//	row = mysql_fetch_row(res_ptr)
			//CString s;
			

			while ( row = mysql_fetch_row(res_ptr))
			{
				//if (counter == 100)
				//	break;
				if(row[0] != NULL && row[1] != NULL)
					{
				nh.SetIP(row[0]);
				nh.setport(atoi(row[1]));

				m_connection_manager.parent_ips.push_back(nh);
//				counter++;
			//	row = mysql_fetch_row(res_ptr)
					}
			}
			mydb.clearResult();
			mydb.dbOQuery("delete from parent_ips where ip like '%';");
			mydb.dbClose();
	//connection_manager.ConnectNewHosts(CSLSKSpooferDlg::GetInstance()->todo.hosts);*/

//	CSLSKSpooferDlg::GetInstance()->SetTimer(1,1*1000,0);
	//CSLSKSpooferDlg::GetInstance()->SetTimer(2,60*1000,0);
//	CSLSKSpooferDlg::GetInstance()->SetTimer(3,5*60*1000,0);
//	CSLSKSpooferDlg::GetInstance()->SetTimer(6,5*1000,0);
//	CSLSKSpooferDlg::GetInstance()->SetTimer(5,30*1000,0); //checking idle modules
	//char* temp = new char[20];
	//sprintf(temp, "%d", (CTime::GetCurrentTime()).Format("%Y %m %d %H %M %S"));
//	strcpy(temp, (CTime::GetCurrentTime()).Format("%Y%m%d%H%M%S"));
//	Log(temp);
	m_dlg->SetTimer(1,1000,0);
	m_dlg->SetTimer(2,60*1000,0);
	m_dlg->SetTimer(3,5*60*1000,0);
	m_dlg->SetTimer(6,3*60*60*1000,0);
	
//	
}

void SLSKController::AddServerConnection(void)
{
	SupernodeHost nh;
	nh.SetIP("38.115.131.131");
	nh.setport(2240);
	m_connection_manager.server_ips.push_back(nh);
}

void SLSKController::stop(void)
{
	//WSocket::Cleanup();
	m_connection_manager.KillModules();
	//noise_manager.KillModules();
//	CSLSKSpooferDlg::GetInstance()->account_info.freeAccountInfo();
}

/*void SLSKController::GetProjects(void)
{
	int project_id;
	char* proj = new char[100];

	ifstream fin;
	fin.open("C:\\SLSKInfo\\projects.txt", ios::in);
	while(!fin.eof())
	{
		fin.getline(proj, 100);
		m_dlg->LogProjects(proj);
	}

	delete proj;
	fin.close();
}
*/
void SLSKController::UpdateProjects(void)
{
	int project_id;
	char* proj = new char[100];

	ifstream fin;
	fin.open("C:\\SLSKInfo\\projects.txt", ios::in);

	ofstream keywordout;
	keywordout.open("C:\\SLSKInfo\\keywords.txt", ios::out);

	ofstream piout;
	piout.open("C:\\SLSKInfo\\projectsinfo.txt", ios::out);

	db mydb;
	mydb.dbConnection("38.118.160.161", "onsystems", "ebertsux37", "project_management");


	while(!fin.eof())
	{
		fin.getline(proj, 100);

		CString query = "Select id, artist, album from projects where project_name = '";
		//char* temp = proj;
	
		query.Append(proj);
		query.Append("';");
	//	strcat(query, proj);//temp);
	//	strcat(query, "';");
		mydb.dbSQuery((char*)(LPCTSTR)query);



		MYSQL_RES *res_ptr;
	    res_ptr = mydb.getResult();
		MYSQL_ROW row;
		row = mysql_fetch_row(res_ptr);

		project_id = atoi(row[0]);

		piout << project_id << endl;
		piout << row[1] << endl;
		piout << row[2] << endl;

		mydb.clearResult();

		query = "Select * from weighted_ex_keywords where project_id = '";
		CString s;
		s.AppendFormat("%d", project_id);
		query.AppendFormat("%d", project_id);

		//strcat(query, (const char*)(LPCTSTR)s);
	//	strcat(query, "';");
		query.Append("';");
		mydb.dbSQuery((char*)(LPCTSTR)query);

		res_ptr = mydb.getResult();
		
		while (row = mysql_fetch_row(res_ptr))
		{
			keywordout << row[0] << "\n" << row[1] << "\n" << row[2] << endl;
		}

		mydb.clearResult();

		query = "Select track_number, track_name from track_listing where project_id = '";
	
		query.Append(s);
		query.Append("';");
	//	strcat(query, (const char*)(LPCTSTR)s);
	//	strcat(query, "';");
		mydb.dbSQuery((char*)(LPCTSTR)query);

		res_ptr = mydb.getResult();

		int count = mysql_num_rows(res_ptr);

		piout << count << endl;

		while(row = mysql_fetch_row(res_ptr))
		{
			piout << row[0] << "\n" << row[1] << endl;
		}

		mydb.clearResult();
	
	}

		delete proj;
		mydb.dbClose();
		fin.close();
		piout.close();
		keywordout.close();


}
void SLSKController::connectServer(void)
{
}

void SLSKController::connectParents(void)
{
	db mydb;
	mydb.dbConnection("localhost", "onsystems", "sumyungguy37", "slsk");
//	mydb.dbConnection(CSLSKSpooferDlg::GetInstance()->account_info.dbip, CSLSKSpooferDlg::GetInstance()->account_info.dbun, CSLSKSpooferDlg::GetInstance()->account_info.dbpass, CSLSKSpooferDlg::GetInstance()->account_info.dbname);
	mydb.dbSQuery("select parent_ips, parent_ports from parents;");
	MYSQL_RES *res_ptr;
	res_ptr = mydb.getResult();
	MYSQL_ROW row;
	
	int counter = 0;

	SupernodeHost h;
	while(row = mysql_fetch_row(res_ptr))
	{
		counter++;
        h.SetIP(row[0]);
//		h.setport((short)atoi(row[1]);
		m_connection_manager.parent_ips.push_back(h);
	}

	buffer tempstring;
	tempstring.PutInt(counter);
//	CString s = "number of parents = ";
//	s.AppendFormat("%d", counter);
	//char* str = counter;
	m_dlg->Log(tempstring.GetMutableCharPtr());
	mydb.clearResult();
	mydb.dbClose();
}

void SLSKController::connectNewParents(void)
{
}
//void SLSKController::InitAccountAll(void)
//{

//}

void SLSKController::initAccountInfo(void)
{
//	CSLSKSpooferDlg::GetInstance()->account_info.Init();

	//SLSKaccountInfo::GetInstance()->Init();


//	CSLSKSpooferDlg::GetInstance()->account_info.createNewAccountInfo();


//	CSLSKSpooferDlg::GetInstance()->account_info.setAccountInfo();

	//CSLSKSpooferDlg::GetInstance()->account_info.getUserAccounts();
//	account_info.un = account_info.getUN();
//	account_info.pass = account_info.getPass();
//	account_info.port = account_info.gePort();

/*	CSLSKSpooferDlg::GetInstance()->un_edit_box.SetWindowText(CSLSKSpooferDlg::GetInstance()->account_info.getUN());
	CSLSKSpooferDlg::GetInstance()->pass_edit_box.SetWindowText(CSLSKSpooferDlg::GetInstance()->account_info.getPass());
	CString s;
	s.AppendFormat("%d", CSLSKSpooferDlg::GetInstance()->account_info.getPort());
	CSLSKSpooferDlg::GetInstance()->port_edit_box.SetWindowText(s);*/
}

ProjectKeywordsVector& SLSKController::GetProjects(void)
{
	return m_dlg->m_projects;
}

/*vector<TrackInformation *> SLSKController::GetTracks(void)
{
	return m_dlg->m_track_info;
}*/

void SLSKController::OnTimer(UINT nIDEvent)
{
//	vector<SupernodeHost> temphost;
	CString s;
	UINT timer_ret=1;
	//CSLSKSpooferDlg::GetInstance()->KillTimer(nIDEvent);
	switch(nIDEvent)
	{
	case 1: // 1 second

	//	m_connection_manager.CalcParentConnections();
	//	m_connection_manager.CalcPeerConnections();
	//	m_connection_manager.CalcServerConnections();


		if(m_dlg->m_projects_changed == true)
		{
			m_connection_manager.SetNewInformation();
			m_dlg->m_projects_changed = false;
		}

		if (m_server_started == true)
		{
		if (m_connection_manager.server_ips.size() > 0)
		{
	
		//temphost = CSLSKSpooferDlg::GetInstance()->todo.hosts;
		
			m_connection_manager.ConnectNewHosts(m_connection_manager.server_ips, 1);
		}

		if (m_connection_manager.parent_ips.size() > 0)
		{
			m_connection_manager.ConnectNewHosts(m_connection_manager.parent_ips, 2);
		}

		if (m_connection_manager.tasks.size() > 0)
		{

			vector<SLSKtask *>::iterator iter = m_connection_manager.tasks.begin();

			for (int i = 0; i < (int)m_connection_manager.tasks.size(); i++, iter++)
			{
				m_connection_manager.ConnectNewHosts(m_connection_manager.tasks[i]);
				m_connection_manager.tasks.erase(iter);
			}
			//connection_manager.ConnectNewHosts(tasks);
		}
		}
	//	if (temphost.size() == 0)
	//		CSLSKSpooferDlg::GetInstance()->todo.hosts.clear();
	//	CSLSKSpooferDlg::GetInstance()->status_edit_box.GetWindowText(s);
	//	s.Append("Timer has been triggered\n");
	//	CSLSKSpooferDlg::GetInstance()->status_edit_box.SetWindowText(s);
		//connection_manager.TimerHasFired();
		m_dlg->SetTimer(nIDEvent,1*1000,0);
	//	CSLSKSpooferDlg::GetInstance()->SetTimer(nIDEvent,1*1000,0);
		break;
	case 2: // 1 minute
		m_dlg->SetTimer(nIDEvent,60*1000,0);
	//  	CSLSKSpooferDlg::GetInstance()->SetTimer(nIDEvent,60*1000,0);
		break;
	case 3: //5 minute
		m_dlg->SetTimer(nIDEvent,5*60*1000,0);
	//	CSLSKSpooferDlg::GetInstance()->SetTimer(nIDEvent,5*60*1000,0);
		break;
	case 4:
		break;
	case 5:
	//	m_connection_manager.CalcParentConnections();
	//	m_connection_manager.CalcPeerConnections();
	//	m_connection_manager.CalcServerConnections();
		m_dlg->SetTimer(nIDEvent, 10*1000,0);
		break;
	case 6:
		DWORD processid = GetCurrentProcessId();
		HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processid);
		TerminateProcess(hProcess, 0);
	//	m_dlg->EndDialog(0);
		break;
	}

}

void SLSKController::SendTask(SLSKtask *t)
{
}

void SLSKController::Log(const char* log)
{
	m_dlg->Log(log);
}

void SLSKController::setServerStatus(const char* status)
{
//	char *s = new char[strlen(status)];
//	strcpy(s, status);
	m_dlg->setServerStatus(status);
}

void SLSKController::setParentStatus(const char* status)
{
	m_dlg->setParentStatus(status);
}

void SLSKController::setPeerStatus(const char* status)
{
	m_dlg->setPeerStatus(status);
}

void SLSKController::setPeerConnectingStatus(const char* status)
{
	m_dlg->setPeerConnectingStatus(status);
}

void SLSKController::setParentConnectingStatus(const char* status)
{
	m_dlg->setParentConnectingStatus(status);
}

void SLSKController::setServerModStatus(const char* status)
{
	m_dlg->setServerModStatus(status);
}

void SLSKController::setParentModStatus(const char* status)
{
	m_dlg->setParentModStatus(status);
}

void SLSKController::setPeerModStatus(const char* status)
{
	m_dlg->setPeerModStatus(status);
}
