#pragma once
#include "Resource.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "TabSelection.h"
#include "ClientModule.h"
#include <vector>
using namespace std;

class SoulSeekClientDll;
//class ClientModule;
// SoulSeekClientDlg dialog

class SoulSeekClientDlg : public CDialog
{
	DECLARE_DYNAMIC(SoulSeekClientDlg)

public:
	SoulSeekClientDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~SoulSeekClientDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG1 };

protected:

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedStart();
	void WriteToLog(int level,int object,LPCTSTR strToWrite);
	void InitParent(SoulSeekClientDll *parent);
	void EnableLoginButton();

	void DeleteClient(ClientModule* cmod);
	vector<ClientModule *> m_clientMods;

	//Set General stuff
	void SetParentSocketData(int moreConnections, int moreParents, int moreModules, int morePeers, int morePM);
private:
	SoulSeekClientDll *p_parent;
	bool m_Connected;
	void AppendString(LPCTSTR tempString, CRichEditCtrl *rEBox, int red, int green, int blue, bool bold);

	//General Variables
	void RefreshSocketInfo(void);

	//Connection field
	unsigned int m_connectedParents;
	unsigned int m_totalParents;
	unsigned int m_parentModules;
	unsigned int m_peers;
	unsigned int m_peerModules;

	UINT_PTR pClientModTimer;

public:
	//Timer stuff
	afx_msg void OnTimer(UINT nIDEvent);
	void StopTimer();
	// Displays the parents that the program connected to

	CTabSelection m_mainTab;
	CRichEditCtrl m_socket_left;
	CRichEditCtrl m_socket_right;
};
