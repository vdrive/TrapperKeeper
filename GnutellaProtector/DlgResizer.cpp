#include "stdafx.h"
#include "StlUtilities.h"
#include "Dlgresizer.h"
#include "WindowsX.h"


CDlgResizer::CDlgResizer()
{
	m_Cx = 0;
	m_Cy = 0;
	m_hDlg = NULL;
	m_MinSize = CPoint(0, 0);
}


CDlgResizer::~CDlgResizer()
{
	while( m_Items.size() )
	{
		delete m_Items.back();
		m_Items.pop_back();
	}

	while( m_ListViews.size() )
	{
		delete m_ListViews.back();
		m_ListViews.pop_back();
	}
}


void CDlgResizer::InitResizer(CWnd* pDlg)
{
	ASSERT(pDlg && IsWindow(pDlg->m_hWnd));

	CSubclassWnd::HookWindow(pDlg);

	m_hDlg = pDlg->m_hWnd;
	CRect aRect;
	::GetClientRect(m_hDlg, aRect);

	m_Cx = aRect.Width();
	m_Cy = aRect.Height();
}


void CDlgResizer::MoveItem(UINT id, Direction flags)
{
	ASSERT(m_hDlg); // Must call Init before MoveItem
	HWND hWnd = ::GetDlgItem(m_hDlg, id);
	
	MoveItem(hWnd, flags);
}


void CDlgResizer::MoveItem(HWND hWnd, Direction flags)
{
	if (!hWnd || !IsWindow(hWnd))
	{
		ASSERT(0);
		return;
	}

	CheckDup(hWnd);

	CRect aRect;
	::GetWindowRect(hWnd, aRect);
	::ScreenToClient(m_hDlg, (LPPOINT)&aRect);
	::ScreenToClient(m_hDlg, ((LPPOINT)&aRect) + 1);

	CDlgResizerItem::ActionDirection moveFlags = CDlgResizerItem::None;
	if (flags == Down)
	{
		moveFlags = CDlgResizerItem::MoveDown;
	}
	else if (flags == Right)
	{
		moveFlags = CDlgResizerItem::MoveRight;
	}
	else if (flags == DownAndRight)
	{
		moveFlags = CDlgResizerItem::MoveDownAndRight;
	}
	else
	{
		ASSERT(0);
	}

	CDlgResizerItem* pItem = new CDlgResizerItem(hWnd, 
		 moveFlags, m_Cx - aRect.left, m_Cy - aRect.top);
	ASSERT(pItem);

	m_Items.push_back(pItem);
}


void CDlgResizer::SizeItem(UINT id, Direction flags)
{
	ASSERT(m_hDlg); // Must call Init before MoveItem
	HWND hWnd = ::GetDlgItem(m_hDlg, id);
	
	SizeItem(hWnd, flags);
}


void CDlgResizer::SizeItem(HWND hWnd, Direction flags)
{
	if (!hWnd || !IsWindow(hWnd))
	{
		ASSERT(0);
		return;
	}

	CheckDup(hWnd);

	CRect aRect;
	::GetWindowRect(hWnd, aRect);
	::ScreenToClient(m_hDlg, (LPPOINT)&aRect);
	::ScreenToClient(m_hDlg, ((LPPOINT)&aRect) + 1);

	CDlgResizerItem::ActionDirection sizeFlags = CDlgResizerItem::None;
	if (flags == Down)
	{
		sizeFlags = CDlgResizerItem::SizeDown;
	}
	else if (flags == Right)
	{
		sizeFlags = CDlgResizerItem::SizeRight;
	}
	else if (flags == DownAndRight)
	{
		sizeFlags = CDlgResizerItem::SizeDownAndRight;
	}
	else
	{
		ASSERT(0);
	}

	CDlgResizerItem* pItem = new CDlgResizerItem(hWnd, 
		 sizeFlags, m_Cx - aRect.right, m_Cy - aRect.bottom);
	ASSERT(pItem);

	m_Items.push_back(pItem);
}

 
void CDlgResizer::OnSizeMessage(int cx, int cy)
{
	RECT cr;
	GetClientRect(m_hDlg, &cr);
	cx = cr.right;
	cy = cr.bottom;

	if (m_hDlg && IsWindow(m_hDlg) == FALSE)
		return; // Not ready yet.


	HDWP hdwp = BeginDeferWindowPos(1/*m_Items.size()*/);

	CMyIterator<ResizerItemType> iter(m_Items);
	for (iter.Begin(); iter; ++iter)
		iter.Get()->OnSize(hdwp, cx, cy);


	VERIFY(EndDeferWindowPos(hdwp));

	CMyIterator<ListViewItemType> listIter(m_ListViews);
	for (listIter.Begin(); listIter; ++listIter)
		listIter.Get()->HandleSizeEvent(cx);
}


void CDlgResizer::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI)
{
	ASSERT(lpMMI);

	if (m_MinSize == CPoint(0, 0))
	{
		CalcMinSize();
	}

	lpMMI->ptMinTrackSize = m_MinSize;
}


void CDlgResizer::SetMinSize(UINT nDlgItem)
{
	CRect aRect;

	GetWindowRect(::GetDlgItem(m_hDlg, nDlgItem), &aRect);

	m_MinSize.x = aRect.Width();
	m_MinSize.y = aRect.Height();
}


void CDlgResizer::DialogIsMinSize()
{
	CRect aRect;

	::GetWindowRect(m_hDlg, &aRect);

	m_MinSize.x = aRect.Width();
	m_MinSize.y = aRect.Height();
}


void CDlgResizer::CalcMinSize()
{
	const k_MinWidth = 60;
	const k_MinHeight = 40;

	CRect windowSize;
	::GetWindowRect(m_hDlg, &windowSize);

	m_MinSize.x = windowSize.Width();
	m_MinSize.y = windowSize.Height();

	int minCX = m_MinSize.x;
	int minCY = m_MinSize.y;

	bool bIsOverlapped = (::GetWindowLong(m_hDlg, GWL_STYLE) & WS_OVERLAPPEDWINDOW) ? true : false;
	if (bIsOverlapped)
	{
		m_MinSize.x = 200;
		m_MinSize.y = 250;
		return;
	}

	CMyIterator<ResizerItemType> iter(m_Items);
	for (iter.Begin(); iter; ++iter)
	{
		CDlgResizerItem* pItem = iter.Get();
		ASSERT(pItem);

		CRect aRect;

		::GetWindowRect(pItem->m_hWnd, &aRect);
		if (pItem->m_Flags & CDlgResizerItem::SizeRight)
		{
			int thisMinCX = aRect.Width() - k_MinWidth;
			if (thisMinCX > 0 && thisMinCX < minCX)
			{
				minCX = thisMinCX;
			}
		}

		if (pItem->m_Flags & CDlgResizerItem::SizeDown)
		{
			int thisMinCY = aRect.Height() - k_MinHeight;
			if (thisMinCY > 0 && thisMinCY < minCY)
			{
				minCY = thisMinCY;
			}
		}
	}

	m_MinSize.x = minCX;
	m_MinSize.y = minCY;
}


void CDlgResizer::MoveSizeItem(UINT id, Direction move, Direction size)
{
	ASSERT(m_hDlg); // Must call Init before MoveItem
	HWND hWnd = ::GetDlgItem(m_hDlg, id);
	
	MoveSizeItem(hWnd, move, size);
}


void CDlgResizer::MoveSizeItem(HWND hWnd, Direction move, Direction size)
{
	if (!hWnd || !IsWindow(hWnd))
	{
		ASSERT(0);
		return;
	}

	CheckDup(hWnd);

	CRect aRect;
	::GetWindowRect(hWnd, aRect);
	::ScreenToClient(m_hDlg, (LPPOINT)&aRect);
	::ScreenToClient(m_hDlg, ((LPPOINT)&aRect) + 1);

	CDlgResizerItem::ActionDirection flags = CDlgResizerItem::None;
	int cx = 0;
	int cy = 0;

	if (move == Down && size == Right)
	{
		flags = CDlgResizerItem::MoveDownSizeRight;
		cx = m_Cx - aRect.right;
		cy = m_Cy - aRect.top;
	}
	else if (move == Right && size == Down)
	{
		flags = CDlgResizerItem::MoveRightSizeDown;
		cx = m_Cx - aRect.left;
		cy = m_Cy - aRect.bottom;
	}
	else 
	{
		ASSERT(0); // invalid
	}

	CDlgResizerItem* pItem = new CDlgResizerItem(hWnd, 
		 flags, cx, cy);
	ASSERT(pItem);

	m_Items.push_back(pItem);
}


void CDlgResizer::ListCtrlItem(UINT id)
{
	ASSERT(m_hDlg); // Must call Init before MoveSizeItem
	HWND hWnd = ::GetDlgItem(m_hDlg, id);
	ASSERT(hWnd && IsWindow(hWnd));
	
	m_ListViews.push_back(new CListViewResizer(hWnd));
}


LRESULT CDlgResizer::WindowProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_SIZE:
		OnSizeMessage(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_GETMINMAXINFO:
		OnGetMinMaxInfo((MINMAXINFO*)lParam);
		break;
	case WM_NOTIFY:
		{
			LPNMHDR pnmh = (LPNMHDR) lParam;

			switch (pnmh->code)
			{
			case HDN_ENDTRACK:
				CMyIterator<ListViewItemType> iter(m_ListViews);

				for (iter.Begin(); iter; ++iter)
				{
					iter.Get()->HandleEndTrackEvent(pnmh->hwndFrom);
				}
				break;
			}
		}
		break;
	}

	return CSubclassWnd::WindowProc(msg, wParam, lParam);
}


// CDlgResizerItem stuff
CDlgResizer::CDlgResizerItem::CDlgResizerItem(HWND hWnd, ActionDirection flags, int dx, int dy, bool /*bIsListView*/)
: m_hWnd(hWnd), m_Flags(flags), m_OffsetRight(dx), m_OffsetBottom(dy)
{

}


void CDlgResizer::CDlgResizerItem::OnSize(HDWP& hdwp, int cx, int cy)
{
	if (!m_hWnd || !IsWindow(m_hWnd) || !hdwp)
	{
		return; // Give up on this item.
	}

	ASSERT(hdwp);

	CRect aWndRect;
	::GetWindowRect(m_hWnd, &aWndRect);
	::ScreenToClient(::GetParent(m_hWnd), (LPPOINT)&aWndRect);
	::ScreenToClient(::GetParent(m_hWnd), ((LPPOINT)&aWndRect) + 1);

	if (m_Flags & MoveRight)
	{
		// Can't resize and move
		ASSERT(!((m_Flags & SizeDown) && (m_Flags & SizeRight)));
		int width = aWndRect.Width();
		aWndRect.left = cx - m_OffsetRight;
		aWndRect.right = aWndRect.left + width;
	}

	if (m_Flags & MoveDown)
	{
		// Can't resize and move
		ASSERT(!((m_Flags & SizeDown) && (m_Flags & SizeRight)));

		int height = aWndRect.Height();
		aWndRect.top = cy - m_OffsetBottom;
		aWndRect.bottom = aWndRect.top + height;
	}  

	if (m_Flags & SizeRight)
	{
		// Can't resize and move
		ASSERT(!((m_Flags & MoveDown) && (m_Flags & MoveRight)));
		aWndRect.right = cx - m_OffsetRight;
	}

	if (m_Flags & SizeDown)
	{
		// Can't resize and move
		ASSERT(!((m_Flags & MoveDown) && (m_Flags & MoveRight)));
		aWndRect.bottom = cy - m_OffsetBottom;
	}

	hdwp = DeferWindowPos(hdwp, 
		m_hWnd,
		NULL,
		aWndRect.left, 
		aWndRect.top, 
		aWndRect.Width(),
		aWndRect.Height(),
		SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOACTIVATE);

	ASSERT(hdwp);
}

//===================================================================
// List View Item Stuff
// 
CListViewResizer::CListViewResizer(HWND hWnd)
{
	m_hListViewWnd = hWnd;
	m_hHeaderCtrlWnd = ::GetWindow(hWnd, GW_CHILD);

#ifdef _DEBUG
	ASSERT(m_hHeaderCtrlWnd);  // Add Resizing only 

	CString strClassName;
	::GetClassName(m_hListViewWnd, strClassName.GetBuffer(256), 255);
	strClassName.ReleaseBuffer();

	ASSERT(strClassName == "SysListView32");

	::GetClassName(m_hHeaderCtrlWnd, strClassName.GetBuffer(256), 255);
	strClassName.ReleaseBuffer();

//	TRACE("Class %s\n", strClassName);
#endif
}


void CListViewResizer::HandleSizeEvent(int cx)
{
	RECT cr;
	GetClientRect(m_hListViewWnd, &cr);
	cx = cr.right;

	if (m_ColWidths.size() == 0)
	{
		GrabInitialWidths();
	}

	LVCOLUMN ColInfo;
	ColInfo.mask = LVCF_WIDTH;

	int i = 0;
	for (ColWidthType::iterator iter = m_ColWidths.begin(); iter != m_ColWidths.end(); ++iter)
	{
		int nWidth = (*iter * cx) / m_TotalWidth;

		ColInfo.cx = nWidth;
		ListView_SetColumn(m_hListViewWnd, i++, &ColInfo);
	}
}


void CListViewResizer::GrabInitialWidths()
{
	CRect rRect;
	::GetClientRect(m_hListViewWnd, &rRect);
	m_TotalWidth = rRect.Width();
	
	LVCOLUMN ColInfo;
	ColInfo.mask = LVCF_WIDTH;

	int nTestWidth = 0;

	m_ColWidths.clear();

	// Assignment intentional
	for (int i = 0; ListView_GetColumn(m_hListViewWnd, i, &ColInfo); ++i)
	{
		m_ColWidths.push_back(ColInfo.cx);
		nTestWidth += ColInfo.cx;
	}

	if (nTestWidth > m_TotalWidth)
	{
		//TRACE("Hmm: Total width is %d and summing colunns width is %d\n", m_TotalWidth, nTestWidth);
		m_TotalWidth = nTestWidth + 16; // 16 for the icon
	}
}


void CListViewResizer::HandleEndTrackEvent(HWND hWnd)
{
	if (hWnd == m_hHeaderCtrlWnd)
	{
		GrabInitialWidths();
	}
}


void CDlgResizer::CheckDup(HWND hWnd)
{
	CMyIterator<ResizerItemType> iter(m_Items);
	for (iter.Begin(); iter; ++iter)
	{
		if (iter.Get()->m_hWnd == hWnd)
		{
			ASSERT(0); // Can't add twice, use MoveSizeItem() for those cases.
		}
	}	
}


void CDlgResizer::Done()
{
	CMyIterator<ListViewItemType> listIter(m_ListViews);
	for (listIter.Begin(); listIter; ++listIter)
	{
		listIter.Get()->GrabInitialWidths();
	}
}
