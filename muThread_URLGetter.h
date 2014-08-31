// ============================================================================
//  muThread_URLGetter
// ============================================================================
#pragma once

#include "muThread.h"


// ============================================================================
//  muThread_URLGetter
// ============================================================================
class muThread_URLGetter : public muThread
{
public:
	muThread_URLGetter(muFrame* frame);

	// thread execution starts here
	virtual void *Entry();

private:
	void GetInfoServerURL();
	void DownloadVersionInfo();

private:
	wxArrayString m_urlArray;
};



// ============================================================================
//  EOF
// ============================================================================