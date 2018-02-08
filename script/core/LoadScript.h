#ifndef __MX_LoadScript_H__
#define __MX_LoadScript_H__


#pragma once

//
#include <stdio.h>
#include <tchar.h>
#include <algorithm>
#include <map>
#include <vector>
#include <string>


// 使用TIME库
// 影响stamptime等功能
#define _MX_LOADSCRIPT_USE_TIME_

//
namespace MX{	

//
#define __mxscript_out	__out
#define __mxscript_in		__in
#define __mxscript_inout	__inout


//
#define ScriptInteger		int
#define ScriptUInteger		unsigned int
#define ScriptInteger64		__int64
#define ScriptUInteger64	unsigned __int64
#define ScriptFloat			float
#define ScriptNumber		double
#define ScriptBoolean		bool
#define ScriptCharA			char
#define ScriptCharW			wchar_t			//linux/unix utf32
#define ScriptStringA		std::string
#define ScriptStringW		std::wstring	//linux/unix utf32


//
enum LoadScriptType
{
	LoadScriptType_Unknow,
	LoadScriptType_Ansi,
	LoadScriptType_Unicode,
	LoadScriptType_UTF8,
	LoadScriptType_Max,
};

//
struct LoadScriptTime
{
public:
	short		year;
	short		month;
	short		day;
	short		hour;
	short		minute;
	short		second;

	short		wday;
};

/*
//
class LoadScriptLines
{
public:
	LoadScriptLines(){ m_tempOffset = 0; m_tempLength = 0; }
	
	//
	virtual	long	SizeOfChar() = 0;
	virtual	long	LineCount() = 0;
	virtual bool	LineText(long line, __loadscript_out ScriptStringA& text){ return true; }
	virtual bool	LineText(long line, __loadscript_out ScriptStringW& text){ return true; }

	// 返回行数量
	long			AppendBytes(unsigned char* bytes, long length, bool eof = false);

protected:
	long			ParseBytes(bool eof);

	virtual bool	AppendLine(void* bytes, unsigned long length) = 0;

private:
	long							m_tempOffset;
	long							m_tempLength;
	std::vector<unsigned char>		m_tempBytes;
};

//
class LoadScriptLinesA : public LoadScriptLines
{
public:
	virtual bool	AppendLine(const char* text)
	{
		m_stringLines.push_back(text);
		return true;
	}
protected:
	virtual	long	SizeOfChar() { return sizeof(char); }
	virtual	long	LineCount(){ return (long)m_stringLines.size(); }
	virtual bool	LineText(long line, __loadscript_out ScriptStringA& text)
	{ 
		if(line >= (long)m_stringLines.size()){ return false; }
		text	= m_stringLines[line];
		return true; 
	}
	virtual bool	AppendLine(void* bytes, unsigned long length)
	{
		return this->AppendLine((const char*)bytes);
	}
private:
	std::vector<ScriptStringA>		m_stringLines;
};


//
class LoadScriptLinesW : public LoadScriptLines
{
public:
	virtual bool	AppendLine(const wchar_t* text)
	{
		m_stringLines.push_back(text);
		return true;
	}
protected:
	virtual	long	SizeOfChar() { return sizeof(wchar_t); }
	virtual	long	LineCount(){ return (long)m_stringLines.size(); }
	virtual bool	LineText(long line, __loadscript_out ScriptStringW& text)
	{ 
		if(line >= (long)m_stringLines.size()){ return false; }
		text	= m_stringLines[line];
		return true; 
	}
	virtual bool	AppendLine(void* bytes, unsigned long length)
	{
		return this->AppendLine((const wchar_t*)bytes);
	}
private:
	std::vector<ScriptStringW>		m_stringLines;
};

//
class LoadScriptStream
{
public:
	LoadScriptStream()
		:	m_Type (LoadScriptType_Unknow),
			m_nLength(0),
			m_pLines(NULL)
	{ }
	virtual ~LoadScriptStream()
	{
		if(m_pLines)
		{
			delete m_pLines;
			m_pLines = NULL;
		}
	}

	virtual bool		Load(){ return this->LoadImpl(); }
		
	long				LineCount()
	{ 
		if(m_pLines == NULL){ return 0;}
		return m_pLines->LineCount();
	}
	bool				LineText(long row, __loadscript_out ScriptString& text);

protected:
	virtual bool		LoadImpl() = 0;

public:
	LoadScriptType		m_Type;
	__int64				m_nLength;
	LoadScriptLines*	m_pLines;
};

//
class LoadScriptFromFile : public LoadScriptStream
{
public:
	LoadScriptFromFile();
	LoadScriptFromFile(const ScriptString& stringFileName);
	virtual ~LoadScriptFromFile();

	bool				Load(const ScriptString& stringFileName);
	virtual bool		Load();

protected:
	virtual bool		LoadImpl();

public:
	FILE*				m_pFile;
	__int64				m_nFileLength;
	ScriptString		m_stringFileName;
};

//
class LoadScriptCacheLineBase
{
public:
	LoadScriptCacheLineBase()
	{
		m_lRow		= 0;
	}

	bool ValueIsNull(long index, __loadscript_out bool& result)
	{
		ScriptString text;
		if(!this->GetValueText(index, text))
		{
			return false;
		}

		result = text.empty();
		return true;
	}
	bool LineIsComment()
	{
		for(unsigned int i = 0; i < m_stringText.length(); i++)
		{
			if(m_stringText[i] == ' '){ continue; }
			if(i + 1 < m_stringText.length() && m_stringText[i] == '/' && m_stringText[i + 1] == '/')
			{ return true; }
			else if(m_stringText[i] == '#')
			{ return true; }
		}
		return false;
	}

	virtual bool				LineText(LoadScriptStream* stream) = 0;
	virtual bool				GetValueText(long index, __loadscript_out ScriptString& text) = 0;

protected:
	virtual bool				ParseText() = 0;
	virtual bool				ParseTextImpl(__loadscript_out std::vector<ScriptString>& values)
	{
		values.clear();

		//
		ScriptString	value;

		//对字符串以退格键进行分割
		for (int n = 0; n < (int)m_stringText.length(); n ++)
		{
			long	character = m_stringText[n];
			if(character == '\t')
			{
				values.push_back(value);
				value = _T("");
			}
			else
			{
				value += (ScriptChar)character;
			}
		}

		//
		if(value.length() >= 0)
		{
			values.push_back(value);
		}
		return true;
	}

protected:
	long						m_lRow;
	ScriptString				m_stringText;

};


class LoadScriptCacheLineT : public LoadScriptCacheLineBase
{
	friend class LoadScript;
public:
	LoadScriptCacheLineT(){ }

	void operator ++ (){ m_lRow ++; }
	void operator -- (){ m_lRow = m_lRow - 1 >= 0 ? m_lRow - 1 : 0; }
	void operator ++ (int){ ++(*this); }
	void operator -- (int){ --(*this); }

	virtual bool		LineText(LoadScriptStream* stream)
	{
		if(!stream){ return false; }

		bool result = false;
		while(stream->LineText(m_lRow, m_stringText))
		{
			if(this->LineIsComment()){ continue; }
			else
			{
				result = true;
				break;
			}
			(*this) ++;
		}
		return true;
	}

	virtual bool		GetValueText(long index, __loadscript_out ScriptString& text)
	{
		if(index < 0 || index >= (long)m_stringValues.size()){ return false; }

		text = m_stringValues[index];
		return true;
	}
private:
	virtual bool		ParseText()
	{
		return this->ParseTextImpl(m_stringValues);
	}

protected:
	std::vector<ScriptString>	m_stringValues;
};



class LoadScriptCacheLineTH : public LoadScriptCacheLineBase
{
	friend class LoadScript;
public:
	LoadScriptCacheLineTH()
	{
		m_lFirstRow				= 1;
		m_lColumnCount			= 0;
	}

	void operator ++ (){ m_lRow ++; }
	void operator -- (){ m_lRow = m_lRow - m_lFirstRow >= 0 ? m_lRow - m_lFirstRow : m_lFirstRow; }
	void operator ++ (int){ ++(*this); }
	void operator -- (int){ --(*this); }

	virtual bool		LineText(LoadScriptStream* stream)
	{
		if(!stream){ return false; }

		bool result = false;
		while(stream->LineText(m_lRow, m_stringText))
		{
			if (this->LineIsComment()){  }
			else { result = true; break; }
			m_lRow ++;
		}
		return result;
	}

	virtual bool		GetValueText(long index, __loadscript_out ScriptString& text)
	{
		if(index < 0 || index >= (long)m_stringValues.size()){ return false; }

		text = m_stringValues[index];
		return true;
	}

	virtual bool		GetColumnText(long col, __loadscript_out ScriptString& text)
	{
		return this->GetValueText(col, text);
	}

	virtual long		GetColumnIndexByName(const ScriptChar* name, bool ignore_case = true)
	{
		long result	= -1;
		for (int i = 0; i < m_lColumnCount; i ++)
		{
			ScriptString text = m_ColumnNames[i];
			if( (ignore_case == true && _tcsicmp(name, text.c_str()) == 0) ||	//不区分大小写
				(ignore_case == false && _tcscmp(name, text.c_str()) == 0))		//区分大小写
			{
				result = i; break;
			}
		}
		return result;
	}
	virtual bool		GetColumnTextByName(const ScriptChar* name, __loadscript_out ScriptString& text, bool ignore_case = true)
	{
		long col = GetColumnIndexByName(name, ignore_case);
		return this->GetColumnText(col, text);
	}
private:
	virtual bool		ParseColumnText()
	{
		if(!this->ParseTextImpl(m_ColumnNames))
		{
			m_stringText		= _T("");
			m_lColumnCount		= 0;
			return false;
		}

		m_stringText			= _T("");
		m_lColumnCount			= (long)m_ColumnNames.size();

		m_lFirstRow				= m_lRow;
		m_lRow ++;

		//检测是否有空的列名
		//设置默认列名
		for (int i = 0; i < m_lColumnCount; i++)
		{
			if(m_ColumnNames[i].empty())
			{
				ScriptChar	name[50] = {0, };
				_stprintf_s(name, _T("[NULL_%04d]"), i);
				m_ColumnNames[i] = name;
			}
		}
		return true;
	}

	virtual bool		ParseText()
	{
		if(!this->ParseTextImpl(m_stringValues))
		{
			return false;
		}

		if(m_lColumnCount != (long)m_stringValues.size())
		{
			ScriptTrace(_T("<%s> Column count error! max count %d."), _T(__FUNCTION__), m_lColumnCount);
		}
		return true;
	}

protected:
	std::vector<ScriptString>	m_stringValues;

	long						m_lFirstRow;
	long						m_lColumnCount;
	std::vector<ScriptString>	m_ColumnNames;
};

//
class LoadScriptValue
{
public:
	LoadScriptValue()
	{ this->Clear(); }

	void			Clear()
	{
		m_bError	= false;
		m_bNull		= true;
		m_Value		= _T("");
	}

	bool			IsNull(){ return m_bNull; }
	bool			IsError(){ return m_bError; }

	void			operator = (const LoadScriptValue& value)
	{
		m_bError	= value.m_bError;
		m_bNull		= value.m_bNull;
		m_Value		= value.m_Value;
	}

	operator			bool();
	operator			int();
	operator			long();
	operator			float();
	operator			const ScriptChar*();

	bool				ToBool();
	float				ToFloat();
	long				ToSignedLongDec();
	long				ToSignedLongHex();
	unsigned long		ToUnsignedLongDec();
	unsigned long		ToUnsignedLongHex();
	const ScriptChar*	ToStringPtrC();

#ifdef LOADSCRIPT_USE_TIME
	LoadScriptTime		StampTimeToTime10();
	LoadScriptTime		StampTimeToTime16();
#endif //LOADSCRIPT_USE_TIME

public:
	bool			m_bError;
	bool			m_bNull;
	ScriptString	m_Value;
};

template<class _Ty>
class LoadScriptCacheLinePtr
{
public:
	LoadScriptCacheLinePtr(_Ty& pointer)
	{
		m_pPointer		= &pointer;
	}
	LoadScriptCacheLinePtr(_Ty* pointer)
	{
		m_pPointer		= pointer;
	}

	//
	_Ty*						operator -> () { return m_pPointer; }
	__inline LoadScriptValue	operator[] (long index)
	{
		LoadScriptValue	result;
		if(!m_pPointer->GetValueText(index, result.m_Value))
		{ 
			result.m_bError = true;
			return result; 
		}

		// 如果文本为空,返回NULL
		result.m_bNull	= false;
		if(result.m_Value.empty())
		{ result.Clear(); }

		return result;
	}
	__inline LoadScriptValue	operator[] (const ScriptChar* name)
	{
		LoadScriptValue	result;
		if(!m_pPointer->GetColumnTextByName(name, result.m_Value))
		{ 
			result.m_bError = true;
			return result; 
		}

		// 如果文本为空,返回NULL
		result.m_bNull	= false;
		if(result.m_Value.empty())
		{ result.Clear(); }

		return result;
	}

private:
	_Ty*			m_pPointer;
};

//
class LoadScript
{
public:
	LoadScript(LoadScriptStream* stream) : m_pStream(stream) 
	{
	}
	virtual ~LoadScript(void){}

	virtual bool		Load(){ return true; };

	virtual bool		Next(LoadScriptCacheLineT& cache_line);
	virtual bool		Next(LoadScriptCacheLineTH& cache_line);

protected:
	LoadScriptStream*		m_pStream;
	long					m_lStartReadLine;
};
*/
//
} //namespace MX

//
#endif //__MX_LoadScript_H__