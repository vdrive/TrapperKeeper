#include "httpconnection.h"
#include "stdafx.h"
#include "afxinet.h"
#include "zlib.h"

bool SetCookie(CInternetSession * p_session, const char * url)
{
	CString cookie_data;
	//DAY, DD-MMM-YYYY HH:MM:SS GMT
	CTime now=CTime::GetCurrentTime();
	now += CTimeSpan(7, 8, 0, 0); // +7 day, GMT - 8 
	p_session->GetCookie(url, "", cookie_data); // bool
	//TRACE("cookie for '%s', '%s', time: %ld\n", url, cookie_data, now.GetTime());
	CString date = now.Format("%a, %d-%b-%Y %H:%M:%S GMT"); 
	bool b_set = p_session->SetCookie(url, "expires", date.GetBuffer()) == 1;
	p_session->SetCookie(url, "test", "true");
	bool b_get = p_session->GetCookie(url, "expires", cookie_data) == 1; // bool
	//TRACE("cookie for '%s', '%s', (%d/%d), time: %ld\n", url, cookie_data, b_set, b_get, now.GetTime());
	return true;
}

int GetData(const char * url, unsigned short **body, string referer) 
{
	return GetHTTPData(url, body, referer);
}

int PostData(const char * url, unsigned short **body, string referer, string post_info) 
{
	return GetHTTPData(url, body, referer, true, post_info);
}

int GetHTTPData(const char * url, unsigned short **body, string referer, bool b_post, string post_data) 
{
	CInternetSession session("Azereus 2.2.0.2");
	//DWORD dwValue;
	//session.QueryOption(INTERNET_OPTION_CONNECT_TIMEOUT, dwValue);
	//TRACE("timeout: %d\n", dwValue );
	session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 10*1000);
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
			//TRACE("Error: can only use URLs beginning with http://\n");
			//TRACE("URL: '%s'\n", url);
			line =  "request failed - can only use URLs beginning with http://";
		} 

		if (bProgressMode) 
		{ 
			TRACE( _T("Opening Internet...")); 
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
			string header = "Accept: text/xml,application/xml,application/xhtml+xml,text/html;q=0.9,text/plain;q=0.8,image/png,*/*;q=0.5\r\n";
			header += "Accept-Language: en-us,en;q=0.5\n\r";
			header += "Accept-Encoding: gzip\r\n";
			header += "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\n\r";
			pFile->SendRequest(header.c_str(),(DWORD)header.length()); 
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

			// if the user cancelled the diaTRACE, bail out 

			if (dwPrompt != ERROR_INTERNET_FORCE_RETRY) 
			{ 
				TRACE(_T("Access denied: Invalid password\n")); 
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
				TRACE( _T("Error: Site redirects with no new location")); 
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
				TRACE( _T("Caution: redirected to ")); 
				TRACE((LPCTSTR) strNewLocation); 
			} 

			// figure out what the old place was 
			if (!AfxParseURL(strNewLocation, dwServiceType, strServerName, strObject, nPort)) 
			{ 
				TRACE(_T("Error: the redirected URL could not be parsed.")); 
				line =  "request failed - the redirected URL could not be parsed.";
			} 

			if (dwServiceType != INTERNET_SERVICE_HTTP) 
			{ 
				TRACE(_T("Error: the redirected URL does not reference a HTTP resource."));
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
		SetCookie(&session, strServerName);
		SetCookie(&session, url);

	}
	catch (exception e) {
		TRACE(e.what());
		line = "request failed - status code N/A";
	}
	catch (CInternetException * e) {
		TCHAR error[256];
		if(e->GetErrorMessage(error, 256)) {
			//TRACE("URL: '%s'\n", url);
			TRACE("%s\n", error);
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

// size = size of zipped data.  returns size of unzipped data
int UnZip(unsigned short ** dest, unsigned short * source, int len) {
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

string GetData(const char * url, const string referer) 
{

	CInternetSession session("Mozilla/4.0");
	session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 10*1000);
	CHttpConnection * pServer = NULL;
	CHttpFile* pFile = NULL;
	bool bProgressMode = false; 
	bool bStripMode = false; 
	string line = "";

	try {
		CString strServerName; 
		CString strObject; 
		INTERNET_PORT nPort; 
		DWORD dwServiceType; 

		if (!AfxParseURL(url, dwServiceType, strServerName, strObject, nPort) || 
		dwServiceType != INTERNET_SERVICE_HTTP) 
		{ 
			TRACE( _T("Error: can only use URLs beginning with http://") );
			return "request failed - can only use URLs beginning with http://";
		} 

		if (bProgressMode) 
		{ 
			TRACE("Opening Internet...%s\n", url);
			VERIFY(session.EnableStatusCallback(TRUE)); 
		} 
		
		pServer = session.GetHttpConnection(strServerName, nPort); 
		string file = url;


		int file_start = -1;
		if(file.length() > 8) {
			file_start = (int)file.find("//");
			file_start = (int)file.find("/", file_start+2);
			if(file_start > (int)file.length()) file_start = -1;
		}
		if(file_start > 0) file = file.substr(file_start);
		else file = "";
		LPCTSTR ref = NULL;
		if(referer != "") ref = referer.c_str();
 		pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_GET,file.c_str(), ref); 

		SetCookie(&session, url); 

		pFile->SendRequest(); 

		DWORD dwRet; 
		pFile->QueryInfoStatusCode(dwRet); 

		// if access was denied, prompt the user for the password 

		if (dwRet == HTTP_STATUS_DENIED) 
		{ 
			DWORD dwPrompt; 
			dwPrompt = pFile->ErrorDlg(NULL, ERROR_INTERNET_INCORRECT_PASSWORD, 
			FLAGS_ERROR_UI_FLAGS_GENERATE_DATA | FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS, NULL); 

			// if the user cancelled the diaTRACE, bail out 

			if (dwPrompt != ERROR_INTERNET_FORCE_RETRY) 
			{ 
				TRACE( _T("Access denied: Invalid password"));
				return "request failed - Access denied: Invalid password";
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
				TRACE( _T("Error: Site redirects with no new location") );
				return "request failed - Site redirects with no new location";
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
				TRACE( _T("Caution: redirected to "));
				TRACE( (LPCTSTR) strNewLocation ); 
			} 

			// figure out what the old place was 
			if (!AfxParseURL(strNewLocation, dwServiceType, strServerName, strObject, nPort)) 
			{ 
				TRACE( _T("Error: the redirected URL could not be parsed.") );
				return "request failed - the redirected URL could not be parsed.";
			} 

			if (dwServiceType != INTERNET_SERVICE_HTTP) 
			{ 
				TRACE( _T("Error: the redirected URL does not reference a HTTP resource.") );
				return "request failed - the redirected URL does not reference a HTTP resource";
			} 

			// try again at the new location 
			pServer = session.GetHttpConnection(strServerName, nPort); 
			pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_GET, strObject);
			pFile->SendRequest(); 

			pFile->QueryInfoStatusCode(dwRet); 
			if (dwRet != HTTP_STATUS_OK) 
			{ 
				//TRACE( _T("Error: Got status code ") dwRet );
				return "request failed - Got status code " + dwRet;
			} 
		}

		TCHAR sz[1024]; 
		sz[1023] = 0;
		while (pFile->ReadString(sz, 1023)) 
		{ 
			line += sz;
		} 	
		SetCookie(&session, strServerName);
		SetCookie(&session, url);

	}
	catch (exception e) {
		TRACE(e.what());
		line = "request failed - status code N/A";
	}
	catch (CInternetException *e) {
		TCHAR error[256];
		if(e->GetErrorMessage(error, 256)) {
			TRACE(error);
		}
		e->Delete();
		e = NULL;
		line = "request failed - status code N/A";
	}
	if(pServer != NULL) {
		pServer->Close();
		delete pServer;
		pServer = NULL;
	}
	if(pFile != NULL) {
		pFile->Close();
		delete pFile;
		pFile = NULL;
	}
	return line;

}
