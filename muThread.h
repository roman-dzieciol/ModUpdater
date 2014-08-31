// ============================================================================
//  muThread
// ============================================================================
#pragma once


class muThread : public wxThread
{
public:
	muThread(muFrame* frame);
	virtual ~muThread();

	// thread execution starts here
	virtual void *Entry() = 0;

	// called when the thread exits - whether it terminates normally or is
	// stopped with Delete() (but not when it is Kill()ed!)
	virtual void OnExit();

public:
	virtual void PostProgress( const wxChar *pszFormat, ... );
	virtual void SetProgress(wxUint32 progressCurrent, wxUint32 progressTotal);
	virtual void SetDone(wxUint32 result, const wxChar *pszFormat, ... );

public:
	virtual bool DownloadFile(const wxString& url, wxOutputStream* stream, bool showProgress=false);
	void LogDownloadStats(CURL* curl);

public:
	bool MD5File(wxFileName fileName, wxString& digest, bool postProgress=true);
	bool MD5String(wxString data, wxString& digest);
	wxString MD5Print(void* data);

public:
	muFrame* GetFrame() const { return m_frame; }
	bool WantsDownloadProgress() const { return m_progressDownload; }

protected:
	muFrame* m_frame;
	wxUint32 m_doneEvent;
	wxUint32 m_doneResult;
	wxString m_doneText;
	wxUint32 m_progressEvent;
	wxUint32 m_progressCurrent;
	wxUint32 m_progressTotal;
	bool m_progressDownload;
};



// ============================================================================
//  EOF
// ============================================================================