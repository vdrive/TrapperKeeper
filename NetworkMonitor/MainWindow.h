#pragma once
#include "NetworkSystem.h"
#include "RackWindow.h"

// CMainWindow

class CMainWindow : public CWnd
{
	DECLARE_DYNAMIC(CMainWindow)

public:
	CMainWindow();
	virtual ~CMainWindow();

protected:
	DECLARE_MESSAGE_MAP()
	CBitmap m_back_buffer;
	CRect m_client_rect;
	NetworkSystem *mp_network_system;

	int m_old_network_count;

	CFont m_big_font;
	CFont m_little_font;
	CPen m_white_pen;

	CScrollBar m_vscroll_bar;
	int m_vscroll_pos;
	int m_vscroll_page_size;

	CRackWindow m_rack_window;

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	void SetNetworkSystem(NetworkSystem* ns);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	void UpdateScrollBars(void);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
private:
	void RenderGradients(CDC& dc);
public:
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	void MouseOff(void);
	afx_msg void OnDestroy();
	void SetDllListBox(CListBox& list_box);
	afx_msg void OnAcceleratorDownKey();
	afx_msg void OnAcceleratorUpKey();
	Network* GetSelectedNetwork(void);
};


