#pragma once
#include "..\tkcom\Buffer2000.h"

class MetaSpooferUtilityFunctions
{
public:
	MetaSpooferUtilityFunctions(void);
	~MetaSpooferUtilityFunctions(void);

	static void WriteEmuleTag(int val,byte cmd,Buffer2000& output,int emule_version){
		if(emule_version==44160){
			if (val <= 0xFF){
				output.WriteByte(0x89);
				output.WriteByte(cmd);
				output.WriteByte((byte)val);
			}
			else if (val <= 0xFFFF){
				output.WriteByte(0x88);
				output.WriteByte(cmd);
				output.WriteWord(val);
			}
			else{
				output.WriteByte(0x83);
				output.WriteByte(cmd);
				output.WriteDWord(val);
			}
		}
		else{
			output.WriteByte(0x03);
			output.WriteWord(0x01);
			output.WriteByte(cmd);
			output.WriteDWord(val);
		}
	}

	static void WriteEmuleTag(const char* val,byte cmd,Buffer2000& output,int emule_version){
		if(emule_version==44160){
			int str_len = (int)strlen(val);
			if (str_len >= 1 && str_len <= 16){
				output.WriteByte(0x80|(0x11 + str_len - 1));
				output.WriteByte(cmd);
				output.WriteString(val);
			}
			else{
				output.WriteByte(0x82);
				output.WriteByte(cmd);
				output.WriteWord(str_len);
				output.WriteString(val);
			}
		}
		else{
			output.WriteByte(0x02);
			output.WriteWord(0x01);
			output.WriteByte(cmd);
			output.WriteWord((WORD)strlen(val));
			output.WriteString(val);
		}
	}


	static string GetExtension(const char* file_name){
		int str_len=(int)strlen(file_name);

		string extension="";
		for(int i=str_len-1;i>=0;i--){
			unsigned char ch=file_name[i];
			if(ch=='.'){
				return extension;
			}
			else{
				string tmp;
				tmp+=ch;
				tmp+=extension;
				extension=tmp;
			}
		}
		return extension;
	}

	static const char* GetCodec(const char* extension,const char* file_name){
		if(stricmp(extension,"avi")==0){
			CString lc_file_name=file_name;
			lc_file_name=lc_file_name.MakeLower();

			if(lc_file_name.Find("xvid")!=-1)
				return "xvid";
			if(lc_file_name.Find("divx")!=-1)
				return "dx50";
			else return "xvid";
		}
		else if(stricmp(extension,"mpg")==0 || stricmp(extension,"mpeg")==0){
			return "yuy2";
		}
		else if(stricmp(extension,"wmv")==0){
			return "wmv2";
		}
		else return "";
	}

	static int GetBitrate(const char* extension,const char* file_name){
		if(stricmp(extension,"avi")==0){
			return 600+(rand()%500);
		}
		else if(stricmp(extension,"mpg")==0 || stricmp(extension,"mpeg")==0){
			return 192;
		}
		else if(stricmp(extension,"wmv")==0){
			return 1440;
		}
		else if(stricmp(extension,"mp3")==0 || stricmp(extension,"wma")==0){
			if(rand()&1)
				return 192;
			else return 160;
		}
		else return 0;
	}

	static int CalculateMediaLength(const char* extension,const char* file_name,unsigned int file_size){
		CString lc_file_name=file_name;
		lc_file_name=lc_file_name.MakeLower();

		if((stricmp(extension,"avi")==0 || stricmp(extension,"mpg")==0 || stricmp(extension,"wmv")==0) || stricmp(extension,"mpeg")==0 ){
			if (lc_file_name.Find("cd1") != -1 ||
				lc_file_name.Find("cd 1") != -1 ||
				lc_file_name.Find("cd2") != -1 ||
				lc_file_name.Find("cd 2") != -1){

				return 3000+(rand()%600);//make it shorter for a cd 1 or cd 2
			}
			else{
				return 3600+2200+(rand()%1400);
			}
		}
		else if(stricmp(extension,".mp3")==0 || stricmp(extension,".wma")==0 || stricmp(extension,".wav")==0){
			srand(file_size/50000);  //generate the media length according to the size of the file
			return 180+(rand()%120);//new_random.nextInt(120); //between 3 and 5 minutes long
		}

		return 0;
	}

	static const char* GetFileType(const char* extension){
		if(stricmp(extension,"avi")==0 || stricmp(extension,"mpg")==0 || stricmp(extension,"wmv")==0 || stricmp(extension,"mpeg")==0 ){
			return "Video";
		}
		else if(stricmp(extension,".mp3")==0 || stricmp(extension,".wma")==0 || stricmp(extension,".wav")==0){
			return "Audio";
		}
		else if(stricmp(extension,".par")==0 || stricmp(extension,".zip")==0 || stricmp(extension,".rar")==0 || stricmp(extension,".iso")==0 || stricmp(extension,".bin")==0 || stricmp(extension,".cue")==0){
			return "Pro";
		}
		else if(stricmp(extension,".txt")==0 || stricmp(extension,".doc")==0 || stricmp(extension,".rtf")==0 || stricmp(extension,".nfo")==0){
			return "Doc";
		}
		return "";
	}
};
