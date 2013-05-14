#include "StdAfx.h"
#include "falsedecoydata.h"
#include "SpoofingSystem.h"
#include "..\tkcom\tinysql.h"

FalseDecoyData::FalseDecoyData(void)
{
	mb_has_inited=false;
}

FalseDecoyData::~FalseDecoyData(void)
{
}

UINT FalseDecoyData::Run(){
	Timer last_decoy_get;
	bool b_has_got_decoys=false;
	srand(timeGetTime());

	int get_new_decoys_time=60*26+(rand()%7200);

	while(!this->b_killThread){
		Sleep(400);
		
		if(!last_decoy_get.HasTimedOut(get_new_decoys_time) && b_has_got_decoys)
			continue;
		{
			CSingleLock lock(&m_data_lock);
			mv_false_decoys.Clear();
		}
		srand(timeGetTime());
		MetaSpooferReference system_ref;

		system_ref.System()->LogToFile("FalseDecoyData::Run() updating decoys BEGIN");

		b_has_got_decoys=true;
		last_decoy_get.Refresh();
		
		// ***  LOAD FALSE DECOYS *****
		system_ref.System()->CleanDirectory("c:\\syncher\\rcv\\Edonkey False_Decoys");  
		TRACE("FalseDecoyData::Run() Loading False Decoys\n");
		TinySQL sql;
		sql.Init("38.119.64.66","onsystems","tacobell","metamachine",3306);
		sql.Query("select now()",true);
		//lets query a seed based on current month and day

		int seed=1;
		if(sql.m_num_rows>0){
			CString the_date=sql.mpp_results[0][0].c_str();
			CString month=the_date.Mid(5,2);
			CString day=the_date.Mid(8,2);
			TRACE("FalseDecoyData::Run() Creating false decoys: seed month was %s, seed day was %s\n",month,day);
			seed=(atoi(month)/6)*100+(atoi(day))/2;
		}

		if(INTERDICTION_ONLY){
			//sql.Query("select project,file_name,file_size from false_decoy_table order by rand() limit 0",true);
		}
		else{
			if(TYDEBUGMODE){
				sql.Query("select project,file_name,file_size,priority,hash_count,evil_file from false_decoy_table order by rand() limit 0",true);
			}
			else{
				sql.Query("select project,file_name,file_size,priority,hash_count,evil_file from false_decoy_table order by rand() limit 1500",true);
			}
		}
		
		srand(timeGetTime());

		vector <bool> v_buse_decoy;
		for(int i=0;i<(int)sql.m_num_rows;i++){
			int priority=atoi(sql.mpp_results[i][3].c_str());
			priority=max(10,priority);
			if((rand()%100)<=priority)
				v_buse_decoy.push_back(true);
			else 
				v_buse_decoy.push_back(false);
		}

		unsigned int orig_seed=rand();
		//UINT megs_created_so_far=0;
		UINT file_size_limit=(1<<31);  //2 gig file size limit

		if(sql.m_num_rows>0){
			system_ref.System()->ClearFalseDecoys();  //discard the old ones since we are about to read in new ones.  Otherwise the decoys will consume more and more data
		}

		for(int i=0;i<(int)sql.m_num_rows && !this->b_killThread;i++){
			bool b_is_big=false;
			bool b_is_super_big=false;
			string project=sql.mpp_results[i][0];
			string file_name=sql.mpp_results[i][1];
			UINT size=atoi(sql.mpp_results[i][2].c_str());
			UINT hash_count=atoi(sql.mpp_results[i][4].c_str());
			UINT evil_file=atoi(sql.mpp_results[i][5].c_str());

			bool b_evil_file=false;
			if(evil_file>0)
				b_evil_file=true;
			//hash_count=min(hash_count,30);
			//hash_count=max(hash_count,1);
			hash_count=7;
			if(!v_buse_decoy[i])  //see if we marked this one for ignoring
				continue;
			//UINT size_in_megs=((size/(1<<20))+1);
			if(size>file_size_limit){  //if its bigger than 2 gigs, ignore this file
				continue;
			}
			//if(size>10000000){  //if we are dealing with a big file... (have this check so we can continue making small music files if we go past our limit on movies)
 			//	if( megs_created_so_far + size_in_megs > decoy_allocation_limit)  //can we create this decoy and stay under our limit?
			//		continue;
			//}

			//megs_created_so_far+=size_in_megs;  //number of megs in this file plus one
			
			//v_projects.push_back(project);
			//v_files.push_back(file_name);
			//create the file on disk
	//		if(size>500000)
	//			size-=100000;
			
			
			
	//		if(size<9728000){
	//			if((rand()%4)==0){  //1 in 4 chance that we will inflate an mp3 up to a big size.
	//				size=9728002+(120*rand());
	//				while((size%137)!=0)
	//					size++;
	//			}
	//		}
			while((size%137)!=0)
				size++;
			//int seed=rand()%2;  //one of two different seeds equals 2 different hashes per filename

			for(int hash_index=0;hash_index<(int)hash_count;hash_index++){
				srand(orig_seed+=137);
				int low_limit=100-(5*hash_index);  //rapid drop off in popularity
				low_limit=max(3,low_limit);
				if(((rand()%100)<=low_limit)){
					CreateFalseDecoy(project.c_str(),file_name.c_str(),size,seed+hash_index,false,b_evil_file);
					Sleep(2);
				}
			}
		}

		srand(timeGetTime());

		CreateFalseDecoy("Test","ubertestsong.tst",8645285,seed+4,true,false);
		CreateFalseDecoy("Test","ubertestmovie.tst",828595285,seed+5,true,false);
		CreateFalseDecoy("Test","uberbestmovie.tst",818735221,seed+6,true,true);

		mb_has_inited=true;
		system_ref.System()->LogToFile("FalseDecoyData::Run() updating decoys END");
	}

	return 0;
}

void FalseDecoyData::CreateFalseDecoy(const char* project,const char* file_name, UINT file_size,int seed,bool const_seed,bool evil_file)
{
	//CString full_path;
	//CString path;
	//full_path.Format("c:\\syncher\\rcv\\Edonkey False_Decoys\\%s\\%s",directory,file_name);
	//path.Format("c:\\syncher\\rcv\\Edonkey False_Decoys\\%s",directory);

	int orig_seed=seed;

	while((file_size%137)!=0)
		file_size++;

	TRACE("SpoofingSystem::CreateFalseDecoy() STARTED %s file_size=%d seed=%d\n",file_name,file_size,seed);
	//CreateDirectory(path,NULL);
	//HANDLE file = CreateFile(full_path,			// open file at local_path 
    //            GENERIC_WRITE,              // open for writing 
    //            0,							// we don't like to share
    //            NULL,						// no security 
    //            CREATE_ALWAYS,				// create new whether file exists or not
    //            FILE_ATTRIBUTE_NORMAL,		// normal file 
    //            NULL);						// no attr. template 

	//if(file==INVALID_HANDLE_VALUE || file==NULL){
	//	CString log_msg;
	//	int error=GetLastError();

	//	TRACE("MetaSpoofer Spoofing System:  Failed to create synthetic file %s with error %d.\n",full_path,error);
	//	return;
	//}

	UINT amount_written=0;

	//byte buf[(1<<14)];

	char *p=(char*)file_name;
	int file_name_sum=0;
	int count=1;
	while(*p){ //add up the sum of the ascii values in the letters of the file_name
		file_name_sum+=(count/4)**(p++);
		count++;
	}
	p=(char*)project;
	count=1;
	while(*p){ //also add the sum of the ascii values in the letters of the directory
		file_name_sum+=(count/4)*(*(p++));
		count++;
	}
	seed+=file_name_sum;
	
	if(const_seed){
		seed=orig_seed;
	}

	srand(seed);  //a fairly unique seed for this file, and other servers will derive the same seed.  This seed will change as a function of the date.  So each time trapper keeper restarts (so long as it is a different day), a new unique file will be generated

	byte sig_buf[SIGNATURELENGTH];

	if(file_size>9000000){
		for(int i=0;i<SIGNATURELENGTH;i++){
			sig_buf[i]=(byte)(rand());  //random data
			byte b=sig_buf[i];
			int x=5;
		}
	}
	else{
		for(int i=0;i<SIGNATURELENGTH;i++){
			sig_buf[i]=(rand()&1);  //either a zero or a one
		}
	}
	
	//this will cause the creation of a new file that is unique, yet will have the ability to predict what its own data would be.
	DonkeyFile *new_false_decoy=new DonkeyFile(file_name,file_size,sig_buf,evil_file,evil_file&&const_seed);  //evil_file&&const_seed means that we are trying to create an evil test file

	if(const_seed)
		new_false_decoy->ForceCompletable();  //always make our test files completable (you can finish downloading them)

	new_false_decoy->SetProject(project);
	new_false_decoy->m_seed=seed;

	for(int i=0;i<(int)mv_false_decoys.Size();i++){
		DonkeyFile *tdf=(DonkeyFile*)mv_false_decoys.Get(i);
		if(memcmp(tdf->m_hash,new_false_decoy->m_hash,16)==0){
			for(int sigindex=0;sigindex<SIGNATURELENGTH;sigindex++){
				TRACE("MetaMachine (0) SIGNATURE COMPARE BYTE %d: 0x%02X - 0x%02X\n",sigindex,tdf->m_signature[sigindex],new_false_decoy->m_signature[sigindex]);
			}
			TRACE("MetaMachine (1) Discarding false decoy length %u because of matching hash %s.\n",file_size,new_false_decoy->m_shash.c_str());
			TRACE("MetaMachine (2) Discarded seed, length, file_name was %u, %u , %s\n",new_false_decoy->m_seed,new_false_decoy->m_size,new_false_decoy->m_file_name.c_str());
			TRACE("MetaMachine (3) Matching hash seed, length, file_name was %u, %u, %s.\n",tdf->m_seed,tdf->m_size,tdf->m_file_name.c_str());
			ASSERT(0);
			delete new_false_decoy;  //we aren't adding this to a vector so delete it.
			return;
		}
	}

	mv_false_decoys.Add(new_false_decoy);
	MetaSpooferReference system_ref;
	system_ref.System()->AddFalseDecoy(new_false_decoy);

	TRACE("SpoofingSystem::CreateFalseDecoy() FINISHED %s file_size=%d seed=%d\n",file_name,file_size,seed);
	//while(amount_written<file_size && !this->b_killThread){
	//	UINT amount_to_write=min((1<<14),file_size-amount_written);
	//	DWORD tmp;
	//	WriteFile(file,buf,amount_to_write,&tmp,NULL);
	//	amount_written+=amount_to_write;
	//	Sleep(0);  //let other programs run a little
	//}

	//close the file
	//CloseHandle(file);
}