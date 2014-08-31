// ============================================================================
//  muThread_IntegrityChecker
// ============================================================================
//	TODO
//	* See if separating loading from hashing, and hashing several files 
//  simultaneously would be faster. Make it adaptive - hash simultaneous only 
//  if CPU is lagging behind and there are unused cores.
// ============================================================================

#include "muPrecompile.h"
#include "muApp.h"
#include "muFrame.h"
#include "muThread_IntegrityChecker.h"
#include "muEvents.h"
#include "muException.h"


muThread_IntegrityChecker::muThread_IntegrityChecker( muFrame* frame )
: m_currentVer(0)
, muThread(frame)
{
	m_doneEvent = Updater_OnIntegrityDone;
	m_progressEvent = Updater_OnIntegrityProgress;
}

void* muThread_IntegrityChecker::Entry()
{
	try
	{
		// Load manifest
		LoadManifest();

		// Verify manifest
		VerifyManifest();

		// Finished
		SetDone(Updater_Done, wxT("Current files verified."));
	}
	catch( const muException& e )
	{
		SetDone(Updater_Fail, wxT("Exception: %s"), e.What().c_str());
	}
	catch(...)
	{
		SetDone(Updater_Fail, wxT("Unknown Exception!"));
	}
	return NULL;
}


void muThread_IntegrityChecker::LoadManifest()
{
	PostProgress(wxT("Loading manifest..."));

	// Get local download path
	//wxFileName fileName = wxFileName( wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath()
	//	, wxString::Format(wxT("%s"), info->m_fileName.c_str()) );

	// Manifest path
	wxFileName md5Path = wxFileName(wxT("MD5.md5"));
	if( !md5Path.Normalize(wxPATH_NORM_ALL, muApp::GetAppDir().GetPath()) )
		throw muException(wxT("Invalid file path! %s"), md5Path.GetFullPath().c_str());

	// Manifest file
	wxTextFile file;
	if( !file.Open(md5Path.GetFullPath()) )
		throw muException(wxT("Failed to open file! %s"), md5Path.GetFullPath().c_str());

	// Parse manifest
	for( size_t i=0; i<file.GetLineCount(); i++ )
	{
		// Parse line
		wxString line = file[i].Trim().Trim(false);
		if( line.Left(1) == ';' )
		{
			ParseComment(line.Mid(1));
		}
		else
		{
			ParseHash(line);
		}
	}

	// Update progress
	PostProgress(wxT("Manifest loaded."));
}

bool muThread_IntegrityChecker::ParseComment(const wxString& line)
{
	// Parse
	int pos = line.Find('=');
	if( pos != wxNOT_FOUND )
	{
		wxString varName = line.Left(pos).Trim().Trim(false);
		wxString varValue = line.Mid(pos+1).Trim().Trim(false);
		if( !varName.IsEmpty() && !varValue.IsEmpty() )
		{
			// Identify
			if( wxString(wxT("VER")).IsSameAs(varName, false) )
			{
				// Current version
				if( varValue.ToULong(&m_currentVer) )
				{
					// Update GUI
					UpdaterEvent event(UpdaterCommandEvent, Updater_OnCurrentVersion);
					event.SetInt(m_currentVer);
					wxPostEvent(m_frame, event);
					return true;
				}
			}
			else if( wxString(wxT("SUM")).IsSameAs(varName, false) )
			{
				// Expected hash sum
				m_md5Sum = varValue;
				return true;
			}
		}
	}

	return false;
}

bool muThread_IntegrityChecker::ParseHash(const wxString& line)
{
	// Parse
	int pos = line.Find(wxT(" *"));
	if( pos != wxNOT_FOUND )
	{
		wxString varMD5 = line.Left(pos).Trim().Trim(false);
		wxString varPath = line.Mid(pos+2).Trim().Trim(false);
		if( !varMD5.IsEmpty() && !varPath.IsEmpty() )
		{
			// Normalize path
			wxFileName fileName = wxFileName(varPath);
			if( !fileName.Normalize(wxPATH_NORM_ALL, muApp::GetAppDir().GetPath()) )
			{
				PostProgress(wxT("Invalid path! %s"), fileName.GetFullPath().c_str());
				return false;
			}

			// Store path, MD5
			m_md5Array.Add(varMD5);
			m_pathArray.Add(fileName.GetFullPath());

			// Data for md5Sum
			m_md5Data.Append(varMD5);

			// Store size for progress
			m_progressTotal += fileName.GetSize().ToULong();
			return true;
		}
	}
	return false;
}


void muThread_IntegrityChecker::VerifyManifest()
{
	PostProgress(wxT("Verifying manifest..."));

	// Verify manifest
	if( m_currentVer == 0 
	||  m_md5Sum.IsEmpty() 
	||  m_md5Data.IsEmpty() )
		throw muException(wxT("Manifest invalid!"));

	// Verify hash sum
	wxString md5Current;
	if(!MD5String(m_md5Data.MakeLower(), md5Current) || !m_md5Sum.IsSameAs(md5Current, false) )
		throw muException(wxT("Hash sum invalid! Got: %s, expected: %s"), md5Current.c_str(), m_md5Sum.c_str());

	// Verify files
	unsigned long md5Count = m_md5Array.GetCount();
	for( unsigned int i=0; i<md5Count; i++ )
	{
		// Exit check
		if( TestDestroy() )
			return;

		// Update progress
		PostProgress(wxT("Verifying: %s %s"), m_md5Array[i].c_str(), m_pathArray[i].c_str());

		// Calc MD5
		wxString md5Current;
		wxFileName fileName = wxFileName(m_pathArray[i]);
		if( !MD5File(fileName.GetFullPath(), md5Current) )
			throw muException(wxT("Failed to verify file! %s"), fileName.GetFullPath().c_str());

		// Compare MD5
		if( !m_md5Array[i].IsSameAs(md5Current, false) )
			throw muException(wxT("Verification failed! Got: %s"), md5Current.c_str());

		// Sleep
		wxThread::Sleep(1);
	}   

	PostProgress(wxT("Manifest verified."));
}

// ============================================================================
//  EOF
// ============================================================================