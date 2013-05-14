// wma generatorView.cpp : implementation of the CwmageneratorView class
//

#include "stdafx.h"
#include "wma generator.h"

#include "wma generatorDoc.h"
#include "wma generatorView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CwmageneratorView

IMPLEMENT_DYNCREATE(CwmageneratorView, CView)

BEGIN_MESSAGE_MAP(CwmageneratorView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
END_MESSAGE_MAP()

// CwmageneratorView construction/destruction

CwmageneratorView::CwmageneratorView()
{
	// TODO: add construction code here

}

CwmageneratorView::~CwmageneratorView()
{
}

BOOL CwmageneratorView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CwmageneratorView drawing

void CwmageneratorView::OnDraw(CDC* /*pDC*/)
{
	CwmageneratorDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);

	// TODO: add draw code for native data here
}


// CwmageneratorView printing

BOOL CwmageneratorView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CwmageneratorView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CwmageneratorView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CwmageneratorView diagnostics

#ifdef _DEBUG
void CwmageneratorView::AssertValid() const
{
	CView::AssertValid();
}

void CwmageneratorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CwmageneratorDoc* CwmageneratorView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CwmageneratorDoc)));
	return (CwmageneratorDoc*)m_pDocument;
}
#endif //_DEBUG


// CwmageneratorView message handlers
