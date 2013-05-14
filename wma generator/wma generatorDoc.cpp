// wma generatorDoc.cpp : implementation of the CwmageneratorDoc class
//

#include "stdafx.h"
#include "wma generator.h"

#include "wma generatorDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CwmageneratorDoc

IMPLEMENT_DYNCREATE(CwmageneratorDoc, CDocument)

BEGIN_MESSAGE_MAP(CwmageneratorDoc, CDocument)
END_MESSAGE_MAP()


// CwmageneratorDoc construction/destruction

CwmageneratorDoc::CwmageneratorDoc()
{
	// TODO: add one-time construction code here

}

CwmageneratorDoc::~CwmageneratorDoc()
{
}

BOOL CwmageneratorDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CwmageneratorDoc serialization

void CwmageneratorDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}


// CwmageneratorDoc diagnostics

#ifdef _DEBUG
void CwmageneratorDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CwmageneratorDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CwmageneratorDoc commands
