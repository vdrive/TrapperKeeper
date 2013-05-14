// wma generatorView.h : interface of the CwmageneratorView class
//


#pragma once


class CwmageneratorView : public CView
{
protected: // create from serialization only
	CwmageneratorView();
	DECLARE_DYNCREATE(CwmageneratorView)

// Attributes
public:
	CwmageneratorDoc* GetDocument() const;

// Operations
public:

// Overrides
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CwmageneratorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in wma generatorView.cpp
inline CwmageneratorDoc* CwmageneratorView::GetDocument() const
   { return reinterpret_cast<CwmageneratorDoc*>(m_pDocument); }
#endif

