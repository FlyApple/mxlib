#include "MXRegisterUtil.h"


//
namespace MXUtility
{
	//
	RegisterUtil::RegisterUtil(HKEY hKey)
		:	m_hRootKey(hKey)
	{
		m_hKey = NULL;
	}

	RegisterUtil::~RegisterUtil(void)
	{
		this->Close();
	}

	VOID	RegisterUtil::Close( )
	{
		if(m_hKey)
		{
			RegCloseKey(m_hKey);
			m_hKey = NULL;
		}

		memset(m_szKeyText, 0, sizeof(TCHAR) * 200);
	}

	BOOL	RegisterUtil::Open(LPCTSTR lpszKey, BOOL bCreate)
	{
		if(m_hKey){ return FALSE; }

		if(bCreate)
		{
			if(RegCreateKeyEx(m_hRootKey, lpszKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ| KEY_WRITE, NULL, &m_hKey, NULL) != ERROR_SUCCESS)
			{
				return FALSE;
			}
		}
		else
		{
			if(RegOpenKeyEx(m_hRootKey, lpszKey, REG_OPTION_NON_VOLATILE, KEY_READ| KEY_WRITE, &m_hKey) != ERROR_SUCCESS)
			{
				return FALSE;
			}
		}

		_tcscpy_s(m_szKeyText, 200, lpszKey);
		return TRUE;
	}

	BOOL	RegisterUtil::Read(LPCTSTR lpszValue, LPVOID lpData, LPDWORD lpnLength, DWORD dwType)
	{
		if(!m_hKey){ return FALSE; }

		if(RegQueryValueEx(m_hKey, lpszValue, NULL, &dwType, (LPBYTE)lpData, lpnLength) != ERROR_SUCCESS)
		{
			return FALSE;
		}
		return TRUE;
	}

	BOOL	RegisterUtil::Read(LPCTSTR lpszValue, LPTSTR lpData, DWORD nLength, LPDWORD lpnOutLength)
	{
		if(lpnOutLength){ *lpnOutLength = nLength * sizeof(TCHAR); }
		if(!this->Read(lpszValue, lpData, lpnOutLength, REG_SZ))
		{
			if(lpnOutLength){ *lpnOutLength = 0; }
			return FALSE;
		}

		if(lpnOutLength){ *lpnOutLength = (*lpnOutLength) / sizeof(TCHAR); }
		return TRUE;
	}

	BOOL	RegisterUtil::Read(LPCTSTR lpszValue, DWORD& dwData)
	{
		DWORD	nLength = sizeof(DWORD);
		return this->Read(lpszValue, &dwData, &nLength, REG_DWORD);
	}

	BOOL	RegisterUtil::Read(LPCTSTR lpszValue, DWORD64& dwData)
	{
		DWORD	nLength = sizeof(DWORD);
		return this->Read(lpszValue, &dwData, &nLength, REG_QWORD);
	}
}


