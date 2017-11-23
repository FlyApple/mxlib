
#ifndef __MX_FILEVERSIONINFO_H__
#define __MX_FILEVERSIONINFO_H__


#pragma once

//
#include <winver.h>

//
#pragma comment( lib, "version.lib" )


//
namespace MXUtility
{
	//
	struct LANGANDCODEPAGE 
	{
		WORD wLanguage;
		WORD wCodePage;
	};

	//	
	class FileVersionInfo
	{
		typedef std::basic_string<TCHAR>		string;
	public:
		FileVersionInfo(string stringFullName)
		{
			DWORD dwHandle = 0;
			DWORD dwVerInfoSize = GetFileVersionInfoSize( stringFullName.c_str(), &dwHandle );
			if( dwVerInfoSize )
			{  
				BYTE* pVerInfoBuf = new BYTE[dwVerInfoSize];
				if(GetFileVersionInfo( stringFullName.c_str(), dwHandle, dwVerInfoSize, pVerInfoBuf ))
				{
					LANGID liLangID = GetUserDefaultLangID( );

					//
					LANGANDCODEPAGE*	pTranslation		= NULL;
					UINT				nTranslationLen		= 0;
					if( VerQueryValue( pVerInfoBuf, _T("\\VarFileInfo\\Translation"), (LPVOID*)&pTranslation, &nTranslationLen ) &&
						pTranslation != NULL && nTranslationLen > 0 )
					{
						LANGANDCODEPAGE*	pTranslationCur = NULL;
						for(UINT n = 0 ; n < nTranslationLen / sizeof(LANGANDCODEPAGE); n ++ )
						{ 
							if( pTranslation[n].wLanguage == (WORD)liLangID )
							{
								pTranslationCur = &pTranslation[n];
								break; 
							}
						}

						if(pTranslationCur == NULL)
						{
							pTranslationCur = &pTranslation[0];
						}

						TCHAR*	pszString = NULL;
						UINT	uiStringLen = 0;
						TCHAR	szBlock[MAX_PATH] = {0};

						wsprintf( szBlock, _T("\\StringFileInfo\\%04x%04x\\ProductName"), pTranslationCur->wLanguage, pTranslationCur->wCodePage );
						VerQueryValue( pVerInfoBuf, szBlock, (LPVOID*)&pszString, &uiStringLen );
						if( pszString && pszString[0] != '\0' )
						{ m_stringProductName = pszString; }

						wsprintf( szBlock, _T("\\StringFileInfo\\%04x%04x\\CompanyName"), pTranslationCur->wLanguage, pTranslationCur->wCodePage );
						VerQueryValue( pVerInfoBuf, szBlock, (LPVOID*)&pszString, &uiStringLen );
						if( pszString && pszString[0] != '\0' )
						{ m_stringCompanyName = pszString; }

						wsprintf( szBlock, _T("\\StringFileInfo\\%04x%04x\\LegalCopyright"), pTranslationCur->wLanguage, pTranslationCur->wCodePage );
						VerQueryValue( pVerInfoBuf, szBlock, (LPVOID*)&pszString, &uiStringLen );
						if( pszString && pszString[0] != '\0' )
						{ m_stringLegalCopyright = pszString; }

						wsprintf( szBlock, _T("\\StringFileInfo\\%04x%04x\\FileVersion"), pTranslationCur->wLanguage, pTranslationCur->wCodePage );
						VerQueryValue( pVerInfoBuf, szBlock, (LPVOID*)&pszString, &uiStringLen );
						if( pszString && pszString[0] != '\0' )
						{ m_stringFileVersion = pszString; }

						wsprintf( szBlock, _T("\\StringFileInfo\\%04x%04x\\ProductVersion"), pTranslationCur->wLanguage, pTranslationCur->wCodePage );
						VerQueryValue( pVerInfoBuf, szBlock, (LPVOID*)&pszString, &uiStringLen );
						if( pszString && pszString[0] != '\0' )
						{ m_stringProductVersion = pszString; }

						wsprintf( szBlock, _T("\\StringFileInfo\\%04x%04x\\FileDescription"), pTranslationCur->wLanguage, pTranslationCur->wCodePage );
						VerQueryValue( pVerInfoBuf, szBlock, (LPVOID*)&pszString, &uiStringLen );
						if( pszString && pszString[0] != '\0' )
						{ m_stringDescription = pszString; }
					}
				}

				//
				delete [] pVerInfoBuf;
				pVerInfoBuf = NULL;
			}
		}

		const string&	GetProductName( ) { return m_stringProductName; }
		const string&	GetCompanyName( ) { return m_stringCompanyName; }
		const string&	GetLegalCopyright( ) { return m_stringLegalCopyright; }
		const string&	GetFileVersion( ) { return m_stringFileVersion; }
		const string&	GetProductVersion( ) { return m_stringProductVersion; }
		const string&	GetDescription( ) { return m_stringDescription; }

	private:	
		string	m_stringProductName;
		string	m_stringCompanyName;
		string	m_stringLegalCopyright;
		string	m_stringFileVersion;
		string	m_stringProductVersion;
		string	m_stringDescription;

	};
}

//
#endif //?__MX_FILEVERSIONINFO_H__
