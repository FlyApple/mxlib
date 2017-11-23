#ifndef __MX_WINDOWENUM_H__
#define __MX_WINDOWENUM_H__


#pragma once

//
#include <windows.h>

//
#include <tchar.h>
#include <map>
#include <list>
#include <string>

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
	#define MX_WINDOW_TITLE_MAXLEN	100
	
	//
	class WindowNode;
	class WindowEnum;

	//
	typedef void	(__stdcall *WindowEnumProc)(const WindowNode* node);
	typedef std::list<WindowNode*>				WindowNodeList;
	typedef std::map<HWND, const WindowNode*>	WindowHandleMap;

	//
	class WindowNode
	{
		friend BOOL CALLBACK EnumChildWindowsProc(_In_ HWND hWnd, _In_ LPARAM lParam);
	public:
		WindowNode( )	: we (NULL), parent_node (NULL)
		{
			hwnd		= NULL;
			ZeroMemory(title_name, MX_WINDOW_TITLE_MAXLEN * sizeof(TCHAR));
			ZeroMemory(class_name, MX_WINDOW_TITLE_MAXLEN * sizeof(TCHAR));
		}
		~WindowNode( )
		{
			for (WindowNodeList::iterator i = child_node_list.begin();
				i != child_node_list.end(); i ++)
			{
				if(*i)
				{
					WindowNode* node = *i; *i = NULL;
					delete node;
				}
			}

			child_node_list.clear();
		}

		LONG	ChildNodeCount( ) const { return (LONG)child_node_list.size(); }

	public:
		HWND	hwnd;
		TCHAR	title_name[MX_WINDOW_TITLE_MAXLEN];
		TCHAR	class_name[MX_WINDOW_TITLE_MAXLEN];

	public:
		WindowEnum*				we;
		WindowNode*				parent_node;
		WindowNodeList			child_node_list;
	};

	//
	class WindowEnum
	{
		friend BOOL CALLBACK EnumWindowsProc(_In_ HWND hWnd, _In_ LPARAM lParam);
		friend BOOL CALLBACK EnumChildWindowsProc(_In_ HWND hWnd, _In_ LPARAM lParam);
	public:
		WindowEnum( )
		{
			m_WindowEnumProc	= NULL;
		}
		virtual ~WindowEnum( )
		{
			this->Clear();
		}

		void	operator += (WindowEnumProc fn);

	public:
		BOOL	Clear( );
		BOOL	Start( );

		const WindowNode*	Lookup(LPCTSTR lpszTitle);

		LONG	Count( ){ return (LONG)m_WindowNodeList.size(); }
		LONG	HandleCount( ){ return (LONG)m_WindowHandleMap.size(); }

	protected:
		void	CallbackWindowNodeProc(const WindowNode* node);

	private:
		WindowEnumProc			m_WindowEnumProc;
		WindowNodeList			m_WindowNodeList;
		WindowHandleMap			m_WindowHandleMap;
	};

	//
	__inline void	WindowEnum::operator += (WindowEnumProc fn)
	{
		m_WindowEnumProc	= fn;
	}

	__inline void	WindowEnum::CallbackWindowNodeProc(const WindowNode* node)
	{
		if(node)
		{
			m_WindowHandleMap[node->hwnd]	= node;
		}

		if(m_WindowEnumProc)
		{
			m_WindowEnumProc(node);
		}
	}
}



//
#endif //__MX_WINDOWENUM_H__