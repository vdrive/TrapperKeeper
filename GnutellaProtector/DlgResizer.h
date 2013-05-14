#ifndef _CRESIZEDLGHELPER_H
#define _CRESIZEDLGHELPER_H


#pragma warning(push, 1)
#include <vector>
#pragma warning(pop)


#include "Subclass.h"


class CListViewResizer
{
public:
	CListViewResizer(HWND hWnd);

	void HandleEndTrackEvent(HWND hWnd);
	void HandleSizeEvent(int cx);

	// Don't normally call, but sometimes....
	void GrabInitialWidths();
	
	int m_TotalWidth;


protected:
	typedef std::vector<int> ColWidthType;
	ColWidthType m_ColWidths;
	
	HWND m_hListViewWnd;
	HWND m_hHeaderCtrlWnd;
};


class CDlgResizer : private CSubclassWnd
{
public:
	CDlgResizer();
	virtual ~CDlgResizer();

	enum Direction 
	{ 
		None = 0,
		Right = 0x1,
		Down  = 0x2,
		DownAndRight = Down | Right,
		RightAndDown = Down | Right,
	};

	void InitResizer(CWnd* pDlg); 
	void MoveItem(UINT id, Direction flags);
	void MoveItem(HWND hWnd, Direction flags);
	void SizeItem(UINT id, Direction flags);
	void SizeItem(HWND hWnd, Direction flags);
	void MoveSizeItem(UINT id, Direction move, Direction size);
	void MoveSizeItem(HWND hWnd, Direction move, Direction size);
	
	void ListCtrlItem(UINT id);

	void DialogIsMinSize();
	void SetMinSize(UINT dlgItem);
	void SetMinSize(const CPoint& minSize) { m_MinSize = minSize; }
	CPoint GetMinSize() const { return m_MinSize; }
	void CalcMinSize();

	// Called by CSubclassWnd
	LRESULT WindowProc(UINT msg, WPARAM wp, LPARAM lp);

	// Don't need to call these unless you don't subclass
	void OnSizeMessage(int cx, int cy);
	void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);

	// Don't normally need to call this...
	void Done();


protected:
	// Nested class

	class CDlgResizerItem
	{
	public:
		enum ActionDirection { 
			None = 0,
			MoveRight = 0x1,
			MoveDown = 0x2,
			MoveDownAndRight = MoveDown | MoveRight,
			SizeRight = 0x4,
			SizeDown = 0x8,
			SizeDownAndRight = SizeDown | SizeRight,
			MoveDownSizeRight = MoveDown | SizeRight,
			MoveRightSizeDown = MoveRight | SizeDown,
		};
		CDlgResizerItem(HWND hWnd, ActionDirection flags, int dx, int dy, bool bIsListView = false);
		void OnSize(HDWP& hdwp, int cx, int cy);

		friend class CDlgResizer;

	protected:
		HWND m_hWnd;
		int m_Flags;
		int m_OffsetRight;
		int m_OffsetBottom;
	};


	void CheckDup(HWND hWnd);

	typedef std::vector<CDlgResizerItem*> ResizerItemType;
	ResizerItemType m_Items;

	typedef std::vector<CListViewResizer*> ListViewItemType;
	ListViewItemType m_ListViews;

	int m_Cx;
	int m_Cy;
	HWND m_hDlg;
	CPoint m_MinSize;
};

#endif
