// VirtualProjectStatusListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "VirtualProjectStatusListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// VirtualProjectStatusListCtrl

VirtualProjectStatusListCtrl::VirtualProjectStatusListCtrl()
{
//	m_db_interface.OpenSupplyConnection("127.0.0.1","onsystems","ebertsux37","fasttrack_fake_supply");
}

VirtualProjectStatusListCtrl::~VirtualProjectStatusListCtrl()
{
//	m_db_interface.CloseSupplyConnection();
}


BEGIN_MESSAGE_MAP(VirtualProjectStatusListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(VirtualProjectStatusListCtrl)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetdispinfo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VirtualProjectStatusListCtrl message handlers

void VirtualProjectStatusListCtrl::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LVITEM* pItem= &(pDispInfo->item);

	UINT i;
	int index=pItem->iItem;

	// Valid text buffer?
	if (pItem->mask & LVIF_TEXT)
	{
		switch(pItem->iSubItem)
		{
			case SUB_PS_PROJECT:
			{
				strcpy(pItem->pszText,v_data[index].m_project_name.c_str());
				break;
			}
			/*
			case SUB_PS_WRITTEN:
			{
				if(v_data[index].m_has_been_written_to_db)
					strcpy(pItem->pszText,"Y");
				else
					strcpy(pItem->pszText,"N");
				break;
			}
			*/
			case SUB_PS_QUERY_HIT:
			{
				sprintf(pItem->pszText,"%u",v_data[index].m_query_hit_count);
				break;
			}
			/*
			case SUB_PS_TRACK_QUERY:
			{
				if(v_data[index].v_track_query_count.size()>0)
				{
					sprintf(pItem->pszText,"%u",v_data[index].v_track_query_count[0]);
					for(i=1;i<v_data[index].v_track_query_count.size();i++)
					{
						sprintf(&pItem->pszText[strlen(pItem->pszText)]," %u",v_data[index].v_track_query_count[i]);
					}
				}

				break;
			}
			*/
			case SUB_PS_DECOY:
			{
				sprintf(pItem->pszText,"%u",v_data[index].m_fake_query_hit_count);
				break;
			}
			case SUB_PS_EFFICIENCY:
			{
				sprintf(pItem->pszText,"%.2f",(float)100*((float)v_data[index].m_fake_query_hit_count/(float)(v_data[index].m_fake_query_hit_count+v_data[index].m_query_hit_count)));
				break;
			}
			case SUB_PS_TRACK_QUERY_HIT:
			{
				if(v_data[index].v_track_query_hit_count.size()>0)
				{
					sprintf(pItem->pszText,"%u",v_data[index].v_track_query_hit_count[0]);
					for(i=1;i<v_data[index].v_track_query_hit_count.size();i++)
					{
						sprintf(&pItem->pszText[strlen(pItem->pszText)]," %u",v_data[index].v_track_query_hit_count[i]);
					}
				}
				
				break;
			}
		}
	}

	// Valid Image?
	if(pItem->mask & LVIF_IMAGE)
	{
		pItem->iImage=-1;
	}

	// Indent?
	if(pItem->mask & LVIF_INDENT)
	{
		pItem->iIndent=0;	// always set the indent to 0
	}

	// 
	if(pItem->mask & LVIF_PARAM)
	{
		pItem->lParam=0;	// always set the lParam to 0, since I don't know what it's for.  Maybe it's the data.
	}

	// Always set the state to 0. because I don't know what else to do right now.
	if(pItem->mask & LVIF_STATE)
	{
		pItem->state=0;
	}

	*pResult = 0;
}

//
//
//
int VirtualProjectStatusListCtrl::GetItemCount()
{
	return (int)v_data.size();
}

//
//
//
void VirtualProjectStatusListCtrl::DeleteAllItems()
{
	v_data.clear();
}

//
//
//
void VirtualProjectStatusListCtrl::IncrementCounters(ProjectStatus &status)
{
	UINT i;
	
	// Find the project, if it exists. If not, then make one.
	for(i=0;i<v_data.size();i++)
	{
		if(strcmp(status.ProjectName(),v_data[i].m_project_name.c_str())==0)
		{
			v_data[i].m_query_hit_count+=status.m_total_query_hit_count;
			v_data[i].m_fake_query_hit_count+=status.m_total_fake_query_hit_count;
			v_data[i].IncrementTrackCounters(status);
			v_data[i].IncrementFakeTrackCounters(status);
			return;
		}
	}

	VirtualProjectStatusListCtrlItem item;
	item.m_project_name=status.ProjectName();
	item.m_query_hit_count=status.m_total_query_hit_count;
	item.m_fake_query_hit_count=status.m_total_fake_query_hit_count;
	item.IncrementTrackCounters(status);
	item.IncrementFakeTrackCounters(status);

	v_data.push_back(item);

	sort(v_data.begin(),v_data.end());	// sort the data by name
}

//
//
//
void VirtualProjectStatusListCtrl::ClearProjectStatus(string &project)
{
	UINT i;
	
	// Find the project, if it exists. If not, then make one.
	for(i=0;i<v_data.size();i++)
	{
		if(strcmp(v_data[i].m_project_name.c_str(),project.c_str())==0)
		{
			v_data[i].ClearCounters();
			return;
		}
	}
}

//
//
//
void VirtualProjectStatusListCtrl::SaveReport()
{
	CStdioFile out;
	if(out.Open("fasttrack_report.txt",CFile::modeWrite|CFile::modeCreate|CFile::typeText)!=0)
	{
		out.WriteString("Project\tReal Supply\tDecoys\tEfficiency\tTrack Supply\n");
		char temp[1024];
		for(UINT i=0; i<v_data.size(); i++)
		{
			out.WriteString(v_data[i].m_project_name.c_str());
			out.WriteString("\t");
			ultoa(v_data[i].m_query_hit_count,temp,10);
			out.WriteString(temp);
			out.WriteString("\t");
			ultoa(v_data[i].m_fake_query_hit_count,temp,10);
			out.WriteString(temp);
			out.WriteString("\t");
			sprintf(temp,"%.2f",(float)100*((float)v_data[i].m_fake_query_hit_count/(float)(v_data[i].m_fake_query_hit_count+v_data[i].m_query_hit_count)));
			out.WriteString(temp);
			out.WriteString("\t");

			if(v_data[i].v_track_query_hit_count.size() > 0 )
			{
				sprintf(temp,"%u",v_data[i].v_track_query_hit_count[0]);
				for(UINT j=1;j<v_data[i].v_track_query_hit_count.size();j++)
				{
					sprintf(&temp[strlen(temp)]," %u",v_data[i].v_track_query_hit_count[j]);
				}
				out.WriteString(temp);
			}
			out.WriteString("\n");
		}
		out.Close();
	}
}

//
//
//
/*
void VirtualProjectStatusListCtrl::WriteFakeSuppliesToDB()
{
	UINT i;
	for(i=0;i<v_data.size();i++)
	{
		if(!v_data[i].m_has_been_written_to_db)
		{
			if( (CTime::GetCurrentTime() - v_data[i].m_last_update_time).GetTotalSeconds() > 60)
			{
				if(v_data[i].v_track_fake_query_hit_count.size()>0)
					m_db_interface.InsertFakeSupplies(v_data[i]);
				v_data[i].m_has_been_written_to_db=true;
			}
		}
	}
}
*/