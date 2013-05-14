//AUTHOR:  Ty Heath
//LAST MODIFIED:  3.12.2003

#pragma once
#include "..\SamplePlugin\Interface.h"
#include <strstream>

//TO USE:
//1.  Derive a class from TKFileTransferInterface
//
//2.  Override the following methods (whichever ones you need):
//		void GetFileFailed(char* source , char* local_path , char* remote_path, char *reason ){}
//		void RecievedFile(char* source , char* local_path , char* remote_path ){}  //override this to be notified when you have recieved a new file, This only happens if you called GetFile().
//		void FileProgress(char *source , char *local_path , char *remote_path , float progress ){}  //override this to be notified once every so often of file transfer progress. //progress is a fraction between 0 and 1.  1=100%=done.  This object will only recieve progress notifications for file transfers initiated via GetFile().
//
//3.  Call 	GetFile( char *source, char* local_path , char* remote_path ); to get a file from the source.
//
//4.  Call Register() sometime after your DllStart function has been called (or during its call).



//definitions for internal file transfer service use.  
#define TKFILETRANSFERCODE 214
#define TKFILETRANSFERCODEADV 216

class TKFileTransferInterface : private Interface  //private inheritance to prevent anyone from overriding anything they shouldn't, like InterfaceReceivedData
{
//************************INTERNAL INTERFACE USE, IGNORE ************************//
	//A class for this interface to talk to the TKFileTransfer plugin (also used for trapperkeeper to trapperkeeper communication).  This is not for your use.
public:
	//IPInterface (Inter Process Interface) is a flexible data system that allows the communication of any number of strings, ints, floats, as well as a raw byte data buffer to exchange information from interface->plugin, plugin->interface, plugin->plugin, trapperkeeper->trapperkeeper.
	//NESTED CLASS:  IPInterface  (pay no attention to this class, it is for internal communication.  It is so that the interface and plugin can effectively make function calls to each other.  This also applies externally from trapper keeper to trapper keeper)
	class IPInterface{ 
	public:
		//*** the two members that should be in a class nested in file transfer interface ***//
		enum IPTYPE {GETFILE=1,FAILED,RECEIVEDFILE,PROGRESS,FILEDATA,FILEDATARECEIVED,FILETRANSFERDONE,RESUME=100, 
			P2PREQUESTPART=150,		//to request a part of a file
			P2PNOSUCHFILE,			//response saying the client has no such part of a file
			P2PFILEPART,			//response including the part of the file that was requested
			P2PREQUESTFILESTATUS,	//response requesting how much of a file that a host has
			P2PFILESTATUS,			//response saying how much of a file a host has
			P2PSERVERADDRESS,		//notification of indexing server address
			P2PFILELIST,			//a host notifying the index server which files it has
			P2PGETFILEHOSTS,		//a request to the index server for a list of hosts for a particular file
			P2PFILEHOSTS,			//a response to a requests for a list of hosts for a particular file
			P2PGETFILE,				//a request by a dll for a file
			P2PRECEIVEDFILE,			//a request by a dll for a file
			P2PEXISTS,				//a dll wanting to know if p2p is installed on this trapper keeper
		};
		byte m_type; //actually transmitted as a single byte for efficiency even though an enum is really an int.
		//***
		//*** general methods and members that should actually be in a class in the base interface system ***//
		vector <string> v_strings;  //only supports length up to 255 different strings
		vector <int> v_ints;	//only supports length up to 255 different ints
		vector <float> v_floats;	//only supports length up to 255 different floats

	private:
		byte* m_data;
		UINT m_data_length;
		bool mb_copy;

	public:
		//data shouldn't be deleted as this object will delete it
		void SetData(byte* data,UINT data_length){
			ASSERT(m_data==NULL);
			
			if(mb_copy){
				m_data=new byte[data_length];
				memcpy(m_data,data,data_length);
			}
			else{
				m_data=data;
			}
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
			mb_copy=false;
		}

		~IPInterface(){
			if(mb_copy && m_data!=NULL)
				delete[] m_data;
		}

		UINT WriteToBuffer(byte* buffer)
		{
			UINT pos=0;
			//write out strings
			buffer[pos++]=TKFILETRANSFERCODE;
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
			if(code!=TKFILETRANSFERCODE)
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
			pos+=4;
			if(icount>0){
				mb_copy=true;
				SetData(buffer+pos,icount);
			}
		
			return pos;
		}
	}; //end nested class IPInterface
	
	//This is a modified IPInterface that can handle large arrays of strings, ints and floats
	class IPInterfaceDataSet{ 
	public:
		//*** the two members that should be in a class nested in file transfer interface ***//
		//*** general methods and members that should actually be in a class in the base interface system ***//
		vector <string> v_strings;  //only supports length up to INT_MAX different strings
		vector <int> v_ints;	//only supports length up to INT_MAX different ints
		vector <float> v_floats;	//only supports length up to INT_MAX different floats

	public:


		UINT WriteToBuffer(byte* buffer)
		{
			UINT pos=0;
			//write out strings
			buffer[pos++]=(byte)TKFILETRANSFERCODEADV;
			*((UINT*)&buffer[pos])=(UINT)v_strings.size();
			pos+=sizeof(UINT);
			for(UINT i=0;i<v_strings.size();i++){
				strcpy((char*)(buffer+pos),v_strings[i].c_str());
				pos+=(UINT)(v_strings[i].size()+1);
			}

			*((UINT*)&buffer[pos])=(UINT)v_ints.size();
			pos+=sizeof(UINT);
			for(UINT i=0;i<v_ints.size();i++){
				*((UINT*)(buffer+pos))=v_ints[i];
				pos+=4;
			}

			*((UINT*)&buffer[pos])=(UINT)v_floats.size();
			pos+=sizeof(UINT);
			for(UINT i=0;i<v_floats.size();i++){
				*((float*)(buffer+pos))=v_floats[i];
				pos+=4;
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
			if(code!=TKFILETRANSFERCODEADV)
				return 0;

			UINT count1=*((UINT*)(buffer+pos));
			pos+=sizeof(UINT);
			for(UINT j=0;j<count1;j++){
				string tmp;
				for(UINT i=pos; buffer[i]!='\0';i++){
					tmp+=buffer[i];
				}
				v_strings.push_back(tmp);
				pos+=(UINT)(tmp.size()+1); //1 for the null char padding
			}

			UINT count2=*((UINT*)(buffer+pos));
			pos+=sizeof(UINT);
			for(UINT i=0;i<count2;i++){
				v_ints.push_back(*((UINT*)(buffer+pos)));
				pos+=4;
			}
			
			UINT count3=*((UINT*)(buffer+pos));
			pos+=sizeof(UINT);
			for(UINT i=0;i<count3;i++){
				v_floats.push_back(*((float*)(buffer+pos)));
				pos+=4;
			}
		
			return pos;
		}
	}; //end nested class IPInterface

private:
	static UINT session_id_counter;  //a session id counter that gets incremented within a plugin with every new TKFileTransferInterace object created
	UINT m_session_id;  //assigned from the session_id_counter, it uniquely describes this interface object within a single plugin
	bool mb_registered;  //flag for whether or not this interface has been registered

	static AppID m_file_transfer_app;  //declare the services that this interface talks to so I can reference it readably later on
	bool InterfaceReceivedData(AppID from_app_id, void* input_data, void* output_data);  //overriden from Interface and called when dll data comes in

	//*********************************END INTERNAL ATTRIBUTES/METHODS/CLASSES**********************************//

	//***************** USE THESE FUNCTIONS *****************************//
public:
	TKFileTransferInterface();
	~TKFileTransferInterface(void);

	//call this to send a file from here to the dest
	//void SendFile(char* dest, char* local_path, char* dest_path);

	//call this to get a file from the source and save it anywhere you want
	//It can return false for the following reasons:  The TKFileTransfer plugin isn't running, this interface hasn't registered yet, or this plugin has too many GetFiles happening concurrently.  A return of true doesn't necessarily mean you will receive the file, as you won't know that until it successfully finishes transfering.
	//If you are sure that you aren't transferring a LOT of files over a short time period, then there is no need to check this value other than for robustness.
	bool GetFile( const char *source, const char* local_path , const char* remote_path );

	//Call this to get a file via the p2p network, you must know the sha1 hash of the file though.
	bool P2PGetFile( const char *source, const char* local_path , const char* hash);

	//Nothing will work until Register is called.  It should be called anytime during or after your Dll's DllStart() method has been called by TrapperKeeper
	void Register(Dll* pDll);

	bool HasP2P();  //Call to determine if the file transfer service is p2p capable.  (an old dll may still be installed)

	//this function is for the load balancer
	//bool GetDistributeMap(TKSyncherMap *map);

//*************** OVERRIDE THESE FUNCTIONS IF YOU WANT ***************//
//Overide these methods via a derived class to handle the notifications provided by the File Transfer Service.
//Do not directly call these methods.  They will be called to notify you.
protected:  
	//override this to be notified when a SendFile request this object made is complete.
	//virtual void SendFileComplete(char* dest, char* local_path, char* dest_path){}

	//override this to be notified if a SendFile request this object made failed!
	//virtual void SendFileFailed(char* dest, char* local_path, char* dest_path, char *reason){}

	//override this to be notified if a GetFile request this object made failed, note P2PGetFile's can not fail.  They'll just never complete theoretically.
	virtual void GetFileFailed(char* source_ip , char* local_path , char* remote_path, char *reason ){}

	//override this to be notified when you have recieved a new file, This only happens if you called GetFile().
	virtual void RecievedFile(char* source_ip , char* local_path , char* remote_path ){}

	//override this to be notified when you have recieved a new p2p file, This only happens if you called P2PGetFile().
	virtual void P2PRecievedFile(char* local_path , const char* hash, UINT file_size ){}

	//override this to be notified once every so often of file transfer progress. (A notification actually happens everytime another peice of the file has been received from the source)
	//progress is a fraction between 0 and 1.  1=100%=done.  This object will only recieve progress notifications for file transfers initiated via GetFile().
	virtual void FileProgress(char *source_ip , char *local_path , char *remote_path , float progress ){}

	//override this to be notified once every so often of file transfer progress.
	//progress is a fraction between 0 and 1.  1=100%=done.  This object will only recieve progress notifications for file transfers initiated via SendFile().
	//virtual void SendFileProgress(char *dest, char *local_path, char *dest_path, float progress){}	
};