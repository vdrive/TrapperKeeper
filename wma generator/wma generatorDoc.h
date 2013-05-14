// wma generatorDoc.h : interface of the CwmageneratorDoc class
//


#pragma once

class CwmageneratorDoc : public CDocument
{
protected: // create from serialization only
	CwmageneratorDoc();
	DECLARE_DYNCREATE(CwmageneratorDoc)

// Attributes
public:

// Operations
public:

// Overrides
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CwmageneratorDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};


