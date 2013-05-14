#include "StdAfx.h"
#include "projectdbinterface.h"
#include "QueryKeyword.h"
#include "HashCount.h"
#include "SupplyKeywords.h"

ProjectDBInterface::ProjectDBInterface(void)
{
}

//
//
//
ProjectDBInterface::~ProjectDBInterface(void)
{
}

//
//
//
bool ProjectDBInterface::OpenDBConnection(/*CString ip, CString login, CString pass, CString db*/)
{
	p_conn = mysql_init(NULL);
#ifdef _DEBUG
#ifdef DCMASTER2
	if (!mysql_real_connect(p_conn, "dcmaster.mediadefender.com", "onsystems", "ebertsux37", "project_management2", 0, NULL, 0))
#else
	if (!mysql_real_connect(p_conn, "dcmaster.mediadefender.com", "onsystems", "ebertsux37", "project_management", 0, NULL, 0))
#endif
#else
#ifdef DCMASTER2
	if (!mysql_real_connect(p_conn, "dcmaster.mediadefender.com", "onsystems", "ebertsux37", "project_management2", 0, NULL, 0))
#else
	if (!mysql_real_connect(p_conn, "127.0.0.1", "onsystems", "ebertsux37", "project_management", 0, NULL, 0))
#endif
#endif
	{
		/*
		CString errmsg = "Failed to connect to database: Error: ";
		errmsg += mysql_error(p_conn);
		TRACE(errmsg);
		MessageBox(NULL,errmsg,"Error",MB_OK|MB_ICONERROR);
		*/
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " ProjectDBInterface::OpenDBConnection(...) ";
			log += "Failed to connect to database: Error: ";
			log += mysql_error(p_conn);
			log += "\n\n";
			file.WriteString(log);
			file.Close();
		}
		return false;
	}
	return true;
}

//
//
//
int ProjectDBInterface::ReadProjectKeywords(vector<ProjectKeywords>& keywords)
{
	int ret;
	char temp[32];
	memset(temp, 0, sizeof(temp));

	/* Getting data from projects table */
	CString query_str = "select id,project_name,artist,album,owner,search_type,search_string,size_threashold,spoof_multiplier,kazaa_swarm_load,gnutella_load from projects where active = 'A'";
	ret = mysql_query(p_conn,query_str);
	
	if(mysql_error(p_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " ReadProjectKeywords::projects(...) ";
			log += query_str;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}
		else
		{
			DWORD error = GetLastError();
			MessageBox(NULL, "File Open Error: SQL_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}
	if(ret != 0)
		return ret;

	MYSQL_RES *res=NULL;
	MYSQL_ROW row;

	res=mysql_store_result(p_conn);	// allocates memory
	if(res != NULL)
	{
		UINT num_row = (UINT)mysql_num_rows(res);
		if(num_row > 0)
		{
			while ((row = mysql_fetch_row(res)))
			{
				if(row[0] != NULL && row[1] != NULL)
				{
					ProjectKeywords keyword;
					keyword.m_id = atoi(row[0]);
					keyword.m_project_name = row[1];
					keyword.m_artist_name = row[2];
					keyword.m_album_name = row[3];
					keyword.m_owner = row[4];
					//if(strcmp(row[5],"A")==0)
						keyword.m_project_active = true;
					//else
						//keyword.m_project_active = false;
					if(strcmp(row[5],"AUDIO")==0)
						keyword.m_search_type = ProjectKeywords::search_type::audio;
					else if(strcmp(row[5],"VIDEO")==0)
						keyword.m_search_type = ProjectKeywords::search_type::video;
					else if(strcmp(row[5],"EVERYTHING")==0)
						keyword.m_search_type = ProjectKeywords::search_type::everything;
					else if(strcmp(row[5],"IMAGES")==0)
						keyword.m_search_type = ProjectKeywords::search_type::images;
					else if(strcmp(row[5],"DOCUMENTS")==0)
						keyword.m_search_type = ProjectKeywords::search_type::documents;
					else if(strcmp(row[5],"SOFTWARE")==0)
						keyword.m_search_type = ProjectKeywords::search_type::software;
					else if(strcmp(row[5],"PLAYLISTS")==0)
						keyword.m_search_type = ProjectKeywords::search_type::playlists;
					else if(strcmp(row[5],"NONE")==0)
						keyword.m_search_type = ProjectKeywords::search_type::none;
					keyword.m_supply_keywords.m_search_string = strlwr(row[6]);
					keyword.m_supply_keywords.m_supply_size_threshold = atoi(row[7]);
					//if(row[12]!=NULL)
					//{
					//	if(strcmp(row[12], "T")==0)
							keyword.m_auto_multiplier_enabled = true;
					//	else
					//		keyword.m_auto_multiplier_enabled = false;
					//}
					//else
					//	keyword.m_auto_multiplier_enabled = false;

					
					keyword.m_query_multiplier = 100;
					if(row[8]!=NULL)
					{
                        int	mult = atoi(row[8]);
						if((mult<100)&&(keyword.m_auto_multiplier_enabled==true))
						{
							keyword.m_query_multiplier = 100;
						}
						else if(mult<10000)
						{
							keyword.m_query_multiplier = mult;
						}
					}
					//if(row[14]!=NULL)
					//{
					//	if(strcmp(row[14],"T")==0)
					//		keyword.m_bearshare_disting_enabled = true;
					//}
					//else
						keyword.m_bearshare_disting_enabled = false;
					//if(row[15]!=NULL)
					//	keyword.m_kazaa_dist_power = atoi(row[15]);
					int swarm_load=10;
					if(row[9]!=NULL)
					{
						swarm_load=atoi(row[9]);
					}
					if(swarm_load >=0 && swarm_load <= 100)
						keyword.m_kazaa_swarm_load=swarm_load;
					int gnutella_load=10;
					if(row[10]!=NULL)
					{
						gnutella_load=atoi(row[10]);
					}
					if(gnutella_load >=0 && gnutella_load <= 100)
						keyword.m_gnutella_load=gnutella_load;
					keywords.push_back(keyword);
				}
			}
		}
	}
	mysql_free_result(res);	// free memory



	/* Getting data from ex_killwords table */
	query_str = "select * from ex_killwords";
	ret = mysql_query(p_conn,query_str);
	
	if(mysql_error(p_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " ReadProjectKeywords::ex_killwords(...) ";
			log += query_str;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}
		else
		{
			DWORD error = GetLastError();
			MessageBox(NULL, "File Open Error: SQL_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}
	if(ret != 0)
		return ret;

	res=NULL;
	res=mysql_store_result(p_conn);	// allocates memory
	if(res != NULL)
	{
		UINT num_row = (UINT)mysql_num_rows(res);
		if(num_row > 0)
		{
			while ((row = mysql_fetch_row(res)))
			{
				if(row[0] != NULL && row[1] != NULL)
				{
					int project_id = atoi(row[0]);
					for(UINT i=0; i<keywords.size(); i++)
					{
						if(keywords[i].m_id == project_id)
						{
							QueryKeyword word;
							word.keyword = strlwr(row[1]);
							word.weight = atoi(row[2]);
							keywords[i].m_query_keywords.v_exact_killwords.push_back(word);
							break;
						}
					}
				}
			}
		}
	}
	mysql_free_result(res);	// free memory


	/* Getting data from hash_counts table */
	query_str = "select * from hash_counts";
	ret = mysql_query(p_conn,query_str);
	
	if(mysql_error(p_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " ReadProjectKeywords::hash_counts(...) ";
			log += query_str;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}
		else
		{
			DWORD error = GetLastError();
			MessageBox(NULL, "File Open Error: SQL_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}
	if(ret != 0)
		return ret;

	res=NULL;
	res=mysql_store_result(p_conn);	// allocates memory
	if(res != NULL)
	{
		UINT num_row = (UINT)mysql_num_rows(res);
		if(num_row > 0)
		{
			while ((row = mysql_fetch_row(res)))
			{
				if(row[0] != NULL && row[1] != NULL)
				{
					int project_id = atoi(row[0]);
					for(UINT i=0; i<keywords.size(); i++)
					{
						if(keywords[i].m_id == project_id)
						{
							HashCount hc;
							hc.m_track = atoi(row[1]);
							hc.m_hash_count = atoi(row[2]);
							int year,month,day;
							sscanf(row[3], "%d-%d-%d",&year,&month,&day);
							hc.m_expiration_date = CTime(year,month,day,0,0,0);
							keywords[i].v_tracks_hash_counts.push_back(hc);
							break;
						}
					}
				}
			}
		}
	}
	mysql_free_result(res);	// free memory


	/* Getting data from killwords table */
	query_str = "select * from killwords";
	ret = mysql_query(p_conn,query_str);
	
	if(mysql_error(p_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " ReadProjectKeywords::killwords(...) ";
			log += query_str;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}
		else
		{
			DWORD error = GetLastError();
			MessageBox(NULL, "File Open Error: SQL_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}
	if(ret != 0)
		return ret;

	res=NULL;
	res=mysql_store_result(p_conn);	// allocates memory
	if(res != NULL)
	{
		UINT num_row = (UINT)mysql_num_rows(res);
		if(num_row > 0)
		{
			while ((row = mysql_fetch_row(res)))
			{
				if(row[0] != NULL && row[1] != NULL)
				{
					int project_id = atoi(row[0]);
					for(UINT i=0; i<keywords.size(); i++)
					{
						if(keywords[i].m_id == project_id)
						{
							//killword for demand
							QueryKeyword word;
							word.keyword = strlwr(row[1]);
							if(row[2] != NULL)
								word.weight = atoi(row[2]);
							keywords[i].m_query_keywords.v_killwords.push_back(word);
							
							//killword for supply
							SupplyKeyword killword;
							killword.m_track = 0;
							char * killword_buf = new char[strlen(row[1])+1];
							strcpy(killword_buf, row[1]);
							killword.v_keywords.push_back(strlwr(killword_buf));
							keywords[i].m_supply_keywords.v_killwords.push_back(killword);
							break;
						}
					}
				}
			}
		}
	}
	mysql_free_result(res);	// free memory


	/* Getting data from qrp_keywords table */
	query_str = "select * from qrp_keywords";
	ret = mysql_query(p_conn,query_str);
	
	if(mysql_error(p_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " ReadProjectKeywords::qrp_keywords(...) ";
			log += query_str;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}
		else
		{
			DWORD error = GetLastError();
			MessageBox(NULL, "File Open Error: SQL_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}
	if(ret != 0)
		return ret;

	res=NULL;

	res=mysql_store_result(p_conn);	// allocates memory
	if(res != NULL)
	{
		UINT num_row = (UINT)mysql_num_rows(res);
		if(num_row > 0)
		{
			while ((row = mysql_fetch_row(res)))
			{
				if(row[0] != NULL && row[1] != NULL)
				{
					int project_id = atoi(row[0]);
					for(UINT i=0; i<keywords.size(); i++)
					{
						if(keywords[i].m_id == project_id)
						{
							keywords[i].m_qrp_keywords.v_keywords.push_back(strlwr(row[1]));
							break;
						}
					}
				}
			}
		}
	}
	mysql_free_result(res);	// free memory


	/* Getting track data from keywords table */
	query_str = "select * from keywords order by project_id,search_index,track_number";
	ret = mysql_query(p_conn,query_str);
	
	if(mysql_error(p_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " ReadProjectKeywords::keywords(...) ";
			log += query_str;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}
		else
		{
			DWORD error = GetLastError();
			MessageBox(NULL, "File Open Error: SQL_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}
	if(ret != 0)
		return ret;

	res=NULL;

	res=mysql_store_result(p_conn);	// allocates memory
	if(res != NULL)
	{
		UINT num_row = (UINT)mysql_num_rows(res);
		if(num_row > 0)
		{
			while ((row = mysql_fetch_row(res)))
			{
				if(row[0] != NULL && row[1] != NULL)
				{
					int project_id = atoi(row[0]);
					int track_number = atoi(row[2]);
					for(UINT i=0; i<keywords.size(); i++)
					{
						if(keywords[i].m_id == project_id)
						{
							bool found = false;
							for(UINT j=0; j<keywords[i].m_supply_keywords.v_keywords.size(); j++)
							{
								if(keywords[i].m_supply_keywords.v_keywords[j].m_track == track_number)
								{
									found = true;
									char * keyword_buf = new char[strlen(row[1])+1];
									strcpy(keyword_buf, row[1]);
									keywords[i].m_supply_keywords.v_keywords[j].v_keywords.push_back(strlwr(keyword_buf));
									break;
								}
							}
							if(!found)
							{
								SupplyKeyword word;
								word.m_track = track_number;
								word.m_track_name = QueryTrackName(row[0], row[2],word.m_single);
								/*word.m_track_name = strupr(row[2]);

								char * end = strchr(row[2], ' ');
								char * begin = row[2];

								while(end != NULL)
								{
									end[0]='\0';

									char * keyword_buf = new char[strlen(begin)+1];
									strcpy(keyword_buf, begin);

									word.v_keywords.push_back(strlwr(keyword_buf));

									begin = end+1;
									end = strchr(begin, ' ');
								}

								char * keyword_buf = new char[strlen(begin)+1];
								strcpy(keyword_buf, begin);
								word.v_keywords.push_back(strlwr(keyword_buf));
								*/
								char * keyword_buf = new char[strlen(row[1])+1];
								strcpy(keyword_buf, row[1]);
								word.v_keywords.push_back(strlwr(keyword_buf));
								keywords[i].m_supply_keywords.v_keywords.push_back(word);
							}
							break;
						}
					}
				}
			}
		}
	}
	mysql_free_result(res);	// free memory


	/* Getting data from weighted_ex_keywords table */
	query_str = "select * from weighted_ex_keywords";
	ret = mysql_query(p_conn,query_str);
	
	if(mysql_error(p_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " ReadProjectKeywords::weighted_ex_keywords(...) ";
			log += query_str;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}
		else
		{
			DWORD error = GetLastError();
			MessageBox(NULL, "File Open Error: SQL_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}
	if(ret != 0)
		return ret;

	res=NULL;

	res=mysql_store_result(p_conn);	// allocates memory
	if(res != NULL)
	{
		UINT num_row = (UINT)mysql_num_rows(res);
		if(num_row > 0)
		{
			while ((row = mysql_fetch_row(res)))
			{
				if(row[0] != NULL && row[1] != NULL)
				{
					int project_id = atoi(row[0]);
					for(UINT i=0; i<keywords.size(); i++)
					{
						if(keywords[i].m_id == project_id)
						{
							QueryKeyword word;
							word.keyword = strlwr(row[1]);
							word.weight = atoi(row[2]);
							keywords[i].m_query_keywords.v_exact_keywords.push_back(word);
							break;
						}
					}
				}
			}
		}
	}
	mysql_free_result(res);	// free memory


	/* Getting data from weighted_keywords table */
	query_str = "select * from weighted_keywords";
	ret = mysql_query(p_conn,query_str);
	
	if(mysql_error(p_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " ReadProjectKeywords::weighted__keywords(...) ";
			log += query_str;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}
		else
		{
			DWORD error = GetLastError();
			MessageBox(NULL, "File Open Error: SQL_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}
	if(ret != 0)
		return ret;

	res=NULL;

	res=mysql_store_result(p_conn);	// allocates memory
	if(res != NULL)
	{
		UINT num_row = (UINT)mysql_num_rows(res);
		if(num_row > 0)
		{
			while ((row = mysql_fetch_row(res)))
			{
				if(row[0] != NULL && row[1] != NULL)
				{
					int project_id = atoi(row[0]);
					for(UINT i=0; i<keywords.size(); i++)
					{
						if(keywords[i].m_id == project_id)
						{
							QueryKeyword word;
							word.keyword = strlwr(row[1]);
							word.weight = atoi(row[2]);
							keywords[i].m_query_keywords.v_keywords.push_back(word);
							break;
						}
					}
				}
			}
		}
	}
	mysql_free_result(res);	// free memory

	/* Getting data from action_items table */
	query_str = "select * from action_items";
	ret = mysql_query(p_conn,query_str);
	
	if(mysql_error(p_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " ReadProjectKeywords::action_items(...) ";
			log += query_str;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}
		else
		{
			DWORD error = GetLastError();
			MessageBox(NULL, "File Open Error: SQL_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}
	if(ret != 0)
		return ret;

	res=NULL;

	res=mysql_store_result(p_conn);	// allocates memory
	if(res != NULL)
	{
		UINT num_row = (UINT)mysql_num_rows(res);
		if(num_row > 0)
		{
			while ((row = mysql_fetch_row(res)))
			{
				if(row[0] != NULL && row[1] != NULL)
				{
					int project_id = atoi(row[0]);
					for(UINT i=0; i<keywords.size(); i++)
					{
						if(keywords[i].m_id == project_id)
						{
							int action = atoi(row[1]);
							switch(action)
							{
								case 1:
								{
									keywords[i].m_directconnect_spoofing_enabled = true;
									break;
								}
								case 2:
								{
									keywords[i].m_directconnect_decoys_enabled = true;
									break;
								}
								case 3:
								{
									keywords[i].m_directconnect_supply_enabled = true;
									break;
								}
								case 4:
								{
									keywords[i].m_fasttrack_spoofing_enabled = true;
									break;
								}
								case 5:
								{
									keywords[i].m_fasttrack_decoys_enabled = true;
									break;
								}
								case 6:
								{
									keywords[i].m_fasttrack_swarming_enabled = true;
									break;
								}
								case 7:
								{
									keywords[i].m_fasttrack_supply_enabled = true;
									break;
								}
								case 8:
								{
									keywords[i].m_gnutella_spoofing_enabled = true;
									break;
								}
								case 9:
								{
									keywords[i].m_gnutella_decoys_enabled = true;
									break;
								}
								case 10:
								{
									keywords[i].m_gnutella_swarming_enabled = true;
									break;
								}
								case 11:
								{
									keywords[i].m_gnutella_supply_enabled = true;
									break;
								}
								case 12:
								{
									keywords[i].m_gnutella_demand_enabled = true;
									break;
								}
								case 13:
								{
									keywords[i].m_piolet_spoofing_enabled = true;
									break;
								}
								case 14:
								{
									keywords[i].m_piolet_supply_enabled = true;
									break;
								}
								case 15:
								{
									keywords[i].m_piolet_demand_enabled = true;
									break;
								}
								case 18:
								{
									keywords[i].m_emule_spoofing_enabled = true;
									break;
								}
								case 19:
								{
									keywords[i].m_emule_decoys_enabled = true;
									break;
								}
								case 20:
								{
									keywords[i].m_emule_swarming_enabled = true;
									break;
								}
								case 21:
								{
									keywords[i].m_emule_supply_enabled = true;
									break;
								}
								case 22:
								{
									keywords[i].m_winmx_decoy_enabled = true;
									break;
								}
								case 23:
								{
									keywords[i].m_winmx_supply_enabled = true;
									break;
								}
								case 26:
								{
									keywords[i].m_gift_spoofing_enabled = true;
									break;
								}
								case 27:
								{
									keywords[i].m_gift_decoys_enabled = true;
									break;
								}
								case 28:
								{
									keywords[i].m_gift_supply_enabled = true;
									break;
								}
								case 30:
								{
									keywords[i].m_winmx_spoofing_enabled = true;
									break;
								}
								case 31:
								{
									keywords[i].m_winmx_swarming_enabled = true;
									break;
								}
								case 39:
								{
									keywords[i].m_soulseek_demand_enabled = true;
									break;
								}
								case 40:
								{
									keywords[i].m_soulseek_supply_enabled = true;
									break;
								}
								case 41:
								{
									keywords[i].m_soulseek_spoofing_enabled = true;
									break;
								}
								case 42:
								{
									keywords[i].m_soulseek_decoying_enabled = true;
									break;
								}
								case 43:
								{
									keywords[i].m_bittorrent_spoofing_enabled = true;
									break;
								}
								case 44:
								{
									keywords[i].m_bittorrent_supply_enabled = true;
									break;
								}
								case 47:
								{
									keywords[i].m_bittorrent_decoy_enabled = true;
									break;
								}
								case 49:
								{
									keywords[i].m_bittorrent_interdiction_enabled = true;
									break;
								}
								case 50:
								{
									keywords[i].m_overnet_supply_enabled = true;
									break;
								}
								case 51:
								{
									keywords[i].m_overnet_demand_enabled = true;
									break;
								}
								case 52:
								{
									keywords[i].m_overnet_spoofing_enabled = true;
									break;
								}
								case 53:
								{
									keywords[i].m_overnet_decoys_enabled = true;
									break;
								}
							}
							break;
						}
					}
				}
			}
		}
	}
	mysql_free_result(res);	// free memory


	return ret;
}

//
//
//
void ProjectDBInterface::CloseDBConnection()
{
	mysql_close(p_conn);
}

//
//
//
string ProjectDBInterface::QueryTrackName(const char* project_id, const char* track_number, bool& single)
{
	string empty_track = "N/A";

	CString query_str = "select track_name,single from track_listing where project_id = ";
	query_str += project_id;
	query_str += " and track_number = ";
	query_str += track_number;

	int ret = mysql_query(p_conn,query_str);
	
	if(mysql_error(p_conn)[0] != '\0')
	{
		CStdioFile file;
		if( file.Open("SQL_Error_Log.txt", CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite, NULL) != 0)
		{
			file.SeekToEnd();

			CString log;
			log = CTime::GetCurrentTime().Format("%Y-%m-%d %H:%M:%S");
			log += " QueryTrackName(...) ";
			log += query_str;
			log += "\n";
			log += "Error: ";
			log += mysql_error(p_conn);
			log += "\n\n";

			file.WriteString(log);
			file.Close();
		}
		else
		{
			DWORD error = GetLastError();
			MessageBox(NULL, "File Open Error: SQL_Error_Log.txt", "File Open Error", MB_OK);
		}

		TRACE(mysql_error(p_conn));
		TRACE("\n");
		TRACE(query_str);
		TRACE("\n");
	}
	if(ret != 0)
		return empty_track;

	MYSQL_RES *res=NULL;
	MYSQL_ROW row;

	res=NULL;
	res=mysql_store_result(p_conn);	// allocates memory
	if(res != NULL)
	{
		UINT num_row = (UINT)mysql_num_rows(res);
		if(num_row > 0)
		{
			row = mysql_fetch_row(res);
			string track_name = row[0];
			if(row[1]!=NULL)
			{
				if(strcmp(row[1],"T")==0)
					single=true;
				else
					single=false;
			}
			else
				single=false;			
			mysql_free_result(res);	// free memory
			return track_name;
		}
	}
	return empty_track;
}