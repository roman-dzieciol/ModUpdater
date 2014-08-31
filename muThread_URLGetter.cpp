// ============================================================================
//  muThread_URLGetter
// ============================================================================

#include "muPrecompile.h"
#include "muApp.h"
#include "muFrame.h"
#include "muThread_URLGetter.h"
#include "muEvents.h"
#include "muException.h"



muThread_URLGetter::muThread_URLGetter( muFrame* frame )
: muThread(frame)
{
	m_doneEvent = Updater_OnURLGetterDone;
	m_progressEvent = Updater_OnURLGetterProgress;
}


void* muThread_URLGetter::Entry()
{
	try
	{
		// Get info server url
		GetInfoServerURL();

		// See which versions are available
		DownloadVersionInfo();

		// Finished
		SetDone(Updater_Done, wxT("Finished searching for latest version."));
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

void muThread_URLGetter::GetInfoServerURL()
{
	// Mirrors file path
	wxFileName mirrorrsPath = wxFileName(wxT("UpdaterMirrors.xml"));
	if( !mirrorrsPath.Normalize(wxPATH_NORM_ALL, muApp::GetAppDir().GetPath()) )
		throw muException(wxT("Invalid file path! %s"), mirrorrsPath.GetFullPath().c_str());

	// Load mirrors file
	TiXmlDocument xmlDoc;
	if( !xmlDoc.LoadFile(mirrorrsPath.GetFullPath().mb_str()) )
		throw muException(wxT("XML file could not be opened!\n%s\n%s"), mirrorrsPath.GetFullPath(), muGetXMLError(xmlDoc).c_str());

	// Get the url element
	TiXmlElement* elem = TiXmlHandle(xmlDoc.RootElement()).FirstChildElement("mirrors").FirstChildElement("url").ToElement();
	if( !elem )
		throw muException(wxT("Invalid file! %s"), mirrorrsPath.GetFullPath().c_str());

	// Load url's
	while(elem)
	{
		wxString text = wxString::FromUTF8(elem->GetText());
		if( !text.empty() )
		{
			m_urlArray.Add(text);
			++m_progressTotal;
		}
		elem = elem->NextSiblingElement("url");
	}

	if( m_urlArray.IsEmpty() )
		throw muException(wxT("Empty file! %s"), mirrorrsPath.GetFullPath().c_str());
}

void muThread_URLGetter::DownloadVersionInfo()
{
	for( unsigned int i=0; i<m_urlArray.size(); i++ )
	{
		// Exit check
		if( TestDestroy() )
			return;

		wxString url = wxString::Format(wxT("%s/Latest.txt"), m_urlArray[i].c_str());

		wxMemoryOutputStream dataStream;
		if( !DownloadFile(url, &dataStream) )
		{
			// Update progress
			++m_progressCurrent;
			PostProgress(wxT(""));
			continue;
		}

		wxMemoryInputStream memStream(dataStream);
		wxTextInputStream textStream(memStream);
		wxString line = textStream.ReadLine();
		long version = 0;
		if( line.ToLong(&version) )
		{
			// Update GUI
			UpdaterEvent event(UpdaterCommandEvent, Updater_OnLatestVersion);
			event.SetInt(version);
			event.SetString(m_urlArray[i]);
			wxPostEvent(m_frame, event);
		}

		// Update progress
		++m_progressCurrent;
		PostProgress(wxT(""));

		// Sleep
		wxThread::Sleep(1);
	}    
}



// ============================================================================
//  EOF
// ============================================================================