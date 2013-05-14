#pragma once
#include "Resource.h"
#include <afxwin.h>
#include <afxcmn.h>
#include "afxcmn.h"
#include "afxwin.h"

#define SUB_TORRENT_NAME		0
#define SUB_INFO_HASH			1

class BTInput;
class BTTorrentGeneratorDlg
	: public CDialog
{
	DECLARE_DYNAMIC(BTTorrentGeneratorDlg)
	enum { IDD = IDD_BTTORRENTGENERATOR_DLG };

// functions
public:
	BTTorrentGeneratorDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~BTTorrentGeneratorDlg();
	virtual BOOL OnInitDialog();

	void DisplayTorrents(vector<string> *pvFilenames, vector<string> *pvHashes);

	void ResetProgressBar(size_t nTotal);
	void UpdateProgressBar(void);
	void UpdateProgressText(const char *pMsg);


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()

// variables
private:
	CListCtrl			m_torrentList;
	CProgressCtrl		m_generationProgressBar;
	CStatic				m_generationProgressStatic;
};
