#include "StdAfx.h"
#include "trackerconnector.h"
#include "afxinet.h"
#include "afxwin.h"
#include <iostream>
#include "zlib.h"

TrackerConnector::TrackerConnector(void)
{
}

TrackerConnector::~TrackerConnector(void)
{
}

int TrackerConnector::RegisterWithTracker(char * url,unsigned short **body, string referer)
{
	CInternetSession session("Azereus 2.2.0.0");
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

			line =  "request failed - can only use URLs beginning with http://";
		} 

		if (bProgressMode) 
		{ 
			//Log( _T("Opening Internet...")); 
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
		//cout << "file: " << file << endl;
		
		
 		pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_GET,file.c_str()); 

		pFile->SendRequest(); 

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
				//Log(_T("Access denied: Invalid password\n")); 
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
				//Log( _T("Error: Site redirects with no new location")); 
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
				//Log( _T("Caution: redirected to ")); 
				//Log((LPCTSTR) strNewLocation); 
			} 

			// figure out what the old place was 
			if (!AfxParseURL(strNewLocation, dwServiceType, strServerName, strObject, nPort)) 
			{ 
				//Log(_T("Error: the redirected URL could not be parsed.")); 
				line =  "request failed - the redirected URL could not be parsed.";
			} 

			if (dwServiceType != INTERNET_SERVICE_HTTP) 
			{ 
				//Log(_T("Error: the redirected URL does not reference a HTTP resource."));
				line =  "request failed - the redirected URL does not reference a HTTP resource";
			} 

			// try again at the new location 
			pServer = session.GetHttpConnection(strServerName, nPort); 
			pFile = pServer->OpenRequest(CHttpConnection::HTTP_VERB_GET, strObject);
			//pFile->AddRequestHeaders(szHeaders); 
			pFile->SendRequest(); 

			pFile->QueryInfoStatusCode(dwRet); 
			if (dwRet != HTTP_STATUS_OK) 
			{ 
				cout << _T("Error: Got status code ") << dwRet << endl; 
				line =  "request failed - Got status code " + dwRet;
			} 
		}

		//cout << _T("Status Code is ") << dwRet << endl; 
		CString content_type = "";
		pFile->QueryInfo(HTTP_QUERY_RAW_HEADERS_CRLF , content_type);
		if(content_type.Find(": gzip") > 0) isGzipped = true;

		const int read_size = 1024;
		TCHAR sz[read_size+1]; 
		UINT nRead = pFile->Read(sz, read_size); 
		//cout << nRead << " ";
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
				//if(!isGzipped) cout << sz[i];
				buf[i+curPos] = sz[i];
			}
			curPos += nRead;
			nRead = pFile->Read(sz, read_size); 
			//cout << nRead << " ";
			size += nRead;
		}
		//cout << endl;
		unsigned short * temp = new unsigned short[size+1];
		for(int i = 0; i < size; i++) {
			temp[i] = buf[i];
		}
		//if(!isGzipped) cout << endl;
		delete [] buf;
		buf = temp;
	}
	catch (exception e) {
		//Log(e.what());
		line = "request failed - status code N/A";
	}
	catch (CInternetException * e) {
		TCHAR error[256];
		if(e->GetErrorMessage(error, 256)) {
//			Log(error);
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
	//cout << endl << "SIZE: " << size << endl;
	return size;

}

int TrackerConnector::UnZip(unsigned short ** dest, unsigned short * source, int len) 
{
		bool write = false;
		//if(write) TorrentFile::WriteFile((unsigned short*)source, len, cur_title, "zip");
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
				cout << strm.msg;
			}
			if(zip_status != Z_OK) { 
				//TorrentFile::WriteFile(source, len, cur_title, "zip");
				//TorrentFile::WriteFile((unsigned short*)uncompr, size, cur_title, "txt");
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
		//if(write) TorrentFile::WriteFile((unsigned short*)uncompr, size, cur_title, "txt");
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
