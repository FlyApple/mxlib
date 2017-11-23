
#include "MXWindowEnum.h"



//
namespace MXUtility
{

	//
	BOOL CALLBACK EnumChildWindowsProc(_In_ HWND hWnd, _In_ LPARAM lParam)
	{
		WindowNode*	node = reinterpret_cast<WindowNode*>(lParam);
		if(node)
		{
			WindowNode* child_node = new WindowNode( );
			if(child_node)
			{
				child_node->we			= node->we;
				child_node->hwnd		= hWnd;
				child_node->parent_node	= node;

				GetWindowText(hWnd, child_node->title_name, MX_WINDOW_TITLE_MAXLEN);
				GetClassName(hWnd, child_node->class_name, MX_WINDOW_TITLE_MAXLEN);

				node->child_node_list.push_back(child_node);

				if(child_node->we)
				{
					child_node->we->CallbackWindowNodeProc(child_node);
				}

				// 繼續枚舉子窗口
				if(::EnumChildWindows(child_node->hwnd, EnumChildWindowsProc, (LPARAM)child_node))
				{
					//nothing
				}
			}

		}

		//To continue enumeration, the callback function must return TRUE; to stop enumeration, it must return FALSE. 
		return TRUE;
	}

	//
	BOOL CALLBACK EnumWindowsProc(_In_ HWND hWnd, _In_ LPARAM lParam)
	{
		WindowEnum* we = reinterpret_cast<WindowEnum*>(lParam);
		if(we)
		{
			WindowNode* node = new WindowNode( );
			if(node)
			{
				node->we	= we;
				node->hwnd	= hWnd;

				GetWindowText(hWnd, node->title_name, MX_WINDOW_TITLE_MAXLEN);
				GetClassName(hWnd, node->class_name, MX_WINDOW_TITLE_MAXLEN);

				we->m_WindowNodeList.push_back(node);

				if(node->we)
				{
					node->we->CallbackWindowNodeProc(node);
				}


				// 枚舉子窗口
				if(::EnumChildWindows(node->hwnd, EnumChildWindowsProc, (LPARAM)node))
				{
					//nothing
				}
			}
		}

		//To continue enumeration, the callback function must return TRUE; to stop enumeration, it must return FALSE. 
		return TRUE;
	}

	//
	BOOL	WindowEnum::Clear( )
	{
		m_WindowHandleMap.clear();

		for (WindowNodeList::iterator i = m_WindowNodeList.begin();
			i != m_WindowNodeList.end(); i ++)
		{
			if(*i)
			{
				WindowNode* node = *i; *i = NULL;
				delete node;
			}
		}

		m_WindowNodeList.clear();
		return TRUE;
	}

	//
	BOOL	WindowEnum::Start( )
	{
		//
		this->Clear();

		//
		if(!EnumWindows(EnumWindowsProc, (LPARAM)this))
		{
			return FALSE;
		}

		//
		return TRUE;
	}

	//
	const WindowNode*	WindowEnum::Lookup(LPCTSTR lpszTitle)
	{
		for (WindowHandleMap::iterator i = m_WindowHandleMap.begin();
			i != m_WindowHandleMap.end(); i ++)
		{
			if(i->second)
			{
				std::tstring title = i->second->title_name;
				if((LONG)title.find(lpszTitle) >= 0)
				{
					return i->second;
				}
			}
		}
		return NULL;
	}
}

