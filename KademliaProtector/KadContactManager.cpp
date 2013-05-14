#include "StdAfx.h"
#include "kadcontactmanager.h"
#include "kadcontact.h"
#include "..\tkcom\timer.h"
#include <afxsock.h>		// MFC socket extensions
#include "UInt128.h"

#define SEARCHTOLERANCE				16777216

KadContactManager::KadContactManager(void)
{
	m_main_boot_strap_index=0;
	m_second_boot_strap_index=0;
	mp_save_contact_thread=NULL;
	m_status="Loading...";
}

KadContactManager::~KadContactManager(void)
{
}

void KadContactManager::AddContacts(Vector &v_contacts)
{
	CSingleLock lock(&m_lock,TRUE);
	for(int i=0;i<(int)v_contacts.Size();i++){
		KadContact *kc=(KadContact*)v_contacts.Get(i);
		byte *hash=kc->m_hash.getDataPtr();
		if((hash[7] == (hash[6]+7)) && (hash[8] == (hash[6]+13))){
			v_contacts.Remove(i);//one of ours, don't add as a contact
			i--;
			continue;
		}

		byte hash0=hash[3];
		//byte hash1=hash[1];

		/*
		int find_index=-1;
		
		for(int contact_index=0;contact_index<(int)mv_contacts[hash0].Size();contact_index++){
			KadContact *okc=(KadContact*)mv_contacts[hash0].Get(contact_index);
			if(okc->m_hash.compareTo(kc->m_hash)==0){
				find_index=contact_index;
				break;
			}
		}*/
		int find_index=mv_contacts[hash0].BinaryFind(kc);
		if(find_index==-1){
			find_index=mv_new_contacts.BinaryFind(kc);
			/*
			for(int contact_index=0;contact_index<(int)mv_new_contacts.Size();contact_index++){
				KadContact *okc=(KadContact*)mv_new_contacts.Get(contact_index);
				if(okc->m_hash.compareTo(kc->m_hash)==0){
					find_index=contact_index;
					break;
				}
			}*/

		}
		else{
			KadContact *okc=(KadContact*)mv_contacts[hash0].Get(find_index);
			okc->m_create_time=CTime::GetCurrentTime();  //refresh this contact
		}

		if(find_index!=-1){  //we already know about this contact
			v_contacts.Remove(i);
			i--;
			continue;
		}
	}

	for(int i=0;i<(int)v_contacts.Size();i++){
		KadContact *kc=(KadContact*)v_contacts.Get(i);
		for(int j=i+1;j<(int)v_contacts.Size();j++){
			KadContact *kc2=(KadContact*)v_contacts.Get(j);
			if(memcmp(kc->m_hash.getDataPtr(),kc2->m_hash.getDataPtr(),16)==0){
				v_contacts.Remove(j);
				j--;
			}
		}
	}


	if(v_contacts.Size()>0){
		for(int i=0;i<(int)v_contacts.Size();i++){
			KadContact *kc=(KadContact*)v_contacts.Get(i);
			mv_new_contacts.Add(kc);
		}
		mv_new_contacts.Sort(1);
	}
}


void KadContactManager::Update(void)
{
	static Timer last_contact_update;
	static Timer last_contact_purge;
	
	if((last_contact_update.HasTimedOut(60)) && mv_new_contacts.Size()>0){
		

		bool b_needs_sorted[256];
		for(int i=0;i<256;i++)
			b_needs_sorted[i]=false;

		/*
		for(int i=0;i<(int)mv_new_contacts.Size();i++){
			KadContact *nc=(KadContact*)mv_new_contacts.Get(i);
			for(int j=i+1;j<(int)mv_new_contacts.Size();j++){
				KadContact *nc2=(KadContact*)mv_new_contacts.Get(j);

				if(nc==nc2){
					int x=5;
					ASSERT(0);
				}
			}
		}*/

		CSingleLock lock(&m_lock,TRUE);

		for(int i=0;i<(int)mv_new_contacts.Size();i++){
			KadContact *nc=(KadContact*)mv_new_contacts.Get(i);
			byte *hash=nc->m_hash.getDataPtr();
			byte hash0=hash[3];
			mv_contacts[hash0].Add(nc);
			b_needs_sorted[hash0]=true;
		}

		for(int i=0;i<256;i++){  //sort the arrays that need sorted
			if(b_needs_sorted[i])
				mv_contacts[i].Sort(1);
		}

		//mv_contacts.Append(&mv_new_contacts);
		mv_new_contacts.Clear();

		//mv_contacts.Sort(1);
		last_contact_update.Refresh();
	}

	if(last_contact_purge.HasTimedOut(60*60)){
		bool b_changed=false;
		CSingleLock lock(&m_lock,TRUE);

		for(int j=0;j<256;j++){

			/*
			for(int i=0;i<(int)mv_contacts[j].Size() && mv_contacts[j].Size()>200;i++){
				KadContact *kc=(KadContact*)mv_contacts[j].Get(i);
				if(kc->IsExpired()){
					mv_contacts[j].Remove(i);
					i--;
				}
			}*/

			while(mv_contacts[j].Size()>2500){
				int rand_index=rand()%mv_contacts[j].Size();
				mv_contacts[j].Remove(rand_index);
			}
			
			/*
			if(mv_contacts[j].Size()>4000){
				//randomly pick a group to purge if we are getting a little unwieldy
				TRACE("KadContactManager::Update(void) Removing 1999 out of %d contacts from array %d due to oversize.\n",mv_contacts[j].Size(),j);
				mv_contacts[j].RemoveRange(rand()%(mv_contacts[j].Size()-2000),1999);
				TRACE("KadContactManager::Update(void) Contact removal complete.\n",mv_contacts[j].Size(),j);
			}*/
		}

		//TEST - CHECK FOR DUPLICATES
		/*
		for(int k=0;k<256;k++){
			for(int i=0;i<(int)mv_contacts[k].Size();i++){
				KadContact *kc=(KadContact*)mv_contacts[k].Get(i);
				for(int j=i+1;j<(int)mv_contacts[k].Size();j++){
					KadContact *kc2=(KadContact*)mv_contacts[k].Get(j);
					if(memcmp(kc->m_hash.getDataPtr(),kc2->m_hash.getDataPtr(),16)==0){
						int x=5;
 						ASSERT(0);
					}
				}
			}
		}*/


		SaveContactList();
		last_contact_purge.Refresh();
	}
}

void KadContactManager::SaveContactList(void)
{
	if(mp_save_contact_thread!=NULL){
		mp_save_contact_thread->StopThread();
		delete mp_save_contact_thread;
	}

	CSingleLock lock(&m_lock,TRUE);

	mp_save_contact_thread=new SaveContactThread(mv_contacts);
	mp_save_contact_thread->StartThreadLowPriority();
}

void KadContactManager::StartUp(void)
{
	CreateDirectory(("c:\\KadProtector"),NULL);

	HANDLE file = CreateFile(("c:\\KadProtector\\contacts.dat"),			// open file at local_path 
                GENERIC_READ,              // open for writing 
                0,							// we don't like to share
                NULL,						// no security 
                OPEN_EXISTING,				// create new whether file exists or not
                FILE_ATTRIBUTE_NORMAL,		// normal file 
                NULL);						// no attr. template 

	
	if(file!=INVALID_HANDLE_VALUE && file!=NULL){
		DWORD bleh=0;
		DWORD file_length=GetFileSize(file,&bleh);
		if(file_length<100){  //file corrupt for some reason
			CloseHandle(file);
			file=INVALID_HANDLE_VALUE;
			m_status="Error loading contacts, corrupt file[0].  Might recover.";
		}
	}

	if(file==INVALID_HANDLE_VALUE || file==NULL){
		//file doesn't exist, so lets open the default one that got synched
		file = CreateFile(("c:\\syncher\\rcv\\executables\\kadcontacts.dat"),			// open file at local_path 
					GENERIC_READ,              // open for writing 
					0,							// we don't like to share
					NULL,						// no security 
					OPEN_EXISTING,				// create new whether file exists or not
					FILE_ATTRIBUTE_NORMAL,		// normal file 
					NULL);						// no attr. template 
		if(file==INVALID_HANDLE_VALUE || file==NULL){
			CString log_msg;
			int error=GetLastError();
			//log_msg.Format("FAILED:  Couldn't create the file %s to received it from %s, system error code = %d.\n",m_local_path.c_str(),m_source.c_str(),GetLastError());
			//g_file_transfer_dialog.Log((char*)(LPCSTR)log_msg);
			TRACE("KadContactManager::StartUp(void) failed because it couldn't open the contact file.  Error code = %d\n",error);
			return;
		}
	}
	
	DWORD bleh=0;
	DWORD file_length=GetFileSize(file,&bleh);
	
	if(file_length==0 || file_length>10000000  || file_length<8){
		CloseHandle(file);
		m_status="Error loading contacts, file length is invalid.";
		DeleteFile("c:\\KadProtector\\contacts.dat");  //corruption removal technique
		return;
	}
	
	for(int i=0;i<256;i++)
		mv_contacts[i].Clear();

	byte *data=new byte[file_length];

	DWORD start_int=0;

	DWORD end_int=0;

	ReadFile(file,data,file_length,&bleh,NULL);
	byte *ptr=data;
	byte *end=data+file_length;
	start_int=*(DWORD*)ptr;
	end_int=*(DWORD*)(end-sizeof(DWORD));

	//DWORD start_int=2445652;
	//DWORD end_int=3445653;

	if(start_int!=2445652 || end_int!=3445653){
		CloseHandle(file);
		m_status="Error loading contacts, start and end keys are invalid.";
		DeleteFile("c:\\KadProtector\\contacts.dat");  //corruption removal technique
		return;
	}

	ptr+=sizeof(DWORD);

	end-=sizeof(DWORD); //ignore validation int we wrote on end

	Vector v_contacts;
	v_contacts.EnsureSize(100000);
	TRACE("Reading in contacts...\n");
	int count=0;
	while(ptr<end && count++<10000){
		KadContact *kc=new KadContact();
		ptr=kc->ReadFromBuffer(ptr);
		v_contacts.Add(kc);
	}
	TRACE("Finished reading contacts, adding them permanently\n");
	AddContacts(v_contacts);

	//we can go ahead and delete it (to fix corruption if it ever occurs)
	DeleteFile("c:\\KadProtector\\contacts.dat");  //corruption detection
	
	TRACE("KadContactManager::StartUp(void) added %d contacts.\n",v_contacts.Size()); 

	m_status="Finished Loading Contacts";

	if(data==NULL){
		m_status="Error loading contacts, couldn't allocate buffer.";
	}
	else{
		delete [] data;
	}
	CloseHandle(file);

	this->StartThreadLowPriority();
}

void KadContactManager::Shutdown(void)
{

	if(mp_save_contact_thread!=NULL){
		mp_save_contact_thread->StopThread();
		delete mp_save_contact_thread;
	}
	mp_save_contact_thread=NULL;

	this->StopThread();
}

void KadContactManager::GetNextBootStrapContact(Vector &v_tmp)
{

//	m_second_boot_strap_index=0;

	CSingleLock lock(&m_lock,TRUE);

	if(m_main_boot_strap_index>255)
		m_main_boot_strap_index=0;

	if(m_second_boot_strap_index>=(int)mv_contacts[m_main_boot_strap_index].Size()){
		m_main_boot_strap_index++;
		m_second_boot_strap_index=0;
	}

	if(m_main_boot_strap_index>255){
		m_main_boot_strap_index=0;
	}



	if(m_second_boot_strap_index>=(int)mv_contacts[m_main_boot_strap_index].Size()){
		return;
	}
	
	if(GetContactCount()==0){
		ASSERT(0);
	}


	v_tmp.Add(mv_contacts[m_main_boot_strap_index].Get(m_second_boot_strap_index++));
	//return (KadContact*)mv_contacts.Get(m_boot_strap_index++);
}

UINT KadContactManager::GetContactCount(void)
{
	int count=0;
	for(int i=0;i<256;i++){
		count+=mv_contacts[i].Size();
	}
	return count;
}

UINT KadContactManager::GetNewCachedContactCount(void)
{
	return mv_new_contacts.Size();
}

void KadContactManager::GetContactsForKey(CUInt128& key, Vector& v_contacts)
{
	TRACE("KadContactManager::GetContactsForKey() BEGIN\n");
	CSingleLock lock(&m_lock,TRUE);
	int contacts_added=0;

	byte key_zero=key.getDataPtr()[3];

	for(int i=0;i<(int)mv_contacts[key_zero].Size();i++){
		KadContact *kc=(KadContact*)mv_contacts[key_zero].Get(i);

		ULONG distance=key.calcDistance(kc->m_hash);
		if(distance>SEARCHTOLERANCE)
			continue;
		else{
			//ULONG val1=key.get32BitChunk(0);
			//ULONG val2=kc->m_hash.get32BitChunk(0);
			//TRACE("Found contact with distance %d, keys = %u,%u\n",distance,val1,val2);
			v_contacts.Add(kc);
		}
	}	
	TRACE("KadContactManager::GetContactsForKey() %d contacts found END\n",v_contacts.Size());
}

UINT KadContactManager::SaveContactThread::Run(){
	CreateDirectory(("c:\\KadProtector"),NULL);

	HANDLE file = CreateFile(("c:\\KadProtector\\contacts.dat"),			// open file at local_path 
                GENERIC_WRITE,              // open for writing 
                0,							// we don't like to share
                NULL,						// no security 
                CREATE_ALWAYS,				// create new whether file exists or not
                FILE_ATTRIBUTE_NORMAL,		// normal file 
                NULL);						// no attr. template 

	if(file==INVALID_HANDLE_VALUE || file==NULL){
		//something is seriously wrong
		return 0;
	}

	DWORD start_int=2445652;
	DWORD end_int=3445653;
	DWORD tmp;
	WriteFile(file,&start_int,sizeof(DWORD),&tmp,NULL);  //write an integer at the start of the data so we can check it to make sure the data is valid
	int count=0;
	for(int j=0;j<256;j++){
		int len=mv_contacts[j].Size();
		for(int i=0;i<len && i<150;i++){  //only need to save about X from each, just enough to bootstrap the next bootup
			count++;
			((KadContact*)mv_contacts[j].Get(i))->WriteToFile(file);
		}
	}

	WriteFile(file,&end_int,sizeof(DWORD),&tmp,NULL);  //write an integer at the end of the file so we can check it to see if we finished our last save operation.

	TRACE("KadContact::SaveContactThread::SaveContactList(void) %d contacts have been written to file \"c:\\KadProtector\\contacts.dat\".\n",count); 

	CloseHandle(file);	
	
	return 0;
}

KadContactManager::SaveContactThread::SaveContactThread(Vector *v_contacts){
	for(int i=0;i<256;i++){
		mv_contacts[i].Copy(&v_contacts[i]);
	}
}
char* KadContactManager::GetContactLoadStatus(void)
{
	return m_status;
}

UINT KadContactManager::Run(void)
{
	while(!this->b_killThread){
		this->Update();
		Sleep(500);
	}
	return 0;
}
