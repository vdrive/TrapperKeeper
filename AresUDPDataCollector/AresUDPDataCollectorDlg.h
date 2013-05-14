#pragma once
#include "resource.h"

// CAresUDPDataCollectorDlg dialog

class CAresUDPDataCollectorDlg : public CDialog
{
	DECLARE_DYNAMIC(CAresUDPDataCollectorDlg)

public:
	CAresUDPDataCollectorDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAresUDPDataCollectorDlg();

// Dialog Data
	enum { IDD = IDD_ARESDCUDPDIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
