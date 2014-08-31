// ============================================================================
//  muThread_IntegrityChecker
// ============================================================================
#pragma once

#include "muThread.h"


// ============================================================================
//  muThread_IntegrityChecker
// ============================================================================
class muThread_IntegrityChecker : public muThread
{
public:
	muThread_IntegrityChecker(muFrame* frame);

	// thread execution starts here
	virtual void *Entry();

private:
	void LoadManifest();
	bool ParseComment(const wxString& line);
	bool ParseHash(const wxString& line);
	void VerifyManifest();

private:
	wxArrayString m_md5Array;
	wxArrayString m_pathArray;
	wxString m_md5Data;
	wxString m_md5Sum;
	unsigned long m_currentVer;
};



// ============================================================================
//  EOF
// ============================================================================