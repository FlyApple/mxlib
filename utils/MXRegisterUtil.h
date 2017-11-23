#ifndef __MX_REGISTER_UTIL_H__
#define __MX_REGISTER_UTIL_H__

#pragma once

//
#include <windows.h>

#include <tchar.h>

//
namespace MXUtility
{
	//
	class RegisterUtil
	{
	public:
		RegisterUtil(HKEY hKey = HKEY_CURRENT_USER);
		virtual ~RegisterUtil(void);

	public:
		BOOL	Open(LPCTSTR lpszKey, BOOL bCreate = FALSE);
		VOID	Close( );

	public:
		BOOL	Read(LPCTSTR lpszValue, LPVOID lpData, LPDWORD lpnLength, DWORD nType = REG_BINARY);
		BOOL	Read(LPCTSTR lpszValue, LPTSTR lpData, DWORD nLength, LPDWORD lpnOutLength = NULL);
		BOOL	Read(LPCTSTR lpszValue, DWORD& dwData);
		BOOL	Read(LPCTSTR lpszValue, DWORD64& dwData);

	private:
		HKEY			m_hRootKey;
		HKEY			m_hKey;

		TCHAR			m_szKeyText[200];
	};
}


#endif //__MX_REGISTER_UTIL_H__