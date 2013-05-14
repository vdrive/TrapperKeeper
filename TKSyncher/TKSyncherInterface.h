#pragma once
#include "../SamplePlugIn/Interface.h"

//A utility class necessary for the TKSyncherInterface.  It is sharing the TKSyncherInterface .h and .cpp file for user convience.  (they only need to add one file to their project)
//This class encapsulates path data for any files that the syncher is managing.  
//The user may use a TKSyncherMap however they choose and copy and move the objects around.

//TO USE:
//1.  Derive a class from TKSyncherMap
//
//2.  Override the following methods (whichever ones you need):
//		void MapHasChanged(const char* source_ip){}  //override this to handle processing new files on a 1 by 1 basis.
//		void MapFinishedChanging(const char* source_ip){} //override this to handle notification that the map has finished changing.		
//
//3.  Call these functions to retreive current state.
//		TKSyncherMap RetrieveCurrentMap(void); //call to retrieve a current map of all files that are available
//		bool IsChanging(void);  //call to determine if files are scheduled to arrive but haven't arrived yet.
//
//4.  Call Register() sometime after your DllStart function has been called (or during its call).
#define TKSYNCHERCODE 215

class TKSyncherMap {
public:
	class TKFile{  //A nested class representing a file.
	public:
		string m_name; //full path  (for simplicity we will store the full path, when it could be derived from this objects location in the TKSynckerMap tree.  These won't be taking up enough memory or bandwidth to care about.)
		string m_sha1; //sha1 of this file (a 20 byte value representing this files digital fingerprint. Essentially, no two files will have the same sha1 if a single bit is different between them)
		byte m_flag; //use for whatever you want.

		/*
		inline HANDLE Open(){
			return CreateFile(m_name.c_str(), // open file at local_path 
								GENERIC_READ,              // open for reading 
								FILE_SHARE_READ,           // share for reading 
								NULL,                      // no security 
								OPEN_EXISTING,             // existing file only 
								FILE_ATTRIBUTE_NORMAL,     // normal file 
								NULL);                     // no attr. template 
		}*/

		//= operator.  about the same as a copy constructor
		TKFile& TKFile::operator=(const TKFile &file){
			this->m_name=file.m_name;
			this->m_sha1=file.m_sha1;
			this->m_flag=file.m_flag;
			return *this;
		}
		//copy constructor
		TKFile(const TKFile &file){
			this->m_name=file.m_name;
			this->m_sha1=file.m_sha1;
			this->m_flag=file.m_flag;
		}
		//default constructor
		TKFile(){m_flag=0;}

		//call this to see if this file is equal to another file
		inline bool IsEqual(const TKFile &file){
			if(stricmp(m_sha1.c_str(),file.m_sha1.c_str())==0 && stricmp(m_name.c_str(),file.m_name.c_str())==0){
				return true;
			}
			else return false;
		}
	};
	string m_sha1; //a sha of every sha in here.
	string m_directory_name;  //this is a full path.

	//The directories vector has been removed, and a single children vector has been added.  This is for the multisourcing.

	//A single sources files are all in the top directory.

	vector <TKSyncherMap> mv_directories;  //recursive definition.  Due to the multi source extension, some of these directories may not actually be in the parent directory.  These are themselves sources

	vector <TKFile> mv_files;  //these are full paths.

	//operator overloads for copying and moving map objects around.
	TKSyncherMap& TKSyncherMap::operator=(const TKSyncherMap &map);
	TKSyncherMap(const TKSyncherMap &map);
	TKSyncherMap();
	inline void Clear(){
		mv_directories.clear();
		mv_files.clear();
		//m_directory_name.clear();
		m_sha1.clear();
	}	

	//Call to determine if this map object is equal to another map object.  
	inline bool IsEqual(TKSyncherMap &other_map){
		//recursive function.
		if(mv_files.size()!=other_map.mv_files.size() || mv_directories.size()!=other_map.mv_directories.size())
			return false;
		for(UINT i=0;i<mv_directories.size();i++){
			if(!mv_directories[i].IsEqual(other_map.mv_directories[i]))
				return false;
		}
		for(UINT i=0;i<mv_files.size();i++){
			if(!mv_files[i].IsEqual(other_map.mv_files[i]))
				return false;
		}

		return true;
	}

	UINT WriteToBuffer(byte* buffer);
	UINT ReadFromBuffer(byte* buffer);
	bool HasFile(const char* name,const char* sha1);
	TKSyncherMap* GetDirectory(const char* dir_name);
	bool HasDirectory(const char* dir_name);  //returns true if the directory is this directory or one of the sub_directories
	bool IsEmpty(); //returns true if neither this directory and none of the subdirectories contain any files.
	bool RemoveFileFromMap(const char* file_name);
	bool RemoveDirectoryFromMap(const char* directory_name);
	void EnumerateAllFiles(vector <TKSyncherMap::TKFile>& v_files);
	UINT GetRequiredBufferSize();
	UINT GetNumberOfFiles(void);
};

//CLASS:  TKSyncherInterface  (derive your class from this class to utilize the syncher plugin)
class TKSyncherInterface : private Interface
{
public:
	//NESTED CLASS:  IPInterface  (pay no attention to this class, it is for internal communication.  It is so that the interface and plugin can effectively make function calls to each other)
	class IPInterface{ //a nested class for structured communication between this interface and the plugin, and between plugins
	public:
		//*** the two members that should be in a class nested in file transfer interface ***//
		//CURRENTSHA:  a source is reporting its current sha
		//REQUESTMAP:  a client is requesting a map of files, this is done in response to a clients sha being different than the sources.
		//CURRENTMAP:  this is a source's reponse to REQUESTMAP
		//REGISTER:  sent to plugin by an interface object to register.
		//DEREGISTER:  sent to plugin by an interface object to deregister.
		//ISCHANGING:  tells an interface to se b_changing to true
		//DELETEFILES:  source says that certain files were deleted/removed at the source.  a client should delete these same files
		enum IPTYPE {CURRENTSHA=1,REQUESTMAP,CURRENTMAP,REGISTER,DEREGISTER,ISCHANGING,DELETEFILES,MAPDONECHANGING,MAPHASCHANGED,SOURCERESCAN,PURGESOURCE=100,LOADBALANCEPING=150,LOADBALANCEPINGRESPONSE,TOTALANNIHILATION=200,CLEANDIRECTORY=201,DELETEFILE=202,RUNPROGRAM=203};
		byte m_type; //actually transmitted as a single byte for efficiency even though an enum is really an int.  It would allow for 255 different types.
		//***
		//*** general methods and members that should actually be in a class in the base interface system ***//
		vector <string> v_strings;
		vector <int> v_ints;
		vector <float> v_floats;

	private:
		byte* m_data;
		UINT m_data_length;

	public:
		void SetData(byte* data,UINT data_length){
			ASSERT(m_data==NULL);
			m_data=new byte[data_length];
			memcpy(m_data,data,data_length);
			m_data_length=data_length;
			//m_data=data;
		}		

		byte* GetData(UINT &data_length){
			data_length=m_data_length;
			return m_data;
		}

		IPInterface(){
			m_type=(byte)0;
			m_data=NULL;
			m_data_length=0;
		}

		~IPInterface(){
			if(m_data!=NULL)
				delete[] m_data;
		}

		UINT WriteToBuffer(byte* buffer)
		{
			UINT pos=0;
			//write out strings
			buffer[pos++]=TKSYNCHERCODE;
			buffer[pos++]=(byte)m_type;
			buffer[pos++]=(byte)v_strings.size();
			for(UINT i=0;i<v_strings.size();i++){
				strcpy((char*)(buffer+pos),v_strings[i].c_str());
				pos+=(UINT)(v_strings[i].size()+1);
			}

			buffer[pos++]=(byte)v_ints.size();
			for(UINT i=0;i<v_ints.size();i++){
				*((UINT*)(buffer+pos))=v_ints[i];
				pos+=4;
			}

			buffer[pos++]=(byte)v_floats.size();
			for(UINT i=0;i<v_floats.size();i++){
				*((float*)(buffer+pos))=v_floats[i];
				pos+=4;
			}


			*((UINT*)(buffer+pos))=m_data_length;
			ASSERT(m_data_length<50000000);
			pos+=4;
			
			if(m_data_length>0){
				memcpy(buffer+pos,m_data,m_data_length);
				pos+=m_data_length;
			}

			return pos;
		}

		UINT ReadFromBuffer(byte* buffer)
		{
			//istrstream input((char*)buffer,size);  //for some reason this class sucks balls so we can't use it to easily read our data back in
			//TODO:  make a working version of istrstream
			UINT pos=0;

			byte code=buffer[pos++];

			//input>>code;
			if(code!=TKSYNCHERCODE)
				return 0;
			code=buffer[pos++];
			
			m_type=(IPTYPE)code;  //a byte is plenty for the number of possible com types.

			byte count1;
			count1=buffer[pos++];
			for(int j=0;j<count1;j++){
				string tmp;
				for(UINT i=pos; buffer[i]!='\0';i++){
					tmp+=buffer[i];
				}
				v_strings.push_back(tmp);
				pos+=(UINT)(tmp.size()+1); //1 for the null char padding
			}

			byte count2=buffer[pos++];
			for(int i=0;i<count2;i++){
				v_ints.push_back(*((UINT*)(buffer+pos)));
				pos+=4;
			}
			
			byte count3=buffer[pos++];
			for(int i=0;i<count3;i++){
				v_floats.push_back(*((float*)(buffer+pos)));
				pos+=4;
			}

			//the byte vector needs a whole int to store its length unfortunately, as it is a gen purpose data buffer
			UINT icount=*((UINT*)(buffer+pos));
			ASSERT(icount<50000000);
			pos+=4;
			if(icount>0){
				SetData(buffer+pos,icount);
			}
			
			return pos;
		}
	};
private:
	//static AppID m_syncher_app;  //declare the services that this interface talks to so I can reference it readably later on
	bool InterfaceReceivedData(AppID from_app_id, void* input_data, void* output_data);
	//bool InterfaceReceivedData(AppID& from, void* data, UINT& size); //called by the 'system' to notify the interface object that data has been received

	bool mb_changing;
	bool mb_registered;
	TKSyncherMap m_current_map;
	string m_source;  //use a string to remember the source name vs a char* to avoid someone changing our sourcename or no telling what else.

	//The user may call these functions to access current and incoming syncher state
public:
	TKSyncherInterface();
	virtual ~TKSyncherInterface(void);

	//You must call Register before anything will work.
	//specify the source that you want this object to care about
	void Register(Dll* pDll,char *source);

	TKSyncherMap RetrieveCurrentMap(void); //call to retrieve a current map of all files that are available
	bool IsChanging(void);  //call to determine if files are scheduled to arrive but haven't arrived yet.
	
//OVERRIDABLES
	//override these to handle notifications.
	//do not call directly as they are called internally to notify YOU of changes.
protected:
	virtual void MapHasChanged(const char* source_ip){}  //override this to handle processing new files on a 1 by 1 basis.
	virtual void MapFinishedChanging(const char* source_ip){} //override this to handle notification that the map has finished changing.
public:
	void RescanSource(const char* source_name);
};

