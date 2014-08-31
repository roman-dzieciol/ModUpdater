// ============================================================================
//  PackagesInfo
// ============================================================================
#pragma once


wxString muParseText(const TiXmlElement* elem);

// ----------------------------------------------------------------------------
// DownloadInfo
// ----------------------------------------------------------------------------

enum {
	UpdatePlatform_None = 0,
	UpdatePlatform_Win,
	UpdatePlatform_Mac,
	UpdatePlatform_Linux,
	UpdatePlatform_Any,
	UpdatePlatform_MAX
};

enum {
	UpdateType_None = 0,
	UpdateType_Installer,
	UpdateType_Patch,
	UpdateType_Archive,
	UpdateType_MAX
};

class DownloadInfo
{
public:
	DownloadInfo()
		: m_platform(UpdatePlatform_None)
		, m_type(UpdateType_None)
		, m_requires(0)
		, m_thread(0)
	{

	}

public:
	bool Parse(TiXmlNode* node, muThread_Updater* thread);
	bool IsOK();

public:
	long ParsePlatform(const wxString& text);
	long ParseType(const wxString& text);
	wxString ParsePlatform(long idx);
	wxString ParseType(long idx);
	wxString ParseString(const TiXmlElement* elem);

protected:
	static const wxString s_Platforms[UpdatePlatform_MAX];
	static const wxString s_Types[UpdateType_MAX];

public:
	wxString m_updateName;
	wxString m_fileName;
	std::vector<wxString> m_URLS;
	long m_platform;
	long m_type;
	long m_requires;
	wxString m_md5;
	muThread_Updater* m_thread;
};


// ----------------------------------------------------------------------------
// PackagesInfo
// ----------------------------------------------------------------------------
class PackagesInfo
{
public:
	PackagesInfo()
	: m_fileFormat(0)
	, m_compatibleFormat(0)
	, m_updateVersion(0)
	, m_thread(0)
	{
		m_downloads.reserve(8);
	}

	bool Parse(wxMemoryInputStream& inputStream, muThread_Updater* thread);
	bool IsOK();
	DownloadInfo* FindBestUpdate( long platform, long currentVersion, long latestVersion );
	wxString ParseString(const TiXmlElement* elem);

public:
	long m_fileFormat;
	long m_compatibleFormat;
	long m_updateVersion;
	wxString m_changelog;
	std::vector<DownloadInfo> m_downloads;
	muThread_Updater* m_thread;
};


// ============================================================================
//  EOF
// ============================================================================