// RackWindow.cpp : implementation file
//

#include "stdafx.h"
#include "RackWindow.h"
#include "DLLObject.h"
#include "..\..\RackRecoverySystem\RackRecoverInterface.h"



#define RACKCELLHEIGHT 25
#define RACKTOPMARGIN 20
#define RACKBOTTOMMARGIN 5
#define RACKLEFTMARGIN 5
#define RACKRIGHTMARGIN 5
// CRackWindow

IMPLEMENT_DYNAMIC(CRackWindow, CWnd)
CRackWindow::CRackWindow()
{
	m_vscroll_pos=0;
	mp_selected_network=NULL;
	mp_network_system=NULL;
	m_vscroll_page_size=1;

	m_tk_on_offset=1;
	m_rack_on_offset=1;
	m_rack_history_offset=1;
	m_tk_history_offset=1;
	m_mouse_over_rack=-1;
	b_mouse_on=false;
}

CRackWindow::~CRackWindow()
{
	if(mp_selected_network)
		mp_selected_network->deref();
}


BEGIN_MESSAGE_MAP(CRackWindow, CWnd)
	ON_WM_CREATE()
	ON_WM_MOUSEWHEEL()
	ON_WM_VSCROLL()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_SETFOCUS()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()



// CRackWindow message handlers


int CRackWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_big_font.CreatePointFont(100,"Arial Bold",NULL);
	m_very_little_font.CreatePointFont(85,"Courier New",NULL);
	m_little_font.CreatePointFont(90,"Courier New",NULL);
	m_little_font_bold.CreatePointFont(90,"Courier New Bold",NULL);
	m_white_pen.CreatePen(PS_SOLID,1,RGB(255,255,255));
	
	m_vscroll_bar.Create(  SBS_VERT|SBS_RIGHTALIGN|WS_CHILD|WS_VISIBLE ,  CRect(0,5,10,500),  this,  1);
	
	UpdateScrollBars();
	SetupButtons();
	return 0;
}

BOOL CRackWindow::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	int new_pos=(int)m_vscroll_bar.GetScrollPos()-(zDelta>>1);
	OnVScroll(SB_THUMBPOSITION,new_pos,&m_vscroll_bar);

	//return CWnd::OnMouseWheel(nFlags, zDelta, pt);
	return TRUE;
}

void CRackWindow::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if(nSBCode==8)
		return;

	m_remote_control_button.ShowWindow(SW_HIDE);
	m_restart_button.ShowWindow(SW_HIDE);
	m_kill_kazaas_button.ShowWindow(SW_HIDE);
	m_mouse_over_rack=-1;

	mp_selected_rack_dll_list_box->ResetContent();

	int tmp_pos=nPos;
	if(nSBCode==3){
		tmp_pos=pScrollBar->GetScrollPos()+m_vscroll_page_size-2*RACKCELLHEIGHT;	
	}
	else if(nSBCode==2){
		tmp_pos=pScrollBar->GetScrollPos()-(m_vscroll_page_size-2*RACKCELLHEIGHT);
	}
	tmp_pos=max(0,tmp_pos);
	tmp_pos=min(tmp_pos,pScrollBar->GetScrollLimit());
	nPos=tmp_pos;
	m_vscroll_pos=nPos;



	//invalidate the part of the screen that may change
	this->Invalidate(TRUE);

	pScrollBar->SetScrollPos(nPos,TRUE);

	this->SetFocus();
}

void CRackWindow::UpdateScrollBars(void)
{
	this->GetClientRect(&m_client_rect);

//#define RACKCELLHEIGHT 25
//#define RACKTOPMARGIN 5
//#define RACKBOTTOMMARGIN 5

	if(m_vscroll_bar.GetSafeHwnd()){
		m_vscroll_bar.MoveWindow(m_client_rect.right-20,RACKTOPMARGIN,20,m_client_rect.Height()-(RACKTOPMARGIN+RACKBOTTOMMARGIN),TRUE);
	}

	if(mp_selected_network==NULL)
		return;

	this->GetClientRect(&m_client_rect);
	int max_height=mp_selected_network->mv_racks.Size();

	max_height*=RACKCELLHEIGHT;

	SCROLLINFO si;

	m_vscroll_bar.GetScrollInfo(&si);
	si.fMask=SIF_PAGE|SIF_RANGE;
	m_vscroll_page_size=m_client_rect.Height()-(RACKTOPMARGIN+RACKBOTTOMMARGIN);
	si.nPage=m_vscroll_page_size;
	si.nMin=0;

	si.nMax=(UINT)max((int)max_height,(int)si.nPage);
	m_vscroll_bar.SetScrollInfo(&si,TRUE);
}

void CRackWindow::OnSize(UINT nType, int cx, int cy)
{
	//TRACE("CRackWindow::OnSize() %d %d.\n",cx,cy);
	CWnd::OnSize(nType, cx, cy);
	m_client_rect=CRect(0,0,cx,cy);

	CDC *pdc=this->GetDC();
	m_back_buffer.DeleteObject();
	m_back_buffer.m_hObject=NULL;  //have to set this to null because the CGDIObject::DeleteObject function forgets to do so.
	m_back_buffer.CreateCompatibleBitmap(pdc,cx,cy);
	this->ReleaseDC(pdc);

	UpdateScrollBars();

	m_rack_on_offset=RACKLEFTMARGIN+2;
	m_tk_on_offset=210;
	if(cx>1000){
		m_tk_history_offset=cx/3;
		m_rack_history_offset=cx/3+cx/3;
	}
	else{
		m_tk_history_offset=cx/2;
		m_rack_history_offset=cx/2+cx/4;
	}
	// TODO: Add your message handler code here
}

void CRackWindow::OnPaint()
{
	srand(0);
	CPaintDC screen_dc(this); // device context for painting
	CDC dc;
	dc.CreateCompatibleDC(NULL);
	CBitmap *ob=dc.SelectObject(&m_back_buffer);
	CFont *of=dc.SelectObject(&m_big_font);
	CPen *op=dc.SelectObject(&m_white_pen);
	dc.FillSolidRect(&m_client_rect,RGB(20,20,20));

	dc.SetTextColor(RGB(255,255,255));
	if(mp_selected_network){
		CString tmp_str;
		tmp_str.Format("%s - Rack IPs",mp_selected_network->m_name.c_str());
		dc.TextOut(RACKLEFTMARGIN,0,tmp_str);
	}
	else
		dc.TextOut(RACKLEFTMARGIN+15,0,"RACK IPs");

	//dc.TextOut(m_rack_on_offset,1,"(On/Off)");
	dc.TextOut(m_tk_on_offset,0,"TrapperKeeper");
	dc.TextOut(m_tk_history_offset,0,"TK History");
	dc.TextOut(m_rack_history_offset,0,"Rack History");

	dc.FillSolidRect(RACKLEFTMARGIN,RACKTOPMARGIN-3,m_client_rect.Width()-RACKRIGHTMARGIN,3,RGB(255,255,255));
	dc.SelectObject(&m_little_font);
	dc.SetBkMode(TRANSPARENT);
	
	CRect clip_rect;
	dc.GetClipBox(&clip_rect);
	CRect new_clip_rect(RACKLEFTMARGIN,RACKTOPMARGIN,m_client_rect.Width()-RACKRIGHTMARGIN,m_client_rect.bottom-RACKBOTTOMMARGIN);
	dc.IntersectClipRect(&new_clip_rect);

	vector <bool> vb_system_on;
	if(mp_network_system){
		if(m_client_rect.Width()>1000){
			mp_network_system->EnumerateOnTime((m_client_rect.Width()/3)-30+1,vb_system_on);
		}
		else{
			mp_network_system->EnumerateOnTime((m_client_rect.Width()>>2)-30+1,vb_system_on);
		}
	}

	CRect the_clip_rect;
	screen_dc.GetClipBox(&the_clip_rect);

	for(int i=0;mp_selected_network && i<(int)mp_selected_network->mv_racks.Size();i++){
		Rack* rack=(Rack*)mp_selected_network->mv_racks.Get(i);

		//do some prelimiary tests to see if we should draw this
		if(RACKTOPMARGIN+i*RACKCELLHEIGHT-m_vscroll_pos > the_clip_rect.bottom)
			continue;

		if(RACKTOPMARGIN+i*RACKCELLHEIGHT+RACKCELLHEIGHT-m_vscroll_pos < the_clip_rect.top)
			continue;

		int rack_on=rack->IsAlive();
		int tk_on=rack->IsTKOn();

		if(i==m_mouse_over_rack){
			CRect rack_rect(RACKLEFTMARGIN,RACKTOPMARGIN+i*RACKCELLHEIGHT-m_vscroll_pos,m_client_rect.Width()-RACKRIGHTMARGIN,RACKTOPMARGIN+i*RACKCELLHEIGHT+RACKCELLHEIGHT-m_vscroll_pos);
			dc.FillSolidRect(&rack_rect,RGB(0,80,80));
		}

		if(!rack_on){  //draw in bold red for dead rack
			dc.SelectObject(&m_little_font_bold);	
			dc.SetTextColor(RGB(255,100,100));
		}
		else{
			dc.SetTextColor(RGB(100,255,100));	
		}
		CString tmp_rack_str;
		tmp_rack_str.Format("%s v%d",rack->m_ip.c_str(),rack->m_version);
		dc.TextOut(RACKLEFTMARGIN+27,RACKTOPMARGIN+i*RACKCELLHEIGHT+5-m_vscroll_pos,tmp_rack_str);
		dc.SelectObject(&m_little_font);
		dc.SetTextColor(RGB(255,255,255));

		dc.FillSolidRect(RACKLEFTMARGIN,RACKTOPMARGIN+i*RACKCELLHEIGHT-m_vscroll_pos,m_client_rect.Width()-(RACKLEFTMARGIN+RACKRIGHTMARGIN),1,RGB(190,190,190));

		//top outline
		dc.FillSolidRect(RACKLEFTMARGIN,RACKTOPMARGIN+i*RACKCELLHEIGHT+1-m_vscroll_pos,m_client_rect.Width()-(RACKLEFTMARGIN+RACKRIGHTMARGIN),1,RGB(120,120,120));

		//bottom outline
		dc.FillSolidRect(RACKLEFTMARGIN,RACKTOPMARGIN+i*RACKCELLHEIGHT+RACKCELLHEIGHT-1-m_vscroll_pos,m_client_rect.Width()-(RACKLEFTMARGIN+RACKRIGHTMARGIN),1,RGB(255,255,255));
		if(i==mp_selected_network->mv_racks.Size()-1){
			dc.FillSolidRect(RACKLEFTMARGIN,RACKTOPMARGIN+i*RACKCELLHEIGHT+RACKCELLHEIGHT-m_vscroll_pos,m_client_rect.Width()-(RACKLEFTMARGIN+RACKRIGHTMARGIN),1,RGB(190,190,190));
			dc.FillSolidRect(RACKLEFTMARGIN,RACKTOPMARGIN+i*RACKCELLHEIGHT+RACKCELLHEIGHT+1-m_vscroll_pos,m_client_rect.Width()-(RACKLEFTMARGIN+RACKRIGHTMARGIN),1,RGB(120,120,120));	
		}

		//draw rack on status lights
		
		if(rack_on){
			//dc.FillSolidRect(m_rack_on_offset,RACKTOPMARGIN+i*RACKCELLHEIGHT+4-m_vscroll_pos,RACKCELLHEIGHT-8,RACKCELLHEIGHT-8,RGB(0,255,0));
		}
		else{
			dc.FillSolidRect(m_rack_on_offset,RACKTOPMARGIN+i*RACKCELLHEIGHT+8-m_vscroll_pos,RACKCELLHEIGHT-4,RACKCELLHEIGHT-16,RGB(255,0,0));
		}

		//draw tk on status lights
		
		if(tk_on && rack_on){
			//dc.FillSolidRect(m_tk_on_offset+45,RACKTOPMARGIN+i*RACKCELLHEIGHT+4-m_vscroll_pos,RACKCELLHEIGHT-8,RACKCELLHEIGHT-8,RGB(0,255,0));
		}
		else if(!tk_on && rack_on){
			dc.SelectObject(&m_little_font_bold);	
			dc.SetTextColor(RGB(255,100,100));
			dc.TextOut(m_tk_on_offset+30,RACKTOPMARGIN+i*RACKCELLHEIGHT+5-m_vscroll_pos,"Crashed!");
			dc.SelectObject(&m_little_font);
			dc.SetTextColor(RGB(255,255,255));

			//dc.FillSolidRect(m_tk_on_offset+45,RACKTOPMARGIN+i*RACKCELLHEIGHT+4-m_vscroll_pos,RACKCELLHEIGHT-8,RACKCELLHEIGHT-8,RGB(255,0,0));
		}
		//else{
		//	dc.FillSolidRect(m_tk_on_offset+45,RACKTOPMARGIN+i*RACKCELLHEIGHT+4-m_vscroll_pos,RACKCELLHEIGHT-8,RACKCELLHEIGHT-8,RGB(100,100,100));
		//}
		bool b_wide_graphs=false;
		
		if(m_client_rect.Width()>1000)
			b_wide_graphs=true;
		
		int history_width=(m_client_rect.Width()>>2)-30;
		int history_y=RACKTOPMARGIN+i*RACKCELLHEIGHT+5-m_vscroll_pos;
		int history_height=RACKCELLHEIGHT-10;

		if(b_wide_graphs)
			history_width=(m_client_rect.Width()/3)-30;

		if(rack->m_comment.size()>0){
			history_y=RACKTOPMARGIN+i*RACKCELLHEIGHT+3-m_vscroll_pos;
			history_height=RACKCELLHEIGHT-15;
		}

		RenderRackHistory(dc,m_rack_history_offset,history_y,history_width,history_height,rack,vb_system_on);
		RenderTKHistory(dc,m_tk_history_offset,history_y,history_width,history_height,rack,vb_system_on);

		if(rack->m_comment.size()>0){
			dc.SelectObject(&m_very_little_font);
			dc.TextOut(m_tk_history_offset+2,history_y+history_height-2,rack->m_comment.c_str());
			dc.SelectObject(&m_little_font);
		}
	}

	screen_dc.BitBlt(0,0,m_client_rect.Width(),m_client_rect.Height(),&dc,0,0,SRCCOPY);
	dc.SelectObject(ob);
	dc.SelectObject(of);
	dc.SelectObject(op);
}

void CRackWindow::SelectNetwork(Network* p_network)
{
	
	if(mp_selected_network)
		mp_selected_network->deref();

	mp_selected_network=p_network;
	if(mp_selected_network)
		mp_selected_network->ref();
	



	UpdateScrollBars();
	//SetupButtons();
	OnVScroll(SB_THUMBPOSITION,0,&m_vscroll_bar);
	

}

void CRackWindow::OnLButtonDown(UINT nFlags, CPoint point)
{
	this->SetFocus();
	CWnd::OnLButtonDown(nFlags, point);
}

void CRackWindow::OnMouseMove(UINT nFlags, CPoint point)
{
	Rack *p_rack=NULL;
	b_mouse_on=true;
	int old_mouse_over_rack=m_mouse_over_rack;
	m_mouse_over_rack=-1;
	CRect invalidate_rect1;
	CRect invalidate_rect2;
	for(int i=0;mp_selected_network && i<(int)mp_selected_network->mv_racks.Size() && point.y>RACKTOPMARGIN;i++){
		CRect rect(RACKLEFTMARGIN,RACKTOPMARGIN+i*RACKCELLHEIGHT-m_vscroll_pos,m_client_rect.Width()-RACKRIGHTMARGIN,RACKTOPMARGIN+i*RACKCELLHEIGHT+RACKCELLHEIGHT-m_vscroll_pos);
		if(rect.PtInRect(point)){
			invalidate_rect1=rect;
			m_mouse_over_rack=i;
		}
	}

	if(m_mouse_over_rack!=old_mouse_over_rack){
		if(old_mouse_over_rack!=-1){
			invalidate_rect2=CRect(RACKLEFTMARGIN,RACKTOPMARGIN+old_mouse_over_rack*RACKCELLHEIGHT-m_vscroll_pos,m_client_rect.Width()-RACKRIGHTMARGIN,RACKTOPMARGIN+old_mouse_over_rack*RACKCELLHEIGHT+RACKCELLHEIGHT-m_vscroll_pos);
			this->InvalidateRect(invalidate_rect2,FALSE);
		}
		if(m_mouse_over_rack!=-1){
			this->InvalidateRect(invalidate_rect1,FALSE);
		}
		else{
			mp_selected_rack_dll_list_box->ResetContent();
		}


		//this->Invalidate(TRUE);
	}

	if(m_mouse_over_rack==-1){	
		m_remote_control_button.ShowWindow(SW_HIDE);
		m_restart_button.ShowWindow(SW_HIDE);
		m_kill_kazaas_button.ShowWindow(SW_HIDE);
	}
	else if(m_mouse_over_rack!=old_mouse_over_rack){
		CRect rect(RACKLEFTMARGIN+135,RACKTOPMARGIN+m_mouse_over_rack*RACKCELLHEIGHT-m_vscroll_pos+1,RACKLEFTMARGIN+170,RACKTOPMARGIN+m_mouse_over_rack*RACKCELLHEIGHT+RACKCELLHEIGHT-m_vscroll_pos-1);	
		m_remote_control_button.MoveWindow(rect,TRUE);
		rect.OffsetRect(38+10,0);
		rect.right+=24;
		m_restart_button.MoveWindow(rect,TRUE);
		rect.right+=24;
		rect.OffsetRect(38+24+10,0);
		m_kill_kazaas_button.MoveWindow(rect,TRUE);

		m_remote_control_button.UpdateWindow();
		m_restart_button.UpdateWindow();
		m_kill_kazaas_button.UpdateWindow();
		if(old_mouse_over_rack==-1){
			m_remote_control_button.ShowWindow(SW_SHOWNORMAL);
			m_restart_button.ShowWindow(SW_SHOWNORMAL);
			m_kill_kazaas_button.ShowWindow(SW_SHOWNORMAL);
		}

		
		mp_selected_rack_dll_list_box->ResetContent();
		if(mp_selected_network && (int)mp_selected_network->mv_racks.Size()>m_mouse_over_rack){
			Rack *rack=(Rack*)mp_selected_network->mv_racks.Get(m_mouse_over_rack);
			Vector v_dlls;
			rack->GetDLLs(v_dlls);
			for(int i=0;i<(int)v_dlls.Size();i++){
				DLLObject *dll=(DLLObject*)v_dlls.Get(i);
				mp_selected_rack_dll_list_box->AddString(dll->m_name.c_str());
			}
		}
	}


	CWnd::OnMouseMove(nFlags, point);
}

Network* CRackWindow::GetSelectedNetwork(void)
{
	return mp_selected_network;
}

void CRackWindow::RenderTKHistory(CDC &dc, int x, int y, int width, int height, Rack* rack,vector <bool> &vb_system_on)
{
	vector <bool> vb_on;
	rack->EnumerateTKHistory(width+1,vb_on);
	for(int i=width;i>=0;i--){
		if(!vb_on[width-i] && vb_system_on[width-i]){
			dc.FillSolidRect(x+i,y,1,height,RGB(255,0,0));
		}
		else if(vb_system_on[width-i]){
			dc.FillSolidRect(x+i,y,1,height,RGB(0,255,0));
		}
		else{
			dc.FillSolidRect(x+i,y,1,height,RGB(80,80,80));
		}
	}

	//draw the 1 day tick marks
	int hour=1;
	for(int i=width;i>=0;i--,hour++){
		if(hour%24==0){
			dc.FillSolidRect(x+i-1,y,3,height/2-4,RGB(0,0,0));
			dc.FillSolidRect(x+i-1,y+height-height/2+4,3,height/2-4,RGB(0,0,0));
		}
	}
}

void CRackWindow::RenderRackHistory(CDC& dc, int x, int y, int width, int height, Rack* rack,vector <bool> &vb_system_on)
{
	vector <bool> vb_on;
	rack->EnumerateRackHistory(width+1,vb_on);
	for(int i=width;i>=0;i--){
		if(!vb_on[width-i] && vb_system_on[width-i]){
			dc.FillSolidRect(x+i,y,1,height,RGB(255,0,0));
		}
		else if(vb_system_on[width-i]){
			dc.FillSolidRect(x+i,y,1,height,RGB(0,255,0));
		}
		else{
			dc.FillSolidRect(x+i,y,1,height,RGB(80,80,80));
		}
	}

	//draw the 1 day tick marks
	int hour=1;
	for(int i=width;i>=0;i--,hour++){
		if(hour%24==0){
			dc.FillSolidRect(x+i-1,y,3,height/2-4,RGB(0,0,0));
			dc.FillSolidRect(x+i-1,y+height-height/2+4,3,height/2-4,RGB(0,0,0));
		}
	}
}

void CRackWindow::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);

	//TRACE("CRackWindow::OnSetFocus().\n");
}

void CRackWindow::SetupButtons(void)
{
	/*
	for(int i=0;i<70;i++){
		m_remote_control_buttons[i].DestroyWindow();
		m_restart_buttons[i].DestroyWindow();
	}
	*/

	/*
	for(int i=0;mp_selected_network && i<(int)mp_selected_network->mv_racks.Size();i++){
		CRect rect(RACKLEFTMARGIN+140,RACKTOPMARGIN+i*RACKCELLHEIGHT-m_vscroll_pos+2,RACKLEFTMARGIN+175,RACKTOPMARGIN+i*RACKCELLHEIGHT+RACKCELLHEIGHT-m_vscroll_pos-2);
		m_remote_control_buttons[i].Create("R/C",WS_VISIBLE,rect,this,i);
		rect.OffsetRect(40,0);
		rect.right+=30;
		m_restart_buttons[i].Create("Restart",WS_VISIBLE,rect,this,i);
	}
	*/

	CRect rect(RACKLEFTMARGIN+140,RACKTOPMARGIN+RACKCELLHEIGHT-m_vscroll_pos+2,RACKLEFTMARGIN+175,RACKTOPMARGIN+RACKCELLHEIGHT+RACKCELLHEIGHT-m_vscroll_pos-2);
	m_remote_control_button.Create("R/C",WS_CHILD,rect,this,50);
	rect.OffsetRect(40,0);
	rect.right+=30;
	m_restart_button.Create("Restart",WS_CHILD,rect,this,51);
	rect.right+=70;
	m_kill_kazaas_button.Create("Kill Kazaas",WS_CHILD,rect,this,52);
}

void CRackWindow::MouseOff(void)
{
	if(b_mouse_on){
		m_remote_control_button.ShowWindow(SW_HIDE);
		m_restart_button.ShowWindow(SW_HIDE);
		m_kill_kazaas_button.ShowWindow(SW_HIDE);
		m_mouse_over_rack=-1;
		this->Invalidate(TRUE);
		mp_selected_rack_dll_list_box->ResetContent();
	}
	b_mouse_on=false;
}

void CRackWindow::SetNetworkSystem(NetworkSystem* p_network_system)
{
	mp_network_system=p_network_system;
}

BOOL CRackWindow::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	//return CWnd::OnEraseBkgnd(pDC);
	return TRUE;
}

void CRackWindow::SetDllListBox(CListBox& list_box)
{
	mp_selected_rack_dll_list_box=&list_box;
}

BOOL CRackWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if(wParam==50){  //remote control
		if(mp_selected_network && (int)mp_selected_network->mv_racks.Size()>m_mouse_over_rack){
			Rack *rack=(Rack*)mp_selected_network->mv_racks.Get(m_mouse_over_rack);
			mp_network_system->RemoteControlRack(rack->m_ip.c_str());
		}
	}
	else if(wParam==51){  //restart
		if(mp_selected_network && (int)mp_selected_network->mv_racks.Size()>m_mouse_over_rack){
			Rack *rack=(Rack*)mp_selected_network->mv_racks.Get(m_mouse_over_rack);
			RackRecoverInterface rri;
			rri.RestartRack(rack->m_ip.c_str());
		}

	}

	else if(wParam==52){  //restart
		if(mp_selected_network && (int)mp_selected_network->mv_racks.Size()>m_mouse_over_rack){
			Rack *rack=(Rack*)mp_selected_network->mv_racks.Get(m_mouse_over_rack);
			RackRecoverInterface rri;
			rri.KillKazaas(rack->m_ip.c_str());
		}

	}

	return CWnd::OnCommand(wParam, lParam);
}
