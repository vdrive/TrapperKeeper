#pragma once
#include "..\tkcom\ThreadedObject.h"
#include "..\tkcom\Vector.h"
#include <afxmt.h>

class SearchResultProcessor : public ThreadedObject
{
protected:
	CCriticalSection m_lock;
	Vector mv_results_to_process;

	void PrepareStringForDatabase(CString &str)
	{
		str.Replace("\\","\\\\");
		str.Replace("'","\\'");
	}


	string GetProjectTableName(const char* project_name)
	{
		int len=(int)strlen(project_name);
		string new_name;
		bool b_all_digit=true;
		for(int i=0;i<len && i<50;i++){
			if(!isdigit(project_name[i]))
				b_all_digit=false;
			if(isdigit(project_name[i]) || isalpha(project_name[i])){
				new_name+=project_name[i];
			}
			else{
				new_name+="_";
			}
		}

		if(b_all_digit || stricmp("case",new_name.c_str())==0){
			string tmp="a";
			tmp+=new_name;
			new_name=tmp;
		}	

		return new_name;
	}


public:
	SearchResultProcessor(void);
	~SearchResultProcessor(void);
	UINT Run(void);
	
	void AddResults(Vector* v_results);
};
