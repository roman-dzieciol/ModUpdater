// ============================================================================
//  muFrame
// ============================================================================

#include "muPrecompile.h"
#include "muApp.h"
#include "muFrame.h"
#include "muThread_IntegrityChecker.h"
#include "muEvents.h"
#include "muThread_Updater.h"


// ----------------------------------------------------------------------------
// main frame :: static
// ----------------------------------------------------------------------------

const wxDouble muFrame::m_integrityScale = 75.0f;
const wxDouble muFrame::m_locatorScale = 25.0f;
const wxDouble muFrame::m_updatingScale = 100.0f;



// ----------------------------------------------------------------------------
// main frame
// ----------------------------------------------------------------------------

muFrame::muFrame(const wxString& title)
: m_integrityProgress(0)
, m_locatorProgress(0)
, m_updatingProgress(0)
, m_currentVersion(0)
, m_latestVersion(0)
, m_integrityDone(false)
, m_locatorDone(false)
, m_updatingDone(false)
, m_startGame(true)
, wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxDefaultSize
		  , wxMINIMIZE_BOX 
		  | wxSYSTEM_MENU 
		  | wxCAPTION 
		  | wxCLOSE_BOX 
		  | wxCLIP_CHILDREN )
{
	//
	// Controls
	//

	// Main panel
	wxPanel* mainPanel = new wxPanel(this, wxID_ANY);

	// Static sizer, must be created first
	//wxStaticBoxSizer *staticSizer = new wxStaticBoxSizer( wxHORIZONTAL, mainPanel, wxT("Checking for updates...") );
	//wxStaticBoxSizer *staticSizer = new wxStaticBoxSizer( wxHORIZONTAL, mainPanel, wxT("") );
	wxBoxSizer *staticSizer = new wxBoxSizer( wxHORIZONTAL );

	// Details label
	m_detailsLabel = new wxStaticText(mainPanel, wxID_STATIC, wxT("Initializing..."), wxDefaultPosition, wxSize(-1,-1));

	// Progress bar
	m_progressGauge = new wxGauge(mainPanel, wxID_STATIC, 100, wxDefaultPosition, wxSize(352,-1));

	// Skip button
	wxButton* skipButton = new wxButton(mainPanel, wxID_OK, wxT("Skip"));

	//
	// Sizers
	// 

	// progress gauge + skip button horizontal

	wxBoxSizer *progressSizer = new wxBoxSizer( wxHORIZONTAL );
	progressSizer->Add( m_progressGauge, wxSizerFlags(0).Align(wxALIGN_NOT).Expand().Border(wxALL));
	progressSizer->Add( skipButton, wxSizerFlags(0).Align(wxALIGN_NOT).Expand().Border(wxALL));

	// details above
	wxBoxSizer *detailsSizer = new wxBoxSizer( wxVERTICAL );
	detailsSizer->Add( m_detailsLabel, wxSizerFlags(0).Align(wxALIGN_NOT).Expand().Border(wxALL));
	detailsSizer->Add( progressSizer, wxSizerFlags(0).Align(wxALIGN_NOT).Expand());

	// static sizer
	staticSizer->Add( detailsSizer, wxSizerFlags(0).Align(wxALIGN_NOT).Expand());

	// panel sizer
	wxBoxSizer *panelSizer = new wxBoxSizer( wxVERTICAL );
	panelSizer->Add( staticSizer, wxSizerFlags(0).Align(wxALIGN_NOT).Expand().Border(wxALL));
	mainPanel->SetSizerAndFit( panelSizer );     

	// panel sizer
	wxBoxSizer *frameSizer = new wxBoxSizer( wxVERTICAL );
	frameSizer->Add( mainPanel, wxSizerFlags(1).Expand());
	SetSizerAndFit( frameSizer );      
	frameSizer->SetSizeHints( this ); 


	// Events
	Connect( wxID_OK, wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(muFrame::OnSkipButton), NULL, this );

	Connect( wxID_ANY, wxEVT_CLOSE_WINDOW, wxCloseEventHandler(muFrame::OnClose), NULL, this );

	Connect( Updater_OnLog, UpdaterCommandEvent, UpdaterEventHandler(muFrame::OnLog), NULL, this );
	Connect( Updater_OnCurrentVersion, UpdaterCommandEvent, UpdaterEventHandler(muFrame::OnCurrentVersion), NULL, this );
	Connect( Updater_OnLatestVersion, UpdaterCommandEvent, UpdaterEventHandler(muFrame::OnLatestVersion), NULL, this );
	
	Connect( Updater_OnUpdateDownload, UpdaterCommandEvent, UpdaterEventHandler(muFrame::OnUpdateDownload), NULL, this );
	Connect( Updater_OnUpdateInstall, UpdaterCommandEvent, UpdaterEventHandler(muFrame::OnUpdateInstall), NULL, this );

	Connect( Updater_OnIntegrityProgress, UpdaterCommandEvent, UpdaterEventHandler(muFrame::OnIntegrityProgress), NULL, this );
	Connect( Updater_OnIntegrityDone, UpdaterCommandEvent, UpdaterEventHandler(muFrame::OnIntegrityDone), NULL, this );

	Connect( Updater_OnURLGetterProgress, UpdaterCommandEvent, UpdaterEventHandler(muFrame::OnURLGetterProgress), NULL, this );
	Connect( Updater_OnURLGetterDone, UpdaterCommandEvent, UpdaterEventHandler(muFrame::OnURLGetterDone), NULL, this );

	Connect( Updater_OnUpdatingProgress, UpdaterCommandEvent, UpdaterEventHandler(muFrame::OnUpdatingProgress), NULL, this );
	Connect( Updater_OnUpdatingDone, UpdaterCommandEvent, UpdaterEventHandler(muFrame::OnUpdatingDone), NULL, this );


	// Frame icon
	SetIcon(wxICON(muPrecompile));
}

void muFrame::DisplayCheckingProgress()
{
	wxDouble progress = m_integrityProgress * m_integrityScale;
	progress += m_locatorProgress * m_locatorScale;
	m_progressGauge->SetValue(progress);
	//m_progressGauge->Pulse();
	//wxLogMessage(wxString::Format(wxT("progress %0.2f %0.2f %0.2f"), progress, m_integrityProgress, m_locatorProgress));
}

void muFrame::DisplayUpdatingProgress()
{
	wxDouble progress = m_updatingProgress * m_updatingScale;
	m_progressGauge->SetValue(progress);
	//m_progressGauge->Pulse();
}


// ----------------------------------------------------------------------------
//  main frame :: events
// ----------------------------------------------------------------------------

void muFrame::OnSkipButton(wxCommandEvent& WXUNUSED(event))
{
	wxLogVerbose(wxT("Skipping..."));
	Close(true);
}

void muFrame::OnClose(wxCloseEvent& event)
{
	wxLogVerbose(wxT("Closing..."));

	if( m_startGame )
	{
		wxGetApp().StartGame();
	}

	event.Skip();
}

void muFrame::OnLog(UpdaterEvent& event)
{
	// thread-safe logging
	int type = event.GetInt();
	wxString text = event.GetString();
	switch(type)
	{
	case UpdaterLog_Error:		wxLogError(text); break;
	case UpdaterLog_Warning:	wxLogWarning(text); break;
	case UpdaterLog_Message:	wxLogMessage(text); break;
	case UpdaterLog_Verbose:	wxLogVerbose(text); break;
	default:					wxLogMessage(text); break;
	}
}

void muFrame::OnUpdateDownload(UpdaterEvent& event)
{
	wxString changelog = event.GetString();
	wxLogVerbose(wxT("Displaying changelog: %s"), changelog.c_str());
	wxLaunchDefaultBrowser(changelog);
	RequestUserAttention();
}

void muFrame::OnUpdateInstall(UpdaterEvent& event)
{
	m_installerPath = event.GetString();
	wxLogVerbose(wxT("Installer path: %s"), m_installerPath.c_str());
}

void muFrame::OnCurrentVersion(UpdaterEvent& event)
{
	m_currentVersion = event.GetInt();
	m_detailsLabel->SetLabel(wxString::Format(wxT("Checking Gunreal v%i... "), m_currentVersion));
}

void muFrame::OnLatestVersion(UpdaterEvent& event)
{
	int version = event.GetInt();
	if( version >= m_latestVersion )
	{
		m_latestVersion = version;
		m_latestURL.Add(event.GetString());

		// Log
		wxLogVerbose(wxT("Latest version: %d at: %s"), version, event.GetString().c_str());
	}
}

void muFrame::OnIntegrityProgress(UpdaterEvent& event)
{
	m_integrityProgress = wxDouble(event.GetInt()) / wxDouble(event.GetExtraLong());
	DisplayCheckingProgress();
	if( !event.GetString().IsEmpty() )
		wxLogVerbose(wxT("[Integrity] %s"), event.GetString().c_str());
}

void muFrame::OnIntegrityDone(UpdaterEvent& event)
{
	m_integrityDone = true;
	m_integrityProgress = 1.0f;
	DisplayCheckingProgress();
	if( !event.GetString().IsEmpty() )
		wxLogVerbose(wxT("[Integrity] %s"), event.GetString().c_str());

	/*if( event.GetInt() != Updater_Done )
	{
		wxLogVerbose(wxT("Integrity check failed!"));
	}
	else*/
	{
		TryUpdating();
	}
}

void muFrame::OnURLGetterProgress(UpdaterEvent& event)
{
	m_locatorProgress = wxDouble(event.GetInt()) / wxDouble(event.GetExtraLong());
	DisplayCheckingProgress();
	if( !event.GetString().IsEmpty() )
		wxLogVerbose(wxT("[URLGetter] %s"), event.GetString().c_str());
}

void muFrame::OnURLGetterDone(UpdaterEvent& event)
{
	m_locatorDone = true;
	m_locatorProgress = 1.0f;
	DisplayCheckingProgress();
	if( !event.GetString().IsEmpty() )
		wxLogVerbose(wxT("[URLGetter] %s"), event.GetString().c_str());

	if( event.GetInt() != Updater_Done )
	{
		wxLogVerbose(wxT("Searching for latest version failed!"));
	}
	else
	{
		TryUpdating();
	}
}

void muFrame::TryUpdating()
{
	if( !m_integrityDone || !m_locatorDone )
		return;

	if( m_latestVersion <= m_currentVersion )
	{
		wxLogVerbose(wxT("You have the latest version! %d"), m_currentVersion);
		m_detailsLabel->SetLabel(wxString::Format(wxT("Your Gunreal v%i is the latest version."), m_currentVersion));
		Close(true);
	}
	else
	{
		wxLogVerbose(wxT("Updated version available! %d"), m_latestVersion);
		m_detailsLabel->SetLabel(wxString::Format(wxT("Downloading Gunreal v%i..."), m_latestVersion));
		DisplayUpdatingProgress();

		// Updating Thread
		muThread_Updater* updatingThread = new muThread_Updater(this, m_currentVersion, m_latestVersion, m_latestURL);
		if( updatingThread->Create() != wxTHREAD_NO_ERROR )
		{
			wxLogError(wxT("Can't create muThread_Updater!"));
			Close(true);
		}

		if( updatingThread->Run() != wxTHREAD_NO_ERROR )
		{
			wxLogError(wxT("Can't start muThread_Updater!"));
			Close(true);
		}
	}
}


void muFrame::OnUpdatingProgress(UpdaterEvent& event)
{
	if( event.GetExtraLong() > 0.0f )
		m_updatingProgress = wxDouble(event.GetInt()) / wxDouble(event.GetExtraLong());
	else
		m_updatingProgress = 0.0f;

	DisplayUpdatingProgress();
	if( !event.GetString().IsEmpty() )
		wxLogVerbose(wxT("[Updater] %s"), event.GetString().c_str());

	if( m_updatingProgress > 0 )
	{
		double dlnow = event.GetInt();
		double dltotal = event.GetExtraLong();
		static wxStopWatch eventWatch;
		static double lastdl = 0;

		double dldelta = dlnow - lastdl;
		dldelta /= 1024.0f;
		lastdl = dlnow;

		eventWatch.Pause();
		double speed = 0;
		if( eventWatch.Time() > 0.0f )
			speed = dldelta / eventWatch.Time();
		eventWatch.Start();

		static double speedAvg = 0.0f;
		if( speed >= 0.0f )
			speedAvg = (speedAvg*0.95f + speed*0.05f);

		wxString remainingText = wxT("?");
		if( speedAvg > 0 )
		{
			double secondsRemaining = (dltotal-dlnow) / 1024.0f / (speedAvg * 1000.0f);
			wxTimeSpan span = wxTimeSpan::Seconds(secondsRemaining);
			remainingText = span.Format();
		}

		// update label every second only
		static wxStopWatch limitWatch;
		limitWatch.Pause();
		if( limitWatch.Time() > 1000 )
		{
			m_detailsLabel->SetLabel(wxString::Format(	
				wxT("Downloading Gunreal v%i... %s remaining (%03.1lf MB at %0.0lf KB/s)")
				, m_latestVersion
				, remainingText.c_str()
				, dltotal / 1024.0f / 1024.0f
				, speedAvg * 1000.0f
				));

			limitWatch.Start();
		}
		else
			limitWatch.Resume();
	}
}

#if 0
void muFrame::OnUpdatingProgress(UpdaterEvent& event)
{
	if( event.GetExtraLong() > 0.0f )
		m_updatingProgress = wxDouble(event.GetInt()) / wxDouble(event.GetExtraLong());
	else
		m_updatingProgress = 0.0f;

	DisplayUpdatingProgress();
	if( !event.GetString().IsEmpty() )
		wxLogVerbose(event.GetString());

	if( m_updatingProgress > 0 )
	{
		double dlnow = event.GetInt();
		//double dltotal = event.GetExtraLong();
		static wxStopWatch eventWatch;
		static double lastdl = 0;

		double dldelta = dlnow - lastdl;
		dldelta /= 1024;
		lastdl = dlnow;

		eventWatch.Pause();
		double speed = 0;
		if( eventWatch.Time() > 0.0f )
			speed = dldelta / eventWatch.Time() * 1000.0f;
		eventWatch.Start();

		static double speedAvg = 0.0f;
		if( speed >= 0.0f )
			speedAvg = (speedAvg*0.95f + speed*0.05f);

		// update label every second only
		static wxStopWatch limitWatch;
		limitWatch.Pause();
		if( limitWatch.Time() > 1000 )
		{
			m_detailsLabel->SetLabel(wxString::Format(
				wxT("Downloading Gunreal v%i - %0.1lf%% at %0.0lfKb/s...")
				, m_latestVersion, m_updatingProgress*100.0f, speedAvg));
			
			limitWatch.Start();
		}
		else
			limitWatch.Resume();
	}
}
#endif

void muFrame::OnUpdatingDone(UpdaterEvent& event)
{
	m_updatingDone = true;
	m_updatingProgress = 1.0f;
	DisplayUpdatingProgress();
	if( !event.GetString().IsEmpty() )
		wxLogVerbose(wxT("[Updater] %s"), event.GetString().c_str());

	if( event.GetInt() != Updater_Done )
	{
		wxLogVerbose(wxT("Updating failed!"));
	}
	else if( !m_installerPath.empty() )
	{
		// Store game folder in registry
		wxRegKey* key = new wxRegKey(wxT("HKEY_CURRENT_USER\\SOFTWARE\\Unreal Technology\\Installed Apps\\UT2004_ModUpdater"));
		if(!key->Exists() )
			key->Create();
		key->SetValue(wxT("Folder"), muApp::GetGameDir().GetFullPath().c_str());

		// Ask user for permission to update
		if( wxYES == wxMessageBox
			( wxT("Do you want to update Gunreal?") 
			, wxT("Question")
			, wxYES_NO | wxCENTRE | wxICON_QUESTION, this) )
		{
			wxLogVerbose(wxT("Installing: %s"), m_installerPath.c_str());
			m_startGame = false;
			wxExecute(wxString::Format(wxT("%s %s"), m_installerPath.c_str(), wxGetApp().GetCmdLine().c_str() ));
		}
		else
		{
			wxLogVerbose(wxT("User choosed not to install: %s"), m_installerPath.c_str());
		}
	}

	Close(true);
}

// ============================================================================
//  EOF
// ============================================================================
