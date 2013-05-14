#include "StdAfx.h"
#include "decoypool.h"
#include "..\tkcom\TinySQL.h"
#include "..\tkcom\Timer.h"
#include "..\MetaMachineSpoofer\DonkeyFile.h"
#include "KademliaProtectorSystem.h"
#include "..\tkcom\FrameHeader.h"
#include "KadUtilityFunctions.h"

DecoyPool::DecoyPool(void)
{
}

DecoyPool::~DecoyPool(void)
{
}

UINT DecoyPool::Run(void)
{
	Timer reload_timer;
	bool b_has_loaded=false;
	srand(GetTickCount());

	int pause_time=0;//rand()%600;  TYDEBUG
	Timer init_timer;

	while(!this->b_killThread && !init_timer.HasTimedOut(pause_time)){
		Sleep(100);
		continue;
	}

	while(!this->b_killThread){
		Sleep(100);

		//generate a new list of decoys
		if(reload_timer.HasTimedOut(6*60*60) || !b_has_loaded){
			reload_timer.Refresh();
			b_has_loaded=true;
			TinySQL sql;
			if(!sql.Init("63.216.246.62","onsystems","tacobell","metamachine",3306))
				continue;
			if(!sql.Query("select now()",true))
				continue;
			CString log_msg;
			//lets query a seed based on current month and day
			int seed=15;
			if(sql.m_num_rows>0){
				CString the_date=sql.mpp_results[0][0].c_str();
				CString month=the_date.Mid(5,2);
				CString day=the_date.Mid(8,2);
				TRACE("FalseDecoyData::Run() Creating false decoys: seed month was %s, seed day was %s\n",month,day);
				seed=(atoi(month)*32)+(atoi(day));
				seed/=7;
			}


////IMPORT
			//get ready to load in our precomputed files from our spoofers so that we can push them out from kademlia
			if(!sql.Query("select count(*) from reverse_decoy_pool",true)){  //TYDEBUG
				
				log_msg.Format("FAILED to query database for reverse_decoy_pool size because %s. (will retry later) ",sql.m_fail_reason.c_str());
				TRACE("%s\n",(LPCSTR)log_msg);
				continue;
			}

			if(sql.m_num_rows<1){
				log_msg.Format("FAILED to query database for reverse_decoy_pool size because Corrupt Count. (will retry later) ");
				TRACE("%s\n",(LPCSTR)log_msg);
				continue;
			}

			int count=atoi(sql.mpp_results[0][0].c_str());

			int NUM_FILES=4000;

			int offset=rand()%(max(1,max(1,count-(NUM_FILES/3))/100));
			offset*=100;  //offset was scaled down by 100 because rand() only goes up to 65k
		
			offset=max(0,offset-(NUM_FILES/3)); //we do this so that we have a good chance of selecting the ones in front and the back, since we are randoming for a range.  If we didn't do this there would be about 1/35000 chance that the start would be picked, not good
			offset=min(offset,max(0,count-(NUM_FILES)));

			CString query;
			//gather a large list of other files we need to collect
			query.Format("select signature,hash,hash_set,length from reverse_decoy_pool limit %u,%u",offset,NUM_FILES);

			if(!sql.Query(query,true)){  //TYDEBUG
				log_msg.Format("FAILED to query database for reverse_decoy_pool because %s. (will retry later) ",sql.m_fail_reason.c_str());
				TRACE("%s\n",(LPCSTR)log_msg);
				continue;
			}
			
////END IMPORT


			
//			if(!sql.Query("select signature,hash,hash_set,length from reverse_decoy_pool",true))
//				continue;

			
			byte sig[SIGNATURELENGTH];
			DonkeyFile tmp_df("",50000,sig,false,false,true);
			srand(seed);

			vector <UINT> v_music_sizes;
			vector <UINT> v_movie_sizes;

			int num_files=1000;

			for(int i=0;i<num_files;i++){
				int tmp_music_size=3000000+(5000*(rand()%1000))+rand();  //between 3 and 5 megs
				int tmp_movie_size=500000000+((rand()%400)*(rand()%1000)*(rand()%1000))+rand();  //between 500 and 900 megs
				while((tmp_music_size%137)!=0)
					tmp_music_size++;

				while((tmp_movie_size%137)!=0)
					tmp_movie_size++;

				v_music_sizes.push_back(tmp_music_size);
				v_movie_sizes.push_back(tmp_movie_size);
			}

			Vector v_tmp_files[256];

			//load all the preconfigured decoys into the system, so that we may serve them
			for(int i=0;i<(int)sql.m_num_rows;i++){
				if((i%50)==0){
					TRACE("Creating reverse decoy %d out of %d\n",i,sql.m_num_rows);
				}
				const char* str_hash=sql.mpp_results[i][1].c_str();
				const char* str_hash_set=sql.mpp_results[i][2].c_str();
				const char* str_sig=sql.mpp_results[i][0].c_str();
				UINT len=(UINT)atoi(sql.mpp_results[i][3].c_str());

				byte byte_hash[16];
				byte byte_sig[SIGNATURELENGTH];
				byte* byte_hash_set=NULL;
				
				for(int j=0;j<16;j++){
					char ch1=str_hash[j*2];
					char ch2=str_hash[j*2+1];
					byte val1=KadUtilityFunctions::ConvertCharToInt(ch1);
					byte val2=KadUtilityFunctions::ConvertCharToInt(ch2);
					byte hash_val=((val1&0xf)<<4)|(val2&0xf);
					byte_hash[j]=hash_val;
				}				

				for(int j=0;j<SIGNATURELENGTH;j++){
					char ch1=str_sig[j*2];
					char ch2=str_sig[j*2+1];
					byte val1=KadUtilityFunctions::ConvertCharToInt(ch1);
					byte val2=KadUtilityFunctions::ConvertCharToInt(ch2);
					byte hash_val=((val1&0xf)<<4)|(val2&0xf);
					byte_sig[j]=hash_val;
				}			

				int num_parts=len/PARTSIZE;
				if(num_parts>0){
					num_parts++;
				}

				if(num_parts>0 && len>=PARTSIZE){  //try to set the hash set data

					if(strlen(str_hash_set)!=num_parts*32){
						continue;  //we don't want to add this swarm, since we don't have all the data we need (hashset)
					}
					byte_hash_set=new byte[num_parts*16];
					for(int part_index=0;part_index<num_parts;part_index++){
						for(int j=0;j<16;j++){
							char ch1=str_hash_set[part_index*32+j*2];
							char ch2=str_hash_set[part_index*32+j*2+1];
							byte val1=KadUtilityFunctions::ConvertCharToInt(ch1);
							byte val2=KadUtilityFunctions::ConvertCharToInt(ch2);
							byte hash_val=((val1&0xf)<<4)|(val2&0xf);
							byte_hash_set[part_index*16+j]=hash_val;
						}				
					}		
				}



				DonkeyFile *df=new DonkeyFile("not available",byte_hash,len,byte_hash_set);
				df->mb_reverse_pool_file=true;
				memcpy(df->m_signature,byte_sig,SIGNATURELENGTH);

				//DEBUGGING
				/*
				if(byte_hash_set!=NULL && df->m_size>(PARTSIZE*5)){
					//byte check_hash[16];
					//df->CreateHashFromInput(NULL,NULL,num_parts*16,check_hash,byte_hash_set);
					//int stat_cmp=memcmp(check_hash,byte_hash,16);

					byte *tmp_buffer=new byte[PARTSIZE];  //temporary buffer to hold a lot of data
					byte check_part_hash0[16];  //hash of this predicted data
					byte check_part_hash1[16];  //hash of this predicted data
					df->PredictData(tmp_buffer,0,PARTSIZE);  //fill this temporary buffer
					df->CreateHashFromInput(NULL,NULL,PARTSIZE,check_part_hash0,tmp_buffer);
					df->PredictData(tmp_buffer,PARTSIZE,PARTSIZE);  //fill this temporary buffer
					df->CreateHashFromInput(NULL,NULL,PARTSIZE,check_part_hash1,tmp_buffer);

					//check the calculated hash against the real hash
					int stat_cmp0=memcmp((byte*)(byte_hash_set+(16*0)),check_part_hash0,16);
					int stat_cmp1=memcmp((byte*)(byte_hash_set+(16*1)),check_part_hash1,16);
					int stat_cmp2=memcmp(check_part_hash1,check_part_hash0,16);

					delete []tmp_buffer;
				}*/
				//END DEBUGGING
				
				v_tmp_files[df->m_hash[0]].Add(df);
				if(byte_hash_set!=NULL)
					delete []byte_hash_set;
			}

			//create X music decoys for our ed2k servers
			for(int i=0;i<num_files;i++){
				TRACE("Creating new music decoy %d...\n",i);
				CreateSignature(sig);
				DonkeyFile *df=new DonkeyFile("not available",v_music_sizes[i],sig,false,false,true);
				v_tmp_files[df->m_hash[0]].Add(df);
			}

			//create X movie decoys for our ed2k servers
			for(int i=0;i<(num_files>>2);i++){
				TRACE("Creating new movie decoy... %d\n",i);
				CreateSignature(sig);
				DonkeyFile *df=new DonkeyFile("",v_movie_sizes[i],sig,false,false,false);
				v_tmp_files[df->m_hash[0]].Add(df);
			}

			CSingleLock lock(&m_lock,FALSE);
			lock.Lock();
			for(int i=0;i<256;i++){
				mv_decoys[i].Copy(&v_tmp_files[i]);
			}
			lock.Unlock();
			TRACE("Finished creating decoys\n");
			//report these decoys to the database

			srand(GetTickCount());

			bool b_update_database=false;
			if((rand()%15)==0){ 
				b_update_database=true;
			}

			query="insert ignore into decoy_pool values ";
			int insert_count=0;
			KademliaProtectorSystemReference ref;
			bool b_cleared=false;
			for(int i=0;i<256;i++){
				for(int j=0;j<(int)mv_decoys[i].Size();j++){
					DonkeyFile *df=(DonkeyFile*)mv_decoys[i].Get(j);
					if(!b_cleared){
						b_cleared=true;
						ref.System()->GetKadUDPSystem()->GetPublisher()->UnpublishDecoys();
					}
					ref.System()->GetKadUDPSystem()->GetPublisher()->AddDecoyToPublishFileList(df);
					CString tmp;
					if(!df->mb_reverse_pool_file){  //don't want to insert these into the ed2k server pool if they don't belong
						if(insert_count++>0)
							tmp.Format(",('%s',%u,now())",df->m_shash.c_str(),df->m_size);
						else
							tmp.Format("('%s',%u,now())",df->m_shash.c_str(),df->m_size);
						query+=tmp;
					}
				}
			}
			if(insert_count>0 && b_update_database){
				TinySQL sql2;
				if(!sql2.Init("63.216.246.62","onsystems","tacobell","metamachine",3306))
					continue;

				sql2.Query("delete from decoy_pool where time_created<(now()-INTERVAL 1 DAY)",false);
				sql2.Query(query,false);
			}

		}
	}
	return 0;
}

void DecoyPool::CreateSignature(byte* sig)
{
	FillBufferWithMP3Data(sig,SIGNATURELENGTH);
	//for(int i=0;i<SIGNATURELENGTH;i++){
	//	sig[i]=(rand()%30);
	//}
}

bool DecoyPool::GetDecoyFile(Vector& v, byte* hash)
{
	CSingleLock lock(&m_lock,TRUE);
	for(int i=0;i<(int)mv_decoys[hash[0]].Size();i++){
		DonkeyFile *df=(DonkeyFile*)mv_decoys[hash[0]].Get(i);
		if(memcmp(df->m_hash,hash,16)==0){
			v.Add(df);
			return true;
		}
	}
	return false;
}

void DecoyPool::FillBufferWithMP3Data(byte* mp3_buffer, UINT mp3_buf_len)
{
	int NOISE_BUFFER_LEN	=((4*1024)+1);	// 4K + 1
	int NOISE_DATA_LEN		=(1<<18)-NOISE_BUFFER_LEN;	// 256K


	// Else use random noise
	int i;

	byte* p_noise_data_buf=new byte[NOISE_DATA_LEN+NOISE_BUFFER_LEN];
	memset(p_noise_data_buf,0,NOISE_DATA_LEN+NOISE_BUFFER_LEN);
	
	FrameHeader hdr(32,44.1);
	unsigned int frame_len=hdr.ReturnFrameSize();

	unsigned int offset=0;
	while(1)
	{
		if((NOISE_DATA_LEN-offset)<sizeof(FrameHeader))
		{
			break;
		}
		memcpy(&p_noise_data_buf[offset],&hdr,sizeof(FrameHeader));
		offset+=sizeof(FrameHeader);

		if((NOISE_DATA_LEN-offset)<frame_len)
		{
			break;
		}
			
		for(i=0;i<(int)frame_len-(int)sizeof(FrameHeader);i++)
		{
//			p_noise_data_buf[offset+i]=(i+rand()%2)%256;	// noisy hell
//			p_noise_data_buf[offset+i]=rand()%256;			// less frequent noise (on winamp, but not on mediaplayer)
			
			p_noise_data_buf[offset+i]=rand()%12;			// quiet noise
		}

		offset+=frame_len-sizeof(FrameHeader);
	}

	// Init remainder of dookie
	for(i=offset;i<NOISE_DATA_LEN;i++)
	{
		p_noise_data_buf[i]=rand()%256;
	}

	// Copy out the buffer padding
	memcpy(&p_noise_data_buf[NOISE_DATA_LEN],&p_noise_data_buf[0],NOISE_BUFFER_LEN);

	if(mp3_buf_len<(UINT)(NOISE_DATA_LEN+NOISE_BUFFER_LEN))
		memcpy(mp3_buffer,p_noise_data_buf,mp3_buf_len);

	delete []p_noise_data_buf;
}
