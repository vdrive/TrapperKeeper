#include "StdAfx.h"
#include "usenetpostercom.h"
#include "UsenetPosterDll.h"
#include "UsenetSocket.h"

UsenetPosterCom::UsenetPosterCom(void)
{
}

UsenetPosterCom::~UsenetPosterCom(void)
{
}

void UsenetPosterCom::InitParent(UsenetPosterDll *dll)
{
	p_dll = dll;
}

void UsenetPosterCom::DataReceived(char *source_ip, void *data, UINT data_length)
{
	// This is the number of headers we just received
	byte *ptr = (byte *)data;
	int num_headers = data_length/sizeof(Header);

	// Log it
	char msg[1024];
	sprintf(msg, "*** Received %d new headers for posting.", num_headers);
	p_dll->Log("Com",""); p_dll->Log("Com", msg); p_dll->Log("Com", "");

	// Find which server has the lowest posting load:
	UsenetSocket *socket_ptr = NULL;
	int low_load = 999999999;
	for (int index=0; index < (int)p_dll->v_usenet_socket_ptrs.size(); index++)
	{
		if (p_dll->v_usenet_socket_ptrs[index]->m_is_active == false)
			continue;
		
		int this_load = (int)p_dll->v_usenet_socket_ptrs[index]->v_headers.size();

		if (this_load < low_load)
		{
			socket_ptr = p_dll->v_usenet_socket_ptrs[index];
			low_load = this_load;
		}
	}

	if (socket_ptr == NULL)
	{
		MessageBox(NULL, "Banned on all servers", "New accounts needed", MB_OK);
		return;
	}

	// Push each header individually onto vector for posting
	for (int i=0; i < num_headers; i++)
	{
		Header head = *(Header*)ptr;
		socket_ptr->v_headers.push_back(head);
		ptr += sizeof(Header);
	}

	// Change the displayed queue size
	LVFINDINFO info;
	info.flags = LVFI_STRING;
	info.psz = socket_ptr->m_nickname;
	int nIndex= p_dll->m_dlg.m_servers_list.FindItem(&info);

	char str[128];
	itoa((int)socket_ptr->v_headers.size(), str, 10);
	p_dll->m_dlg.m_servers_list.SetItemText(nIndex, 1, str);


	// This is a check to see if our connection has been dropped for some reason.
	if (socket_ptr->IsSocket() && socket_ptr->m_posting==true)
	{
		socket_ptr->Close();
		socket_ptr->m_posting = false;
	}

	// And if we have new headers, and we are not posting... post them.
	if (socket_ptr->m_posting == false)
		p_dll->StartPosting(socket_ptr);
}