// ============================================================================
//  muThread_Updater
// ============================================================================
#pragma once

#include "muThread.h"
#include "muPackagesInfo.h"


// ============================================================================
//  muThread_Updater
// ============================================================================
class muThread_Updater : public muThread
{
public:
	muThread_Updater(muFrame* frame, int currentVersion, int latestVersion, wxArrayString urls);

	// thread execution starts here
	virtual void *Entry();

	friend class PackagesInfo;
	friend class DownloadInfo;

private:
	DownloadInfo* DownloadPackageInfo(const wxString& url);
	wxString DownloadUpdate(DownloadInfo* info);
	void InstallUpdate(const wxString& fileName);
	bool VerifyDownload(const wxString& fileName, const wxString& md5Digest);

private:
	int m_currentVersion;
	int m_latestVersion;
	wxArrayString m_versionURLs;
	PackagesInfo m_pkgInfo;
};



// ============================================================================
//  EOF
// ============================================================================