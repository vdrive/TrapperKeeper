#pragma once
#include "..\tkcom\vector.h"

class InputFile
{
private:
	class InputGroup : public Object
	{
	public:
		string m_name;
		vector <string> mv_dependents;

		bool IsValid(){
			if(m_name.size()>0 && mv_dependents.size()>0)
				return true;
			else return false;
		}
	};

	class InputMetaData : public Object
	{
	public:
		string m_name;
		Vector mv_groups;	

		bool IsValid(){
			if(m_name.size()>0 && mv_groups.Size()>0)
				return true;
			else return false;
		}
	};

	class InputProject : public Object
	{
	public:
		string m_name;
		vector<string> mv_description;
		vector<string> mv_keywords;
		
		bool IsValid(){
			if(m_name.size()>0 && (mv_description.size()>0 || mv_keywords.size()>0))
				return true;
			else return false;
		}
	};


	string m_current_hash;

public:
	InputFile(void);
	~InputFile(void);
	bool HasChanged(void);
	void RescanInputFile(void);
private:
	bool ExtractLine(CString &line,CString& tag, CString& data);
	void GenerateMetaData(const char* name, byte* buffer, vector <string> &v_chosen_groups,bool b_test_positive);
	Vector mv_input_projects;
	//Vector mv_input_groups;
	Vector mv_input_meta_data;
public:
	void GetMetaData(const char* project, byte* description_buffer, byte* keyword_buffer);
	
};
