// MainWindow.cpp : implementation file
//

#include "stdafx.h"
#include "MainWindow.h"
#include "Network.h"
#include "resource.h"


#define NETWORKCELLHEIGHT 19
#define NETWORKTOPMARGIN 25
#define NETWORKBOTTOMMARGIN 5
#define NETWORKLEFTMARGIN 24

// CMainWindow

IMPLEMENT_DYNAMIC(CMainWindow, CWnd)
CMainWindow::CMainWindow()
{
	TRACE("CMainWindow::CMainWindow().\n");
	mp_network_system=NULL;
	m_old_network_count=0;
	m_vscroll_pos=0;
	m_vscroll_page_size=1;
	

}

CMainWindow::~CMainWindow()
{
}


BEGIN_MESSAGE_MAP(CMainWindow, CWnd)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_VSCROLL()
	ON_WM_TIMER()
	ON_WM_MOUSEWHEEL()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
END_MESSAGE_MAP()



// CMainWindow message handlers


void CMainWindow::OnSize(UINT nType, int cx, int cy)
{
	//TRACE("CMainWindow::OnSize() %d %d.\n",cx,cy);
	CWnd::OnSize(nType, cx, cy);
	m_client_rect=CRect(0,0,cx,cy);

	CDC *pdc=this->GetDC();
	if(m_back_buffer.GetSafeHandle()){
		m_back_buffer.DeleteObject();
	}
	m_back_buffer.CreateCompatibleBitmap(pdc,cx,cy);
	this->ReleaseDC(pdc);

	UpdateScrollBars();

	CRect tmp_rect(m_client_rect.left+320,5,m_client_rect.right,m_client_rect.bottom-5);
	//this->ClientToScreen(&tmp_rect);
	m_rack_window.MoveWindow(tmp_rect,TRUE);
	// TODO: Add your message handler code here
}

void CMainWindow::OnPaint()
{
	CPaintDC screen_dc(this); // device context for painting
	CDC dc;
	dc.CreateCompatibleDC(NULL);
	CBitmap *ob=dc.SelectObject(&m_back_buffer);
	CFont *of=dc.SelectObject(&m_big_font);
	CPen *op=dc.SelectObject(&m_white_pen);
	dc.FillSolidRect(&m_client_rect,RGB(0,0,0));

	Vector *nv=&mp_network_system->mv_networks;

	int alive_txt_offset=165;
	int total_txt_offset=215;


	dc.SetTextColor(RGB(255,255,255));
	dc.TextOut(NETWORKLEFTMARGIN+15,1,"Networks");
	dc.TextOut(NETWORKLEFTMARGIN+alive_txt_offset,1,"Alive /");
	dc.TextOut(NETWORKLEFTMARGIN+total_txt_offset,1,"Total");
	dc.FillSolidRect(NETWORKLEFTMARGIN,NETWORKTOPMARGIN-3,250,3,RGB(255,255,255));
	dc.SelectObject(&m_little_font);
	dc.SetBkMode(TRANSPARENT);
	
	CRect clip_rect;
	dc.GetClipBox(&clip_rect);
	CRect new_clip_rect(NETWORKLEFTMARGIN,NETWORKTOPMARGIN,NETWORKLEFTMARGIN+350,m_client_rect.bottom-NETWORKBOTTOMMARGIN);
	dc.IntersectClipRect(&new_clip_rect);

	for(int i=0;i<(int)nv->Size();i++){
		Network* nw=(Network*)nv->Get(i);

		if(nw==m_rack_window.GetSelectedNetwork()){
			RenderGradients(dc);
			dc.FillSolidRect(NETWORKLEFTMARGIN,NETWORKTOPMARGIN+i*NETWORKCELLHEIGHT+1-m_vscroll_pos,300,NETWORKCELLHEIGHT-2,RGB(0,0,200));
			//dc.FillSolidRect(NETWORKLEFTMARGIN+280,NETWORKTOPMARGIN,20,m_client_rect.bottom-(NETWORKTOPMARGIN+NETWORKBOTTOMMARGIN),RGB(0,0,200));
			
		}
		dc.TextOut(NETWORKLEFTMARGIN+15,NETWORKTOPMARGIN+i*NETWORKCELLHEIGHT+2-m_vscroll_pos,nw->m_name.c_str());

		if(nw->GetTotalRacks()>0){
			CString tmp;
			tmp.Format("%d",nw->GetAliveRacks());
			dc.TextOut(NETWORKLEFTMARGIN+alive_txt_offset+29,NETWORKTOPMARGIN+i*NETWORKCELLHEIGHT+2-m_vscroll_pos,tmp);
			int crashed=nw->GetCrashedRacks();

			if(crashed>0){
				dc.SetTextColor(RGB(255,100,100));
				tmp.Format("(%d)",crashed);
				dc.TextOut(NETWORKLEFTMARGIN+alive_txt_offset-5,NETWORKTOPMARGIN+i*NETWORKCELLHEIGHT+2-m_vscroll_pos,tmp);
				dc.SetTextColor(RGB(255,255,255));
			}

			tmp.Format("/ %d",nw->GetTotalRacks());
			dc.TextOut(NETWORKLEFTMARGIN+total_txt_offset,NETWORKTOPMARGIN+i*NETWORKCELLHEIGHT+2-m_vscroll_pos,tmp);
		}

		dc.FillSolidRect(NETWORKLEFTMARGIN,NETWORKTOPMARGIN+i*NETWORKCELLHEIGHT-m_vscroll_pos,250,1,RGB(190,190,190));

		//top outline
		dc.FillSolidRect(NETWORKLEFTMARGIN,NETWORKTOPMARGIN+i*NETWORKCELLHEIGHT+1-m_vscroll_pos,250,1,RGB(120,120,120));

		//bottom outline
		dc.FillSolidRect(NETWORKLEFTMARGIN,NETWORKTOPMARGIN+i*NETWORKCELLHEIGHT+NETWORKCELLHEIGHT-1-m_vscroll_pos,250,1,RGB(255,255,255));
	}

	screen_dc.BitBlt(0,0,m_client_rect.Width(),m_client_rect.Height(),&dc,0,0,SRCCOPY);
	dc.SelectObject(ob);
	dc.SelectObject(of);
	dc.SelectObject(op);
	//TRACE("CMainWindow::OnPaint().\n");
}

int CMainWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	TRACE("CMainWindow::OnCreate().\n");

	m_big_font.CreatePointFont(110,"Arial Bold",NULL);
	m_little_font.CreatePointFont(90,"Courier New Bold",NULL);
	m_white_pen.CreatePen(PS_SOLID,1,RGB(255,255,255));
	
	m_vscroll_bar.Create(  SBS_VERT|SBS_RIGHTALIGN|WS_CHILD|WS_VISIBLE ,  CRect(0,5,10,500),  this,  1);
	
	this->GetClientRect(&m_client_rect);
	CRect tmp_rect(m_client_rect.left+320,5,m_client_rect.right-5,m_client_rect.bottom-5);

	m_rack_window.CreateEx(NULL,NULL,NULL,WS_CLIPCHILDREN|WS_VISIBLE|WS_CHILD|WS_BORDER,tmp_rect,this,0,NULL);
	

	UpdateScrollBars();
	this->SetTimer(1,1000,NULL);
	this->SetTimer(2,10000,NULL);
	return 0;
}

BOOL CMainWindow::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default

	//return CWnd::OnEraseBkgnd(pDC);
	return TRUE;
}

void CMainWindow::SetNetworkSystem(NetworkSystem* ns)
{
	mp_network_system=ns;
	m_rack_window.SetNetworkSystem(ns);
	
}

void CMainWindow::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if(nSBCode==8)
		return;

	int tmp_pos=nPos;
	if(nSBCode==3){
		tmp_pos=pScrollBar->GetScrollPos()+m_vscroll_page_size-2*NETWORKCELLHEIGHT;	
	}
	else if(nSBCode==2){
		tmp_pos=pScrollBar->GetScrollPos()-(m_vscroll_page_size-2*NETWORKCELLHEIGHT);
	}
	tmp_pos=max(0,tmp_pos);
	tmp_pos=min(tmp_pos,pScrollBar->GetScrollLimit());
	nPos=tmp_pos;
	m_vscroll_pos=nPos;



	//invalidate the part of the screen that may change
	CRect client_rect;
	this->GetClientRect(&client_rect);
	CRect ir(0,0,350,client_rect.bottom-0);
	this->InvalidateRect(&ir,TRUE);

	pScrollBar->SetScrollPos(nPos,TRUE);

	this->SetFocus();
}

void CMainWindow::UpdateScrollBars(void)
{
	this->GetClientRect(&m_client_rect);

	if(m_vscroll_bar.GetSafeHwnd()){
		m_vscroll_bar.MoveWindow(0,NETWORKTOPMARGIN,20,m_client_rect.Height()-(NETWORKTOPMARGIN+NETWORKBOTTOMMARGIN),TRUE);
	}

	if(mp_network_system==NULL)
		return;

//#define NETWORKCELLHEIGHT 19
//#define NETWORKTOPMARGIN 25
//#define NETWORKBOTTOMMARGIN 5

	this->GetClientRect(&m_client_rect);
	int max_height=mp_network_system->mv_networks.Size();

	max_height*=NETWORKCELLHEIGHT;

	SCROLLINFO si;

	m_vscroll_bar.GetScrollInfo(&si);
	si.fMask=SIF_PAGE|SIF_RANGE;
	m_vscroll_page_size=m_client_rect.Height()-(NETWORKTOPMARGIN+NETWORKBOTTOMMARGIN);
	si.nPage=m_vscroll_page_size;
	si.nMin=0;

	si.nMax=(UINT)max((int)max_height,(int)si.nPage);
	m_vscroll_bar.SetScrollInfo(&si,TRUE);
}

void CMainWindow::OnTimer(UINT nIDEvent)
{
	if(nIDEvent==1){
		if(mp_network_system!=NULL && mp_network_system->mv_networks.Size()!=m_old_network_count){
			m_old_network_count=mp_network_system->mv_networks.Size();
			UpdateScrollBars();
		}
	}
	else if(nIDEvent==2){
		this->Invalidate(TRUE);
		m_rack_window.Invalidate(TRUE);
	}
	CWnd::OnTimer(nIDEvent);
}

BOOL CMainWindow::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	int new_pos=(int)m_vscroll_bar.GetScrollPos()-(zDelta>>1);
	OnVScroll(SB_THUMBPOSITION,new_pos,&m_vscroll_bar);

	return TRUE;//CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void CMainWindow::OnLButtonDown(UINT nFlags, CPoint point)
{
	this->SetFocus();

	Network *p_network=NULL;
	for(int i=0;i<(int)mp_network_system->mv_networks.Size() && point.y>NETWORKTOPMARGIN && point.y<m_client_rect.bottom-NETWORKBOTTOMMARGIN;i++){
		CRect rect(NETWORKLEFTMARGIN,NETWORKTOPMARGIN+i*NETWORKCELLHEIGHT-m_vscroll_pos,NETWORKLEFTMARGIN+250,NETWORKTOPMARGIN+i*NETWORKCELLHEIGHT+NETWORKCELLHEIGHT-m_vscroll_pos);
		if(rect.PtInRect(point)){
			p_network=(Network*)mp_network_system->mv_networks.Get(i);
		}
	}

	m_rack_window.SelectNetwork(p_network);

	this->Invalidate(TRUE);

	CWnd::OnLButtonDown(nFlags, point);
}

void CMainWindow::OnMouseMove(UINT nFlags, CPoint point)
{
	//this->SetFocus();
	m_rack_window.MouseOff();
	CWnd::OnMouseMove(nFlags, point);
}

void CMainWindow::RenderGradients(CDC& dc)
{
	TRIVERTEX        vert[2] ;
	GRADIENT_RECT    gRect;
	vert [0] .x      = NETWORKLEFTMARGIN+270;
	vert [0] .y      = NETWORKTOPMARGIN;
	vert [0] .Red    = 0x0000;
	vert [0] .Green  = 0x0000;
	vert [0] .Blue   = 0x0000;
	vert [0] .Alpha  = 0x0000;

	vert [1] .x      = NETWORKLEFTMARGIN+300;
	vert [1] .y      = m_client_rect.Height()-(NETWORKBOTTOMMARGIN); 
	vert [1] .Red    = 0x0000;
	vert [1] .Green  = 0x0000;
	vert [1] .Blue   = 0xff00;
	vert [1] .Alpha  = 0xff00;

	gRect.UpperLeft  = 0;
	gRect.LowerRight = 1;
	dc.GradientFill(vert,2,&gRect,1,GRADIENT_FILL_RECT_H);


	//vert [0] .x      = max(m_client_rect.Width()>>1,m_client_rect.Width()-m_pos);
	//vert [1] .x      = m_client_rect.Width()-m_pos+64;
	//dc.GradientFill(vert,2,&gRect,1,GRADIENT_FILL_RECT_H);
}

void CMainWindow::OnSetFocus(CWnd* pOldWnd)
{
	CWnd::OnSetFocus(pOldWnd);

	//TRACE("CMainWindow::OnSetFocus().\n");
}

void CMainWindow::MouseOff(void)
{
	m_rack_window.MouseOff();
}

void CMainWindow::OnDestroy()
{
	CWnd::OnDestroy();

	this->KillTimer(1);
	this->KillTimer(2);
}

void CMainWindow::SetDllListBox(CListBox& list_box)
{
	m_rack_window.SetDllListBox(list_box);
}

void CMainWindow::OnAcceleratorDownKey()
{
	Network *p_network=m_rack_window.GetSelectedNetwork();
	//Network *p_network=NULL;
	for(int i=0;i<(int)mp_network_system->mv_networks.Size();i++){
		if(p_network==mp_network_system->mv_networks.Get(i)){
			if(i<(int)mp_network_system->mv_networks.Size()-1){
				p_network=(Network*)mp_network_system->mv_networks.Get(i+1);
				break;
			}
			else{
				p_network=(Network*)mp_network_system->mv_networks.Get(0);
			}
		}
	}

	m_rack_window.SelectNetwork(p_network);

	this->Invalidate(TRUE);
}

void CMainWindow::OnAcceleratorUpKey()
{
	Network *p_network=m_rack_window.GetSelectedNetwork();
	//Network *p_network=NULL;
	for(int i=0;i<(int)mp_network_system->mv_networks.Size();i++){
		if(p_network==mp_network_system->mv_networks.Get(i)){
			if(i>0){
				p_network=(Network*)mp_network_system->mv_networks.Get(i-1);
				break;
			}
			else{
				p_network=(Network*)mp_network_system->mv_networks.Get(mp_network_system->mv_networks.Size()-1);
			}
		}
	}

	m_rack_window.SelectNetwork(p_network);

	this->Invalidate(TRUE);
}

Network* CMainWindow::GetSelectedNetwork(void)
{
	return m_rack_window.GetSelectedNetwork();
}
