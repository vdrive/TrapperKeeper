#include "StdAfx.h"
#include "dccommands.h"
#include <mmsystem.h>

DCCommands::DCCommands(void)
{
	//ip_address=new char[200];
}

DCCommands::~DCCommands(void)
{
	//delete [] ip_address;
}

//creates a random file size
char* DCCommands::GetSpoofFileSize()
{
	char buffer[200];
	//srand(timeGetTime());
	return itoa(rand()%3000000+1500000,buffer,10);
}

CString DCCommands::SendKey(char* data)	//returns all the lock and pk to establish client-server handshake
{
	char* pk;
	char* ptr;
	ptr = strstr(data,"$Lock");
	pk =strstr(data," Pk=");
	if(ptr==NULL || pk==NULL || ptr>pk)
	{
		return "Empty";
	}
	ptr=ptr+6;	//remove the $lock from string
	char lock[256];//lock[256];
	int i = 0;
	while(ptr != pk && i<256)
	{
		lock[i] = *ptr;
		ptr++;
		i++;
	}
	lock[i]='\0';	//end character
	CString lockstring = lock;
	m_key = "$Key ";
	m_key=m_key+m_keymanager.LocktoKey(lockstring);
	m_key=m_key+ "|$ValidateNick ";
	m_key=m_key+ "new_big_pirates|";
	return m_key;
}
void DCCommands::SenyMyInfo(char* data,char* str_myinfo)	//returns all $MyInfo string to send info to the hub
{
	char* MyInfo;
	MyInfo="$Version 1,0091|$GetNickList|$MyINFO $ALL new_big_pirates none<DC V:2.02,M:A,H:1/0/0,S:6,0:6>$ $LAN(T1).$none$85000000000$|";
	strcpy(str_myinfo,MyInfo);
	return;
}
//if error return false, else return true
bool DCCommands::SendMyNick(char* data,char* str_mynick)	//returns $MyNick info when a $connect to me is received
{
	char MyNick[200]="$MyNick";
	CString temp=data;
	int position = temp.Find("|",0);
	if(position<0)
		return false;
	temp=temp.Left(position);
	strcat(MyNick," new_big_pirates|$Lock ");
	strcat(MyNick,m_keymanager.CreateLock());
	strcat(MyNick," Pk=");
	strcat(MyNick,m_keymanager.CreatePK());
	strcat(MyNick,"$Version 1,0091|");
	//strcat(MyNick,"|");
	char* send_MyNick=&MyNick[0];
	strcpy(str_mynick,MyNick);
	return true;
}
void DCCommands::SendSRPasive(char* data,vector<string> &v_sr)		//returns the $SR command when a passive search is received
{
	/*char *tok=strtok(data,"|");
	char *search_for;
	bool found = false;
	while(tok!=NULL && found==false)
	{
		search_for=strstr(tok,"logilogo");	//create a local function that checks all the weights from the dc master
		if(search_for!=NULL)
		{
			data=tok;	//get the tok which has the search criteria
			found=true;
		}
			tok=strtok(NULL,"|");
	}

	if(search_for !=NULL)
	{
		CString temp=data;
		char *tok=strtok(temp.GetBuffer(temp.GetLength())," ");
		char *extract_hub="";
		char *extract_nick="";
		int tok_num=0;
		while(tok!=NULL && tok_num<1)
		{
			tok=strtok(NULL," ");
			extract_hub=tok;
			tok_num++;
		}

		tok=strtok(extract_hub,":");
		tok_num=0;
		while(tok!=NULL)
		{
			if(tok_num!=0)
				extract_nick=tok;	//hub name
			tok=strtok(NULL,":");
			tok_num++;
		}

		char SR[200];
		for(int x=0;x<NUM_SPOOFS;x++)
		{
			memset(SR,0,200);
			int* ptr= NULL;//(int*)&SR[0];
			char* ptr2=&SR[0];
			strcpy(SR,"$SR new_big_pirates dc\\shared files\\logilogo.mp3");	//file path
			ptr2 += strlen(SR);
			ptr=(int*)ptr2;
			*ptr = 0x5;	//include ascii char 5
			ptr2+=1;
			char file_size[100]="12345688 3/3";
			strcat(SR,"12345688 3/3");	//filesize
			ptr2 += strlen(file_size);
			ptr=(int*)ptr2;
			*ptr =0x05;
			ptr2+=1;
			strcat(SR,"triviahub.servemp3.com (213.112.70.30:411)");
			ptr2 +=strlen("triviahub.servemp3.com (213.112.70.30:411)");
			ptr=(int*)ptr2;
			*ptr = 0x05;
			strcat(SR,extract_nick);
			strcat(SR,"|");
			//char *str_sr;
			//strcpy(str_sr,SR);
			string temp_cstr=SR;
			v_sr.push_back(temp_cstr);
		}
		//v_SR.push_back(SR);	//push the char* to the vector
		//delete [] str_sr;
		return;	//return the vector
	}*/
	return ;	//return an error
}
void DCCommands::SendSRActive(char* data,vector<string> &v_sr,char* port,char* ip_address)		//returns the $SR command when an active search is received
{
	char *tok=strtok(data,"|");
	char *search_for;
	bool found=false;
	while(tok!=NULL && found==false)
	{
		//for loop for the kill words
		search_for=strstr(tok,"logilogo");
		if(search_for!=NULL)
		{
			data=tok;
			found=true;
			//break;
		}
		//
		tok=strtok(NULL,"|");
	}
	if(search_for !=NULL)
	{
		CString temp=data;
		char *tok=strtok(temp.GetBuffer(temp.GetLength())," ");
		char *extract_ip="";
		char *extract_port="";
		int tok_num=0;
		while(tok!=NULL && tok_num<1)
		{
			tok=strtok(NULL," ");
			extract_ip=tok;		//requestee's ip address;
			tok_num++;
		}

		tok=strtok(extract_ip,":");
		tok_num=0;
		while(tok!=NULL)
		{
			if(tok_num==0)
				extract_ip=tok;
			else
				extract_port=tok;
			tok=strtok(NULL,":");
			tok_num++;
		}
	//do multiple spoofing here, return a vector of $sr
		for(int i=0;i<NUM_SPOOFS;i++)
		{
		memset(SR,0,sizeof(SR));	//reset the char buffer
		int* ptr= NULL;//(int*)&SR[0];
		char* ptr2=&SR[0];
		strcpy(SR,"$SR someone_fu dc\\shared files\\logilogo.mp3");	//file path
		ptr2 += strlen(SR);
		ptr=(int*)ptr2;
		*ptr = 0x5;
		ptr2+=1;
		char file_size[100]="12345688 3/3";
		strcat(SR,"12345688 3/3");	//filesize
		ptr2 += strlen(file_size);
		ptr=(int*)ptr2;
		*ptr =0x05;
		ptr2+=1;
		strcat(SR," (130.111.249.113:411)");
		//strcpy(str_sr,SR);
		strcpy(ip_address,extract_ip);
		strcpy(port,extract_port);
		string struc_active;
		//struc_active.ip_address=extract_ip;
		//struc_active.port=extract_port;
		struc_active=SR;
		v_sr.push_back(struc_active);
		}
		return;
	}
	//strcpy(str_sr,"ERROR");
	return;

}
//
//
bool DCCommands::SendConnectToMe(char* data,char* str_connect)
{
	char Connect[200]="$ConnectToMe";
	CString temp=data;
	CString ip_address_temp;
	CString ip_address;
	int position = temp.Find("|",0);
	if(position<0)
		return false;
	temp=temp.Left(position);
	temp.Replace("|","");
	//memset(ip_address,0,sizeof(ip_address));	//clear contents of ip_address
	ip_address=temp.GetBuffer(temp.GetLength());
	//char* tok=strtok(ip_address," ");
	//while(tok!=NULL)
	//{
	//	ip_address=tok;
//		tok=strtok(NULL," ");
//	}
	int cur_pos=0;
	ip_address_temp=temp.Tokenize("' '",cur_pos);
	while(ip_address_temp!="")
	{
		ip_address=ip_address_temp;
		ip_address_temp=temp.Tokenize("' '",cur_pos);
	}
	strcat(Connect," new_big_pirates ");
	strcat(Connect,ip_address);
	strcat(Connect,"|");
	char* send_Connect=&Connect[0];
	strcpy(str_connect,Connect);
	return true;
}