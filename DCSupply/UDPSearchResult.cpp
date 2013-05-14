#include "StdAfx.h"
#include "udpsearchresult.h"
#include "dcsupplydlg.h"


UDPSearchResult::UDPSearchResult(void)
{
}

void UDPSearchResult::DataProcess()
{
	db_connection.InsertDCMasterData();
}
void UDPSearchResult::InitSocket(int socket)
{
	USocket::Startup();	//start up the socket
	int create_result=this->Create(socket);
	if(create_result!=0)	//create the socket
	{
		::AfxMessageBox("Error: Could not create socket");
	}
	//connect to database
	if(!db_connection.Connect("38.119.66.29","root","","mediadefender"))
	{
		::AfxMessageBox("Error: Could not create database connection");
	}
}
UDPSearchResult::~UDPSearchResult(void)
{
	this->Close();
	this->Cleanup();
	db_connection.Disconnect();	//disconnect from database
}
void UDPSearchResult::OnConnect(int error_code)
{
	if(error_code!=0)
	{
		::AfxMessageBox("Error Connecting");
	}
	else
	{
		::AfxMessageBox("Sucessfully Connected");
	}
}

void UDPSearchResult::OnReceive(int error_code)
{
	if(error_code!=0)
	{
		::AfxMessageBox("Error Receiving Data");
		return;	//return due to error
	}
	else
	{
		//error checking
		if(p_supply_keywords==0x00000000)
			return;
		//char buffer[1024];
		char *buffer=new char[1024];
		memset(filename,0,sizeof(filename));	//clear filename buffer
		memset(filesize,0,sizeof(filesize));	//clear filesize buffer
		memset(ip,0,sizeof(ip));			//clear ip buffer
		memset(user,0,sizeof(user));		//clear user buffer
		unsigned int num_filesize;	//size of the file
		CString cstr_user;
		DCSupplyDlg* window=(DCSupplyDlg*)p_window;	//set association to dcsupplydlg window

		unsigned int ip;
		unsigned short port;
		unsigned int num_read;
		bool get_username=false;	//flag whether username has been received
		int result=ReceiveFrom(buffer,1024,&ip,&port,&num_read);
		if(result==0)
		{
			buffer[num_read]='\0';	//add end to string
			if(strstr(buffer,"$SR")==NULL)	//check if $SR is in the string, if not return
				return;
			char* p_buffer=buffer;
			char* tok=strtok(p_buffer,"\\");
			while(tok!=NULL)
			{
				p_buffer=tok;
				//TRACE("BUFFER: %s\n",p_buffer);
				if(!get_username)
				{
					cstr_user=p_buffer;
					cstr_user.Replace("$SR ","");		//get the username
					get_username=true;	//flag we got username
				}
					tok=strtok(NULL,"\\");
			}

			int five_index=-1;
			for(unsigned int probe=0;probe<num_read;probe++)
			{
				if(p_buffer[probe]==0x5)
				{	
					if(p_buffer!=NULL)
					{
						strncpy(filename,p_buffer,probe);
						filename[probe]='\0';
					}
					five_index=probe;
					break;
				}
			}
			//filter file formats we dont' care about to stop processing data
			if(strstr(&filename[0],".bmp")!=NULL || strstr(&filename[0],".jpg")!=NULL || strstr(&filename[0],".pdf")!=NULL || strstr(&filename[0],".txt")!=NULL 
				|| strstr(&filename[0],".BMP")!=NULL || strstr(&filename[0],".JPG")!=NULL || strstr(&filename[0],".PDF")!=NULL || strstr(&filename[0],".TXT")!=NULL)
				return;		//found a filetype we don't care about

			int position=0;
			for(probe=probe+1;probe<num_read;probe++)
			{
				if(p_buffer[probe]==0x5 || p_buffer[probe]==' ')		//copying whole thing
				{
					if(p_buffer!=NULL)
					{
						//strncpy(filesize,p_buffer,probe);		//need to copy only filesize, not whole name
						filesize[probe]='\0';
					}
					five_index=probe;
					num_filesize=atoi(filesize);
					if(num_filesize<=0)
					{
						return;		//filesize is invalid
					}
					break;
				}
				filesize[position]=p_buffer[probe];
				position++;
			}
			//if(num_filesize>p_supply_keywords->m_supply_size_threshold)
			//{
			//	return;	//threshold wasn't met
			//}
			CString ip;
			tok=strtok(p_buffer," ");
			while(tok!=NULL)
			{
				ip=tok;
				tok=strtok(NULL," ");
			}
			ip.Replace("(","");
			ip.Replace(")","");
			ip.Replace("|","");
			position=ip.Find(":",0);
			if(position!=-1)
			{
				ip=ip.Left(position);
			}

			//vector<SupplyKeyword> current_supply_killword;
			//current_supply_killword=p_supply_keywords->v_killwords;
			char* found;
			for(UINT supply_killword_size=0;supply_killword_size<p_supply_keywords->v_killwords.size();supply_killword_size++)
			{
				for(UINT killword_size=0;killword_size<p_supply_keywords->v_killwords[supply_killword_size].v_keywords.size();killword_size++)
				{
					found=strstr(&filename[0],(char*)p_supply_keywords->v_killwords[supply_killword_size].v_keywords[killword_size]);
					if(found!=NULL)
					{
						return; //break;		//found killword, exit search criteria
					}
				}
			}
			UINT found_keyword=0;	//determines how many keywords have been found
			//vector<SupplyKeyword> current_supply_keyword;
			//current_supply_keyword = p_supply_keywords->v_keywords;
			for(UINT supply_keyword_size=0;supply_keyword_size<p_supply_keywords->v_keywords.size();supply_keyword_size++)
			{
				//TRACE("KEYWORD LIST STARTING\n");
				//TRACE("BUFFER: %s Search For: %s\n",p_buffer,p_supply_keywords->m_search_string.c_str());
				for(UINT keyword_size=0;keyword_size<p_supply_keywords->v_keywords[supply_keyword_size].v_keywords.size();keyword_size++)
				{
					found=strstr(&filename[0],(char*)p_supply_keywords->v_keywords[supply_keyword_size].v_keywords[keyword_size]);
					//TRACE("Comparing to: %s\n",(char*)current_supply_keyword[supply_keyword_size].v_keywords[keyword_size]);
					if(found!=NULL)
					{
						//TRACE("Found Keyword: %s\n",(char*)current_supply_keyword[supply_keyword_size].v_keywords[keyword_size]);
						found_keyword++;
					}
				}
				//TRACE("KEYWORD LIST DONE\n");
				
				if(found_keyword==p_supply_keywords->v_keywords[supply_keyword_size].v_keywords.size() && num_filesize>p_supply_keywords->m_supply_size_threshold)
				{
					//TRACE("Project Name: %s Track Name: %s\n",m_project_name,current_supply_keyword[supply_keyword_size].m_track_name.c_str());
					//TRACE("UDPSEARCHRESULT::ONRECEIVE: INSERT INTO RAW DATABASE %s threshold: %d\n",filename,p_supply_keywords->m_supply_size_threshold);
					CString test;
					test.Format("Project: %s filename: %s",m_project_name,filename);
					window->DBUpdate(test.GetBuffer(test.GetLength()));
					db_connection.InsertData(m_project_name,p_supply_keywords->v_keywords[supply_keyword_size].m_track_name.c_str(), p_supply_keywords->v_keywords[supply_keyword_size].m_track,filename,filesize,cstr_user,ip,"direct_connect_supply");
				}
				found_keyword=0;
			}
		}
		delete [] buffer;
	}
}

int UDPSearchResult::ReceiveFrom(void *data,unsigned int len,unsigned int *ip,unsigned short *port,unsigned int*num_read)
{
	return USocket::ReceiveFrom(data,len,ip,port,num_read);
}

void UDPSearchResult::OnSend(int error_code)
{
	
}
int UDPSearchResult::SendTo(void *data,unsigned int len,unsigned int ip,unsigned short port)
{
	return UAsyncSocket::SendTo(data,len,ip,port);
}

void UDPSearchResult::SetProjectData(SupplyKeywords &supply_keywords,char* project_name)
{
	p_supply_keywords=&supply_keywords;
	m_project_name=project_name;
}

void UDPSearchResult::SetDialog(void *dialog)
{
	p_window=dialog;
}

void UDPSearchResult::DCMasterInsert()
{
	db_connection.InsertDCMasterData();
}