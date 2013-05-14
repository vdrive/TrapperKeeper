#include "projectmanager.h"

ProjectManager::ProjectManager(void)
{
	m_query = 
		"select search_string, search_type, project_id, owner, size_threashold \
		from action_items i, projects p \
		where name_id = 49 \
		and p.id = project_id \
		and active = 'A' \
		order by artist";
}

ProjectManager::~ProjectManager(void)
{
	
}

void ProjectManager::SetQuery(string query) 
{
	m_query = query;
}

int ProjectManager::GetProjectData(vector<ProjectData> *v_project_data)
{
	MYSQL conn;
	MYSQL_RES * res = NULL;
	int num = 0;

	v_project_data->clear();

	//Real connect string


	if(m_sql.OpenDB(&conn, "63.221.232.36", "project_management"))
	{
		res = m_sql.Query(&conn, m_query.c_str());

		MYSQL_ROW row;

		if(res != NULL)
		{
			int num_rows = (int)mysql_num_rows(res);
			num = num_rows;

			if(num_rows > 0)
			{
				// Iterate through the rowset
				CString message;
				while ((row = mysql_fetch_row(res)))
				{
					//TRACE("adding search string: %s\n", row[0]);
					ProjectData data;
					data.m_search_types = row[1];
					data.m_search_strings = row[0];
					data.m_id = (row[2]);
					data.m_owner = (row[3]);
					data.m_min_size = (row[4]);
					v_project_data->push_back(data);
				}
			}
		}
		mysql_free_result(res);			// free's mem (::)
		
		TRACE("Getting Kill Words\n");

		// Get killwords for the projects
		for(unsigned int i = 0; i < v_project_data->size(); i++) {
			string query = "SELECT killword \
				FROM killwords \
				where project_id = ";
			query += (*v_project_data)[i].m_id;


			int ret = mysql_query(&conn, query.c_str());
			if (ret != 0)	// check for error
				TRACE(mysql_error(&conn));

			MYSQL_ROW row;
			res = mysql_store_result(&conn); // allocates mem (::)

			if(res != NULL)
			{
				int num_rows = (int)mysql_num_rows(res);

				if(num_rows > 0)
				{
					char * kill_words = new char[3000];
					strcpy(kill_words, "");
					
					// Iterate through the rowset
					while ((row = mysql_fetch_row(res)))
					{
						strcat(kill_words, row[0]);
						strcat(kill_words, " ");
					}
					(*v_project_data)[i].m_kill_words = (string(kill_words));
					delete [] kill_words;
				}
				else (*v_project_data)[i].m_kill_words = ""; // no kill words
			}
			mysql_free_result(res);			// free's mem (::)
		}
		m_sql.CloseDB(&conn);
	}
	return num;
}