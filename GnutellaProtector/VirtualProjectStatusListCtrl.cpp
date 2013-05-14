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
}

VirtualProjectStatusListCtrl::~VirtualProjectStatusListCtrl()
{
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
			
			case SUB_PS_SPOOFS_DISTS_POISONS:
			{
				sprintf(pItem->pszText,"%u : %u : %u",v_data[index].m_spoof_count,v_data[index].m_dist_count,v_data[index].m_poison_count);
				break;
			}
			
			case SUB_PS_QUERY:
			{
				sprintf(pItem->pszText,"%u",v_data[index].m_query_count);
				break;
			}
			/*
			case SUB_PS_QUERY_HIT:
			{
				sprintf(pItem->pszText,"%u",v_data[index].m_query_hit_count);
				break;
			}
			*/
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
			/*
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
			*/
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
			v_data[i].m_query_count+=status.m_total_query_count;
			//v_data[i].m_query_hit_count+=status.m_total_query_hit_count;
			v_data[i].IncrementTrackCounters(status);
			v_data[i].m_spoof_count+=status.m_spoof_count;
			v_data[i].m_dist_count+=status.m_dist_count;
			v_data[i].m_poison_count+=status.m_poison_count;
			return;
		}
	}

	VirtualProjectStatusListCtrlItem item;
	item.m_project_name=status.ProjectName();
	item.m_query_count=status.m_total_query_count;
	//item.m_query_hit_count=status.m_total_query_hit_count;
	item.IncrementTrackCounters(status);
	item.m_spoof_count=status.m_spoof_count;
	item.m_dist_count=status.m_dist_count;
	item.m_poison_count=status.m_poison_count;

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