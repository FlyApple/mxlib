
#ifndef __MX_OSVERSION_H__
#define __MX_OSVERSION_H__

#pragma once

//
namespace MXUtility
{
	//
	enum OSVersionCode
	{
		OS_Unknow			= 0x0000,
		OS_Win2000			= 0x5000,
		OS_WinXP			= 0x5100,
		OS_Win2003			= 0x5200,
		OS_Win2003_R2		= 0x5210,
		OS_WinVista			= 0x6000,
		OS_Win2008			= 0x6010,
		OS_Win7				= 0x6100,
		OS_Win2008_R2		= 0x6110,
		OS_Win8				= 0x6200,
		OS_Win2012			= 0x6210,
		OS_Win8_1			= 0x6300,
		OS_Win2012_R2		= 0x6310,
	};

	//
	class OSVersion
	{
	public:
		OSVersion(void)	: code (OS_Unknow)
		{
			major		= 0;
			minor		= 0;
			build		= 0;
			id			= VER_PLATFORM_WIN32_NT;

			ZeroMemory(desc, sizeof(TCHAR) * 100);
			ZeroMemory(suite, sizeof(TCHAR) * 50);
			ZeroMemory(csd, sizeof(TCHAR) * 100);

			OSVERSIONINFOEX	vi = {0};
			vi.dwOSVersionInfoSize	= sizeof(OSVERSIONINFOEX);
			if(GetVersionEx((OSVERSIONINFO*)&vi))
			{
				major	= vi.dwMajorVersion;
				minor	= vi.dwMinorVersion;
				build	= vi.dwBuildNumber;
				id		= vi.dwPlatformId;
				_tcscpy_s(csd, vi.szCSDVersion);

				if(vi.dwMajorVersion == 5 && vi.dwMinorVersion == 0)
				{
					code		= OS_Win2000;
					wsprintf(desc, _T("Windows 2000"));
				}
				else if(vi.dwMajorVersion == 5 && vi.dwMinorVersion == 1)
				{
					code		= OS_WinXP;
					wsprintf(desc, _T("Windows XP"));
				}
				else if(vi.dwMajorVersion == 5 && vi.dwMinorVersion == 2)
				{
					code		= OS_Win2003;
					wsprintf(desc, _T("Windows Server 2003"));
					if(GetSystemMetrics(SM_SERVERR2))
					{
						code	= OS_Win2003_R2;
						wsprintf(desc, _T("Windows Server 2003 R2"));
					}
				}
				else if(vi.dwMajorVersion == 6 && vi.dwMinorVersion == 0)
				{
					code		= OS_WinVista;
					wsprintf(desc, _T("Windows Vista"));
					if(vi.wProductType == VER_NT_SERVER)
					{
						code	= OS_Win2008;
						wsprintf(desc, _T("Windows Server 2008"));
					}
				}
				else if(vi.dwMajorVersion == 6 && vi.dwMinorVersion == 1)
				{
					code		= OS_Win7;
					wsprintf(desc, _T("Windows 7"));
					if(vi.wProductType == VER_NT_SERVER)
					{
						code	= OS_Win2008_R2;
						wsprintf(desc, _T("Windows Server 2008 R2"));
					}
				}
				else if(vi.dwMajorVersion == 6 && vi.dwMinorVersion == 2)
				{
					code		= OS_Win8;
					wsprintf(desc, _T("Windows 8"));
					if(vi.wProductType == VER_NT_SERVER)
					{
						code	= OS_Win2012;
						wsprintf(desc, _T("Windows Server 2012"));
					}
				}
				else if(vi.dwMajorVersion == 6 && vi.dwMinorVersion == 3)
				{
					code		= OS_Win8_1;
					wsprintf(desc, _T("Windows 8.1"));
					if(vi.wProductType == VER_NT_SERVER)
					{
						code	= OS_Win2012_R2;
						wsprintf(desc, _T("Windows Server 2012 R2"));
					}
				}
				else
				{
					code		= OS_Unknow;
					wsprintf(desc, _T("Unknow"));
				}

				if(vi.wSuiteMask & VER_SUITE_DATACENTER)
				{
					wsprintf(suite, _T("Datacenter"));
				}
				else if(vi.wSuiteMask & VER_SUITE_ENTERPRISE)
				{
					wsprintf(suite, _T("Enterprise"));
				}
				else if(vi.wSuiteMask & VER_SUITE_SINGLEUSERTS)
				{
					wsprintf(suite, _T(""));
				}
				else if(vi.wSuiteMask & VER_SUITE_EMBEDDEDNT)
				{
					wsprintf(suite, _T("Embedded"));
				}
				else if(vi.wSuiteMask & VER_SUITE_PERSONAL)
				{
					wsprintf(suite, _T("Home [Premium|Edition|Basic]"));
				}
				else if(vi.wSuiteMask & VER_SUITE_STORAGE_SERVER)
				{
					wsprintf(suite, _T("Storage Server"));
				}
				else if(vi.wSuiteMask & VER_SUITE_SMALLBUSINESS)
				{
					wsprintf(suite, _T("Small Business Server"));
				}
				else
				{
					wsprintf(suite, _T(""));
				}
			}
		}

	public:
		OSVersionCode		code;
		DWORD				major;
		DWORD				minor;
		DWORD				build;
		DWORD				id;
		TCHAR				csd[100];
		TCHAR				desc[100];
		TCHAR				suite[50];
	};
}



//
#endif //__MX_OSVERSION_H__