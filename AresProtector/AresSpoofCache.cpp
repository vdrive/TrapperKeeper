#include "StdAfx.h"
#include ".\aresspoofcache.h"
#include "..\tkcom\TinySQL.h"
#include <mmsystem.h>
#include "AresFile.h"
#include "aresspoofcache.h"
#include "AresProtectionSystem.h"
#include "AresPoolFile.h"

AresSpoofCache::AresSpoofCache(void)
{
	mb_ready=false;
}

AresSpoofCache::~AresSpoofCache(void)
{
}

UINT AresSpoofCache::Run(){
	srand(timeGetTime());

	int init_delay=10+(rand()%LOADDELAY);
	Timer startup;

	while(!startup.HasTimedOut(init_delay) & !this->b_killThread){
		Sleep(1000);
	}

	Timer last_update;
	bool b_has_updated=false;
	
	unsigned int original_seed=timeGetTime();

	AresProtectionSystemReference ref;
	//m_meta_artist=ref.System()->GetProjectInfoCache()->GetArtist(project);

	while(!this->b_killThread){
		Sleep(100);
		if(last_update.HasTimedOut(60*60*18) || !b_has_updated || (mv_file_groups.Size()==0 && last_update.HasTimedOut(60*60*3))){  //TYDEBUG

			int collisions=0;

			last_update.Refresh();
			b_has_updated=true;

			srand(timeGetTime());
			//generate X completely random hashes for use
			/*
			Vector v_random_hashes;
			int random_hash_index=0;
			for(int i=0;i<5000;i++){
				v_random_hashes.Add(new FileHashObject());
			}*/
			
			vector<bool> v_chances;
			int random_chance_index=0;
			for(int i=0;i<20000;i++){
				if(rand()&1)
					v_chances.push_back(true);
				else v_chances.push_back(false);
			}

			vector<int> v_random_ints;
			int random_int_index=0;
			for(int i=0;i<20000;i++){
				v_random_ints.push_back(rand());
			}
			

			/*
			TinySQL sql;
			if(!sql.Init("38.119.64.66","onsystems","tacobell","metamachine",3306)){
				ASSERT(0);
				continue;
			}

			if(!sql.Query("select project,file_name,file_size,track_id from false_decoy_table order by rand() limit 100",true)){  //TYDEBUG
				ASSERT(0);
				continue;  //don't clear our old spoofs if we can't get any new ones right now
			}


			Vector v_tmp;
			int collisions=0;
			Vector v_decoy_objects;
			for(int i=0;i<(int)sql.m_num_rows && !this->b_killThread;i++){
				CString project=sql.mpp_results[i][0].c_str();
				CString file_name=sql.mpp_results[i][1].c_str();
				file_name=file_name.Trim();
				CString lfile_name=file_name;
				lfile_name=lfile_name.MakeLower(); //ARESTRA
				int size=atoi(sql.mpp_results[i][2].c_str());
				int track=atoi(sql.mpp_results[i][3].c_str());
				if(file_name.GetLength()>90 || file_name.GetLength()<4 || lfile_name.Find("arestra")!=-1 || size<=0)
					continue;

				if(lfile_name.Find(".mp3")==-1 && lfile_name.Find(".wma")==-1 && lfile_name.Find(".avi")==-1 && lfile_name.Find(".mpg")==-1 && lfile_name.Find(".wmv")==-1)
					continue;

				DecoyObject *nd=new DecoyObject(project,file_name,track,size);

				int count=0;
				for(int j=0;j<(int)v_decoy_objects.Size() && count<2;j++){
					DecoyObject *tf=(DecoyObject*)v_decoy_objects.Get(j);
					if(stricmp(tf->m_project.c_str(),project)==0 && tf->m_track==track)
						count++;
				}

				if(count<2){
					v_decoy_objects.Add(nd);
				}
				else
					delete nd;
			}


			for(int i=0;i<(int)v_decoy_objects.Size() && !this->b_killThread;i++){
				if(i&1)
					Sleep(10);
				DecoyObject *tf=(DecoyObject*)v_decoy_objects.Get(i);

				//CString project=tf->;
				//CString file_name=sql.mpp_results[i][1].c_str();
				//file_name=file_name.Trim();
				CString lfile_name=tf->m_file_name.c_str();
				lfile_name=lfile_name.MakeLower(); //ARESTRA

				if(lfile_name.Find("natalie")!=-1 && lfile_name.Find("goin")!=-1){
					int x=5;
				}

				if(!ref.System()->GetProjectInfoCache()->IsProjectActive(tf->m_project.c_str()))
					continue;

				UINT size=atoi(sql.mpp_results[i][2].c_str());
				
				AddDecoyToList(v_tmp,tf->m_file_name.c_str(),tf->m_project.c_str(),tf->m_size,19,v_random_ints[(random_int_index++)%v_random_ints.size()]%3);
				AddDecoyToList(v_tmp,tf->m_file_name.c_str(),tf->m_project.c_str(),tf->m_size,18,v_random_ints[(random_int_index++)%v_random_ints.size()]%10);
				//AddDecoyToList(v_tmp,tf->m_file_name.c_str(),tf->m_project.c_str(),tf->m_size,16,v_random_ints[(random_int_index++)%v_random_ints.size()]%80);
				AddDecoyToList(v_tmp,tf->m_file_name.c_str(),tf->m_project.c_str(),tf->m_size,15,v_random_ints[(random_int_index++)%v_random_ints.size()]%255);
				
			}*/

			//LOAD SWARMS
			TinySQL ares_swarming_sql;
			ares_swarming_sql.Init("206.161.141.35","onsystems","ebertsux37","ares_data",3306);

			if(!ares_swarming_sql.Query("select file_name,hash,size,artist,title,priority,album from ares_swarm_table where priority=100 and size<20000000",true)){  //TYDEBUG
				ASSERT(0);
			}

			Vector v_special_files;
			Vector v_swarms;
			for(int swarm_index=0;swarm_index<(int)ares_swarming_sql.m_num_rows;swarm_index++){
				CString file_name=ares_swarming_sql.mpp_results[swarm_index][0].c_str();
				CString hash=ares_swarming_sql.mpp_results[swarm_index][1].c_str();

				//int priority=atoi(ares_swarming_sql.mpp_results[swarm_index][5].c_str());

				CString lfile_name=file_name;
				lfile_name=lfile_name.MakeLower(); //ARESTRA
				file_name=file_name.Trim();
				if(file_name.GetLength()>90)
					file_name=file_name.Right(90);
				if(file_name.GetLength()<4 || lfile_name.Find("arestra")!=-1 || (lfile_name.Right(3).Compare("mp3")!=0 && lfile_name.Right(3).Compare("wma")!=0))
					continue;

				//if(priority<5)
				//	priority=5;

				//if(priority>100)
				//	priority=100;

				//random chance to skip swarms based on their priority.  so we don't swarm unpopular stuff very hard
				//if((v_random_ints[(random_int_index++)%v_random_ints.size()]%100)>priority)
				//	continue;
				
				UINT size=atoi(ares_swarming_sql.mpp_results[swarm_index][2].c_str());

				CString artist=ares_swarming_sql.mpp_results[swarm_index][3].c_str();

				CString title=ares_swarming_sql.mpp_results[swarm_index][4].c_str();

				artist=artist.Trim();

				if(artist.GetLength()>90)
					artist=artist.Right(90);

				if(title.GetLength()>90)
					title=title.Right(90);

				artist=artist.Trim();
				title=title.Trim();

				if(hash.GetLength()==40 && size>0 && size<2000000000){
					byte byte_hash[20];
					
					for(int j=0;j<20;j++){
						char ch1=hash[j*2];
						char ch2=hash[j*2+1];
						byte val1=ConvertCharToInt(ch1);
						byte val2=ConvertCharToInt(ch2);
						byte hash_val=((val1&0xf)<<4)|(val2&0xf);
						byte_hash[j]=hash_val;
					}				

					//AresFile* naf=new AresFile(byte_hash,artist,title,file_name,"",size,180+rand()%120,false,true);
					AresFile* naf=new AresFile(byte_hash,artist,title,file_name,"",size,180+rand()%120,false,true);
					if(naf->mb_valid){
						v_special_files.Add(naf);
						v_swarms.Add(naf);
					}
					else delete naf;
				}
			}

			{
				CSingleLock lock(&m_lock,TRUE);
				for(int swarm_array_index=0;swarm_array_index<256;swarm_array_index++)
					mv_swarm_file_array[swarm_array_index].Clear();

				//index these swarms for rapid lookup later on
				for(int swarm_index=0;swarm_index<(int)v_swarms.Size();swarm_index++){
					AresFile* swarm=(AresFile*)v_swarms.Get(swarm_index);
					int insert_index=swarm->m_hash[0];
					mv_swarm_file_array[insert_index].Add(swarm);
				}
			}
			//END SPECIAL LOADING

//			Vector v_tmp2;  //now take v_tmp and randomly insert its elements into v_tmp2 so we can randomize the list of files so we don't get one user name sharing 100 versions of the same file name
//			for(int i=0;i<(int)v_tmp.Size() && !this->b_killThread && i<3000;i++){
//				v_tmp2.Insert(v_tmp.Get(i),rand()%max(1,v_tmp2.Size()));
//			}

			
			//if(v_tmp2.Size()>0){
				CSingleLock lock(&m_lock,TRUE);
				//mv_ares_files.Clear();
				//mv_ares_files.Copy(&v_tmp2);

				Vector v_tmp_groups;
				//int num_files=v_tmp2.Size();
				srand(timeGetTime());
				int orig_seed=rand()%10000;

				Vector v_tmp_special;
				v_tmp_special.Copy(&v_special_files);

				/*
				Vector *v_tmp_vector=new Vector();
				for(int j=0;j<5;j++){
					for(int i=0;i<200;i++){
						CString tmp_str;
						tmp_str.Format("ubertesttmp%d %d",j,i);
						CString tmp_file_name=tmp_str+".mp3";

						byte eberts_hash[20];
						srand(33+i+201*j);
						for(int j=0;j<20;j++){
							eberts_hash[j]=rand()%256;
						}

						v_tmp_vector->Insert(new AresFile(eberts_hash,"Ubertestthree",tmp_str,tmp_file_name,"Test Project",7655005,125,false,false),0);
						
					}
					v_tmp_groups.Add(v_tmp_vector);
				}*/

				
				//ADD DECOYS
				/*
				for(int file_index=0;file_index<(int)v_tmp2.Size();){
					Vector *v_tmp_vector=new Vector();

					int group_file_limit=215+rand()%15;
					for(int j=0;j<group_file_limit && file_index<num_files;j++){
						v_tmp_vector->Add(v_tmp2.Get(file_index));
						file_index++;
					}


					CString tmp_str;
					//add a test file

					tmp_str.Format("ubertestfile 732");
					CString tmp_file_name=tmp_str+".mp3";

					byte eberts_hash[20];
					srand(32);
					for(int j=0;j<20;j++){
						eberts_hash[j]=rand()%256;
					}

					v_tmp_vector->Insert(new AresFile(eberts_hash,"Ubertesttwo",tmp_str,tmp_file_name,"Test Project",7655005,125,false,false),0);

					//create some robots decoys
					srand(timeGetTime());

					//do robots
					vector<string> v_titles;
					vector<string> v_file_names;
					vector<string> v_authors;
					v_titles.push_back("Robots");
					v_titles.push_back("Robots - Pixar");
					v_titles.push_back("Robots (Spanish)");
					v_titles.push_back("Robots (Pixar)");
					v_titles.push_back("Robots 2005");
					v_titles.push_back("[TMD]Robots 2005");
					v_titles.push_back("robots.full.2005.freAk.TEAm");

					v_file_names.push_back("robots.avi");
					v_file_names.push_back("robots full.avi");
					v_file_names.push_back("robots complete.avi");
					v_file_names.push_back("robots 1of1.avi");
					v_file_names.push_back("robots.spa.avi");		

					v_authors.push_back("Tp-moviez");
					v_authors.push_back("Robots");
					v_authors.push_back("Saosin");
					v_authors.push_back("Blue Phoenix");
					DoSpecialMovie("ROBOTS-2005-03-03",v_file_names,v_titles,v_authors,1000,*v_tmp_vector);

					//do meet the fockers
					v_titles.clear();
					v_file_names.clear();
					v_authors.clear();
					v_titles.push_back("Meet The Fockers");
					v_titles.push_back("Meet the Fockers");
					v_titles.push_back("Meet the Fockers (Spanish)");
					v_titles.push_back("Meet the Fockers (Ita.)");
					v_titles.push_back("Meet.The.Fockers.Complete");
					v_titles.push_back("Meet The Fockers 2004");
					v_titles.push_back("Meet the fockers pot ts full");
					v_titles.push_back("Meet the fockers 1 of 2 ts");
					v_titles.push_back("Meet the fockers 2 of 2 ts");
					v_titles.push_back("Meet.The.Fockers.full.2004.freAk.TEAm");

					v_file_names.push_back("meet the fockers.mpg");
					v_file_names.push_back("Meet.The.Fockers.avi");
					v_file_names.push_back("meet the fockers.avi");
					v_file_names.push_back("[tmd]meet the fockers.avi");
					v_file_names.push_back("TMD Meet The Fockers.avi");		
					v_file_names.push_back("Tmd Meet The Fockers.avi");	

					v_authors.push_back("Tp-moviez");
					v_authors.push_back("Meet The Fockers");
					v_authors.push_back("Meet the Fockers");
					v_authors.push_back("Meet The Fockers TS");
					v_authors.push_back("Ben Stiller");
					v_authors.push_back("ANGEL");
					v_authors.push_back("Alex");
					v_authors.push_back("Mic");
					v_authors.push_back("funny");
					v_authors.push_back("very cool");
					v_authors.push_back("good");
					v_authors.push_back("Saosin");
					v_authors.push_back("Blue Phoenix");
					DoSpecialMovie("MEET THE FOCKERS-2004-08-13",v_file_names,v_titles,v_authors,1100,*v_tmp_vector);

					//do xxx state of the union
					v_titles.clear();
					v_file_names.clear();
					v_authors.clear();
					v_titles.push_back("XXX State Of The Union");
					v_titles.push_back("XXX.2.State.Of.The.Union");
					v_titles.push_back("XXX.2.State.Of.The.Union");
					v_titles.push_back("TMD XXX.State.Of.The.Union.SAOSiN.TC.2of2");
					v_titles.push_back("TMD XXX.State.Of.The.Union.SAOSiN.TC.1of2");
					v_titles.push_back("Xxx state of the union rip by 313sqn avi");

					v_file_names.push_back("XXX State Of The Union.mpg");
					v_file_names.push_back("XXX State Of The Union.avi");
					v_file_names.push_back("XXX.State.Of.The.Union.avi");
					v_file_names.push_back("[tmd]XXX State Of The Union.avi");
					v_file_names.push_back("TMD XXX State Of The Union.avi");		
					v_file_names.push_back("Tmd XXX State Of The Union.avi");	

					v_authors.push_back("Tp-moviez");
					v_authors.push_back("xxx2");
					v_authors.push_back("xxx 2");
					v_authors.push_back("Xxx");
					v_authors.push_back("Xxx - State of the Union");
					v_authors.push_back("Xxx - The Next Level");
					v_authors.push_back("DVD");
					v_authors.push_back("Saosin");
					v_authors.push_back("very cool");
					v_authors.push_back("good");
					v_authors.push_back("Blue Phoenix");
					DoSpecialMovie("XXX - STATE OF THE UNION-EXTRA 04-28",v_file_names,v_titles,v_authors,1200,*v_tmp_vector);

					//do fever pitch
					v_titles.clear();
					v_file_names.clear();
					v_authors.clear();
					v_titles.push_back("Starwars Episode iii - Revenge of the Sith");
					v_titles.push_back("Starwars Episode 3 Revenge of the Sith");
					v_titles.push_back("starwars.revenge.of.the.sith.saosan");
					v_titles.push_back("TMD Starwars Episode III - Revenge of the Sith");
					v_titles.push_back("[TMD] Starwars Episode III - Revenge of the Sith");
					v_titles.push_back("Starwars Ep. 3 (full movie)");
					v_titles.push_back("Starwars Episode 3 [2005] (full movie)");
					v_titles.push_back("Starwars Episode 3 (Spanish)");
					v_titles.push_back("Starwars Episode 3 (French)");
					v_titles.push_back("Starwars Episode 3 (German)");
					v_titles.push_back("Starwars Episode 3 (Italian)");
					v_titles.push_back("Starwars Episode 3 (Russian)");
					v_titles.push_back("Starwars Episode 3 [Complete]");
					v_titles.push_back("Starwars Episode 3 [Complete]");

					v_file_names.push_back("Starwars Ep 3.mpg");
					v_file_names.push_back("Starwars.Episode.3.avi");
					v_file_names.push_back("Starwars iii.avi");
					v_file_names.push_back("Starwars Episode 3 (full).avi");
					v_file_names.push_back("Starwars Revenge of the Sith.avi");		
					v_file_names.push_back("Starwars Episode 3 - Revenge of the Sith.avi");	

					v_authors.push_back("Lucas");
					v_authors.push_back("Starwars");
					v_authors.push_back("DVD");
					v_authors.push_back("TMD");
					v_authors.push_back("George Lucas");
					v_authors.push_back("Natalie Portman");
					v_authors.push_back("Awesome");
					v_authors.push_back("Cool");
					v_authors.push_back("Lucas 2005");
					v_authors.push_back("New");
					v_authors.push_back("Starwars!!!");
					v_authors.push_back("Lucas/Spielberg");
					v_authors.push_back("Complete");
					DoSpecialMovie("STAR WARS-EPISODE 3-REVENGE OF THE SITH-2005-05-19",v_file_names,v_titles,v_authors,1300,*v_tmp_vector);

					v_tmp_groups.Add(v_tmp_vector);
				}*/

				//ADD SWARMS

				//we need to wait for the decoy pool to get ready
				while(!this->b_killThread && !ref.System()->GetDecoyPool()->mb_ready){
					Sleep(50);
				}

				while(v_tmp_special.Size()>0 && !this->b_killThread){
					Vector *v_tmp_vector=new Vector();
					int group_file_limit=230+rand()%15;	

					for(;v_tmp_special.Size()>0 && v_tmp_vector->Size()<(UINT)group_file_limit;){
						int special_index=rand()%v_tmp_special.Size();
						AresFile *af=(AresFile*)v_tmp_special.Get(special_index);
						//v_tmp_vector->Add(af);  //not swarming any more for now, just decoying

						//for every swarm we add, lets add another file with the same metadata but a different hash for our decoy pool to push out stuff our AresSupernodes spoof
						Vector v_pool_tmp;
						ref.System()->GetDecoyPool()->GetNextDecoy(af->m_size,v_pool_tmp);
						AresPoolFile *apf=NULL;
						if(v_pool_tmp.Size()>0)
							apf=(AresPoolFile*)v_pool_tmp.Get(0);
						
						if(apf!=NULL){
							//borrowing lots of data from swarm to create a pool file, we will keep the pool file hash and size
							bool b_already_has_hash=false;
							//make sure we aren't adding the same hash twice
							for(int check_index=0;check_index<(int)v_tmp_vector->Size();check_index++){
								AresFile* af=(AresFile*)v_tmp_vector->Get(check_index);
								if(memcmp(af->m_hash,apf->m_hash,20)==0){
									b_already_has_hash=true;
									break;
								}
							}							
							if(!b_already_has_hash){
								apf->Init(apf->m_hash,af->m_meta_artist.c_str(),af->m_meta_title.c_str(),af->m_file_name.c_str(),af->m_project.c_str(),apf->m_size,af->m_media_length,false,true);
								//apf->Init(apf->m_hash,"nartist2","ntitle2","nfilename2.mp3","nproject2",apf->m_size,af->m_media_length,false,true);
								v_tmp_vector->Add(apf);
							}
						}
						

						v_tmp_special.Remove(special_index);
					}

					CString tmp_str;
					//add a test file

					tmp_str.Format("ubertestfile 736");
					CString tmp_file_name=tmp_str+".mp3";

					byte eberts_hash[20];
					srand(736);
					for(int j=0;j<20;j++){
						eberts_hash[j]=rand()%256;
					}

					v_tmp_vector->Insert(new AresFile(eberts_hash,"Ubertesttwo",tmp_str,tmp_file_name,"Test Project",7655005,125,false,false),0);

					v_tmp_groups.Add(v_tmp_vector);
				}

				mv_file_groups.Clear();
				mv_file_groups.Copy(&v_tmp_groups);
			//}

			TRACE("AresSpoofCache::Run() Loaded %d spoofs, %d file groups and had %d hash collisions\n",ares_swarming_sql.m_num_rows,mv_file_groups.Size(),collisions);
			mb_ready=true;
		}
		Sleep(100);
	}
	
	return 0;
}


bool AresSpoofCache::IsReady(void)
{
	return mb_ready;
}

int AresSpoofCache::SumString(const char* str)
{
	char *p=(char*)str;
	int str_sum=0;
	int count=1;
	while(*p){ //add up the sum of the ascii values in the letters of the file_name
		str_sum+=(count)**(p++);
		count++;
	}
	
	str_sum=str_sum%10000;
	return str_sum;
}

unsigned int AresSpoofCache::GetSpoofCount(void)
{
	CSingleLock lock(&m_lock,TRUE);
	int count=0;
	for(int i=0;i<(int)mv_file_groups.Size();i++){
		Vector *v=(Vector*)mv_file_groups.Get(i);
		count+=v->Size();
	}
	return count;
}

int AresSpoofCache::GetFileLength(byte* hash)
{
	int index=(int)hash[0];
	CSingleLock lock(&m_lock,TRUE);
	for(int i=0;i<(int)mv_swarm_file_array[index].Size();i++){
		AresFile *file=(AresFile*)mv_swarm_file_array[index].Get(i);
		if(memcmp(file->m_hash,hash,20)==0)
			return file->m_size;
	}
	return 0;
}

void AresSpoofCache::AddDecoyToList(Vector& v_tmp, const char* file_name, const char* project,int file_size,int hash_offset,int set_modifier)
{

	AresProtectionSystemReference ref;

	int name_len=(int)strlen(file_name);
	int proj_len=(int)strlen(project);

	
	int the_file_size=file_size+(hash_offset*10000)+set_modifier*(20000);

	while((the_file_size%137)!=0)
		the_file_size++;

	byte hash[20];
	ZeroMemory(hash,20);

	//set it to be the file name
	for(int i=0;i<name_len;i++){
		byte b=(byte)(file_name[i]);
		hash[i%20]=b;
	}

	//add in the project name to the hash
	for(int i=0;i<proj_len;i++){
		byte b=((int)hash[i%20]+(int)(project[i]))%255;
		hash[i%20]=b;
	}

	hash[hash_offset]=(byte)(((int)hash[hash_offset]+(int)set_modifier)%255);

	int media_length=180+(int)(hash[10]%40)+(int)(hash[11]%40);


	AresFile *new_file=NULL;
	string artist=ref.System()->GetProjectInfoCache()->GetArtist(project);

	new_file=new AresFile(hash,artist.c_str(),"",file_name,project,the_file_size,media_length,false,false); //set last var to true for debugging keywords

	if(new_file->mb_valid){
		bool b_add=true;

		//check for freak duplicate hashes
		for(int m=0;m<(int)v_tmp.Size();m++){
			AresFile *af=(AresFile*)v_tmp.Get(m);
			if(memcmp(af->m_hash,new_file->m_hash,20)==0){
				b_add=false;
				break;
			}
		}

		if(b_add){
			TRACE("Adding file:  %s\n",file_name);
			v_tmp.Add(new_file);
		}
		else delete new_file;
	}
	else{ 
		delete new_file;
	}
}

void AresSpoofCache::DoSpecialMovie(const char* project, vector<string>& v_file_names, vector<string>& v_titles, vector<string>& v_authors,int seed,Vector &v_tmp_vector)
{
	AresProtectionSystemReference ref;
	if(!(ref.System()->GetProjectInfoCache()->IsProjectActive(project)))
		return;

	byte eberts_hash[20];
	//create a really popular hash
	{
		
		srand(seed+rand()%4);  //choose one of two files for this one
		int tmp_file_size=650000000+10000*(rand()%10000)+rand();
		while((tmp_file_size%137)!=0){
			rand();
			tmp_file_size++;
		}

		//create some random hash to spoof
		for(int j=0;j<20;j++){
			eberts_hash[j]=rand()&255;
		}
		
		bool b_add=true;
		for(int m=0;m<(int)v_tmp_vector.Size();m++){
			AresFile *af=(AresFile*)v_tmp_vector.Get(m);
			if(memcmp(af->m_hash,eberts_hash,20)==0){
				b_add=false;
				break;
			}
		}

		if(b_add)
			v_tmp_vector.Insert(new AresFile(eberts_hash,v_authors[rand()%v_authors.size()].c_str(),v_titles[rand()%v_titles.size()].c_str(),v_file_names[rand()%v_file_names.size()].c_str(),project,tmp_file_size,125,false,false),0);
	}
	
	//create a really popular hash
	{
		
		srand(seed+4+rand()%8);  //choose one of eight files for this one
		int tmp_file_size=650000000+10000*(rand()%10000)+rand();
		while((tmp_file_size%137)!=0){
			rand();
			tmp_file_size++;
		}

		//create some random hash to spoof
		for(int j=0;j<20;j++){
			eberts_hash[j]=rand()&255;
		}
		
		bool b_add=true;
		for(int m=0;m<(int)v_tmp_vector.Size();m++){
			AresFile *af=(AresFile*)v_tmp_vector.Get(m);
			if(memcmp(af->m_hash,eberts_hash,20)==0){
				b_add=false;
				break;
			}
		}

		if(b_add)
			v_tmp_vector.Insert(new AresFile(eberts_hash,v_authors[rand()%v_authors.size()].c_str(),v_titles[rand()%v_titles.size()].c_str(),v_file_names[rand()%v_file_names.size()].c_str(),project,tmp_file_size,125,false,false),0);
	}
	

	//create a medium popular hash
	{
		srand(seed+13+rand()%30);
		int tmp_file_size=650000000+10000*(rand()%10000)+rand();
		while((tmp_file_size%137)!=0){
			rand();
			tmp_file_size++;
		}

		//create some random hashes to spoof
		for(int j=0;j<20;j++){
			eberts_hash[j]=rand()%256;
		}
		
		bool b_add=true;
		for(int m=0;m<(int)v_tmp_vector.Size();m++){
			AresFile *af=(AresFile*)v_tmp_vector.Get(m);
			if(memcmp(af->m_hash,eberts_hash,20)==0){
				b_add=false;
				break;
			}
		}

		if(b_add)
			v_tmp_vector.Insert(new AresFile(eberts_hash,v_authors[rand()%v_authors.size()].c_str(),v_titles[rand()%v_titles.size()].c_str(),v_file_names[rand()%v_file_names.size()].c_str(),project,tmp_file_size,125,false,false),0);
	}

	
	//create a totally random file that won't clump with anything
	{
		srand((timeGetTime()+seed)%30000);
		//create some random hash to spoof
		int tmp_file_size=650000000+10000*(rand()%10000)+rand();
		while((tmp_file_size%137)!=0){
			rand();
			tmp_file_size++;
		}

		//create some random hashes to spoof
		for(int j=0;j<20;j++){
			eberts_hash[j]=rand()%256;
		}
		
		bool b_add=true;
		for(int m=0;m<(int)v_tmp_vector.Size();m++){
			AresFile *af=(AresFile*)v_tmp_vector.Get(m);
			if(memcmp(af->m_hash,eberts_hash,20)==0){
				b_add=false;
				break;
			}
		}

		if(b_add)
			v_tmp_vector.Insert(new AresFile(eberts_hash,v_authors[rand()%v_authors.size()].c_str(),v_titles[rand()%v_titles.size()].c_str(),v_file_names[rand()%v_file_names.size()].c_str(),project,tmp_file_size,125,false,false),0);
	}
}
