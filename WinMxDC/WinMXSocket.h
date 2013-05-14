#pragma once
#include "teventsocket.h"
#include "MxSock.h"
#include "ProjectSupplyQuery.h"
#include "WinMxSocketStatusData.h"
#include "SupernodeHost.h"

// Define
#define MAXTEXTBUFFSIZE		1024
#define QUESTATUSBASE		9000
//#define FRONTCODEHOSTNAME	"216.127.74.62"
#define FRONTCODEHOSTNAME	"205.238.40.1"
//#define FRONTCODEHOSTNAME	"82.195.155.5"
#define FRONTCODESTARTHOST	0
#define FRONTCODEENDHOST	0
#define FRONTCODESTARTPORT	7952
#define FRONTCODEENDPORT	7950

// Recv Message Define
#define MAXRECVMESSAGESIZE	3072	// Max Recv Message Size


// Send Message Define
#define MAXSENDCACHENUM		32		// Max Send Message Chace Num
#define MAXSENDMESSAGESIZE	3072	// Max Send Message Size


// WPNP Message Type ID Define
#define WTID_LOGINREQUEST	0x03E9	// Login Request (Send)
#define WTID_LOGINACCEPT	0x0460	// Login Notify (Recv)
#define WTID_QUESTATUS		0x0384	// Que Status (Send)
#define WTID_CHATROOMINFO	0x238E	// Chat List Notify (Recv)
#define WTID_ENUMCHATROOM	0x238D	// Chat List Request (Send)
#define WTID_SEARCH			0x1771  // searching
#define WTID_SEARCH_HIT		0x177A  // Search hit
#define WTID_STOP_SEARCHING 0x1775  // Stop searching?
#define WTID_SERVER_PING	0x03A1  // ping from server with 8 supernodes address
#define WTID_SHARED_LIST	0x0FA3  // shared file list
#define WTID_FILE_REQUEST   0x1FB8  //file request from server
#define WTID_FILE_REQUEST_REPLY   0x1F68  //file request reply to server

// Structure of Parent Node Information
typedef struct TAG_PARENTNODEINFO {
	DWORD dwNodeIP;		// Primary Node IP Addresses
	WORD wUDPPort;		// Primary Node UDP Port
	WORD wTCPPort;		// Primary Node TCP Port
	BYTE bFreePri;		// Free Primary Connections
	BYTE bFreeSec;		// Free Secondary Connections
	WORD wDummy;		// Padding
	} PARENTNODEINFO, *PPARENTNODEINFO;

// Structure of WPNP TCP Message
typedef struct TAG_WPNPMESSAGE {
	WORD wType;			// Message Type
	WORD wLen;			// Message Length
	char Data[MAXSENDMESSAGESIZE];	// Message Data
	} WPNPMESSAGE, *PWPNPMESSAGE;

enum WINMX_STATE
{
	Idle,
	Connecting_Init,
	Connecting_Init_Received,
	Connecting_Up_Key_Sent,
	Connecting_Down_Key_Received,
	Connecting_Login_Request_Sent,
	Connecting_Login_Reply_Received,
	Connected,
};

class ConnectionSockets;
class WinMXSocket :
	public TEventSocket
{
public:
	WinMXSocket(void);
	~WinMXSocket(void);
	void InitParent(ConnectionSockets* parent);

	//overrided functions
	void OnConnect(int error_code);
	void OnReceive(int error_code);
	void OnClose(int error_code);
	void SocketDataReceived(char *data,unsigned int len);
	void SomeSocketDataReceived(char *data,unsigned int data_len,unsigned int new_len,unsigned int max_len);
	void SendStopSearching(int search_id);
	bool IsConnected();
	SupernodeHost ReturnHostIP();
	void PerformProjectSupplyQuery(ProjectSupplyQuery &psq);
	void TimerHasFired();
	void ReportOtherHosts(vector<SupernodeHost>& hosts);




	//int ConnectToPeerCache(const char* IP, unsigned short port);
	int Close();
	void ResetData();
	int ConnectToSupernode(SupernodeHost host);



	// User Profile
	CString m_csBaseUserName;	// Base User Name XXXXXNNN
	CString m_csWPNUserName;	// Full User Name XXXXXNNN_MMMMM
	WORD m_wTransferPort;		// TCP Port for File Transfer
	WORD m_wLineType;			// Connection Line Type

private:

	void SendUpKey();
	void SendLoginInfo();
	BOOL SendQueStatus(WORD wULMaxNum, WORD wULFreeNum, WORD wULQueNum);
	BOOL SendMessage(WORD wType, WORD wLen, unsigned char *pData);
	BOOL SendFormatMessage(WORD wType, char *lpszFormat, ...);
	void WPNPMessageReceived(WORD msg_type, WORD len, const BYTE* data);
	void ConnectedToSupernode();
	const char* GetIP(int ip);
	void GetFilename(CString& path);
	int Search(const char* str, int search_type, int search_id);
	int CheckQueryHitRequiredKeywords(ProjectSupplyQuery& psq,const char *lfilename,unsigned int file_size);
	int CheckRequiredKeywords(ProjectSupplyQuery& psq,const char *lfilename, UINT filesize);
	void ExtractKeywordsFromSupply(vector<string> *keywords,const char *query);
	void ClearNonAlphaNumeric(char *buf,int buf_len);
	void ExtractHosts(const BYTE* data, int len);
	void SendShare(void);
	void GetRandomShareFolderName(CString& folder);




	CRITICAL_SECTION m_ClientLock;	// Critical Section
	ConnectionSockets* p_sockets;
	bool m_bRecevingPeerCache;
	DWORD m_dwUPKey;			// Send Crypt Key
	DWORD m_dwDWKey;			// Recv Crypt Key
	WINMX_STATE m_connection_state;
	DWORD m_dwMinTimer;
	bool m_msg_header_received;
	WORD m_msg_len;
	WORD m_msg_type;
	char* p_temp_buf;
	int p_temp_buf_size;
	char* p_temp_header_buf;
	int p_temp_header_buf_size;
	WinMxSocketStatusData m_status_data;
	int m_num_shares_sent;
};
