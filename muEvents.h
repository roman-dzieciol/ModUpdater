// ============================================================================
//  UpdaterEvent
// ============================================================================
#pragma once

DECLARE_EVENT_TYPE( UpdaterCommandEvent, -1 )

// An custom event that transports a whole wxString.
class UpdaterEvent: public wxCommandEvent
{
public:
	UpdaterEvent( wxEventType commandType = UpdaterCommandEvent, int id = 0 )
		:  wxCommandEvent(commandType, id) 
	{ 
	}

	// You *must* copy here the data to be transported
	UpdaterEvent( const UpdaterEvent &event )
		:  wxCommandEvent(event)
	{ 
	}

	// Required for sending with wxPostEvent()
	wxEvent* Clone() const { return new UpdaterEvent(*this); }
};

typedef void (wxEvtHandler::*UpdaterEventFunction)(UpdaterEvent &);

// This #define simplifies the one below, and makes the syntax less
// ugly if you want to use Connect() instead of an event table.
#define UpdaterEventHandler(func) \
	(wxObjectEventFunction)(wxEventFunction)(wxCommandEventFunction) wxStaticCastEvent(UpdaterEventFunction, &func)

// Define the event table entry. Yes, it really *does* end in a comma.
#define EVT_UPDATER(id, fn) \
	DECLARE_EVENT_TABLE_ENTRY( UpdaterCommandEvent, id, wxID_ANY, (wxObjectEventFunction)(wxEventFunction) (wxCommandEventFunction) wxStaticCastEvent( UpdaterEventFunction, &fn ), (wxObject*) NULL ),

	// Optionally, you can do a similar #define for EVT_MYFOO_RANGE.
#define EVT_MYFOO_RANGE(id1,id2, fn) \
	DECLARE_EVENT_TABLE_ENTRY( UpdaterCommandEvent, id1, id2, UpdaterEventHandler(fn), (wxObject*) NULL ),

// If you want to use the custom event to send more than one sort
// of data, or to more than one place, make it easier by providing
// named IDs in an enumeration.
enum { 
	Updater_Unknown = 1,
	Updater_OnLog,
	Updater_OnUpdateDownload,
	Updater_OnUpdateInstall,
	Updater_OnCurrentVersion,
	Updater_OnLatestVersion,
	Updater_OnIntegrityProgress,
	Updater_OnIntegrityDone,
	Updater_OnURLGetterProgress,
	Updater_OnURLGetterDone,
	Updater_OnUpdatingProgress,
	Updater_OnUpdatingDone
};

enum {
	UpdaterLog_Error,
	UpdaterLog_Warning,
	UpdaterLog_Message,
	UpdaterLog_Verbose
};

enum {
	Updater_None,
	Updater_Done,
	Updater_Fail
};

#define G_IMPLEMENT_LOGEVENT(name) \
inline void guPost##name(wxEvtHandler* dest, const wxString& text) \
{ \
	UpdaterEvent event( UpdaterCommandEvent, Updater_OnLog ); \
	event.SetString(text); \
	event.SetInt(UpdaterLog_##name); \
	wxPostEvent( dest, event ); \
}

G_IMPLEMENT_LOGEVENT(Error)
G_IMPLEMENT_LOGEVENT(Warning)
G_IMPLEMENT_LOGEVENT(Message)
G_IMPLEMENT_LOGEVENT(Verbose)


// ============================================================================
//  EOF
// ============================================================================