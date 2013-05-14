#include "stdafx.h"
#include "VirtualListCtrlFT.h"

/////////////////////////////////////////////////////////////////////////////
// VirtualListCtrlFT

VirtualListCtrlFT::VirtualListCtrlFT()
{
}

VirtualListCtrlFT::~VirtualListCtrlFT()
{
}


BEGIN_MESSAGE_MAP(VirtualListCtrlFT, CListCtrl)
	//{{AFX_MSG_MAP(VirtualListCtrlFT)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnGetdispinfo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VirtualListCtrlFT message handlers

void VirtualListCtrlFT::OnGetdispinfo(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LVITEM* pItem= &(pDispInfo->item);

	int index=pItem->iItem;

	// Valid text buffer?
	if (pItem->mask & LVIF_TEXT)
	{
		switch(pItem->iSubItem)
		{
			case SUB_REMOTE_IP:
			{
				unsigned int ip4=(v_data[index].m_remote_ip>>24)&0xFF;
				unsigned int ip3=(v_data[index].m_remote_ip>>16)&0xFF;
				unsigned int ip2=(v_data[index].m_remote_ip>>8)&0xFF;
				unsigned int ip1=(v_data[index].m_remote_ip>>0)&0xFF;
				sprintf(pItem->pszText,"%u.%u.%u.%u",ip1,ip2,ip3,ip4);

				break;
			}
			case SUB_MODULE:
			{
				_itoa(v_data[index].m_mod,pItem->pszText,10);
				break;
			}
			case SUB_SOCKET:
			{
				_itoa(v_data[index].m_sock,pItem->pszText,10);
				break;
			}
			case SUB_FILENAME:
			{
				strcpy(pItem->pszText,v_data[index].m_filename.c_str());
				break;
			}
			case SUB_RANGE:
			{
				sprintf(pItem->pszText,"%s : %s",FormatSize(v_data[index].m_start).c_str(),FormatSize(v_data[index].m_end).c_str());
				break;
			}
			case SUB_TOTAL_SIZE:
			{
				sprintf(pItem->pszText,"%s",FormatSize(v_data[index].m_total_size).c_str());
				break;
			}
			case SUB_SENT:
			{
				sprintf(pItem->pszText,"%s / %s",FormatSize(v_data[index].m_num_sent).c_str(),FormatSize(v_data[index].m_end-v_data[index].m_start).c_str());
				break;
			}
			case SUB_ELAPSED:
			{
				strcpy(pItem->pszText,FormatElapsed(v_data[index].m_time_elapsed).c_str());
				break;
			}
			case SUB_VERSION:
			{
				strcpy(pItem->pszText,v_data[index].m_client.c_str());
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
unsigned int VirtualListCtrlFT::GetItemCount()
{
	return v_data.size();
}

//
//
//
void VirtualListCtrlFT::DeleteAllItems()
{
	v_data.clear();
}

//
//
//
void VirtualListCtrlFT::ReportStatus(vector<NoiseModuleThreadStatusData> &status)
{
	CTime time=CTime::GetCurrentTime();

	int i,j;
	for(i=0;i<status.size();i++)
	{
		bool found=false;
		for(j=0;j<v_data.size();j++)
		{
			vector<VirtualListControlItemFT>::iterator data_iter=v_data.begin();
			if((v_data[j].m_sock==status[i].m_socket) && (v_data[j].m_mod==status[i].m_mod))
			{
				found=true;

				if(status[i].m_is_idle==true)
				{
					v_data.erase(data_iter);
				}
				else
				{
					v_data[j].m_remote_ip=status[i].m_remote_ip;
					v_data[j].m_filename=status[i].m_filename;
					v_data[j].m_start=status[i].m_start;
					v_data[j].m_end=status[i].m_end;
					v_data[j].m_total_size=status[i].m_total_size;
					v_data[j].m_num_sent=status[i].m_num_sent;
					v_data[j].m_time_elapsed=time-status[i].m_start_time;
					v_data[j].m_client=status[i].m_client;
				}

				break;
			}

			data_iter++;
		}

		// If not found, make a new one
		if(!found && (status[i].m_is_idle==false))
		{
			VirtualListControlItemFT item;
			item.m_mod=status[i].m_mod;
			item.m_sock=status[i].m_socket;
			item.m_remote_ip=status[i].m_remote_ip;
			item.m_filename=status[i].m_filename;
			item.m_start=status[i].m_start;
			item.m_end=status[i].m_end;
			item.m_total_size=status[i].m_total_size;
			item.m_num_sent=status[i].m_num_sent;
			item.m_time_elapsed=time-status[i].m_start_time;
			item.m_client=status[i].m_client;
			
			v_data.push_back(item);
		}
	}

	// Sort the list by ip address
//	sort(v_data.begin(),v_data.end());
}

//
//
//
string VirtualListCtrlFT::FormatSize(unsigned int size)
{
	string ret;
	char buf[4096];

	if(size<1024)	// size < KB
	{
		_itoa(size,buf,10);
	}
	else if(size<1024*1024) // KB < size < MB
	{
		double fsize=(double)size;
		fsize/=1024; // make it KB
		sprintf(buf,"%.2f KB",fsize);
	}
	else	// size > MB
	{
		double fsize=(double)size;
		fsize/=(1024*1024); // make it MB
		sprintf(buf,"%.2f MB",fsize);
	}

	ret=buf;

	return ret;
}

//
//
//
string VirtualListCtrlFT::FormatElapsed(CTimeSpan &span)
{
	string ret;
	char buf[4096];

	if(span.GetTotalHours()==0)	// < hour
	{
		sprintf(buf,"%u:%02u",span.GetMinutes(),span.GetSeconds());
	}
	else	//	> hour
	{
		sprintf(buf,"%u:%02u:%02u",span.GetHours(),span.GetMinutes(),span.GetSeconds());
	}
			
	ret=buf;

	return ret;
}
