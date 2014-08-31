// ============================================================================
//  muEvents
// ============================================================================

#include "muPrecompile.h"
#include "muEvents.h"


DEFINE_EVENT_TYPE( UpdaterCommandEvent );


// Usage:
#if 0
// To Send the Event

...
UpdaterEvent event( UpdaterCommandEvent, Foo_DoFirstThing );
wxString bar( wxT("This is a Foo_DoFirstThing event") );
// Add the exciting data. You can put anything you like
// into the class: ints, structs, binary data...
event.SetText( bar );
wxPostEvent( panel, event );
...

// To receive the event, either use an event table like so:

BEGIN_EVENT_TABLE( MyDestination, wxDestination )
EVT_UPDATER( wxID_ANY, MyDestination::DoSomething )
// or EVT_MYFOO_RANGE( Foo_DoFirstThing, Foo_DoThirdThing, MyDestination::DoSomething )
// or EVT_UPDATER( Foo_DoFirstThing, MyDestination::DoFirstThing)
// or EVT_UPDATER( Foo_DoSecondThing, MyDestination::DoSecondThing)
// or EVT_UPDATER( Foo_DoThirdThing, MyDestination::DoThirdThing)
END_EVENT_TABLE()

// Or use Connect(). You'd probably do this in the MyDestination constructor.
Connect( wxID_ANY, UpdaterCommandEvent,
		UpdaterEventHandler(MyDestination::DoSomething), NULL, this );

// To handle the event:

void MyDestination::DoSomething( UpdaterEvent &event )
{
	switch( event.GetId() )
	{
	case Foo_DoFirstThing:
		wxLogDebug( event.GetText() ); break;
	case Foo_DoSecondThing:
		/* Do something different */ break;
		//  ...
	}
}
#endif

// ============================================================================
//  EOF
// ============================================================================