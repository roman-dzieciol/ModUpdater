// ============================================================================
//  muApp
// ============================================================================

#include "muPrecompile.h"
#include "muApp.h"
#include "muFrame.h"
#include "muThread_IntegrityChecker.h"
#include "muThread_URLGetter.h"



// ----------------------------------------------------------------------------
// the application class
// ----------------------------------------------------------------------------

IMPLEMENT_APP(muApp)

muApp::~muApp()
{    
	delete m_checker;
}

void muApp::OnInitCmdLine(wxCmdLineParser& parser)
{
	wxApp::OnInitCmdLine(parser);
}

// 'Main program' equivalent: the program execution "starts" here
bool muApp::OnInit()
{
	// call the base class initialization method, currently it only parses a
	// few common command-line options but it could be do more in the future
	if( !wxApp::OnInit() )
		return false;

	const wxString title = wxT("Gunreal Updater");
	m_checker = new wxSingleInstanceChecker(title);
	if( m_checker->IsAnotherRunning() )
	{
		wxLogError(_("Updater is already running, aborting."));
		return false;
	}

	// create the main application window
	muFrame *frame = new muFrame(title);

	// center
	frame->Center();
	SetTopWindow(frame); 

	// Mirrors path
	wxFileName logPath = wxFileName(wxT("UpdaterLog.txt"));
	if( logPath.Normalize(wxPATH_NORM_ALL, muApp::GetAppDir().GetPath()) )
	{
		// create log
		logFile.Open(logPath.GetFullPath(), wxT("w"));
		logTarget = new wxLogStderr(logFile.fp()); 
		logTargetOld = wxLog::GetActiveTarget();
		logTarget->SetVerbose(TRUE); 
		wxLog::SetActiveTarget(logTarget); 
	}

	// Log platform details
	LogPlatform();

	// Log commandline
	wxLogVerbose(wxT("Commandline: %s"), muApp::GetCmdLine().c_str());

	// and show it (the frames, unlike simple controls, are not shown when
	// created initially)
	frame->Show(); 

	// Integrity Thread
	muThread_IntegrityChecker* integrityThread = new muThread_IntegrityChecker(frame);
	if( integrityThread->Create() != wxTHREAD_NO_ERROR )
	{
		wxLogError(wxT("Can't create muThread_IntegrityChecker!"));
	}

	if( integrityThread->Run() != wxTHREAD_NO_ERROR )
	{
		wxLogError(wxT("Can't start muThread_IntegrityChecker!"));
	}

	// URLGetter Thread
	muThread_URLGetter* locatorThread = new muThread_URLGetter(frame);
	if( locatorThread->Create() != wxTHREAD_NO_ERROR )
	{
		wxLogError(wxT("Can't create muThread_URLGetter!"));
	}

	if( locatorThread->Run() != wxTHREAD_NO_ERROR )
	{
		wxLogError(wxT("Can't start muThread_URLGetter!"));
	}


	// success: wxApp::OnRun() will be called which will enter the main message
	// loop and the application will run. If we returned false here, the
	// application would exit immediately.
	return true;
}

int muApp::OnExit()
{

	return wxApp::OnExit();
}

wxString muApp::GetUserAgent()
{
	return wxString::Format(wxT("Gunreal-Updater/1.0 (%s)"), wxGetOsDescription().c_str());
}

wxFileName muApp::GetAppDir()
{
	return wxFileName::DirName(wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath());
}

wxString muApp::GetCmdLine()
{
	wxString s;
	for(int i=1; i<argc; ++i)
	{
		s.Append(argv[i]);
		s.Append(wxT(" "));
	}
	return s;
}

wxString muApp::GetGameCommand()
{
	return wxString::Format( wxT("%s %s -mod=Gunreal"), GetGamePath().GetFullPath().c_str(), GetCmdLine().c_str() );
}

wxFileName muApp::GetGamePath()
{
	// Manifest path
	wxFileName path = wxFileName(wxT("..\\System\\UT2004.exe"));
	if( !path.Normalize(wxPATH_NORM_ALL, muApp::GetAppDir().GetPath()) )
	{
		wxLogError(wxT("Invalid file path! %s"), path.GetFullPath().c_str());
	}
	return path;
}

wxFileName muApp::GetGameDir()
{
	// Manifest path
	wxFileName path = wxFileName(wxT("..\\"));
	if( !path.Normalize(wxPATH_NORM_ALL, muApp::GetAppDir().GetPath()) )
	{
		wxLogError(wxT("Invalid dir path! %s"), path.GetFullPath().c_str());
	}
	return path;
}

wxString muApp::GetProductName()
{
	return wxT("Gunreal");
}

void muApp::StartGame()
{
	wxLogVerbose(wxT("Starting the game: %s"), GetGameCommand().c_str());
	wxExecute(GetGameCommand());
}

// ----------------------------------------------------------------------------
//  debug
// ----------------------------------------------------------------------------
void muApp::LogPlatform()
{
	wxPlatformInfo p = wxPlatformInfo::Get();
	wxLogVerbose(wxT("Platform details:") );
	wxLogVerbose(wxT(" * CPU Count: %d"), wxThread::GetCPUCount() );
	wxLogVerbose(wxT(" * OS: %s"), wxGetOsDescription().c_str() );
	wxLogVerbose(wxT(" * OS ID: %s"), p.GetOperatingSystemIdName().c_str() );
	wxLogVerbose(wxT(" * OS Family: %s"), p.GetOperatingSystemFamilyName().c_str() );
	wxLogVerbose(wxT(" * OS Version: %d.%d"), p.GetOSMajorVersion(), p.GetOSMinorVersion() );
	wxLogVerbose(wxT(" * Toolkit Version: %d.%d"), p.GetToolkitMajorVersion(), p.GetToolkitMinorVersion() );
	wxLogVerbose(wxT(" * Architecture: %s"), p.GetArchName().c_str() );
	wxLogVerbose(wxT(" * Endianness: %s"), p.GetEndiannessName().c_str() );
	wxLogVerbose(wxT(" * WX ID: %s"), p.GetPortIdName().c_str() );
	wxLogVerbose(wxT(" * WX Version: %d.%d.%d.%d"), wxMAJOR_VERSION, wxMINOR_VERSION, wxRELEASE_NUMBER, wxSUBRELEASE_NUMBER );
}


// ============================================================================
//  XML related
// ============================================================================
wxString muGetXMLError(const TiXmlDocument& doc)
{
	return wxString::Format(wxT("Ln %d, Col %d, (%d) %s")
		, doc.ErrorRow(), doc.ErrorCol(), doc.ErrorId(), wxString::FromUTF8(doc.ErrorDesc()).c_str() );
}
// ============================================================================
//  EOF
// ============================================================================