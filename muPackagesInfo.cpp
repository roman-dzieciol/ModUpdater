// ============================================================================
//  PackagesInfo
// ============================================================================
//	TODO:
//  * download requirements chain if no full installer available
// ============================================================================

#include "muPrecompile.h"
#include "muPackagesInfo.h"
#include "muThread_Updater.h"


const wxString DownloadInfo::s_Platforms[UpdatePlatform_MAX] = {wxT("None"), wxT("Win"), wxT("Mac"), wxT("Linux"), wxT("Any")};
const wxString DownloadInfo::s_Types[UpdateType_MAX] = {wxT("None"), wxT("Installer"), wxT("Patch"), wxT("Archive")};


// ----------------------------------------------------------------------------
// DownloadInfo
// ----------------------------------------------------------------------------
bool DownloadInfo::IsOK()
{
	if( m_updateName.IsEmpty() 
	||  m_fileName.IsEmpty()
	||  m_md5.IsEmpty() )
		return false;

	if( m_URLS.empty() )
		return false;

	if( m_md5.size() != 32 )
		return false;

	if( m_platform <= UpdatePlatform_None || m_platform >= UpdatePlatform_MAX )
		return false;

	if( m_type <= UpdateType_None || m_type >= UpdateType_MAX )
		return false;

	return true;
}

bool DownloadInfo::Parse(TiXmlNode* node, muThread_Updater* thread)
{
	TiXmlHandle nodeHandle(node);
	m_thread = thread;

	// Read name
	m_updateName = ParseString(nodeHandle.FirstChild("name").ToElement());

	// Read filename
	m_fileName = ParseString(nodeHandle.FirstChild("file").ToElement());

	// Read mirrors
	for( node = nodeHandle.FirstChild("mirrors").FirstChild("url").ToNode(); node; node=node->NextSibling("url") )
	{
		wxString url = ParseString(node->ToElement());
		if( !url.empty() )
			m_URLS.push_back(url);
	}

	// Read platform
	m_platform = ParsePlatform(ParseString(nodeHandle.FirstChild("platform").ToElement()));

	// Read type
	m_type = ParseType(ParseString(nodeHandle.FirstChild("type").ToElement()));

	// Read requirement
	ParseString(nodeHandle.FirstChild("requires").ToElement()).ToLong(&m_requires);

	// Read md5
	m_md5 = ParseString(nodeHandle.FirstChild("md5").ToElement());

	return IsOK();
}

wxString DownloadInfo::ParseString(const TiXmlElement* elem)
{
	wxString result;
	if( elem )
	{
		result = wxString::FromUTF8(elem->GetText());
		m_thread->PostProgress(wxT("%s: %s"), wxString::FromUTF8(elem->Value()).c_str(), result.c_str());
	}
	return result;
}

long DownloadInfo::ParsePlatform(const wxString& text)
{
	for( int i=0; i<UpdatePlatform_MAX; ++i )
		if( s_Platforms[i].IsSameAs(text, false) )
			return i;
	return UpdatePlatform_None;
}

long DownloadInfo::ParseType(const wxString& text)
{
	for( int i=0; i<UpdateType_MAX; ++i )
		if( s_Types[i].IsSameAs(text, false) )
			return i;
	return UpdateType_None;
}

wxString DownloadInfo::ParsePlatform(long idx)
{
	if( idx >= 0 && idx < UpdatePlatform_MAX )
		return s_Platforms[idx];
	return s_Platforms[UpdatePlatform_None];
}

wxString DownloadInfo::ParseType(long idx)
{
	if( idx >= 0 && idx < UpdateType_MAX )
		return s_Types[idx];
	return s_Types[UpdateType_None];
}


// ----------------------------------------------------------------------------
// PackagesInfo
// ----------------------------------------------------------------------------
bool PackagesInfo::IsOK()
{
	if( m_fileFormat <= 0
	||  m_compatibleFormat <= 0
	||  m_updateVersion <= 0 )
		return false;

	// supports v1.0 format only
	if( m_compatibleFormat > 1 )
		return false;

	return true;
}

bool PackagesInfo::Parse(wxMemoryInputStream& inputStream, muThread_Updater* thread)
{
	m_thread = thread;

	/*wxTextInputStream textStream( inputStream );
	do
	{
		wxString line = textStream.ReadLine();
		wxLogVerbose(line);
	}
	while(textStream.GetChar() != 0);*/

	TiXmlDocument doc;
	doc.Parse(static_cast<char*>(inputStream.GetInputStreamBuffer()->GetBufferStart()));
	TiXmlHandle docHandle( doc.RootElement() );

	// Read format version
	ParseString(docHandle.FirstChild("format").FirstChild("revision").ToElement()).ToLong(&m_fileFormat);

	// Read format compatible
	ParseString(docHandle.FirstChild("format").FirstChild("compatible").ToElement()).ToLong(&m_compatibleFormat);

	// Read update version
	ParseString(docHandle.FirstChild("info").FirstChild("version").ToElement()).ToLong(&m_updateVersion);

	// Read update changelog
	m_changelog = ParseString(docHandle.FirstChild("info").FirstChild("changelog").ToElement());

	// Read packages
	for( TiXmlNode* node = docHandle.FirstChild("package").ToNode(); node; node=node->NextSibling("package") )
	{
		thread->PostProgress(wxT("Package"));
		DownloadInfo info;
		if( info.Parse(node, thread) )
		{
			m_downloads.push_back(info);
			thread->PostProgress(wxT("Package parsed."));
		}
		else
		{
			thread->PostProgress(wxT("Package FAILED!"));
		}
	}

	return true;
}

wxString PackagesInfo::ParseString(const TiXmlElement* elem)
{
	wxString result;
	if( elem )
	{
		result = wxString::FromUTF8(elem->GetText());
		m_thread->PostProgress(wxT("%s: %s"), wxString::FromUTF8(elem->Value()).c_str(), result.c_str());
	}
	return result;
}

DownloadInfo* PackagesInfo::FindBestUpdate( long platform, long currentVersion, long latestVersion )
{
	// Picks latest patch if compatible
	// Otherwise latest full installer
	// If no latest installer and latest patch incompatible, returns the patch

	if( m_updateVersion < latestVersion )
		return NULL;

	DownloadInfo* pick = NULL;
	for(size_t i=0; i<m_downloads.size(); ++i)
	{
		// if platform matches
		if( m_downloads[i].m_platform == platform )
		{
			if( pick )
			{
				// if patch
				if( m_downloads[i].m_type == UpdateType_Patch )
				{
					// if compatible, pick it always
					if( m_downloads[i].m_requires <= currentVersion )
						pick = &m_downloads[i];
					
					// otherwise it should be picked only if nothing else exists
					// pick already exists so do nothing
				}

				// else if installer
				else if( m_downloads[i].m_type == UpdateType_Installer )
				{
					// pick only if incompatible patch was picked
					if( pick->m_type == UpdateType_Patch 
					&&  pick->m_requires > currentVersion )
						pick = &m_downloads[i];
				}
			}
			else
			{
				pick = &m_downloads[i];
			}

		}
	}

	return pick;
}


// ============================================================================
//  EOF
// ============================================================================