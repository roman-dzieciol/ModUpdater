// ============================================================================
//  muApp
// ============================================================================
#pragma once


// the application icon (under Windows and OS/2 it is in resources and even
// though we could still include the XPM here it would be unused)
//#if !defined(__WXMSW__) && !defined(__WXPM__)
//#include "muPrecompile.xpm"
//#endif

// IDs for the controls and the menu commands
enum
{
	// menu items
	muPrecompile_Quit = wxID_EXIT,
	muPrecompile_About = wxID_ABOUT
};


// Define a new application type, each program should derive a class from wxApp
class muApp : public wxApp
{
public:
	~muApp();

	// override base class virtuals
	virtual void OnInitCmdLine(wxCmdLineParser& parser);
	virtual bool OnInit();
	virtual int OnExit();

public:
	static wxString GetUserAgent();
	static wxFileName GetAppDir();
	static wxFileName GetGamePath();
	static wxFileName GetGameDir();
	static wxString GetProductName();

public:
	wxString GetCmdLine();
	wxString GetGameCommand();
	void StartGame();

private:
	void LogPlatform();

private:
	wxSingleInstanceChecker* m_checker;
	wxFFile logFile;
	wxLog* logTarget;
	wxLog* logTargetOld;
};

DECLARE_APP(muApp);

wxString muGetXMLError(const TiXmlDocument& doc);

// ============================================================================
//  EOF
// ============================================================================