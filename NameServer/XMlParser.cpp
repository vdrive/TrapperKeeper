#include "StdAfx.h"
#include "xmlparser.h"

XMlParser::XMlParser(void)
{
	v_rack_data.clear();	//clear vector list
}

XMlParser::~XMlParser(void)
{
}
//read the xml file and parses out extraneous data, puts valid data in vector
void XMlParser::ReadXMLFile()
{
	XMLSTRUCT xml_data;
	CStdioFile xmlFile;
	CString xml_line;
	//xmlFile.clear(0);
	CString ip;
	v_temp_data.clear();	//clears the vector
	v_rack_data.clear();	//clears the vector
	v_tag_vector.clear();	//clear the vector
	//xmlFile.Open("name_list.xml", CFile::modeRead|CFile::typeText|CFile::shareDenyNone);		//xml file to open
	//char line[100];				//used to get a line from file
	if(!xmlFile.Open("name_list.xml", CFile::modeRead|CFile::typeText|CFile::shareDenyNone))
		::AfxMessageBox("ERROR: File could not be opened");
	else
	{	
		xmlFile.ReadString(xml_line);
		//xmlFile.getline(line,100);
		//while(xml_line!="")
		while(xmlFile.ReadString(xml_line)!=NULL)
		{
			xml_data.entry_name="";
			xml_data.m_ip.clear();
			xml_data.m_tags.clear();
			//xml_line = line;

			if(xml_line.Find("<ENTRY NAME=")!=-1 || xml_line.Find("<ENTRY NAME= ")!=-1 || xml_line.Find("<ENTRY NAME = ")!=-1 || xml_line.Find("<ENTRY NAME =")!=-1)	//start rack tag
			{
				//xml_line = line;
				xml_line.Remove('\t');	//remove tab spaces
				xml_line.Remove('"');
				xml_line.Replace("<ENTRY NAME= ","");
				xml_line.Replace("<ENTRY NAME=","");
				xml_line.Replace("<ENTRY NAME =","");
				xml_line.Replace("<ENTRY NAME = ","");
				xml_line.Remove('>');
				xml_line.MakeUpper(); //patch
				xml_data.entry_name=xml_line;
				this->UniqueTags(xml_line);
				xmlFile.ReadString(xml_line);
					//xmlFile.getline(line,100);
				//xml_line = line;
				while(xml_line.Find("<IP>")!=-1 || xml_line.Find("<NAME>")!=-1 )
				{
					if(xml_line.Find("<IP>")!=-1)
					{
						xml_line.Remove('\t');
						xml_line.Replace("<IP>","");
						xml_line.Replace("</IP>","");
						if(xml_line.GetLength()>=7)
							xml_data.m_ip.push_back(xml_line);
					}

					if(xml_line.Find("<NAME>")!=-1)
					{
						xml_line.Remove('\t');
						xml_line.Replace("<NAME>","");
						xml_line.Replace("</NAME>","");
						xml_line.MakeUpper(); //patch
						xml_data.m_tags.push_back(xml_line);
						this->UniqueTags(xml_line);		//put tag in unique tag list if unique
					}
					//xmlFile.getline(line,100);
					xmlFile.ReadString(xml_line);
					//xml_line = line;
				}
				v_temp_data.push_back(xml_data);		//push back all the rack info
				//RAPHAELxmlFile.getline(line,100);
			}
			//xmlFile.ReadString(xml_line);
			//xmlFile.getline(line,100);	//added to read next line
		}
		xmlFile.Close();	//closes xml file
	}
	ComputeIPRange();	//computes the ranges of ips and associates them to a tag
	
	XMLSTRUCT temp;
	v_report.clear();
	for(UINT tag_counter=0;tag_counter<v_tag_vector.size();tag_counter++)
	{
		//temp.entry_name="";
		temp.m_vector_ip.clear();
		temp.entry_name=v_tag_vector[tag_counter];
		v_ip_result.clear();
		v_infinite_check.clear();
		v_search_ip.clear();
		this->ResolveNames(v_tag_vector[tag_counter]);
		for(UINT counter=0;counter<v_search_ip.size();counter++)
		{
			temp.m_vector_ip.push_back(this->GetIPStringFromIPInt(v_search_ip[counter]));
		}
		v_report.push_back(temp);
	}
}
//returns the correct vector for each app
vector<string> XMlParser::GetVector(CString tag)
{
	/*v_ip_result.clear();	//clear the vector that will be used
	v_infinite_check.clear();
	//m_duplicate_check.clear();	//clears the list to test duplicate
	v_search_ip.clear();
	this->ResolveNames(tag);	//fill the vector v_ip_result with ip addresses
	for(UINT i=0;i<v_search_ip.size();i++)
	{
		v_ip_result.push_back(this->GetIPStringFromIPInt(v_search_ip[i]));
	}
	return v_ip_result;		//return the vector associated to the tag*/

	for(UINT counter=0;counter<v_report.size();counter++)
	{
		//if(v_report[counter].entry_name==tag)
		if(v_report[counter].entry_name.CompareNoCase(tag)==0)
		{
			return v_report[counter].m_vector_ip;	//send the correct vector
		}
	}
	v_ip_result.clear();	//make it an empty list
	return v_ip_result;	//if not found send an empty vector
}
//will compute the range of ips
void XMlParser::ComputeIPRange()
{
	CString ip_inc;		
	char buff[25];		
	CString ip_des;		//destination ip string
	XMLSTRUCT dummy;	//dummy struct just to put ip in the vector
	CString ip;			//calculates ip for parsing
	//string parsing to get ip
	int source_ip1,source_ip2,source_ip3,source_ip4;
	int dest_ip1,dest_ip2,dest_ip3,dest_ip4;
	//TRACE("SIZE: %d",v_temp_data.size());
	for(UINT j=0;j<v_temp_data.size();j++)
	{
		for(UINT f=0;f<v_temp_data[j].m_ip.size();f++)
		{
			if(v_temp_data[j].m_ip[f].Find("-")!=-1)
			{
				//extracting all the info from the ip string and putting it in int values
				ip=v_temp_data[j].m_ip[f];
				sscanf(v_temp_data[j].m_ip[f],"%u.%u.%u.%u-%u.%u.%u.%u",&source_ip1,&source_ip2,&source_ip3,&source_ip4,&dest_ip1,&dest_ip2,&dest_ip3,&dest_ip4);
				v_dest.push_back((LPCSTR)ip.Right(ip.GetLength()-ip.Find("-")-1));	//keeps track of all the destination ip addresses to be looked up in database
				v_source.push_back((LPCSTR)ip.Left(ip.Find("-")));	//keeps track of all the source ip addresses to be looked up in database
				//TRACE("%d",dest_ip4);
				/*ip_des=ip.Right(ip.GetLength()-ip.Find("-")-1);
				dest_ip1=atoi(ip_des.Left(ip_des.Find(".")));
				ip_des=ip.Right(ip_des.GetLength()-ip_des.Find(".")-1);

				dest_ip2=atoi(ip_des.Left(ip_des.Find(".")));
				ip_des=ip.Right(ip_des.GetLength()-ip_des.Find(".")-1);

				dest_ip3=atoi(ip_des.Left(ip_des.Find(".")));
				ip_des=ip.Right(ip_des.GetLength()-ip_des.Find(".")-1);

				dest_ip4=atoi(ip_des);

				ip=ip.Left(ip.Find("-"));
				source_ip1=atoi(ip.Left(ip.Find(".")));
				ip=ip.Right(ip.GetLength()-ip.Find(".")-1);

				source_ip2=atoi(ip.Left(ip.Find(".")));
				ip=ip.Right(ip.GetLength()-ip.Find(".")-1);

				source_ip3=atoi(ip.Left(ip.Find(".")));
				ip=ip.Right(ip.GetLength()-ip.Find(".")-1);

				source_ip4=atoi(ip);*/
				int flag=0;

		//example, 60.XXX.XXX.XXX 60.50.123.50-60.51.50.50
		if(source_ip2<dest_ip2)
		{
			int ip2=source_ip2;
			int ip3=source_ip3;
			int ip4=source_ip4;
			int flag2=0;	
			//int size=0;

			for(int ip2=source_ip2;ip2<=dest_ip2;ip2++)
			{
				while(flag2!=1)		//flag that determines second least significant set of ip incremented past 255
				{
					while( flag!=1)		//flag that determines the least significant set of ip incremented past 255
					{	//follows format of ip address
						sprintf(buff,"%d.%d.%d.%d",source_ip1,ip2,ip3,ip4);
						//TRACE("\n%s %d",buff);
						dummy.m_tags.clear();
						ip4++;					//increment the fourth most sig. set
						//RAPHAEL dummy.m_one_ip = buff;
						dummy.m_int_ip=this->GetIntIPFromStringIP(buff);
						//put all tags for that entry in the vector
						for(UINT y=0;y<v_temp_data[j].m_tags.size();y++)
							dummy.m_tags.push_back(v_temp_data[j].m_tags[y]);
						dummy.entry_name=v_temp_data[j].entry_name;
						v_rack_data.push_back(dummy);		//pushes back all info
						if(ip4==dest_ip4+1 && ip3==dest_ip3)		//least sign. ip4 and ip3 have reached their destination
							flag=1;
						if(ip4==256)
						{
							ip4=0;
							flag=1;
						}
					}
					flag=0;		//finished calculating range
					if(ip3==dest_ip3+1 && ip2==dest_ip2)
						flag2=1;
					ip3++;			//increment second least significant ip set
					if(ip3==256)	//ip3 can't be larger than 255
					{
						ip3=0;
						flag2=1;	//set flag to exit while loop
					}
				}
				flag2=0;			//set flag to exit while loop
			}
		}
		//example, 64.60.XXX.XXX 64.60.60.0-64.60.61.123
		//if third most significant set source is lower than destination 
		if(source_ip3<dest_ip3)
		{
			int ip4=source_ip4;
			for(int x=source_ip3;x<=dest_ip3;x++)
			{
				while( flag!=1)
				{	//follows format of ip address
					sprintf(buff,"%d.%d.%d.%d",source_ip1,source_ip2,x,ip4);
					dummy.m_tags.clear();
					ip4++;					//increment the fourth most sig. set
					//RAPHAELdummy.m_one_ip = buff;
					dummy.m_int_ip=this->GetIntIPFromStringIP(buff);
					//put all tags for that entry in the vector
					for(UINT y=0;y<v_temp_data[j].m_tags.size();y++)
						dummy.m_tags.push_back(v_temp_data[j].m_tags[y]);
					dummy.entry_name=v_temp_data[j].entry_name;
					v_rack_data.push_back(dummy);		//pushes back all info
					if(ip4==dest_ip4+1 && x==dest_ip3)		//if finished set flag
						flag=1;
					if(ip4==256)
					{
						ip4=0;
						flag=1;
					}
				}
				flag=0;		//finished calculating range
			}
		}
		//123.456.789.XXX
		//fourth most significant set is the only set lower
		else if(source_ip3==dest_ip3)
		{
			while(source_ip4<=dest_ip4)
				{
					dummy.m_tags.clear();
					sprintf(buff,"%d.%d.%d.%d",source_ip1,source_ip2,source_ip3,source_ip4);
					//RAPHAEL dummy.m_one_ip =buff;
					dummy.m_int_ip=this->GetIntIPFromStringIP(buff);
					for(UINT y=0;y<v_temp_data[j].m_tags.size();y++)
						dummy.m_tags.push_back(v_temp_data[j].m_tags[y]);
					dummy.entry_name=v_temp_data[j].entry_name;
					v_rack_data.push_back(dummy);
					if(source_ip4<=255)	//make sure no higher than 255
						source_ip4++;
				}
			}
		}
			//if there is no range just one ip address
			else
			{
				//RAPHAELdummy.m_one_ip=v_temp_data[j].m_ip[f];			//copy single ip
				string ip_address=(LPCSTR)v_temp_data[j].m_ip[f];
				dummy.m_int_ip=this->GetIntIPFromStringIP(ip_address);
				v_dest.push_back(ip_address);
				v_source.push_back(ip_address);
				dummy.entry_name=v_temp_data[j].entry_name;		//copy the entry name
				dummy.m_tags.clear();							//clear all the tags
				for(UINT y=0;y<v_temp_data[j].m_tags.size();y++)	//copy all the tags associated to that entry
					dummy.m_tags.push_back(v_temp_data[j].m_tags[y]);
				v_rack_data.push_back(dummy);					//push data to main vector
			}
		}
	}
}

//returns a vector of unique tags to be displayed as roots
vector<CString> XMlParser::GetTags()
{
	return v_tag_vector;
}

//inserts unique tags into a vector
void XMlParser::UniqueTags(CString tag)
{
	bool insert=false;
	//if list is empty insert
	if(v_tag_vector.size()==0)
		v_tag_vector.push_back(tag);
	else
	{
		for(UINT x=0;x<v_tag_vector.size();x++)
		{
			//if found set flag
			if(v_tag_vector[x]==tag)
				insert=true;
		}
			//insert tag into vector
			if(!insert)
				v_tag_vector.push_back(tag);
	}
}
//will resolve all the name tag inheritance issues using recursion
void XMlParser::ResolveNames(CString entry_name)
{
	v_infinite_check.push_back(entry_name);		// push all entries into a vecor
	//m_duplicate_check.push_back(entry_name);
	//erase the beginning of the vector
	if(!v_tags_vector.empty())
	{
		v_tags_vector.erase(v_tags_vector.begin());
	}
	//go through all the data
	for(UINT i=0;i<v_temp_data.size();i++)
	{
		CString entry=v_temp_data[i].entry_name;
		//go through all entry names to see if it equals the entry tag
		if(v_temp_data[i].entry_name==entry_name)
		{
			for(UINT y=0;y<v_temp_data[i].m_tags.size();y++)
			{
				if(!this->InfiniteLoop(v_temp_data[i].m_tags[y]))	//check if infinite loop exists
				{
					v_tags_vector.push_back(v_temp_data[i].m_tags[y]);	//insert the entry in the main vector to later be checked with recursion
				}
				else
				{
					//Infinite loop detected
					CString error_message;
					error_message.Format("NAME SERVER ERROR: Infinite Loop Detected -> %s data is corrupted",v_temp_data[i].m_tags[y]);
					::AfxMessageBox(error_message);		//display error message
					//v_ip_result.clear();	//clear the whole vector of ips
					v_search_ip.clear();
					return;
				}
			}
				for(UINT x=0;x<v_rack_data.size();x++)
					{
						if(v_rack_data[x].entry_name==entry)
						{
							//int ip_int=this->GetIntIPFromStringIP(v_rack_data[x].m_one_ip);
							m_ip_int = v_rack_data[x].m_int_ip;
							m_found = binary_search(v_search_ip.begin(),v_search_ip.end(),m_ip_int);	//binary search for duplicates
							if(!m_found)
							{
								m_location = lower_bound(v_search_ip.begin(),v_search_ip.end(), m_ip_int);
								v_search_ip.insert(m_location,m_ip_int);			//insert in correct spot
							}
							/*if(!CheckDuplicate(v_rack_data[x].m_one_ip))
							{
								v_ip_result.push_back(v_rack_data[x].m_one_ip);	//put all ips for current entry in result vector
								}*/
						}
					}
		}
	}
		if(v_tags_vector.empty())				//base case: if the vector is empty stop recursion
			return ;

		ResolveNames(v_tags_vector.front());		//recursive call
	}

//returns true if loop detected, else returns false
bool XMlParser::InfiniteLoop(CString check_tag)
{
	int occurence=0;
	//check all 
	for(UINT count=0;count<v_infinite_check.size();count++)
	{
		if(v_infinite_check[count]==check_tag)
			occurence++;
	}
	//if more than two occurences in vector then an infinite loop exists
	if(occurence<=2)
		return false;	//loop not detected
	else
		return true;	//loop detected

}
//returns false if there is no duplicate
//returns true if duplicate has been found
/*bool XMlParser::CheckDuplicate(string check_duplicate)
{
	if(v_ip_result.size()==0)
		return false;
	for(UINT count=0;count<v_ip_result.size();count++)
	{
		if(v_ip_result[count].compare(check_duplicate)==0)
			return true;
	}
	/*int occurence=0;
	if(m_duplicate_check.size()==0)
		return false;
	for(UINT count=0;count<m_duplicate_check.size();count++)
	{
		if(check_duplicate.compare(m_duplicate_check[count])==0)
		{
			occurence++;
			if(occurence>=2)
				return true;
		}
	}
	return false;
}*/

int XMlParser::GetIntIPFromStringIP(string ip)
{
	int ip_int = 0;
	if(ip.length()==0)
		return ip_int;
	
	int ip1,ip2,ip3,ip4;
	sscanf(ip.c_str(),"%u.%u.%u.%u",&ip1,&ip2,&ip3,&ip4);
	ip_int = (ip1<<0)+(ip2<<8)+(ip3<<16)+(ip4<<24);
	return ip_int;
}

//
//
//
string XMlParser::GetIPStringFromIPInt(int ip_int)
{
	string ip;
	char ip_str[16];
	sprintf(ip_str,"%u.%u.%u.%u",(ip_int>>0)&0xFF,(ip_int>>8)&0xFF,(ip_int>>16)&0xFF,(ip_int>>24)&0xFF);
	ip = ip_str;
	return ip;
}

vector<string> XMlParser::GetIPDestVector()
{
	return v_dest;
}

vector<string> XMlParser::GetIPSourceVector()
{
	return v_source;
}