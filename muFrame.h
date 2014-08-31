// ============================================================================
//  muFrame
// ============================================================================
#pragma once



class muFrame : public wxFrame
{
public:
	muFrame(const wxString& title);

private:
	void OnSkipButton(wxCommandEvent& event);
	void OnClose(wxCloseEvent& event);

private:
	void OnLog(UpdaterEvent& event);
	void OnCurrentVersion(UpdaterEvent& event);
	void OnLatestVersion(UpdaterEvent& event);
	void OnUpdateDownload(UpdaterEvent& event);
	void OnUpdateInstall(UpdaterEvent& event);

	void OnIntegrityProgress(UpdaterEvent& event);
	void OnIntegrityDone(UpdaterEvent& event);

	void OnURLGetterProgress(UpdaterEvent& event);
	void OnURLGetterDone(UpdaterEvent& event);

	void OnUpdatingProgress(UpdaterEvent& event);
	void OnUpdatingDone(UpdaterEvent& event);

private:
	void DisplayCheckingProgress();
	void DisplayUpdatingProgress();
	void TryUpdating();

private:
	static const wxDouble m_integrityScale;
	static const wxDouble m_locatorScale;
	static const wxDouble m_updatingScale;

	wxDouble m_integrityProgress;
	wxDouble m_locatorProgress;
	wxDouble m_updatingProgress;

	wxGauge* m_progressGauge;
	wxStaticText* m_detailsLabel;

	int m_currentVersion;
	int m_latestVersion;
	wxArrayString m_latestURL;

	bool m_integrityDone;
	bool m_locatorDone;
	bool m_updatingDone;

	bool m_startGame;

	wxString m_installerPath;
};





// ============================================================================
//  EOF
// ============================================================================