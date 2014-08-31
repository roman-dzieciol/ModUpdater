// ============================================================================
//  muPrecompile :: precompile header
// ============================================================================
#pragma once


// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include <wx/statline.h>
#include <wx/stattext.h>

#include <wx/snglinst.h>
#include <wx/thread.h>
#include <wx/filename.h>
#include <wx/arrstr.h>
#include <wx/stdpaths.h>
#include <wx/textfile.h>
#include <wx/cmdline.h>
#include <wx/msgdlg.h>

#include <wx/mstream.h>
#include <wx/txtstrm.h>
#include <wx/wfstream.h>
#include <wx/msw/registry.h>

#include <vector>

#include "MD5.h"

#include <curl/curl.h>

#define TIXML_USE_STL
#include <tinyxml.h>

// For compilers that support precompilation, includes "wx/wx.h".
#include <wx/wxprec.h>

// for all others, include the necessary headers (this file is usually all you
// need because it includes almost all "standard" wxWidgets headers)
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif




// ----------------------------------------------------------------------------
//  forward declarations
// ----------------------------------------------------------------------------

class muFrame;
class UpdaterEvent;
class muThread_Updater;
class TiXmlNode;



// ============================================================================
//  EOF
// ============================================================================