#pragma once

#include "resource.h"
// CMovieMakerDlg dialog

class CMovieMakerDlg : public CDialog
{
	DECLARE_DYNAMIC(CMovieMakerDlg)

public:
	CMovieMakerDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMovieMakerDlg();

// Dialog Data
	enum { IDD = IDD_MOVIE_MAKER_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
