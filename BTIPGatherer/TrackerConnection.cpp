#include "StdAfx.h"
#include ".\trackerconnection.h"
#include "afxinet.h"
#include <io.h>
#include "zlib.h"

TrackerConnection::TrackerConnection(void)
{
}

TrackerConnection::~TrackerConnection(void)
{
}


PeerList *  TrackerConnection::GetPeers(TorrentFileData * tf, string title) 
{
	char * peer_id = CreatePeerId();

	unsigned short * peers = NULL;
	PeerList * list = NULL;
	char * trackerURL = NULL;
	/*
	try{
		int size = 0;
		vector <string>::iterator v1_Iter;
		vector<string> v_list = tf->GetAnnounceList();
		for(v1_Iter = v_list.begin(); v1_Iter != v_list.end( ) ;v1_Iter++) {
			trackerURL = GetTrackerURL(tf, (*v1_Iter).c_str(), peer_id);
			if(trackerURL[0] == '?') continue;
			size = GetData(trackerURL, &peers, "", message_type); // ** 
			if(peers != NULL && peers[0] == 'd' && (peers[1] == '8' || (peers[1] == '1' && peers[2] == '0'))) {
				PeerList * sub_list = new PeerList(peers, size, peer_id, title);
				if(list != NULL) list->Add(sub_list);
				else list = sub_list;
			} else if (size > 0) {
				char * temp = new char[size+1];
				for(int i = 0; i < size; i++) temp[i] =  (char)peers[i];
				temp[size] = 0;
//				Log(string("BTDataCollector::GetPeers -- Tracker error: ") + string(temp), message_type);
				TRACE("TRACKER ERROR MESSAGE: %s\n", temp);
				TRACE("%s\n", trackerURL);
				delete [] temp;
				temp = NULL;
			}
			delete [] trackerURL;
			if(peers != NULL) delete [] peers; // **
			peers = NULL;
		}
		*/
		int size = 0;
		trackerURL = GetTrackerURL(tf, tf->m_announce_URL.c_str(), peer_id);
		size = GetData(trackerURL, &peers, ""); // ** 
			if(peers != NULL && peers[0] == 'd' && (peers[1] == '8' || (peers[1] == '1' && peers[2] == '0'))) 
			{
				PeerList * sub_list = new PeerList(peers, size, peer_id, title);
				if(list != NULL) list->Add(sub_list);
				else list = sub_list;
			} 
			else if (size > 0) 
			{
				char * temp = new char[size+1];
				for(int i = 0; i < size; i++) temp[i] =  (char)peers[i];
				temp[size] = 0;
//				Log(string("BTDataCollector::GetPeers -- Tracker error: ") + string(temp), message_type);
				TRACE("TRACKER ERROR MESSAGE: %s\n", temp);
				TRACE("%s\n", trackerURL);
				delete [] temp;
				temp = NULL;
			}
			delete [] trackerURL;
			if(peers != NULL) delete [] peers; // **
			peers = NULL;

	
	/*
	catch(exception * e) {
		delete [] trackerURL;
//		Log("BTDataCollector::GetPeers() -- Exception\n", message_type);
//		Log(e->what());
	}
	*/
	delete [] peer_id;
	if(peers != NULL) delete [] peers;
	return list;
}

char * TrackerConnection::CreatePeerId() 
{
	char * chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	char * id = new char [21];
	strcpy(id, "-AZ2202-");
	srand( (unsigned)time( NULL ) );
	for(int i = 8; i < 20; i++) {
		int pos = (int) ( rand() % 62);
		id[i] = chars[pos];
	}
	id[20] = 0;
	return id;
}

char * TrackerConnection::GetTrackerURL(TorrentFileData * tf, const char *url, char * peer_id) 
{
	char * trackerURL = new char[512];

	char temp[2];
	strcpy(temp, " ");
	strcpy(trackerURL, "");
	for(int i = 0; i < (int)strlen(url); i++) {  // gets rid of extra spaces
		temp[0] = url[i];
		if(temp[0] != ' ') {
			strcat(trackerURL, temp);
		}
	}

	int port;
	char cport[5+1];
	port = rand()%10 + 49100;
	_itoa(port,cport,10);

	strcat(trackerURL, "?info_hash=");
	strcat(trackerURL, ConvertToHash(tf->m_hash).c_str());
	strcat(trackerURL, "&peer_id=");
	strcat(trackerURL, peer_id);
	strcat(trackerURL, "&port=");
	strcat(trackerURL, cport);
	strcat(trackerURL, "&uploaded=");
	strcat(trackerURL, "0");
	strcat(trackerURL, "&downloaded=0&left=");
	strcat(trackerURL, "0");
	strcat(trackerURL, "&numwant=1000");
	strcat(trackerURL, "&compact=1&key=");

	char * key_id = GetKeyID();

	strcat(trackerURL, key_id);

	delete [] key_id;

	return trackerURL;
}

string TrackerConnection::ConvertToHash(string hash)
{

	string returnstring;

	for (int i = 0; i < (int)hash.length(); i ++)
	{
		if ( i%2 == 0)
		{
			returnstring += '%';
		}
		returnstring += hash[i];
	}
	return (returnstring);
}

char * TrackerConnection::GetKeyID() 
{
	int key_len = 8;
	char * id = new char [key_len+1];
	strcpy(id, "");
	srand( (unsigned)time( NULL ) );
	for(int i = 0; i < key_len; i++) {
		int pos = (int) ( rand() % 62);
		id[i] = chars[pos];
	}
	id[key_len] = 0;
	return id;
}


int TrackerConnection::GetData(const char * url, unsigned short **body, string referer) 
{
	return GetHTTPData(url, body, referer, false, "");
}

int TrackerConnection::PostData(const char * url, unsigned short **body, string referer, string post_info) 
{
	return GetHTTPData(url, body, referer, true, post_info);
}


int TrackerConnection::GetHTTPData(const char * url, unsigned short **body, string referer, bool b_post, string post_data) 
{
	//Log(url);
	CInternetSession session("Azereus 2.2.0.2");
	CHttpConnection * pServer = NULL;
	CHttpFile* pFile = NULL;
	bool bProgressMode = false; 
	bool bStripMode = false; 
	bool isGzipped = false;
	string line = "";
	int size = 0;
	unsigned short * buf = *body;
	try {
		CString strServerName; 
		CString strObject; 
		INTERNET_PORT nPort; 
		DWORD dwServiceType; 

		if (!AfxParseURL(url, dwServiceType, strServerName, strObject, nPort) || 
		dwServiceType != INTERNET_SERVICE_HTTP) 
		{ 
			//Log( _T("Error: can only use URLs beginning with http://"));
			//TRACE("Error: can only use URLs beginning with http://\n");
			//TRACE("URL: '%s'\n", url);
			line =  "request failed - can only use URLs beginning with http://";
		} 

		if (bProgressMode) 
		{ 
//			Log( _T("Opening Internet..."), message_type); 
			VERIFY(session.EnableStatusCallback(TRUE)); 
		} 
		
		pServer = session.GetHttpConnection(strServerName, INTERNET_FLAG_DONT_CACHE, nPort); 
		
		string file = url;
		int file_start = -1;
		if(file.length() > 8) {
			file_start = (int)file.find("//");
			file_start = (int)file.find("/", file_start+2);
		}
		if(file_start > 0) file = file.substr(file_start);
		else file = "";
		
		if(!b_post) {
			pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_GET,file.c_str(), referer.c_str()); 
			pFile->SendRequest(); 
		}
		else {
			pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_POST, file.c_str(), referer.c_str());
			string header = "Content-Type: application/x-www-form-urlencoded\n\r";
			pFile->SendRequest(header.c_str(),(DWORD)header.length(),(LPVOID)post_data.c_str(), (DWORD)post_data.length() );
		}


		DWORD dwRet; 
		pFile->QueryInfoStatusCode(dwRet); 

		// if access was denied, prompt the user for the password 

		if (dwRet == HTTP_STATUS_DENIED) 
		{ 
			DWORD dwPrompt; 
			dwPrompt = pFile->ErrorDlg(NULL, ERROR_INTERNET_INCORRECT_PASSWORD, 
			FLAGS_ERROR_UI_FLAGS_GENERATE_DATA | FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS, NULL); 

			// if the user cancelled the dialog, bail out 

			if (dwPrompt != ERROR_INTERNET_FORCE_RETRY) 
			{ 
//				Log(_T("Access denied: Invalid password\n"), message_type); 
				line =   "request failed - Access denied: Invalid password";
			} 

			pFile->SendRequest(); 
			pFile->QueryInfoStatusCode(dwRet); 
		} 

		CString strNewLocation; 
		pFile->QueryInfo(HTTP_QUERY_RAW_HEADERS_CRLF, strNewLocation); 

		// were we redirected? 
		// these response status codes come from WININET.H 

		if (dwRet == HTTP_STATUS_MOVED || 
		dwRet == HTTP_STATUS_REDIRECT || 
		dwRet == HTTP_STATUS_REDIRECT_METHOD) 
		{ 
			CString strNewLocation; 
			pFile->QueryInfo(HTTP_QUERY_RAW_HEADERS_CRLF, strNewLocation); 

			int nPlace = strNewLocation.Find(_T("Location: ")); 
			if (nPlace == -1) 
			{ 
//				Log( _T("Error: Site redirects with no new location"), message_type); 
				line =  "request failed - Site redirects with no new location";
			} 

			strNewLocation = strNewLocation.Mid(nPlace + 10); 
			nPlace = strNewLocation.Find('\n'); 
			if (nPlace > 0) 
				strNewLocation = strNewLocation.Left(nPlace); 

			// close up the redirected site 

			pFile->Close(); 
			delete pFile; 
			pServer->Close(); 
			delete pServer; 

			if (bProgressMode) 
			{ 
//				Log( _T("Caution: redirected to "), message_type); 
//				Log((LPCTSTR) strNewLocation, message_type); 
			} 

			// figure out what the old place was 
			if (!AfxParseURL(strNewLocation, dwServiceType, strServerName, strObject, nPort)) 
			{ 
//				Log(_T("Error: the redirected URL could not be parsed."), message_type); 
				line =  "request failed - the redirected URL could not be parsed.";
			} 

			if (dwServiceType != INTERNET_SERVICE_HTTP) 
			{ 
//				Log(_T("Error: the redirected URL does not reference a HTTP resource."), message_type);
				line =  "request failed - the redirected URL does not reference a HTTP resource";
			} 

			// try again at the new location 
			pServer = session.GetHttpConnection(strServerName, nPort); 
			if(!b_post) {
				pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_GET, strObject);
			}
			else {
				pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_POST, file.c_str());
			}
			//pFile->AddRequestHeaders(szHeaders); 
			pFile->SendRequest(); 

			pFile->QueryInfoStatusCode(dwRet); 
			if (dwRet != HTTP_STATUS_OK) 
			{ 
				TRACE(_T("Error: Got status code %d\n"), dwRet);
				line =  "request failed - Got status code " + dwRet;
			} 
		}
		CString content_type = "";
		pFile->QueryInfo(HTTP_QUERY_RAW_HEADERS_CRLF , content_type);
		if(content_type.Find(": gzip") > 0) isGzipped = true;

		const int read_size = 1024;
		TCHAR sz[read_size+1]; 
		UINT nRead = pFile->Read(sz, read_size); 
		int curPos = 0;
 		size = nRead;
		int cur_size = 10240;
		buf = new unsigned short[cur_size];
		while (nRead > 0) 
		{ 
			if(size > cur_size) {
				int new_size = cur_size * 2;
				unsigned short *temp = new unsigned short[new_size];
				for(int i = 0; i < cur_size; i++) {
					temp[i] = buf[i];
				}
				delete [] buf;
				buf = temp;
				cur_size = new_size;
			}
			for(UINT i = 0; i < nRead; i++) {
				buf[i+curPos] = sz[i];
			}
			curPos += nRead;
			nRead = pFile->Read(sz, read_size); 
			size += nRead;
		}
		unsigned short * temp = new unsigned short[size+1];
		for(int i = 0; i < size; i++) {
			temp[i] = buf[i];
		}
		delete [] buf;
		buf = temp;
	}
	catch (exception e) {
//		Log(e.what());
		line = "request failed - status code N/A";
	}
	catch (CInternetException * e) {
		TCHAR error[256];
		if(e->GetErrorMessage(error, 256)) {
//			Log(error);
			TRACE(error);
		}
		e->Delete();
		e = NULL;
		line = "request failed - status code N/A";
	}
	if(pFile != NULL) {
		pFile->Flush();
		pFile->Close();
		delete pFile;
		pFile = NULL;
	}
	if(pServer != NULL) {
		pServer->Close();
		delete pServer;
		pServer = NULL;
	}
	if(isGzipped) {
		unsigned short *data = NULL;
		size = UnZip(&data, buf, size);
		if(size == 0) {
			delete [] data;
			data = NULL;
		}
		delete [] buf;
		*body = data;
	}			
	else *body = buf;
	return size;

}

int TrackerConnection::UnZip(unsigned short ** dest, unsigned short * source, int len) 
{
		bool write = false;
		Bytef *uncompr = NULL;
		z_stream strm;
		uLong comprLen = 10000*sizeof(int); /* don't overflow on MSDOS */
		uLong uncomprLen = comprLen;

		int zip_status = 0;
		//dest = new unsigned short[size*3];
		uncompr = new Bytef[uncomprLen];
		strcpy((char*)uncompr, "garbage");

		// The fields next_in, avail_in, zalloc, zfree and opaque must be initialized
		Bytef * byte_src = new Bytef[len];
		for(int i = 0; i < len; i++) {
			byte_src[i] = (Bytef)source[i];
		}
		//strm.next_in = (Bytef*)source;
		strm.next_in = byte_src;
		strm.avail_in = 0;
		strm.zalloc = (alloc_func)0;
		strm.zfree = (free_func)0;
		strm.opaque = (voidpf)0;
		strm.next_out = (Bytef*)uncompr;

		zip_status = inflateInit2 (&strm, 15+16); 
		if(zip_status != Z_OK) { 
			delete [] uncompr;
			delete [] byte_src;
			return 0;
		}
		int size = 0;
		while (strm.total_out < uncomprLen && strm.total_in < comprLen) {
			size++;
			strm.avail_in = strm.avail_out = 1; /* force small buffers */
			zip_status = inflate(&strm, Z_NO_FLUSH);
			if (zip_status == Z_STREAM_END) break;
			if (zip_status == Z_DATA_ERROR) {
				zip_status = inflateSync(&strm);
			}
			if (zip_status == Z_BUF_ERROR) {
				TRACE("unzip error: %d\n", strm.msg);
			}
			if(zip_status != Z_OK) { 
				delete [] uncompr;
				delete [] byte_src;
				return 0;
			}
			
		}
		zip_status = inflateEnd(&strm); 
		if(zip_status != Z_OK) { 
			delete [] uncompr;
			delete [] byte_src;
			return 0;
		}
		unsigned short * data = *dest;
		data = new unsigned short[size];
		for(int i = 0; i < size; i++) {
			data[i] = uncompr[i];
		}
		delete [] uncompr;
		delete [] byte_src;
		*dest = data;
		return size;
}
