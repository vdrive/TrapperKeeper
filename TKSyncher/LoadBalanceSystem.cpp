#include "StdAfx.h"
#include "loadbalancesystem.h"
#include <mmsystem.h>
#include "NameServerInterface.h"
#include "TinySQL.h"

LoadBalanceSystem::LoadBalanceSystem(void)
: m_main_source("Media.distribute","c:\\syncher\\file deposit\\distribute")
{
	m_file_limit=3000;
	mb_hosts_assigned=false;
}

LoadBalanceSystem::~LoadBalanceSystem(void)
{
}

LoadBalanceSystem::MapHolder* LoadBalanceSystem::GenerateMapHolder(){
	MapHolder *host=new MapHolder();
	host->m_media_map=m_main_source;  //set it equal to the main one, then we will prune things down from there
	host->m_swarmer_map=m_main_source;  //set it equal to the main one, then we will prune things down from there
	
	//do the media map loop to set the media makers projects
	for(int service_index=0;service_index<(int)host->m_media_map.mv_directories.size();service_index++){
		CString service=host->m_media_map.mv_directories[service_index].m_directory_name.c_str();  
		//service is now something like kazaa swarm etc
		int index=service.ReverseFind('\\');  //trim the full path from the service name
		if(index==-1){
			continue;
		}
		service=service.Mid(index+1);
		CString tmp=service;
		tmp=tmp.MakeLower();

		//if this service doesn't belong under the media map, discard it.
		if(tmp.Find("kazaa_swarm")!=-1){
			host->m_media_map.mv_directories.erase(host->m_media_map.mv_directories.begin()+service_index);
			service_index--;
			continue;
		}


		for(int project_index=0;project_index<(int)host->m_media_map.mv_directories[service_index].mv_directories.size();project_index++){
			CString project=host->m_media_map.mv_directories[service_index].mv_directories[project_index].m_directory_name.c_str();
			index=project.ReverseFind('\\');  //trim the full path from the project name
			if(index==-1){
				continue;
			}
			project=project.Mid(index+1);
			CString full_string=service+"\\"+project;
			int load=0;


			LoadBalanceSystem::ProjectInfo* pi=GetProjectInfo(project);
			if(pi){
				load=pi->m_decoy_load;
			}

			if(load==0){
				AddUnassignedProject(full_string);
				load=6;  //set to a minimum of 7 so we are protecting stuff a little in case there is an error.
			}

			//load% chance we keep this project for this service
			if((rand()%100)>load){
				host->m_media_map.mv_directories[service_index].mv_directories.erase(host->m_media_map.mv_directories[service_index].mv_directories.begin()+project_index);
				project_index--;
			}
		}
	}


	//do the swarmer map loop to set the swarmers projects
	for(int service_index=0;service_index<(int)host->m_swarmer_map.mv_directories.size();service_index++){
		CString service=host->m_swarmer_map.mv_directories[service_index].m_directory_name.c_str();  
		//service is now something like kazaa swarm etc
		int index=service.ReverseFind('\\');  //trim the full path from the service name
		if(index==-1){
			continue;
		}
		service=service.Mid(index+1);
		CString tmp=service;
		tmp=tmp.MakeLower();

		//if this service doesn't belong under the swarmer map, discard it.
		if(tmp.Find("kazaa_swarm")==-1){
			host->m_swarmer_map.mv_directories.erase(host->m_swarmer_map.mv_directories.begin()+service_index);
			service_index--;
			continue;
		}

		for(int project_index=0;project_index<(int)host->m_swarmer_map.mv_directories[service_index].mv_directories.size();project_index++){
			CString project=host->m_swarmer_map.mv_directories[service_index].mv_directories[project_index].m_directory_name.c_str();
			index=project.ReverseFind('\\');  //trim the full path from the project name
			if(index==-1){
				continue;
			}
			project=project.Mid(index+1);
			CString full_string=service+"\\"+project;
			int load=0;

			LoadBalanceSystem::ProjectInfo* pi=GetProjectInfo(project);
			if(pi){
				load=pi->m_swarm_load;
			}

			if(load==0){
				AddUnassignedProject(full_string);
				load=5;  //set to a minimum of 5 so we are protecting stuff a little in case there is an error.
			}

			//load% chance we keep this project for this service
			if((rand()%100)>load){
				host->m_swarmer_map.mv_directories[service_index].mv_directories.erase(host->m_swarmer_map.mv_directories[service_index].mv_directories.begin()+project_index);
				project_index--;
			}
		}
	}

	m_desired_average_rack_load+=(host->m_media_map.GetNumberOfFiles()+host->m_swarmer_map.GetNumberOfFiles());

	int trim_attempt=0;

	//track our original sizes, so we can maintain the ratio as we delete files
	int orig_decoy_size=host->m_media_map.GetNumberOfFiles();
	int orig_swarm_size=host->m_swarmer_map.GetNumberOfFiles();
	double original_ratio=(double)orig_decoy_size/(double)max(1,orig_swarm_size);
	double cur_ratio=original_ratio;

	//make sure we are obeying the file limit rules
	UINT cur_media_files=0;
	UINT cur_swarmer_files=0;
	while(((cur_media_files=(UINT)host->m_media_map.GetNumberOfFiles())+(cur_swarmer_files=(UINT)host->m_swarmer_map.GetNumberOfFiles()))>m_file_limit && trim_attempt++<15000){
		double cur_ratio=(double)cur_media_files/(double)max(1,cur_swarmer_files);
		
		if(cur_ratio>original_ratio){
			//ratio of decoys to swarms suggests we should try to remove a decoy project
			if(RemoveRandomProject(host,false)<1){
				RemoveRandomProject(host,true);
			}
		}
		else{
			//ratio of decoys to swarms suggests we should try to remove a swarmed file
			if(RemoveRandomProject(host,true)<1){
				RemoveRandomProject(host,false);
			}
		}
		//RemoveRandomProject(host);
	}
	m_actual_average_rack_load+=(host->m_media_map.GetNumberOfFiles()+host->m_swarmer_map.GetNumberOfFiles());

	m_main_source.CalculateDirectorySHA1(host->m_swarmer_map);  //update this maps sha1 so that clients on the network can accurately reference which map they have
	m_main_source.CalculateDirectorySHA1(host->m_media_map);  //update this maps sha1 so that clients on the network can accurately reference which map they have
	return host;
}

int LoadBalanceSystem::RemoveRandomProject(LoadBalanceSystem::MapHolder* host,bool b_swarm)
{
	if(b_swarm){
		if(host->m_swarmer_map.mv_directories.size()<1)
			return 0;
		int service_index=rand()%(int)host->m_swarmer_map.mv_directories.size();

		//make sure there are projects under this service
		if(host->m_swarmer_map.mv_directories[service_index].mv_directories.size()<1)
			return 0;	

		//choose a random project from this service to remove
		if(host->m_swarmer_map.mv_directories[service_index].mv_directories.size()>0){
			int project_index=rand()%(int)host->m_swarmer_map.mv_directories[service_index].mv_directories.size();
			//choose a random file from this service to remove
			if(host->m_swarmer_map.mv_directories[service_index].mv_directories[project_index].mv_files.size()>0){
				host->m_swarmer_map.mv_directories[service_index].mv_directories[project_index].mv_files.erase(host->m_swarmer_map.mv_directories[service_index].mv_directories[project_index].mv_files.begin()+(rand()%host->m_swarmer_map.mv_directories[service_index].mv_directories[project_index].mv_files.size()));
			}
			else{ //remove an empty directory
				host->m_swarmer_map.mv_directories[service_index].mv_directories.erase(host->m_swarmer_map.mv_directories[service_index].mv_directories.begin()+rand()%host->m_swarmer_map.mv_directories[service_index].mv_directories.size());	
			}

			//host->m_swarmer_map.mv_directories[service_index].mv_directories.erase(host->m_swarmer_map.mv_directories[service_index].mv_directories.begin()+rand()%host->m_swarmer_map.mv_directories[service_index].mv_directories.size());	
			return 1;
		}
	}
	else{
		if(host->m_media_map.mv_directories.size()<1)
			return 0;
		int service_index=rand()%(int)host->m_media_map.mv_directories.size();

		//make sure there are projects under this service
		if(host->m_media_map.mv_directories[service_index].mv_directories.size()<1)
			return 0;	

		//choose a random project from this service to remove
		if(host->m_media_map.mv_directories[service_index].mv_directories.size()>0){
			int remove_index=rand()%(int)host->m_media_map.mv_directories[service_index].mv_directories.size();
			int count=host->m_media_map.mv_directories[service_index].mv_directories[remove_index].GetNumberOfFiles();
			host->m_media_map.mv_directories[service_index].mv_directories.erase(host->m_media_map.mv_directories[service_index].mv_directories.begin()+remove_index);	
			return count;
		}
	}
	return 0;
}


UINT LoadBalanceSystem::Run(){

	TRACE("TKSyncher LoadBalanceSystem::Run().\n");
	//in this thread we will continually reassign files to all the racks
	//get the hosts from the name server here
	NameServerInterface name_server;
	CString tag_name="LOADBALANCER";
	vector <string> v_ips;
	name_server.RequestIP(CString("LOADBALANCER"),v_ips);  //request all ips related to the load balancer
	//create our hosts
	for(int i=0;i<(int)v_ips.size();i++){
		mv_hosts.Add(new FileHost(v_ips[i].c_str()));
	}
	mv_hosts.Sort(true);
	
	srand(timeGetTime());

	m_main_source.Update();  //scan our files in so we know what we are working with.
	TRACE("TKSyncher LoadBalanceSystem::Run() updated main source (scanned in files in \"c:\\syncher\\file deposit\\distribute\")\n");

	m_main_source.mv_files.clear();
	for(int i=0;i<(int)m_main_source.mv_directories.size();i++){
		m_main_source.mv_directories[i].mv_files.clear();  //none of the services should have any files directly under them
	}

	if(mv_hosts.Size()>0){
		//get priorities for each of the projects here and set up our data structures
		TinySQL m_sql;
		m_sql.Init("38.118.160.161","onsystems","ebertsux37","project_management",3306);
		m_sql.Query("select project_name,kazaa_decoy_load,kazaa_swarm_load from projects where active='A'",true);
		
		for(int i=0;i<(int)m_sql.m_num_rows;i++){
			ProjectInfo *pi=new ProjectInfo();
			pi->m_project_name=m_sql.mpp_results[i][0].c_str();
			pi->m_decoy_load=atoi(m_sql.mpp_results[i][1].c_str());
			pi->m_swarm_load=atoi(m_sql.mpp_results[i][2].c_str());
			TRACE("LoadBalanceSystem::Run() Adding project info for project %s with swarmer load %d and decoy load %d.\n",pi->m_project_name.c_str(),pi->m_swarm_load,pi->m_decoy_load);
			mv_project_info.Add(pi);
		}

		//generate X maps, distribute them amongst the little racks
		for(int i=0;i<min(180,(int)mv_hosts.Size());i++){
			mv_map_holders.Add(GenerateMapHolder());
		}
	}

	m_actual_average_rack_load=0;
	m_desired_average_rack_load=0;

	//go through our hosts here and assign their files
	for(int i=0;i<(int)mv_hosts.Size() && !this->b_killThread && mv_map_holders.Size()>0;i++){
		FileHost *host=(FileHost*)mv_hosts.Get(i);
		host->mp_map_holder=(LoadBalanceSystem::MapHolder*)mv_map_holders.Get(i%mv_map_holders.Size());  //evenly distribute the map holders amongst the racks
		//BalanceHostsFiles(host);
		Sleep(5);
	}
	TRACE("TKSyncher LoadBalanceSystem::Run() Assigned files to hosts\n");

	//compute the averages
	m_actual_average_rack_load/=max(1,mv_hosts.Size());    
	m_desired_average_rack_load/=max(1,mv_hosts.Size());

	mb_hosts_assigned=true;
	TRACE("TKSyncher LoadBalanceSystem::Run() Finished\n");

	return 0;
}

void LoadBalanceSystem::Shutdown(void)
{
	m_main_source.CancelUpdate();
	this->StopThread();
}

void LoadBalanceSystem::Startup(void)
{
	this->StartThread();
}

void LoadBalanceSystem::Update(TKComInterface *p_com)
{
	//iterate through a new host every x seconds and assign its files
	static Timer last_host_cycle;
	static Timer last_host_update;
	static int host_index=0;
	if(last_host_update.HasTimedOut(3) && mb_hosts_assigned){  //every 3 seconds we notify another rack about the directory structure it should acquire
		if(host_index>=(int)mv_hosts.Size() && last_host_cycle.HasTimedOut(60*10)){  //cycle through all the racks at most every 4 hours
			host_index=0;
			last_host_cycle.Refresh();
		}
		if(host_index<(int)mv_hosts.Size()){
			//Send a ping message
			FileHost *host=(FileHost*)mv_hosts.Get(host_index);
			host->SentPing();
			TRACE("TKSyncher LoadBalanceSystem::Update()  Updating host %s.\n",host->m_ip.c_str());
			TKSyncherInterface::IPInterface ping_message;
			ping_message.m_type=ping_message.LOADBALANCEPING;
			ping_message.v_strings.push_back(host->mp_map_holder->m_media_map.m_directory_name);
			byte buf[500];  //size for a later conversion to UDP
			UINT nw=ping_message.WriteToBuffer(buf);
			p_com->SendReliableData((char*)host->m_ip.c_str(),buf,nw);

			//send the sha message right after the ping message
			TKSyncherInterface::IPInterface media_sha_message;
			media_sha_message.m_type=media_sha_message.CURRENTSHA;
			media_sha_message.v_strings.push_back(string("Media.distribute"));
			media_sha_message.v_strings.push_back(host->mp_map_holder->m_media_map.m_sha1);
			//byte buf[500];  //size for a later conversion to UDP
			nw=media_sha_message.WriteToBuffer(buf);
			p_com->SendReliableData((char*)host->m_ip.c_str(),buf,nw);

			TKSyncherInterface::IPInterface swarmer_sha_message;
			swarmer_sha_message.m_type=swarmer_sha_message.CURRENTSHA;
			swarmer_sha_message.v_strings.push_back(string("Swarmer.distribute"));
			swarmer_sha_message.v_strings.push_back(host->mp_map_holder->m_swarmer_map.m_sha1);
			//byte buf[500];  //size for a later conversion to UDP
			nw=swarmer_sha_message.WriteToBuffer(buf);
			p_com->SendReliableData((char*)host->m_ip.c_str(),buf,nw);
		}
		host_index++;
	}	
}

byte* LoadBalanceSystem::GetHostMap(char* source_ip,const char* source_name,int& buffer_length)
{
	buffer_length=-1;
	TRACE("TKSyncher LoadBalanceSystem::Update()  host %s requested map for source %s.\n",source_ip,source_name);

	if(!mb_hosts_assigned){  //have we finished assigning files to all the hosts?
		return NULL;
	}

	for(int i=0;i<(int)mv_hosts.Size();i++){
		FileHost *host=(FileHost*)mv_hosts.Get(i);
		if(stricmp(source_ip,host->m_ip.c_str())==0){
			
			if(stricmp(source_name,"Media.distribute")==0){
				UINT ssize=(1<<18)+host->mp_map_holder->m_media_map.GetRequiredBufferSize();  //do an estimate of how much memory this will take
				//REMEMBER TO DELETE[] THIS.  That would be a memory leak from hell
				byte *buf=new byte[ssize];	//a 2 meg buffer to handle a worst case huge map scenarios
				buffer_length=(int)host->mp_map_holder->m_media_map.WriteToBuffer(buf);

				TRACE("TKSyncher LoadBalanceSystem::Update()  returning media map of length %d to host %s.\n",buffer_length,source_ip);
				return buf;
			}
			else if(stricmp(source_name,"Swarmer.distribute")==0){
				UINT ssize=(1<<18)+host->mp_map_holder->m_swarmer_map.GetRequiredBufferSize();  //do an estimate of how much memory this will take
				//REMEMBER TO DELETE[] THIS.  That would be a memory leak from hell
				byte *buf=new byte[ssize];	//a 2 meg buffer to handle a worst case huge map scenarios
				buffer_length=(int)host->mp_map_holder->m_swarmer_map.WriteToBuffer(buf);

				TRACE("TKSyncher LoadBalanceSystem::Update()  returning swarmer map of length %d to host %s.\n",buffer_length,source_ip);
				return buf;
			}
		}
	}
	return NULL;
}

/*
void LoadBalanceSystem::RenameFiles(TKSyncherMap& map)
{
	for(int i=0;i<(int)map.mv_directories.size();i++){
		RenameFiles(map.mv_directories[i]);
	}

	for(int i=0;i<(int)map.mv_files.size();i++){
		CString tmp=map.m_directory_name.c_str();
		tmp=tmp.MakeLower();
		//lie and say all the files have a path of c:\\syncher\\src\\Media.distribute\\x so that the old synchers know how to handle them
		tmp.Replace("c:\\syncher\\file deposit\\","c:\\syncher\\src\\Media.");
		map.m_directory_name=(LPCSTR)tmp;
		map.mv_files[i].m_name=(LPCSTR)tmp;
	}
}
*/


void LoadBalanceSystem::GotPingResponse(const char* source_ip)
{
	for(int i=0;i<(int)mv_hosts.Size();i++){
		FileHost *host=(FileHost*)mv_hosts.Get(i);
		if(stricmp(source_ip,host->m_ip.c_str())==0){
			TRACE("TKSyncher LoadBalanceSystem::Update()  host %s responded to ping.\n",source_ip);
			host->GotPingResponse();
			return;
		}
	}
}

void LoadBalanceSystem::FillProjectInfoTree(CTreeCtrl* p_tree)
{
	if(!mb_hosts_assigned){
		return;
	}

	p_tree->DeleteAllItems();  //clear the tree

	//various HTREEITEMS for references.
	HTREEITEM parent_node1,parent_node2,parent_node3;

	//go through each of the services
	for(int i=0;i<(int)m_main_source.mv_directories.size();i++){
		CString tmp;

		//set project name
		tmp.Format("%s (%d files)",m_main_source.mv_directories[i].m_directory_name.c_str(),m_main_source.mv_directories[i].GetNumberOfFiles());
		parent_node1=p_tree->InsertItem(tmp,0,0);

		//go through each of the projects
		for(int j=0;j<(int)m_main_source.mv_directories[i].mv_directories.size();j++){
			const char *project=m_main_source.mv_directories[i].mv_directories[j].m_directory_name.c_str();
			tmp.Format("%s (%d files)",project,m_main_source.mv_directories[i].mv_directories[j].GetNumberOfFiles());
			parent_node2=p_tree->InsertItem(tmp,0,0,parent_node1);
			
			vector <string> v_ips;
			vector <bool> v_alive;
			int alive_count=0;
			for(int rack_index=0;rack_index<(int)mv_hosts.Size();rack_index++){
				FileHost* host=(FileHost*)mv_hosts.Get(rack_index);
				if(host->HasProject(project)){
					v_ips.push_back(host->m_ip);
					if(host->IsDead())
						v_alive.push_back(false);
					else{
						v_alive.push_back(true);
						alive_count++;
					}
				}
			}


			tmp.Format("Assigned Racks (%d total, %d alive)",v_ips.size(),alive_count);

			
			parent_node3=p_tree->InsertItem((LPCSTR)tmp,0,0,parent_node2);
				
			//add all the racks that this project is assigned too
			for(int rack_index=0;rack_index<(int)v_ips.size();rack_index++){
				tmp.Format("%s %s",v_ips[rack_index].c_str(),(v_alive[rack_index])?"alive":"  dead");
				p_tree->InsertItem((LPCSTR)tmp,0,0,parent_node3);
			}
		}
	}

	p_tree->Invalidate(TRUE);
}

void LoadBalanceSystem::FillRackInfoTree(CTreeCtrl* p_tree)
{
	if(!mb_hosts_assigned){
		return;
	}

	p_tree->DeleteAllItems();  //clear the tree

	//various HTREEITEMS for references.
	HTREEITEM parent_node1,parent_node2,parent_node3;

	//go through each of the racks
	for(int i=0;i<(int)mv_hosts.Size();i++){
		FileHost* host=(FileHost*)mv_hosts.Get(i);

		//set project name
		CString tmp;
		tmp.Format("%s %s",host->m_ip.c_str(),(host->IsDead())?"  dead":"alive");
		parent_node1=p_tree->InsertItem((LPCSTR)tmp,0,0);

		tmp.Format("File Info (%d assigned files)",host->mp_map_holder->m_media_map.GetNumberOfFiles()+host->mp_map_holder->m_swarmer_map.GetNumberOfFiles());
		parent_node1=p_tree->InsertItem((LPCSTR)tmp,0,0,parent_node1);

		//for each of these hosts, enumerate its overall services
		for(int j=0;j<(int)host->mp_map_holder->m_media_map.mv_directories.size();j++){
			tmp.Format("%s (%d files)",host->mp_map_holder->m_media_map.mv_directories[j].m_directory_name.c_str(),host->mp_map_holder->m_media_map.mv_directories[j].GetNumberOfFiles());
			
			parent_node2=p_tree->InsertItem((LPCSTR)tmp,0,0,parent_node1);

			//for each of these services, enumerate the overall projects on it
			for(int project_index=0;project_index<(int)host->mp_map_holder->m_media_map.mv_directories[j].mv_directories.size();project_index++){
				tmp.Format("%s (%d files)",host->mp_map_holder->m_media_map.mv_directories[j].mv_directories[project_index].m_directory_name.c_str(),host->mp_map_holder->m_media_map.mv_directories[j].mv_directories[project_index].GetNumberOfFiles());
				parent_node3=p_tree->InsertItem((LPCSTR)tmp,0,0,parent_node2);
			}
		}

		//for each of these hosts, enumerate its overall services
		for(int j=0;j<(int)host->mp_map_holder->m_swarmer_map.mv_directories.size();j++){
			tmp.Format("%s (%d files)",host->mp_map_holder->m_swarmer_map.mv_directories[j].m_directory_name.c_str(),host->mp_map_holder->m_swarmer_map.mv_directories[j].GetNumberOfFiles());
			
			parent_node2=p_tree->InsertItem((LPCSTR)tmp,0,0,parent_node1);

			//for each of these services, enumerate the overall projects on it
			for(int project_index=0;project_index<(int)host->mp_map_holder->m_swarmer_map.mv_directories[j].mv_directories.size();project_index++){
				tmp.Format("%s (%d files)",host->mp_map_holder->m_swarmer_map.mv_directories[j].mv_directories[project_index].m_directory_name.c_str(),host->mp_map_holder->m_swarmer_map.mv_directories[j].mv_directories[project_index].GetNumberOfFiles());
				parent_node3=p_tree->InsertItem((LPCSTR)tmp,0,0,parent_node2);
			}
		}
	}

	p_tree->Invalidate(TRUE);
}

UINT LoadBalanceSystem::GetActualRackLoad(void)
{
	return m_actual_average_rack_load;
}

UINT LoadBalanceSystem::GetDesiredRackLoad(void)
{
	return m_desired_average_rack_load;
}

LoadBalanceSystem::ProjectInfo* LoadBalanceSystem::GetProjectInfo(const char* project_name)
{
	for(int i=0;i<(int)mv_project_info.Size();i++){
		ProjectInfo *pi=(ProjectInfo*)mv_project_info.Get(i);
		if(stricmp(pi->m_project_name.c_str(),project_name)==0){
			return pi;
		}
	}
	return NULL;
}

void LoadBalanceSystem::GetUnassignedProjects(vector<string> &v_projects)
{
	v_projects=mv_unassigned_projects;
}

void LoadBalanceSystem::AddUnassignedProject(const char* project_name)
{
	for(int i=0;i<(int)mv_unassigned_projects.size();i++){
		if(stricmp(mv_unassigned_projects[i].c_str(),project_name)==0){
			return;
		}
	}
	mv_unassigned_projects.push_back(string(project_name));
	TRACE("LoadBalanceSystem::AddUnassignedProject()  Adding project: %s.\n",project_name);
}
