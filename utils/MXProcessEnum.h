#ifndef __MX_PROCESSENUM_H__
#define __MX_PROCESSENUM_H__


#pragma once

//
#include <windows.h>
#include <psapi.h>

//
#include <tchar.h>

//
#include <algorithm>
#include <vector>
#include <list>
#include <map>

//
#pragma comment(lib, "psapi.lib")

//
#include "MXOSVersion.h"


//
namespace std
{
#ifndef tstring
#ifdef _UNICODE
#define tstring		wstring
#else
#define tstring		string
#endif
#endif
}

//
namespace MXUtility
{
	//
	#define MX_PROCESS_ID_MAXNUM			1024
	#define MX_PROCESS_MODULE_MAXNUM		1024
	#define MX_PROCESS_NAME_MAXLEN			200

	enum ProcessPlatform
	{
		ProcessPlatform_x32,
		ProcessPlatform_x64,
	};

	//
	class ProcessInfo;
	typedef std::list<ProcessInfo*>			ProcessInfoList;
	typedef std::map<DWORD, ProcessInfo*>	ProcessIdMap;

	//
	class ModuleInfo;
	typedef std::list<ModuleInfo*>			ModuleInfoList;

	//
	class ModuleInfo
	{
	public: 
		ModuleInfo( )
		{
			module		= NULL;
			ZeroMemory(name, sizeof(TCHAR) * MX_PROCESS_NAME_MAXLEN);
			ZeroMemory(file, sizeof(TCHAR) * MX_PROCESS_NAME_MAXLEN);
			ZeroMemory(desc, sizeof(TCHAR) * MX_PROCESS_NAME_MAXLEN);
			ZeroMemory(company, sizeof(TCHAR) * MX_PROCESS_NAME_MAXLEN);
			image_size	= 0;
			entry_point	= NULL;
		}

	public:
		HMODULE		module;
		TCHAR		name[MX_PROCESS_NAME_MAXLEN];
		TCHAR		file[MX_PROCESS_NAME_MAXLEN];
		TCHAR		company[MX_PROCESS_NAME_MAXLEN];
		TCHAR		desc[MX_PROCESS_NAME_MAXLEN];
		DWORD		image_size;
		LPVOID		entry_point;
	};

	//
	class ProcessInfo
	{
	public:
		ProcessInfo( )
		{
			id			= 0;
			platform	= ProcessPlatform_x32;
			access		= 0;

			ZeroMemory(name, sizeof(TCHAR) * MX_PROCESS_NAME_MAXLEN);
			ZeroMemory(file, sizeof(TCHAR) * MX_PROCESS_NAME_MAXLEN);

			priority	= 0;
			handle_count= 0;

			ZeroMemory(company, sizeof(TCHAR) * MX_PROCESS_NAME_MAXLEN);
			ZeroMemory(desc, sizeof(TCHAR) * MX_PROCESS_NAME_MAXLEN);
		}

		~ProcessInfo( )
		{
			this->Free( );
		}

		BOOL		Init( );
		BOOL		Free( );

		LONG		ModuleCount( ) const { return (LONG)module_list.size(); }
		BOOL		Modules(std::vector<const ModuleInfo*>& modules) const ;

		const ModuleInfo*	LookupModule(LPCTSTR lpszName);
		const ModuleInfo*	LookupModuleFile(LPCTSTR lpszName);

	public:
		DWORD		id;
		ULONG		platform;
		BOOL		access;
		TCHAR		name[MX_PROCESS_NAME_MAXLEN];
		TCHAR		file[MX_PROCESS_NAME_MAXLEN];
		DWORD		priority;
		DWORD		handle_count;

		TCHAR		company[MX_PROCESS_NAME_MAXLEN];
		TCHAR		desc[MX_PROCESS_NAME_MAXLEN];

	public:
		ModuleInfoList	module_list;
	};

	//
	class ProcessEnum
	{
	public:
		ProcessEnum(void);
		~ProcessEnum(void);

	public:
		BOOL	Clear( );
		BOOL	Start( );

		LONG	Count( ) const { return (LONG)m_ProcessInfoList.size(); }
		BOOL	Processes(std::vector<const ProcessInfo*>& processes) const ;


		const ProcessInfo*	Lookup(LPCTSTR lpszName);
		const ProcessInfo*	LookupFile(LPCTSTR lpszName);
		const ProcessInfo*	LookupModule(LPCTSTR lpszName);
		const ProcessInfo*	LookupModuleFile(LPCTSTR lpszName);
	private:
		ProcessInfoList		m_ProcessInfoList;
		ProcessIdMap		m_ProcessIdMap;
	};

	//
	class ProcessPrivilege
	{
	public:
		ProcessPrivilege( )	:	m_hToken (NULL)
		{
			if(OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES| TOKEN_QUERY| TOKEN_READ, &m_hToken))
			{

			}
		}
		~ProcessPrivilege( )
		{
			if(m_hToken){ CloseHandle(m_hToken); }
		}

	public:
		BOOL	HasWow64( )
		{
#ifdef _WIN64
			return FALSE;
#else
			return TRUE;
#endif
		}

		BOOL	HasAdmin( )
		{
			// VISTA之前是不存在管理員權限
			OSVersion		osv;
			if(osv.code < OS_WinVista)
			{
				return TRUE;
			}

			DWORD			br		= 0;
			TOKEN_ELEVATION	te		= {0};
			if(!GetTokenInformation(m_hToken, TokenElevation, &te, sizeof(TOKEN_ELEVATION), &br))
			{
				return FALSE;
			}

			if(br == 0 || te.TokenIsElevated == 0)
			{
				return FALSE;
			}
			return TRUE;
		}

		BOOL	Privilege(LPCTSTR lpszName = SE_DEBUG_NAME, DWORD nValue = SE_PRIVILEGE_ENABLED)
		{
			LUID	luid	= {0};
			if(!LookupValue(luid, lpszName))
			{
				return FALSE;
			}

			if(!AdjustValue(luid, nValue))
			{
				return FALSE;
			}
			return TRUE;
		}

	private:
		BOOL	LookupValue(LUID& luid, LPCTSTR lpszName = SE_DEBUG_NAME)
		{
			if(m_hToken == NULL){ return FALSE; }

			ZeroMemory(&luid, sizeof(LUID));
			if(!LookupPrivilegeValue(NULL, lpszName, &luid))
			{
				return FALSE;
			}
			return TRUE;
		}

		BOOL	AdjustValue(LUID& luid, DWORD nValue = SE_PRIVILEGE_ENABLED)
		{
			TOKEN_PRIVILEGES	tp	= {0};
			tp.PrivilegeCount		= 1;
			tp.Privileges[0].Luid	= luid;
			tp.Privileges[0].Attributes	= nValue;
			if(!AdjustTokenPrivileges(m_hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
			{
				return FALSE;
			}
			return TRUE;
		}
	private:
		HANDLE	m_hToken;
	};
};


//
#endif //__MX_PROCESSENUM_H__