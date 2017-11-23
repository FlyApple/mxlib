
//
#include "MXProcessEnum.h"
#include "MXPathName.h"
#include "MXFileVersionInfo.h"

//
namespace MXUtility
{
	//
	BOOL		ProcessInfo::Free( )
	{
		for (ModuleInfoList::iterator i = module_list.begin();
			i != module_list.end(); i ++)
		{
			if(*i)
			{
				ModuleInfo* mi = *i; *i = NULL;
				delete mi;
			}
		}

		module_list.clear();
		return TRUE;
	}

	//
	BOOL		ProcessInfo::Init( )
	{
		HANDLE	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |PROCESS_VM_READ, FALSE, id);
		if(hProcess == NULL){ return FALSE; }

		//
		GetProcessImageFileName(hProcess, this->file, MX_PROCESS_NAME_MAXLEN);
		_tcslwr_s(this->file);	//全部轉換為小寫

		//
		PathName<TCHAR>		pn(this->file, true);
		_tcscpy_s(this->name, pn.GetFileName().c_str());
		_tcscpy_s(this->file, pn.GetFullName().c_str());

		//
		FileVersionInfo		fvi(this->file);
		_tcscpy_s(this->company, fvi.GetCompanyName().c_str());
		_tcscpy_s(this->desc, fvi.GetDescription().c_str());

		// Windows Server 2003 and Windows XP:  The handle must have the PROCESS_QUERY_INFORMATION access right.
		GetProcessHandleCount(hProcess, &this->handle_count);

		// Windows Server 2003 and Windows XP:  The handle must have the PROCESS_QUERY_INFORMATION access right.
		this->priority	= GetPriorityClass(hProcess); //獲取進程優先級


		SYSTEM_INFO	si = {0};
		GetNativeSystemInfo(&si);
		if( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL )
		{
			this->platform	= ProcessPlatform_x32;
		}
		else if( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
			si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 )
		{
			BOOL	bWow64Process	= FALSE;
			IsWow64Process(hProcess, &bWow64Process);
			this->platform	= bWow64Process ? ProcessPlatform_x32 : ProcessPlatform_x64;

#ifdef _WIN64
#else
			// 如果自身是32位,將忽略64位進程
			if(this->platform == ProcessPlatform_x64)
			{
				CloseHandle(hProcess);
				return TRUE;
			}
#endif
		}

		//
		DWORD	dwBytesReturned						= 0;
		HMODULE	hModules[MX_PROCESS_MODULE_MAXNUM]	= {0};
		if(EnumProcessModules(hProcess, hModules, sizeof(hModules), &dwBytesReturned))
		{
			//
			LONG	nCount	= (LONG)(dwBytesReturned / sizeof(HMODULE));
			for (LONG i = 0; i < nCount; i ++)
			{
				ModuleInfo*		mi = new ModuleInfo( );
				mi->module		= hModules[i];

				GetModuleFileNameEx(hProcess, mi->module, mi->file, MX_PROCESS_NAME_MAXLEN);
				_tcslwr_s(mi->file);	//全部轉換為小寫

				GetModuleBaseName(hProcess, mi->module, mi->name, MX_PROCESS_NAME_MAXLEN);
				_tcslwr_s(mi->name);	//全部轉換為小寫

				FileVersionInfo		tfvi(mi->file);
				_tcscpy_s(mi->company, tfvi.GetCompanyName().c_str());
				_tcscpy_s(mi->desc, tfvi.GetDescription().c_str());

				MODULEINFO		m = {0};
				GetModuleInformation(hProcess, mi->module, &m, sizeof(MODULEINFO));
				mi->image_size	= m.SizeOfImage;
				mi->entry_point	= m.EntryPoint;

				this->module_list.push_back(mi);
			}
		}

		//
		CloseHandle(hProcess);
		return TRUE;
	}

	//
	BOOL	ProcessInfo::Modules(std::vector<const ModuleInfo*>& modules) const
	{
		modules.resize(this->ModuleCount());

		int	n = 0;
		for (ModuleInfoList::const_iterator i = module_list.begin();
			i != module_list.end(); i ++, n ++)
		{
			modules[n] = *i;
		}
		return TRUE;
	}

	//
	const ModuleInfo*	ProcessInfo::LookupModule(LPCTSTR lpszName)
	{
		std::tstring name = lpszName;
		std::transform(name.begin(), name.end(), name.begin(), tolower);

		for (ModuleInfoList::const_iterator i = module_list.begin();
			i != module_list.end(); i ++)
		{
			if(*i)
			{
				std::tstring temp = (*i)->name;
				if((LONG)temp.find(name) >= 0)
				{
					return *i;
				}
			}
		}
		return NULL;
	}

	//
	const ModuleInfo*	ProcessInfo::LookupModuleFile(LPCTSTR lpszName)
	{
		std::tstring name = lpszName;
		std::transform(name.begin(), name.end(), name.begin(), tolower);

		for (ModuleInfoList::const_iterator i = module_list.begin();
			i != module_list.end(); i ++)
		{
			if(*i)
			{
				std::tstring temp = (*i)->file;
				if((LONG)temp.find(name) >= 0)
				{
					return *i;
				}
			}
		}
		return NULL;
	}


	//
	ProcessEnum::ProcessEnum(void)
	{
	}

	ProcessEnum::~ProcessEnum(void)
	{
		this->Clear();
	}

	//
	BOOL		ProcessEnum::Clear( )
	{
		m_ProcessIdMap.clear();

		for (ProcessInfoList::iterator i = m_ProcessInfoList.begin();
			i != m_ProcessInfoList.end(); i ++)
		{
			if(*i)
			{
				ProcessInfo* pi = *i; *i = NULL;
				delete pi;
			}
		}

		m_ProcessInfoList.clear();
		return TRUE;
	}


	//
	BOOL		ProcessEnum::Start( )
	{
		//
		this->Clear();

		//
		DWORD	dwBytesReturned						= 0;
		DWORD	dwProcessIds[MX_PROCESS_ID_MAXNUM]	= {0};
		if(!::EnumProcesses(dwProcessIds, sizeof(dwProcessIds), &dwBytesReturned))
		{
			return FALSE;
		}

		//
		LONG	nCount	= (LONG)(dwBytesReturned / sizeof(DWORD));

		//
		for (LONG i = 0; i < nCount; i ++)
		{
			ProcessInfo*	pi	= new ProcessInfo( );
			if(pi)
			{
				pi->id			= dwProcessIds[i];
				if(pi->Init())
				{
					pi->access	= TRUE;
				}

				//
				m_ProcessInfoList.push_back(pi);
				m_ProcessIdMap[pi->id]	= pi;
			}
		}

		//
		return TRUE;
	}

	//
	BOOL	ProcessEnum::Processes(std::vector<const ProcessInfo*>& processes) const
	{
		processes.resize(this->Count());

		int n = 0;
		for (ProcessInfoList::const_iterator i = m_ProcessInfoList.begin();
			i != m_ProcessInfoList.end(); i ++, n ++)
		{
			processes[n] = *i;
		}
		return TRUE;
	}

	//
	const ProcessInfo*	ProcessEnum::Lookup(LPCTSTR lpszName)
	{
		std::tstring name = lpszName;
		std::transform(name.begin(), name.end(), name.begin(), tolower);

		for (ProcessInfoList::const_iterator i = m_ProcessInfoList.begin();
			i != m_ProcessInfoList.end(); i ++)
		{
			if(*i)
			{
				std::tstring temp = (*i)->name;
				if((LONG)temp.find(name) >= 0)
				{
					return *i;
				}
			}
		}
		return NULL;
	}

	//
	const ProcessInfo*	ProcessEnum::LookupFile(LPCTSTR lpszName)
	{
		std::tstring name = lpszName;
		std::transform(name.begin(), name.end(), name.begin(), tolower);

		for (ProcessInfoList::const_iterator i = m_ProcessInfoList.begin();
			i != m_ProcessInfoList.end(); i ++)
		{
			if(*i)
			{
				std::tstring temp = (*i)->file;
				if((LONG)temp.find(name) >= 0)
				{
					return *i;
				}
			}
		}
		return NULL;
	}

	//
	const ProcessInfo*	ProcessEnum::LookupModule(LPCTSTR lpszName)
	{
		for (ProcessInfoList::const_iterator i = m_ProcessInfoList.begin();
			i != m_ProcessInfoList.end(); i ++)
		{
			if(*i && (*i)->LookupModule(lpszName))
			{
				return *i;
			}
		}
		return NULL;
	}

	//
	const ProcessInfo*	ProcessEnum::LookupModuleFile(LPCTSTR lpszName)
	{
		for (ProcessInfoList::const_iterator i = m_ProcessInfoList.begin();
			i != m_ProcessInfoList.end(); i ++)
		{
			if(*i && (*i)->LookupModuleFile(lpszName))
			{
				return *i;
			}
		}
		return NULL;
	}
}


