#pragma once
#include "Resource.h"
#include <set>
#include "afxcmn.h"
using namespace std;

// CConnectedPeersDlg dialog

class Peer;
class CConnectedPeersDlg
	: public CDialog
{
	DECLARE_DYNAMIC(CConnectedPeersDlg)

public:
	CConnectedPeersDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConnectedPeersDlg();

	void ConnectedPeers(const set<Peer> *pConnectedPeers);

// Dialog Data
	enum { IDD = IDD_CONNECTED_PEERS_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();

	DECLARE_MESSAGE_MAP()

private:
	CListCtrl m_peerList;
};
