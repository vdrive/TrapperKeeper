#pragma once
#include "Network.h"
#include "Rack.h"
#include "NetworkSystem.h"

// CRackWindow

class CRackWindow : public CWnd
{
	DECLARE_DYNAMIC(CRackWindow)

public:
	CRackWindow();
	virtual ~CRackWindow();

protected:
	int m_vscroll_pos;
	int m_vscroll_page_size;
	CScrollBar m_vscroll_bar;
	CRect m_client_rect;
	CBitmap m_back_buffer;

	CFont m_big_font;
	CFont m_very_little_font;
	CFont m_little_font;
	CFont m_little_font_bold;
	CPen m_white_pen;

	int m_tk_on_offset;
	int m_rack_on_offset;
	int m_rack_history_offset;
	int m_tk_history_offset;
	int m_mouse_over_rack;
	CButton m_remote_control_button;
	CButton m_restart_button;
	CButton m_kill_kazaas_button;
	//CButton m_remote_control_buttons[70];
	//CButton m_restart_buttons[70];

	Network* mp_selected_network;
	NetworkSystem* mp_network_system;

	CListBox* mp_selected_rack_dll_list_box;

	bool b_mouse_on;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	void UpdateScrollBars(void);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	void SelectNetwork(Network* p_network);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	Network* GetSelectedNetwork(void);
	void RenderTKHistory(CDC &dc, int x, int y, int width, int height, Rack* rack,vector <bool> &vb_system_on);
	void RenderRackHistory(CDC& dc, int x, int y, int width, int height, Rack* rack,vector <bool> &vb_system_on);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	void SetupButtons(void);
	void MouseOff(void);
	void SetNetworkSystem(NetworkSystem* p_network_system);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	void SetDllListBox(CListBox& list_box);
protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
};


