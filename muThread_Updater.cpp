// ============================================================================
//  muThread_Updater
// ============================================================================
//	TODO
//  * Mirror randomization
// ============================================================================

#include "muPrecompile.h"
#include "muApp.h"
#include "muFrame.h"
#include "muThread_Updater.h"
#include "muEvents.h"
#include "muException.h"


muThread_Updater::muThread_Updater( muFrame* frame, int currentVersion, int latestVersion, wxArrayString urls )
: m_currentVersion(currentVersion)
, m_latestVersion(latestVersion)
, m_versionURLs(urls)
, muThread(frame)
{
	m_doneEvent = Updater_OnUpdatingDone;
	m_progressEvent = Updater_OnUpdatingProgress;
}

void* muThread_Updater::Entry()
{
	// Try to download update using a list of valid URL's
	for( unsigned int i=0; i<m_versionURLs.size(); i++ )
	{
		try
		{
			// Exit check
			if( TestDestroy() )
				return NULL;

			// Download package info
			DownloadInfo* info = DownloadPackageInfo(m_versionURLs[i]);

			// Download update
			wxString filename = DownloadUpdate(info);

			// Install update
			InstallUpdate(filename);

			// Success
			SetDone(Updater_Done, wxT("Updating finished."));
			return NULL;
		}
		catch( const muException& e )
		{
			PostProgress(wxT("Exception: %s"), e.What().c_str());
		}
		catch(...)
		{
			PostProgress(wxT("Unknown Exception!"));
		}

		// Failure - try next
		wxThread::Sleep(1);
		continue;
	}

	// Failure - none found
	SetDone(Updater_Fail, wxT("Failed to update!"));
	return NULL;
}

DownloadInfo* muThread_Updater::DownloadPackageInfo(const wxString& baseURL)
{
	// Get URL
	wxString url = wxString::Format(wxT("%s/Packages_v%d.xml"), baseURL.c_str(), m_latestVersion);
	PostProgress(wxT("Updating from: %s"), url.c_str());

	// Download package info
	wxMemoryOutputStream dataStream;
	if( !DownloadFile(url, &dataStream) )
		throw muException(wxT("Failed to download %s"), url.c_str());

	// null-terminate just in case
	dataStream.PutC(0);

	// Parse package info
	PostProgress(wxT("Parsing: %s"), url.c_str());
	wxMemoryInputStream inputStream(dataStream);
	if( m_pkgInfo.Parse(inputStream, this) )
	{
		PostProgress(wxT("Parsed: %s"), url.c_str());
	}
	else
		throw muException(wxT("Failed to parse %s"), url.c_str());

	// Get current platform
	long platform = UpdatePlatform_None;
	wxPlatformInfo platformInfo;
	wxOperatingSystemId os = platformInfo.GetOperatingSystemId();
	if( os & wxOS_MAC )
		platform = UpdatePlatform_Mac;
	else if( os & wxOS_WINDOWS )
		platform = UpdatePlatform_Win;
	else if( os & wxOS_UNIX )
		platform = UpdatePlatform_Linux;
	else
		platform = UpdatePlatform_Any;

	// Get matching download info
	DownloadInfo* info = m_pkgInfo.FindBestUpdate(platform, m_currentVersion, m_latestVersion);
	if( !info )
		throw muException(wxT("Could not find matching update"));

	return info;
}

wxString muThread_Updater::DownloadUpdate(DownloadInfo* info)
{
	PostProgress(wxT("Found update: %s"), info->m_updateName.c_str());

	// changelog
	UpdaterEvent downloadEvent( UpdaterCommandEvent, Updater_OnUpdateDownload );
	downloadEvent.SetString(m_pkgInfo.m_changelog);
	wxPostEvent( m_frame, downloadEvent );

	// if we don't meet requirements, abort
	if( info->m_requires > m_currentVersion )
		throw muException(wxT("Update requires installing %d first!"), info->m_requires);

	// Get local download path
	wxFileName fileName = wxFileName(info->m_fileName.c_str());
	if( !fileName.Normalize(wxPATH_NORM_ALL, muApp::GetAppDir().GetPath()) )
		throw muException(wxT("Invalid path! %s"), fileName.GetFullPath().c_str());

	// See if file was already downloaded
	if( VerifyDownload(fileName.GetFullPath(), info->m_md5) )
	{
		PostProgress(wxT("File already downloaded: %s"), fileName.GetFullPath().c_str());
		return fileName.GetFullPath();
	}

	PostProgress(wxT("Downloading: %s"), fileName.GetFullPath().c_str());

	// Open file for writing
	wxFileOutputStream fileStream(fileName.GetFullPath());
	if( !fileStream.IsOk() )
		throw muException(wxT("Failed to open file for writing: %s"), fileName.GetFullPath().c_str());

	// Download file
	if( !DownloadFile(info->m_URLS[0], &fileStream, true) )
		throw muException(wxT("Failed to download: %s"), info->m_URLS[0].c_str());
	
	// Close file before continuing
	fileStream.Close();

	// Verify download
	if( !VerifyDownload(fileName.GetFullPath(), info->m_md5) )
		throw muException(wxT("Downloaded file is corrupted! %s"), fileName.GetFullPath().c_str());

	return fileName.GetFullPath();
}

bool muThread_Updater::VerifyDownload(const wxString& fileName, const wxString& md5Digest)
{
	wxString digest;
	if( !MD5File(fileName, digest, false) || !md5Digest.IsSameAs(digest,false) )
		return false;
	return true;
}

void muThread_Updater::InstallUpdate(const wxString& fileName)
{
	UpdaterEvent installEvent( UpdaterCommandEvent, Updater_OnUpdateInstall );
	installEvent.SetString(fileName);
	wxPostEvent( m_frame, installEvent );
}


// ============================================================================
//  EOF
// ============================================================================