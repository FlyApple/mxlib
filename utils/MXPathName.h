
#ifndef __MX_PATHNAME_H__
#define __MX_PATHNAME_H__

#pragma once

//
namespace MXUtility
{
	//
	template<class _Ty>
	class PathName
	{
		typedef std::basic_string<_Ty>		string;
		typedef _Ty							value;
	public:
		PathName( )
		{

		}
		PathName(string stringFullName, bool volume = false)
		{
			if(volume)
			{
				m_stringFullName	= VolumeNameToDosName(stringFullName);
			}
			else
			{
				m_stringFullName	= stringFullName;
			}

			SplitPathName(m_stringFullName);
		}
		PathName(string stringDirName, string stringTitleName, string stringExtName)
		{
			m_stringDirName		= stringDirName;
			m_stringTitleName	= stringTitleName;
			m_stringExtName		= stringExtName;
			m_stringFileName	= stringTitleName + stringExtName;

			{
				int length = m_stringDirName.length();
				if( m_stringDirName[length - 1] != value('\\') &&
					m_stringDirName[length - 1] != value('/') )
				{
					m_stringFullName = m_stringDirName + value('\\') + m_stringFileName; 
				}
				else
				{
					m_stringFullName =  m_stringDirName + m_stringFileName;
				}
			}
		}

		const string&	GetFullName( ) { return m_stringFullName; }
		const string&	GetDirName( ) { return m_stringDirName; };
		const string&	GetFileName( ) { return m_stringFileName; }
		const string&	GetExtName( ) { return m_stringExtName; }
		const string&	GetTitleName( ) { return m_stringTitleName; }

	private:

		void	SplitPathName(string stringFullName)
		{
			size_t pos = stringFullName.rfind(value('\\'));
			if(pos == -1){ pos = stringFullName.rfind(value('/')); }
			if(pos > 0)
			{
				size_t	length	= pos + 1;
				_Ty		temp[200];
				length = stringFullName._Copy_s(temp, 200, length, 0);
				temp[length]	= value('\0');

				m_stringDirName	= temp;

				length			= stringFullName.length() - pos - 1;
				length = stringFullName._Copy_s(temp, 200, length, pos + 1);
				temp[length]	= value('\0');

				m_stringFileName= temp;
			}

			pos	= m_stringFileName.rfind(value('.'));
			if(pos == -1){ m_stringTitleName = m_stringFileName; }
			else
			{
				size_t	length	= pos;
				_Ty		temp[200];
				length = m_stringFileName._Copy_s(temp, 200, length, 0);
				temp[length]	= value('\0');

				m_stringTitleName = temp;

				length			= m_stringFileName.length() - pos;
				length = m_stringFileName._Copy_s(temp, 200, length, pos);
				temp[length]	= value('\0');

				m_stringExtName = temp;
			}
		}

		string	VolumeNameToDosName(string stringFullName)
		{
			string	name	= stringFullName;

			DWORD	length	= GetLogicalDriveStrings(0, NULL);
			_Ty*	buffer	= new _Ty[length];
			GetLogicalDriveStrings(length, buffer);

			for(DWORD i = 0; i < length; )
			{
				_Ty device[10]; _tcsncpy_s(device, &buffer[i], 2);
				_Ty temp[200] = {0};
				if(QueryDosDevice(device, temp, 200))
				{
					if(_tcsnicmp(stringFullName.c_str(), temp, _tcslen(temp)) == 0)
					{
						name = &buffer[i];
						name += stringFullName.substr(_tcslen(temp) + 1);
					}
				}
				i += (DWORD)_tcslen(&buffer[i]) + 1;
			}

			delete [] buffer;
			return name;
		}
	private:	
		string	m_stringFullName;
		string	m_stringDirName;
		string	m_stringFileName;
		string	m_stringExtName;
		string	m_stringTitleName;
	};
}

//
#endif //?__MX_PATHNAME_H__
