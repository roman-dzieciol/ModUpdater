// ============================================================================
//  muThread
// ============================================================================

#include "muPrecompile.h"
#include "muApp.h"
#include "muFrame.h"
#include "muEvents.h"
#include "muThread.h"


muThread::muThread( muFrame* frame )
: m_frame(frame)
, m_doneEvent(Updater_Unknown)
, m_doneResult(Updater_None)
, m_progressEvent(Updater_Unknown)
, m_progressCurrent(0)
, m_progressTotal(0)
, m_progressDownload(false)
{
}

muThread::~muThread()
{
}

void muThread::OnExit()
{
	// Update GUI
	UpdaterEvent doneEvent( UpdaterCommandEvent, m_doneEvent );
	doneEvent.SetInt(m_doneResult);
	doneEvent.SetString(m_doneText);
	wxPostEvent( m_frame, doneEvent );
}

void muThread::PostProgress( const wxChar *pszFormat, ... )
{
	va_list argptr;
	va_start(argptr, pszFormat);

	wxString s;
	s.PrintfV(pszFormat, argptr);

	va_end(argptr);

	// Update progress, trivial progress events limited to 60fps
	static wxStopWatch eventWatch;
	eventWatch.Pause();
	if( eventWatch.Time() > 16 || !s.empty() )
	{
		UpdaterEvent progressEvent( UpdaterCommandEvent, m_progressEvent );
		progressEvent.SetInt(m_progressCurrent);
		progressEvent.SetExtraLong(m_progressTotal);
		progressEvent.SetString(s);
		wxPostEvent( m_frame, progressEvent );
		eventWatch.Start();
	}
	else
		eventWatch.Resume();
}

void muThread::SetProgress(wxUint32 progressCurrent, wxUint32 progressTotal)
{
	m_progressCurrent = progressCurrent;
	m_progressTotal = progressTotal;
}

void muThread::SetDone(wxUint32 result, const wxChar *pszFormat, ... )
{
	va_list argptr;
	va_start(argptr, pszFormat);

	wxString s;
	s.PrintfV(pszFormat, argptr);

	va_end(argptr);

	if( !TestDestroy() )
	{
		m_doneResult = result;
		m_doneText = s;
	}
}


static int DownloadProgressCallback(void *clientp, double dltotal, double dlnow, double WXUNUSED(ultotal), double WXUNUSED(ulnow))
{
	muThread* thread = static_cast<muThread *>(clientp);

	// Post progress
	if( thread->WantsDownloadProgress() )
	{
		thread->SetProgress(dlnow, dltotal);
		thread->PostProgress(wxT(""));
	}
	return 0;
}

static size_t DownloadWriteCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	// Write to provided stream
	size_t realSize = size * nmemb;
	wxOutputStream* stream = static_cast<wxOutputStream *>(data);
	stream->Write(ptr, realSize);
	return stream->LastWrite();
}

bool muThread::DownloadFile(const wxString& url, wxOutputStream* stream, bool showProgress)
{
	bool bResult = false;
	m_progressDownload = showProgress;

	// Progress
	PostProgress(wxT("Downloading: %s"), url.c_str());

	// init libcurl
	curl_global_init(CURL_GLOBAL_ALL);

	// init the curl session
	CURL *curl = curl_easy_init();
	if(curl) 
	{
		// store pointer to us
		curl_easy_setopt(curl, CURLOPT_PRIVATE, this);

		// request server file time tracking
		curl_easy_setopt(curl, CURLOPT_FILETIME, this);

		// specify URL to get (convert to mb first)
		const wxWX2MBbuf url_buf = wxConvCurrent->cWX2MB(url);
		const char *url_cstr = static_cast<const char *>(url_buf);
		curl_easy_setopt(curl, CURLOPT_URL, url_cstr);

		// setup progress callback
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, DownloadProgressCallback);

		// send all data to this function
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)stream);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, DownloadWriteCallback);

		// some servers don't like requests that are made without a user-agent field, so we provide one  (convert to mb first)
		const wxWX2MBbuf agent_buf = wxConvCurrent->cWX2MB(muApp::GetUserAgent());
		const char *agent_cstr = static_cast<const char *>(agent_buf);
		curl_easy_setopt(curl, CURLOPT_USERAGENT, agent_cstr);

		// get it! 
		if( curl_easy_perform(curl) == CURLE_OK )
		{
			long httpCode ;
			curl_easy_getinfo( curl , CURLINFO_HTTP_CODE , &httpCode );

			// if HTTP 200/OK
			if( httpCode == 200 )
			{
				// Progress
				PostProgress(wxT("Downloaded: %s"), url.c_str());
				bResult = true;
			}
			else
			{
				PostProgress(wxT("ERROR! Server returned code %ld for %s"), httpCode, url.c_str());
			}
		}

		LogDownloadStats(curl);

		// cleanup curl stuff 
		curl_easy_cleanup(curl);
	}

	// we're done with libcurl, so clean it up 
	curl_global_cleanup();

	if( !bResult )
		PostProgress(wxT("ERROR! Failed to download: %s"), url.c_str());

	return bResult;
}

void muThread::LogDownloadStats(CURL* curl)
{
	char* charPtr;
	long longData;
	double doubleData;

	if( CURLE_OK == curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &charPtr) )
	{		
		wxString text = wxString::From8BitData(charPtr);
		PostProgress(wxT("Transfer EFFECTIVE_URL: %s"), text.c_str());
	}

	if( CURLE_OK == curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &longData) )
		PostProgress(wxT("Transfer RESPONSE_CODE: %ld"), longData);

	if( CURLE_OK == curl_easy_getinfo(curl, CURLINFO_HTTP_CONNECTCODE, &longData) )
		PostProgress(wxT("Transfer HTTP_CONNECTCODE: %ld"), longData);

	if( CURLE_OK == curl_easy_getinfo(curl, CURLINFO_FILETIME, &longData) )
		PostProgress(wxT("Transfer FILETIME: %ld"), longData);

	if( CURLE_OK == curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &doubleData) )
		PostProgress(wxT("Transfer TOTAL_TIME: %lf"), doubleData);

	if( CURLE_OK == curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME, &doubleData) )
		PostProgress(wxT("Transfer NAMELOOKUP_TIME: %lf"), doubleData);

	if( CURLE_OK == curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &doubleData) )
		PostProgress(wxT("Transfer CONNECT_TIME: %lf"), doubleData);

	if( CURLE_OK == curl_easy_getinfo(curl, CURLINFO_PRETRANSFER_TIME, &doubleData) )
		PostProgress(wxT("Transfer PRETRANSFER_TIME: %lf"), doubleData);

	if( CURLE_OK == curl_easy_getinfo(curl, CURLINFO_STARTTRANSFER_TIME, &doubleData) )
		PostProgress(wxT("Transfer STARTTRANSFER_TIME: %lf"), doubleData);

	if( CURLE_OK == curl_easy_getinfo(curl, CURLINFO_REDIRECT_TIME, &doubleData) )
		PostProgress(wxT("Transfer REDIRECT_TIME: %lf"), doubleData);

	if( CURLE_OK == curl_easy_getinfo(curl, CURLINFO_REDIRECT_COUNT, &longData) )
		PostProgress(wxT("Transfer REDIRECT_COUNT: %ld"), longData);

	if( CURLE_OK == curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &charPtr) )
	{		
		wxString text = wxString::From8BitData(charPtr);
		PostProgress(wxT("Transfer REDIRECT_URL: %s"), text.c_str());
	}

	if( CURLE_OK == curl_easy_getinfo(curl, CURLINFO_SIZE_UPLOAD, &doubleData) )
		PostProgress(wxT("Transfer SIZE_UPLOAD: %lf"), doubleData);

	if( CURLE_OK == curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &doubleData) )
		PostProgress(wxT("Transfer SIZE_DOWNLOAD: %lf"), doubleData);

	if( CURLE_OK == curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &doubleData) )
		PostProgress(wxT("Transfer SPEED_DOWNLOAD: %lf"), doubleData);

	if( CURLE_OK == curl_easy_getinfo(curl, CURLINFO_SPEED_UPLOAD, &doubleData) )
		PostProgress(wxT("Transfer SPEED_UPLOAD: %lf"), doubleData);

	if( CURLE_OK == curl_easy_getinfo(curl, CURLINFO_NUM_CONNECTS, &longData) )
		PostProgress(wxT("Transfer NUM_CONNECTS: %ld"), longData);

	
}

// ----------------------------------------------------------------------------
// MD5
// ----------------------------------------------------------------------------
bool muThread::MD5File(wxFileName fileName, wxString& digest, bool postProgress)
{
	md5 alg;
	int nLen;
	static const unsigned int chunkSize = 0xFFFF;
	unsigned char chBuffer[chunkSize];

	wxFile file;
	if( file.Open(fileName.GetFullPath(), wxFile::read) )
	{
		int sleepCount = 0;

		// Parse file
		while( !file.Eof() )
		{
			// Exit check
			if( TestDestroy() )
				return false;

			// Read chunk
			nLen = file.Read(chBuffer,chunkSize);
			if( nLen != wxInvalidOffset )
			{
				// Hash chunk
				alg.Update(chBuffer, nLen);

				// Update progress
				if( postProgress )
				{
					m_progressCurrent += nLen;
					PostProgress(wxT(""));
				}
			}

			// Sleep (proper one only after ~6MB to keep it fast)
			if( ++sleepCount % 100 == 0 )
				wxThread::Sleep(1);
			else
				wxThread::Sleep(0);
		}

		// Finish hashing
		alg.Finalize();

		// Get digest as wxString
		digest = MD5Print(alg.Digest());
		return true;
	}

	return false;
}

bool muThread::MD5String(wxString text, wxString& digest)
{
	// Convert string to mb
	const wxWX2MBbuf tmp_buf = wxConvCurrent->cWX2MB(text);
	const char *tmp_str = (const char*) tmp_buf;

	// Hash it
	md5 alg;
	alg.Update((uchar*)tmp_str, text.Len());
	alg.Finalize();

	// Get digest as wxString
	digest = MD5Print(alg.Digest());
	return true;
}

wxString muThread::MD5Print(void* data)
{
	uchar* charData = (uchar*)data;
	if( charData != NULL )
	{
		wxString digest;
		for( int i=0; i<16; i++ )
			digest.Append(wxString::Format(wxT("%0.2x"), charData[i]));
		return digest;
	}
	return wxEmptyString;
}


// ============================================================================
//  EOF
// ============================================================================