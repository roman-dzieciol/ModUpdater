// ============================================================================
//  muException
// ============================================================================
#pragma once


class muException
{
public: 
	muException( const wxChar *pszFormat, ... )
	{
		va_list argptr;
		va_start(argptr, pszFormat);
		m_errorMsg.PrintfV(pszFormat, argptr);
		va_end(argptr);
	}

	muException( const muException& rhs ) : m_errorMsg(rhs.m_errorMsg)
	{
	}

	virtual ~muException()
	{
	}

	muException& operator=( const muException& rhs )
	{
		m_errorMsg = rhs.m_errorMsg; 
		return *this; 
	}

	wxString What() const
	{
		return m_errorMsg;
	}

private:
	wxString m_errorMsg;
};


// ============================================================================
//  EOF
// ============================================================================