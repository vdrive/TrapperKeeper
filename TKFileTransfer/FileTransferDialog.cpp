// FileTransferDialog.cpp : implementation file
//

#include "stdafx.h"
#include "FileTransferDialog.h"
#include "DllInterface.h"

#define HEADERHEIGHT 20
#define CELLHEIGHT 26
#define LMARGIN 5
#define RMARGIN 20
#define TMARGIN 25
#define BMARGIN 5

int FileTransferDialog::FileStat::sort_mode=0;

//The one and only dialog.  There is and always will be only one FileTransferDialog and thus we make it a global object.  This avoids giving the 4-5 classes that reference it pointers of type FileTransferDialog.
namespace filetransferspace{
	FileTransferDialog g_file_transfer_dialog;
	FileCacher g_file_cacher;
}

using namespace filetransferspace;

IMPLEMENT_DYNAMIC(FileTransferDialog, CDialog)
FileTransferDialog::FileTransferDialog(CWnd* pParent /*=NULL*/)
	: CDialog(FileTransferDialog::IDD, pParent)
{
	mb_valid=false;
	p_dll=NULL;
	m_vscroll_pos=0;
	m_cached_file_chunks=0;
	m_cached_hits=0;
	m_cached_hits_per_sec=0;
}

FileTransferDialog::~FileTransferDialog()
{
}

void FileTransferDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(FileTransferDialog, CDialog)
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_WM_SIZING()
	ON_WM_VSCROLL()
	ON_BN_CLICKED(IDC_CLEARFAILURES, ClearFailures)
	ON_BN_CLICKED(IDC_CLEARCOMPLETED, ClearCompleted)
	ON_BN_CLICKED(IDC_SORTIP, SortByIP)
	ON_BN_CLICKED(IDC_SORTFILENAME, SortByName)
	ON_BN_CLICKED(IDC_SHOWP2PINFO, OnShowP2PInfo)
END_MESSAGE_MAP()


// FileTransferDialog message handlers

//called by MFC for our SetTimer request.  This handles posting messages to the GUI at a fixed readable rate.
void FileTransferDialog::OnTimer(UINT nIDEvent)
{
	static bool b_in_timer=false;
	static unsigned long last_cache_measurement=timeGetTime();
	if(b_in_timer)
		return;
	b_in_timer=true;
	
	if(nIDEvent==1 && !mb_valid){
		for(UINT i=0;i<mv_send_failed_stats.Size();i++){
			FileStat* fs=(FileStat*)mv_send_failed_stats.Get(i);
			if(fs->IsReallyOld()){
				mv_send_failed_stats.Remove(i);
				i--;
			}
		}

		for(UINT i=0;i<mv_get_complete_stats.Size();i++){
			FileStat* fs=(FileStat*)mv_get_complete_stats.Get(i);
			if(fs->IsReallyOld()){
				mv_get_complete_stats.Remove(i);
				i--;
			}
		}


		for(UINT i=0;i<mv_send_complete_stats.Size();i++){
			FileStat* fs=(FileStat*)mv_send_complete_stats.Get(i);
			if(fs->IsReallyOld()){
				mv_send_complete_stats.Remove(i);
				i--;
			}
		}

		for(UINT i=0;i<mv_get_failed_stats.Size();i++){
			FileStat* fs=(FileStat*)mv_get_failed_stats.Get(i);
			if(fs->IsReallyOld()){
				mv_get_failed_stats.Remove(i);
				i--;
			}
		}

		this->InvalidateRect(NULL,TRUE);
		UpdateScrollBars();
		mb_valid=true;
	}
	else if(nIDEvent==2){
		this->InvalidateRect(NULL,TRUE);
		UpdateScrollBars();
		mb_valid=true;
		this->SetCachedFileChunks(g_file_cacher.GetCount());
		m_cached_hits_per_sec=(float)m_cached_hits/(((float)((timeGetTime()-last_cache_measurement)))/1000.0f);
		m_cached_hits=0;
		last_cache_measurement=timeGetTime();
	}
	else if(nIDEvent==3){
		//m_system.WriteToLog("FileTransferDialog::OnTimer() BEGIN update system.");
		m_system.Update();
		//m_system.WriteToLog("FileTransferDialog::OnTimer() END update system.");
	}
	else if(nIDEvent==4){
		UINT i=0;
		for(;i<mv_send_jobs.Size();i++){
			SendJob *job=(SendJob*)mv_send_jobs.Get(i);
			if(job->Process()){  //we can process this job now.
				mv_send_jobs.Remove(i);
				i--;
			}
		}
	}

	b_in_timer=false;
	CDialog::OnTimer(nIDEvent);
}

FileTransferDialog::SendJob::SendJob(ComLink *link,FileTransferJob *job){
	p_com_link=link;
	p_job=job;
}

//call to handle this job
bool FileTransferDialog::SendJob::Process(){
	if(p_job->IsDoneLoading()){
		p_job->Process(p_com_link);
		return true;
	}
	else if(p_job->IsLoading()){  //this job is doing a threaded data load.
		return false;
	}
	else{
		p_job->BeginLoad();
		return false;
	}
}

//called by MFC when the dialog's controls have been created
BOOL FileTransferDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_link.Register(p_dll,DllInterface::m_app_id.m_app_id);
	m_link.SetService(&m_system);
	m_system.SetComLink(&m_link);

	m_vscroll_bar.Create(  SBS_VERT|SBS_RIGHTALIGN|WS_CHILD|WS_VISIBLE ,  CRect(700,5,750,400),  this,  1);
	this->RepositionScrollBars();
	UpdateScrollBars();

	title_font.CreatePointFont(90,"Arial Bold");
	data_font.CreatePointFont(80,"Arial");

	g_file_cacher.StartThread();
	m_system.StartFileTransferService();

	m_p2p_info_dialog.SetFileTransferService(&m_system);
	m_p2p_info_dialog.Create(IDD_P2PINFODIALOG,CWnd::GetDesktopWindow());

	this->SetTimer(1,3000,NULL);  //check for activity and redraw at most once every 3ish seconds
	this->SetTimer(2,10000,NULL);  //redraw to update times every 5 seconds
	this->SetTimer(3,333,NULL);  //update the job processing really fast
	this->SetTimer(4,600,NULL);  //redraw to update times every 5 seconds

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

//called by MFC when the dialog is actually destroyed from memory
void FileTransferDialog::OnDestroy()
{
	CDialog::OnDestroy();

	g_file_cacher.StopThread();
	m_system.StopFileTransferService();
	this->KillTimer(1);
	this->KillTimer(2);
	this->KillTimer(3);
	this->KillTimer(4);
}

//called by MFC when the size of the dialog changes
void FileTransferDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	this->RepositionScrollBars();
	UpdateScrollBars();

	CDC *dc=this->GetDC();
	m_back_buffer.DeleteObject();
	m_back_buffer.CreateCompatibleBitmap(dc,cx,cy);

	this->ReleaseDC(dc);
	this->InvalidateRect(NULL);
}

void FileTransferDialog::OnPaint()
{
	CSingleLock lock(&m_lock,TRUE);
	CPaintDC screen_dc(this); // device context for painting

	CDC dc;
	dc.CreateCompatibleDC(NULL);
	CBitmap *old_bitmap=dc.SelectObject(&m_back_buffer);

	CRect client_rect;
	this->GetClientRect(&client_rect);
	dc.FillSolidRect(&client_rect,RGB(120,120,120));

	CPen light_pen(PS_SOLID,1,RGB(150,150,150));
	CPen dark_pen(PS_SOLID,1,RGB(100,100,100));
	CPen white_pen(PS_SOLID,1,RGB(255,255,255));

	CFont*	old_font=dc.SelectObject(&title_font);
	CPen*	old_pen=dc.SelectObject(&white_pen);
	dc.SetBkMode(TRANSPARENT);
	
	//int HEADERHEIGHT=20;
	int cwidth=(client_rect.Width()-(LMARGIN+RMARGIN))/6;
	int width=cwidth*6;

	dc.FillSolidRect(LMARGIN,TMARGIN,width,client_rect.Height()-(TMARGIN+BMARGIN),RGB(0,0,0));
	CRect clip_rect;
	dc.GetClipBox(clip_rect);

	//draw the header
	dc.MoveTo(LMARGIN,TMARGIN);
	dc.LineTo(LMARGIN+width,TMARGIN);

	dc.MoveTo(LMARGIN,TMARGIN+HEADERHEIGHT);
	dc.LineTo(LMARGIN+width,TMARGIN+HEADERHEIGHT);

	dc.MoveTo(LMARGIN,client_rect.Height()-BMARGIN);
	dc.LineTo(LMARGIN+width,client_rect.Height()-BMARGIN);

	dc.SetTextColor(RGB(255,255,255));
	CString str_cache;
	str_cache.Format("Cache Info:  %d chunks, %.2f hits per second",m_cached_file_chunks,m_cached_hits_per_sec);
	dc.TextOut(520,10,str_cache);

	//draw the columns
	dc.SetTextColor(RGB(0,230,0));
	
	static const char* cstr_headers[]={"Receiving","Failed Receiving","Done Receiving","Sending","Failed Sending","Done Sending"};
	dc.SetTextAlign(TA_TOP|TA_CENTER);
	for(UINT i=0;i<7;i++){
		dc.SelectObject(&white_pen);
		dc.MoveTo(LMARGIN+i*cwidth,TMARGIN);
		dc.LineTo(LMARGIN+i*cwidth,client_rect.Height()-BMARGIN);
		if(i<6){
			CString header;
			switch(i){
				case 0: header.Format("%s - %d",cstr_headers[i],mv_get_stats.Size()); break;
				case 1: header.Format("%s - %d",cstr_headers[i],mv_get_failed_stats.Size()); break;
				case 2: header.Format("%s - %d",cstr_headers[i],mv_get_complete_stats.Size()); break;
				case 3: header.Format("%s - %d",cstr_headers[i],mv_send_stats.Size()); break;
				case 4: header.Format("%s - %d",cstr_headers[i],mv_send_failed_stats.Size()); break;
				case 5: header.Format("%s - %d",cstr_headers[i],mv_send_complete_stats.Size()); break;
			}
			
			dc.SelectObject(&light_pen);
			//draw the light highlights top left
			dc.MoveTo(LMARGIN+i*cwidth+1,TMARGIN+1);
			dc.LineTo(LMARGIN+i*cwidth+cwidth-1,TMARGIN+1);
			dc.MoveTo(LMARGIN+i*cwidth+1,TMARGIN+1);
			dc.LineTo(LMARGIN+i*cwidth+1,TMARGIN+HEADERHEIGHT-1);

			//draw the dark highlights bottom right
			dc.SelectObject(&dark_pen);
			dc.MoveTo(LMARGIN+i*cwidth+1,TMARGIN+HEADERHEIGHT-1);
			dc.LineTo(LMARGIN+i*cwidth+cwidth-1,TMARGIN+HEADERHEIGHT-1);
			dc.MoveTo(LMARGIN+i*cwidth+cwidth-1,TMARGIN+1);
			dc.LineTo(LMARGIN+i*cwidth+cwidth-1,TMARGIN+HEADERHEIGHT-1);

			CSize size=dc.GetOutputTextExtent(header);
			dc.TextOut(LMARGIN+i*cwidth+(cwidth>>1),TMARGIN+(HEADERHEIGHT>>1)-(size.cy>>1),header);
		}
	}
	
	CRect new_clip_rect(LMARGIN,TMARGIN+HEADERHEIGHT,LMARGIN+width+1,client_rect.bottom-BMARGIN+1);
	dc.IntersectClipRect(&new_clip_rect);

	dc.SelectObject(&data_font);
	dc.SetTextColor(RGB(255,255,255));
	//draw the getting files
	int y_pos=TMARGIN+HEADERHEIGHT-m_vscroll_pos;
	//int CELLHEIGHT=26;
	dc.SelectObject(&white_pen);
	int ip_width=85;
	int column_index=0;
	for(UINT i=0;i<mv_get_stats.Size() && y_pos<client_rect.Height();i++){
		if(y_pos<0){
			y_pos+=CELLHEIGHT;
			continue;
		}
		FileStat* fs=(FileStat*)mv_get_stats.Get(i);
		//global outline
		dc.Draw3dRect(LMARGIN+column_index*cwidth+1,y_pos,cwidth-2,(CELLHEIGHT),RGB(200,200,200),RGB(100,100,100));
		//progress bar outline
		dc.Draw3dRect(LMARGIN+column_index*cwidth+ip_width,y_pos+(CELLHEIGHT>>1),cwidth-ip_width-1,(CELLHEIGHT>>1),RGB(50,250,50),RGB(50,250,50));
		dc.FillSolidRect(LMARGIN+column_index*cwidth+ip_width+1,y_pos+(CELLHEIGHT>>1)+1,(int)((float)(cwidth-ip_width-3)*fs->GetProgress()),(CELLHEIGHT>>1)-2,RGB(50,50,255));

		CString name=fs->GetFileName();
		name.Replace("c:\\syncher\\rcv","");
		name.Trim();
		while(name.GetLength()>1){
			CString temp="..."+name;
			CSize size=dc.GetOutputTextExtent(temp);
			if(size.cx>cwidth-5)
				name=name.Mid(1);
			else break;
		}
		name="..."+name;
		dc.TextOut(LMARGIN+column_index*cwidth+(cwidth>>1),y_pos,name);
		dc.TextOut(LMARGIN+column_index*cwidth+(ip_width>>1),y_pos+(CELLHEIGHT>>1),CString(fs->GetPeer()));
		
		y_pos+=CELLHEIGHT;
	}

	//draw the failed gets
	y_pos=TMARGIN+HEADERHEIGHT-m_vscroll_pos;
	column_index=1;
	for(UINT i=0;i<mv_get_failed_stats.Size() && y_pos<client_rect.Height();i++){
		if(y_pos<0){
			y_pos+=CELLHEIGHT;
			continue;
		}
		FileStat* fs=(FileStat*)mv_get_failed_stats.Get(i);
		//global outline
		dc.Draw3dRect(LMARGIN+column_index*cwidth+1,y_pos,cwidth-2,(CELLHEIGHT),RGB(200,200,200),RGB(100,100,100));
		//progress bar outline
		dc.Draw3dRect(LMARGIN+column_index*cwidth+ip_width,y_pos+(CELLHEIGHT>>1),cwidth-ip_width-1,(CELLHEIGHT>>1),RGB(50,250,50),RGB(50,250,50));
		dc.FillSolidRect(LMARGIN+column_index*cwidth+ip_width+1,y_pos+(CELLHEIGHT>>1)+1,(int)((float)(cwidth-ip_width-3)*fs->GetProgress()),(CELLHEIGHT>>1)-2,RGB(255,20,25));

		CString name=fs->GetFileName();
		name.Replace("c:\\syncher\\rcv","");
		name.Trim();
		while(name.GetLength()>1){
			CString temp="..."+name;
			CSize size=dc.GetOutputTextExtent(temp);
			if(size.cx>cwidth-5)
				name=name.Mid(1);
			else break;
		}
		name="..."+name;
		dc.TextOut(LMARGIN+column_index*cwidth+(cwidth>>1),y_pos,name);
		dc.TextOut(LMARGIN+column_index*cwidth+(ip_width>>1),y_pos+(CELLHEIGHT>>1),CString(fs->GetPeer()));
		
		y_pos+=CELLHEIGHT;
	}

	//draw the successful gets
	y_pos=TMARGIN+HEADERHEIGHT-m_vscroll_pos;
	column_index=2;
	for(UINT i=0;i<mv_get_complete_stats.Size() && y_pos<client_rect.Height();i++){
		if(y_pos<0){
			y_pos+=CELLHEIGHT;
			continue;
		}
		FileStat* fs=(FileStat*)mv_get_complete_stats.Get(i);
		//global outline
		dc.Draw3dRect(LMARGIN+column_index*cwidth+1,y_pos,cwidth-2,(CELLHEIGHT),RGB(200,200,200),RGB(100,100,100));
		//progress bar outline
		dc.Draw3dRect(LMARGIN+column_index*cwidth+ip_width,y_pos+(CELLHEIGHT>>1),cwidth-ip_width-1,(CELLHEIGHT>>1),RGB(255,255,255),RGB(220,220,220));
		dc.FillSolidRect(LMARGIN+column_index*cwidth+ip_width+1,y_pos+(CELLHEIGHT>>1)+1,(int)((float)(cwidth-ip_width-3)*fs->GetProgress()),(CELLHEIGHT>>1)-2,RGB(20,255,20));

		CString name=fs->GetFileName();
		name.Replace("c:\\syncher\\rcv","");
		name.Trim();
		while(name.GetLength()>1){
			CString temp="..."+name;
			CSize size=dc.GetOutputTextExtent(temp);
			if(size.cx>cwidth-5)
				name=name.Mid(1);
			else break;
		}
		name="..."+name;
		dc.TextOut(LMARGIN+column_index*cwidth+(cwidth>>1),y_pos,name);
		dc.TextOut(LMARGIN+column_index*cwidth+(ip_width>>1),y_pos+(CELLHEIGHT>>1),CString(fs->GetPeer()));
		
		y_pos+=CELLHEIGHT;
	}

	y_pos=TMARGIN+HEADERHEIGHT-m_vscroll_pos;
	column_index=3;
	for(UINT i=0;i<mv_send_stats.Size() && y_pos<client_rect.Height();i++){
		if(y_pos<0){
			y_pos+=CELLHEIGHT;
			continue;
		}
		FileStat* fs=(FileStat*)mv_send_stats.Get(i);
		//global outline
		dc.Draw3dRect(LMARGIN+column_index*cwidth+1,y_pos,cwidth-2,(CELLHEIGHT),RGB(200,200,200),RGB(100,100,100));
		//progress bar outline
		dc.Draw3dRect(LMARGIN+column_index*cwidth+ip_width,y_pos+(CELLHEIGHT>>1),cwidth-ip_width-1,(CELLHEIGHT>>1),RGB(50,250,50),RGB(50,250,50));
		dc.FillSolidRect(LMARGIN+column_index*cwidth+ip_width+1,y_pos+(CELLHEIGHT>>1)+1,(int)((float)(cwidth-ip_width-3)*fs->GetProgress()),(CELLHEIGHT>>1)-2,RGB(50,50,255));

		CString name=fs->GetFileName();
		name.Replace("c:\\syncher\\rcv","");
		name.Trim();
		while(name.GetLength()>1){
			CString temp="..."+name;
			CSize size=dc.GetOutputTextExtent(temp);
			if(size.cx>cwidth-5)
				name=name.Mid(1);
			else break;
		}
		name="..."+name;
		dc.TextOut(LMARGIN+column_index*cwidth+(cwidth>>1),y_pos,name);
		dc.TextOut(LMARGIN+column_index*cwidth+(ip_width>>1),y_pos+(CELLHEIGHT>>1),CString(fs->GetPeer()));
		
		y_pos+=CELLHEIGHT;
	}

	//draw the failed sends
	y_pos=TMARGIN+HEADERHEIGHT-m_vscroll_pos;
	column_index=4;
	for(UINT i=0;i<mv_send_failed_stats.Size() && y_pos<client_rect.Height();i++){
		if(y_pos<0){
			y_pos+=CELLHEIGHT;
			continue;
		}
		FileStat* fs=(FileStat*)mv_send_failed_stats.Get(i);
		//global outline
		dc.Draw3dRect(LMARGIN+column_index*cwidth+1,y_pos,cwidth-2,(CELLHEIGHT),RGB(200,200,200),RGB(100,100,100));
		//progress bar outline
		dc.Draw3dRect(LMARGIN+column_index*cwidth+ip_width,y_pos+(CELLHEIGHT>>1),cwidth-ip_width-1,(CELLHEIGHT>>1),RGB(50,250,50),RGB(50,250,50));
		dc.FillSolidRect(LMARGIN+column_index*cwidth+ip_width+1,y_pos+(CELLHEIGHT>>1)+1,(int)((float)(cwidth-ip_width-3)*fs->GetProgress()),(CELLHEIGHT>>1)-2,RGB(255,20,25));

		CString name=fs->GetFileName();
		name.Replace("c:\\syncher\\rcv","");
		name.Trim();
		while(name.GetLength()>1){
			CString temp="..."+name;
			CSize size=dc.GetOutputTextExtent(temp);
			if(size.cx>cwidth-5)
				name=name.Mid(1);
			else break;
		}
		name="..."+name;
		dc.TextOut(LMARGIN+column_index*cwidth+(cwidth>>1),y_pos,name);
		dc.TextOut(LMARGIN+column_index*cwidth+(ip_width>>1),y_pos+(CELLHEIGHT>>1),CString(fs->GetPeer()));
		
		y_pos+=CELLHEIGHT;
	}

	//draw the successful sends
	y_pos=TMARGIN+HEADERHEIGHT-m_vscroll_pos;
	column_index=5;
	for(UINT i=0;i<mv_send_complete_stats.Size() && y_pos<client_rect.Height();i++){
		if(y_pos<0){
			y_pos+=CELLHEIGHT;
			continue;
		}
		FileStat* fs=(FileStat*)mv_send_complete_stats.Get(i);
		//global outline
		dc.Draw3dRect(LMARGIN+column_index*cwidth+1,y_pos,cwidth-2,(CELLHEIGHT),RGB(200,200,200),RGB(100,100,100));
		//progress bar outline
		dc.Draw3dRect(LMARGIN+column_index*cwidth+ip_width,y_pos+(CELLHEIGHT>>1),cwidth-ip_width-1,(CELLHEIGHT>>1),RGB(255,255,255),RGB(220,220,220));
		dc.FillSolidRect(LMARGIN+column_index*cwidth+ip_width+1,y_pos+(CELLHEIGHT>>1)+1,(int)((float)(cwidth-ip_width-3)*fs->GetProgress()),(CELLHEIGHT>>1)-2,RGB(20,255,20));

		CString name=fs->GetFileName();
		name.Replace("c:\\syncher\\rcv","");
		name.Trim();
		while(name.GetLength()>1){
			CString temp="..."+name;
			CSize size=dc.GetOutputTextExtent(temp);
			if(size.cx>cwidth-5)
				name=name.Mid(1);
			else break;
		}
		name="..."+name;
		dc.TextOut(LMARGIN+column_index*cwidth+(cwidth>>1),y_pos,name);
		dc.TextOut(LMARGIN+column_index*cwidth+(ip_width>>1),y_pos+(CELLHEIGHT>>1),CString(fs->GetPeer()));
		
		y_pos+=CELLHEIGHT;
	}

	screen_dc.BitBlt(0,0,client_rect.Width(),client_rect.Height(),&dc,0,0,SRCCOPY);
	dc.SelectObject(old_font);
	dc.SelectObject(old_pen);
	dc.SelectObject(old_bitmap);
}

void FileTransferDialog::GetFailed(UINT id)
{
	CSingleLock lock(&m_lock,TRUE);
	for(UINT i=0;i<mv_get_stats.Size();i++){
		FileStat* fs=(FileStat*)mv_get_stats.Get(i);
		if(fs->GetID()==id){//move the filestat over to the failed vector
			mv_get_failed_stats.Add(fs);
			mv_get_stats.Remove(i);
			break;
		}
	}

	if(mv_get_failed_stats.Size()>4000){
		mv_get_failed_stats.RemoveRange(0,1000);
	}

	mb_valid=false;
}

void FileTransferDialog::GetProgress(UINT id, float progress)
{
	CSingleLock lock(&m_lock,TRUE);
	for(UINT i=0;i<mv_get_stats.Size();i++){
		FileStat* fs=(FileStat*)mv_get_stats.Get(i);
		if(fs->GetID()==id){
			fs->SetProgress(progress);
			break;
		}
	}
	mb_valid=false;
}

void FileTransferDialog::AddSendJob(FileTransferJob* job, ComLink* link)
{
	for(UINT i=0;i<mv_send_jobs.Size();i++){
		SendJob *sj=(SendJob*)mv_send_jobs.Get(i);
		if(sj->IsJob(job)){
			ASSERT(0);
			return;
		}
	}
	job->ResetLoad();
	mv_send_jobs.Add(new SendJob(link,job));
}

void FileTransferDialog::SetDll(Dll *dll)
{
	p_dll=dll;
}

void FileTransferDialog::NewGet(const char* file_name, const char* peer, UINT id)
{
	CSingleLock lock(&m_lock,TRUE);
	mv_get_stats.Add(new FileStat(file_name,peer,id));

	mb_valid=false;
}

void FileTransferDialog::GetDone(UINT id)
{
	CSingleLock lock(&m_lock,TRUE);
	for(UINT i=0;i<mv_get_stats.Size();i++){
		FileStat* fs=(FileStat*)mv_get_stats.Get(i);
		if(fs->GetID()==id){//move the filestat over to the failed vector
			mv_get_complete_stats.Add(fs);
			mv_get_stats.Remove(i);
			break;
		}
	}

	if(mv_get_complete_stats.Size()>4000){
		mv_get_complete_stats.RemoveRange(0,1000);
	}

	mb_valid=false;
}

HBRUSH FileTransferDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	static HBRUSH background_brush=::CreateSolidBrush(RGB(0,0,100));
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO:  Change any attributes of the DC here

	// TODO:  Return a different brush if the default is not desired
	return background_brush;
}

void FileTransferDialog::NewSend(const char* file_name, const char* peer, UINT id)
{
	CSingleLock lock(&m_lock,TRUE);
	mv_send_stats.Add(new FileStat(file_name,peer,id));

	mb_valid=false;
}

void FileTransferDialog::SendDone(UINT id)
{
	CSingleLock lock(&m_lock,TRUE);
	
	for(UINT i=0;i<mv_send_stats.Size();i++){
		FileStat* fs=(FileStat*)mv_send_stats.Get(i);
		if(fs->GetID()==id){//move the filestat over to the failed vector
			mv_send_complete_stats.Add(fs);
			mv_send_stats.Remove(i);
			break;
		}
	}

	if(mv_send_complete_stats.Size()>4000){
		mv_send_complete_stats.RemoveRange(0,1000);
	}
	
	mb_valid=false;
}

void FileTransferDialog::SendFailed(UINT id)
{
	CSingleLock lock(&m_lock,TRUE);
	for(UINT i=0;i<mv_send_stats.Size();i++){
		FileStat* fs=(FileStat*)mv_send_stats.Get(i);
		if(fs->GetID()==id){//move the filestat over to the failed vector
			mv_send_failed_stats.Add(fs);
			mv_send_stats.Remove(i);
			break;
		}
	}

	if(mv_send_failed_stats.Size()>4000){
		mv_send_failed_stats.RemoveRange(0,1000);
	}
	mb_valid=false;
}

void FileTransferDialog::SendProgress(UINT id, float progress)
{
	CSingleLock lock(&m_lock,TRUE);
	for(UINT i=0;i<mv_send_stats.Size();i++){
		FileStat* fs=(FileStat*)mv_send_stats.Get(i);
		if(fs->GetID()==id){
			//if(progress>1.01){
			//	CString str;
			//	str.Format("File progress is too high on %s to %s, %.2f",fs->GetFileName(),fs->GetPeer(),progress);
				//MessageBox(str,"ERROR",MB_OK);
			//}
			fs->SetProgress(progress);
			break;
		}
	}

	mb_valid=false;
}

BOOL FileTransferDialog::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;

	//return CDialog::OnEraseBkgnd(pDC);
}

void FileTransferDialog::OnSizing(UINT fwSide, LPRECT pRect)
{
	if(pRect->right-pRect->left<700)
		pRect->right=pRect->left+700;
	if(pRect->bottom-pRect->top<300)
		pRect->bottom=pRect->top+300;
	CDialog::OnSizing(fwSide, pRect);

	// TODO: Add your message handler code here
}

void FileTransferDialog::RepositionScrollBars(void)
{
	CRect client_rect;
	this->GetClientRect(&client_rect);

	if(m_vscroll_bar.GetSafeHwnd()){
		m_vscroll_bar.MoveWindow(client_rect.Width()-RMARGIN,TMARGIN+HEADERHEIGHT,19,client_rect.Height()-(TMARGIN+HEADERHEIGHT+BMARGIN),TRUE);
	}
}

void FileTransferDialog::UpdateScrollBars(void)
{
	if(!m_vscroll_bar.GetSafeHwnd())
		return;

	
	CRect client_rect;
	this->GetClientRect(&client_rect);
	//int range=0;

	int max_height=0;
	
	CSingleLock lock(&m_lock,TRUE);
	max_height=max(mv_get_stats.Size(),(UINT)max_height);
	max_height=max(mv_get_complete_stats.Size(),(UINT)max_height);
	max_height=max(mv_get_failed_stats.Size(),(UINT)max_height);
	max_height=max(mv_send_stats.Size(),(UINT)max_height);
	max_height=max(mv_send_complete_stats.Size(),(UINT)max_height);
	max_height=max(mv_send_failed_stats.Size(),(UINT)max_height);
	lock.Unlock();

	max_height*=CELLHEIGHT;

	SCROLLINFO si;
	m_vscroll_bar.GetScrollInfo(&si);
	si.fMask=SIF_PAGE|SIF_RANGE;
	si.nPage=client_rect.Height()-(TMARGIN+HEADERHEIGHT+BMARGIN);
	si.nMin=0;

	si.nMax=(UINT)max((int)max_height,(int)si.nPage);
	m_vscroll_bar.SetScrollInfo(&si,TRUE);
}

void FileTransferDialog::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if(nSBCode==8)
		return;
	m_vscroll_pos=nPos;

	//invalidate the part of the screen that may change
	CRect client_rect;
	this->GetClientRect(&client_rect);
	CRect ir(LMARGIN,TMARGIN+HEADERHEIGHT,client_rect.Width()-RMARGIN,client_rect.bottom-BMARGIN);
	this->InvalidateRect(&ir,TRUE);
	//TRACE("VScrollPos = %d.\n",m_vscroll_pos);
	//TRACE("nSBCode = %d.\n",nSBCode);

	pScrollBar->SetScrollPos(nPos,TRUE);
}

void FileTransferDialog::ClearFailures()
{
	CSingleLock lock(&m_lock,TRUE);
	
	mv_send_failed_stats.Clear();
	mv_get_failed_stats.Clear();

	mb_valid=false;
}

//called when the user pressed the "Clear Completed" button.  It clears both send and get completed stats.
void FileTransferDialog::ClearCompleted()
{
	CSingleLock lock(&m_lock,TRUE);
	
	mv_send_complete_stats.Clear();
	mv_get_complete_stats.Clear();

	mb_valid=false;
}

//called when the user pressed the "Sort by IP" button
void FileTransferDialog::SortByIP()
{
	CSingleLock lock(&m_lock,TRUE);
	FileStat::SetSortMode(0);

	mv_get_stats.Sort(TRUE);
	mv_get_complete_stats.Sort(TRUE);
	mv_get_failed_stats.Sort(TRUE);
	mv_send_stats.Sort(TRUE);
	mv_send_complete_stats.Sort(TRUE);
	mv_send_failed_stats.Sort(TRUE);

	mb_valid=false;
}

//called when the user pressed the "Sort by Name" button
void FileTransferDialog::SortByName()
{
	CSingleLock lock(&m_lock,TRUE);
	FileStat::SetSortMode(1);

	mv_get_stats.Sort(TRUE);
	mv_get_complete_stats.Sort(TRUE);
	mv_get_failed_stats.Sort(TRUE);
	mv_send_stats.Sort(TRUE);
	mv_send_complete_stats.Sort(TRUE);
	mv_send_failed_stats.Sort(TRUE);

	mb_valid=false;
}

bool FileTransferDialog::ReceivedAppData(UINT from_app, byte* data)
{
	return m_system.ReceivedAppData(from_app,data);
}

void FileTransferDialog::SetCachedFileChunks(UINT chunks)
{
	m_cached_file_chunks=chunks;
}

void FileTransferDialog::AddCacheHit(void)
{
	m_cached_hits++;
}

void FileTransferDialog::OnShowP2PInfo()
{
	m_p2p_info_dialog.ShowWindow(SW_SHOWNORMAL);
	m_p2p_info_dialog.BringWindowToTop();
}

void FileTransferDialog::WriteToLog(const char* str)
{
//	m_system.WriteToLog(str);
}
