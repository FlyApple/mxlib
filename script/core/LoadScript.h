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
#define __mxscript_out			__out
#define __mxscript_in			__in
#define __mxscript_inout		__inout


//
#define MXScriptInteger			int
#define MXScriptUInteger		unsigned int
#define MXScriptInteger64		__int64
#define MXScriptUInteger64		unsigned __int64
#define MXScriptFloat			float
#define MXScriptNumber			double
#define MXScriptBoolean			bool
#define MXScriptCharA			char
#define MXScriptCharW			wchar_t			//linux/unix utf32
#define MXScriptStringA			std::string
#define MXScriptStringW			std::wstring	//linux/unix utf32

#if defined(_MSC_VER)
#if defined _UNICODE
#define MXScriptCharT			MXScriptCharW
#define MXScriptStringT			MXScriptStringW	
#else
#define MXScriptCharT			MXScriptCharA
#define MXScriptStringT			MXScriptStringA
#endif
#else
#define MXScriptCharT			MXScriptCharA
#define MXScriptStringT			MXScriptStringA
#endif

//
enum LoadScriptType
{
	LoadScriptType_Unknow,
	LoadScriptType_Default	= 1,
	LoadScriptType_Ansi		= 1,
	LoadScriptType_UTF16LE	= 2,
	LoadScriptType_Unicode	= 2,
	LoadScriptType_UTF8LE	= 3,
	LoadScriptType_UTF8		= 3,
	LoadScriptType_UTF32LE	= 4,
	LoadScriptType_UTF32	= 4,
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


//
class LoadScriptLines
{
public:
	LoadScriptLines()
	{ 
		m_tempOffset = 0; 
		m_tempLength = 0; 
	}

	//
	virtual void	Clear()
	{
		m_tempLength = 0;
		m_tempOffset = 0;
		m_tempBytes.clear();
	}
	
	//
	virtual	MXScriptInteger	SizeOfChar() = 0;
	virtual	MXScriptInteger	LineCount() = 0;
	virtual MXScriptBoolean	LineText(MXScriptInteger line, __mxscript_out MXScriptStringW& text) = 0;

	// 返回行数量
	MXScriptInteger			AppendBytes(unsigned char* bytes, MXScriptInteger length, MXScriptBoolean eof = false);

protected:
	MXScriptInteger			ParseBytes(MXScriptBoolean eof);

	virtual MXScriptBoolean	AppendLine(void* bytes, MXScriptInteger length) = 0;

private:
	MXScriptInteger					m_tempOffset;
	MXScriptInteger					m_tempLength;
	std::vector<unsigned char>		m_tempBytes;
};

//
class LoadScriptLinesEx : public LoadScriptLines
{
public:
	LoadScriptLinesEx(LoadScriptType type)
		: m_Type(type)
	{ 
	}
	
	__inline void	Type(LoadScriptType type){ m_Type = type; }

	//
	virtual void	Clear()
	{
		//
		m_Type = LoadScriptType_Unknow;
		m_LineCount = 0;
		m_stringLines.clear();

		//
		LoadScriptLines::Clear();
	}

	//
	virtual	MXScriptInteger	SizeOfChar()
	{
		switch(m_Type)
		{
		case LoadScriptType_Ansi:{ return 1; }
		case LoadScriptType_UTF16LE:{ return 2; }
		case LoadScriptType_UTF8LE:{ return 1; }
		case LoadScriptType_UTF32LE:{ return 4; }
		}
		return 1;
	}

	virtual	MXScriptInteger	LineCount()
	{
		return m_LineCount;
	}
		
	virtual MXScriptBoolean	AppendLine(const wchar_t* text)
	{
		m_stringLines.push_back(text);
		m_LineCount = (MXScriptInteger)m_stringLines.size();
		return true;
	}

	virtual MXScriptBoolean	LineText(MXScriptInteger line, __mxscript_out MXScriptStringW& text)
	{ 
		if(line >= this->LineCount()){ return false; }

		text	= m_stringLines[line];
		return true; 
	}
protected:
	virtual MXScriptBoolean	AppendLine(void* bytes, MXScriptInteger length);

private:
	LoadScriptType						m_Type;
	MXScriptInteger						m_LineCount;
	std::vector<MXScriptStringW>		m_stringLines;
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

		m_nLength = 0;
	}

	virtual void				Close(){ }

	virtual MXScriptBoolean		Load(){ return this->LoadImpl(); }

protected:
	virtual MXScriptBoolean		LoadImpl() = 0;

public:
	LoadScriptType		m_Type;
	MXScriptInteger64	m_nLength;
	LoadScriptLinesEx*	m_pLines;
};

//
class LoadScriptFromFile : public LoadScriptStream
{
public:
	LoadScriptFromFile();
	virtual ~LoadScriptFromFile();

	virtual void				Close()
	{
		if(m_pFile)
		{
			fclose(m_pFile);
			m_pFile = NULL;
		}

		//
		m_nFileLength = 0;

		//
		LoadScriptStream::Close();
	}

	MXScriptBoolean				Load(const MXScriptStringT& stringFileName);
	virtual MXScriptBoolean		Load();

protected:
	virtual MXScriptBoolean		LoadImpl();

	virtual MXScriptInteger64	ReadImpl();

public:
	FILE*				m_pFile;
	MXScriptInteger64	m_nFileLength;
	MXScriptStringT		m_stringFileName;
};


//
class LoadScriptCacheLineBase
{
public:
	LoadScriptCacheLineBase()
	{
		m_lRow		= 0;
	}

	//
	MXScriptBoolean LineHasComment()
	{
		for(unsigned int i = 0; i < m_stringText.length(); i++)
		{
			if(m_stringText[i] == ' '){ continue; }
			else if(i + 1 < m_stringText.length() && 
				m_stringText[i] == '/' && m_stringText[i + 1] == '/')
			{ return true; }
			else if(m_stringText[i] == '#')
			{ return true; }
			else
			{ return false; }
		}
		return false;
	}

	virtual MXScriptBoolean	LineText(LoadScriptLinesEx* lines) = 0;


	MXScriptBoolean ValueHasNull(MXScriptInteger index, __mxscript_out MXScriptBoolean& result)
	{
		MXScriptStringW text;
		if(!this->ValueText(index, text))
		{ return false; }

		result = text.empty();
		return true;
	}

	virtual MXScriptBoolean				ValueText(MXScriptInteger index, __mxscript_out MXScriptStringW& text) = 0;

protected:
	virtual MXScriptBoolean				ParseText() = 0;
	virtual MXScriptBoolean				ParseTextImpl(__mxscript_out std::vector<MXScriptStringW>& values)
	{
		values.clear();

		//
		MXScriptStringW			value;

		//对字符串以退格键进行分割
		for (int n = 0; n < (int)m_stringText.length(); n ++)
		{
			MXScriptInteger	cc = m_stringText[n];
			if(cc == '\t')
			{
				values.push_back(value);
				value = L"";
			}
			else
			{
				value += (MXScriptCharW)cc;
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
	MXScriptInteger				m_lRow;
	MXScriptStringW				m_stringText;
};

//
class LoadScriptCacheLineT : public LoadScriptCacheLineBase
{
	friend class LoadScript;
public:
	LoadScriptCacheLineT(){ }

	void operator ++ (){ m_lRow ++; }
	void operator -- (){ m_lRow = m_lRow - 1 >= 0 ? m_lRow - 1 : 0; }
	void operator ++ (int){ ++(*this); }
	void operator -- (int){ --(*this); }

	virtual MXScriptBoolean		LineText(LoadScriptLinesEx* lines)
	{
		if(!lines){ return false; }

		MXScriptBoolean result = false;
		while(lines->LineText(m_lRow, m_stringText))
		{
			if(this->LineHasComment())
			{
				//nothing
			}
			else
			{
				result = true;
				break;
			}
			(*this) ++;
		}
		return result;
	}

	virtual MXScriptBoolean		ValueText(MXScriptInteger index, __mxscript_out MXScriptStringW& text)
	{
		if(index < 0 || index >= (MXScriptInteger)m_stringValues.size())
		{ return false; }

		text = m_stringValues[index];
		return true;
	}
private:
	virtual MXScriptBoolean		ParseText()
	{
		return this->ParseTextImpl(m_stringValues);
	}

protected:
	std::vector<MXScriptStringW>	m_stringValues;
};


//
class LoadScriptCacheLineTH : public LoadScriptCacheLineT
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

	virtual MXScriptBoolean		TextByColumnIndex(MXScriptInteger col, __mxscript_out MXScriptStringW& text)
	{
		return this->ValueText(col, text);
	}

	virtual MXScriptInteger		IndexByColumnName(const MXScriptCharA* name, MXScriptBoolean ignore_case = true)
	{
		long result	= -1;
		for (int i = 0; i < m_lColumnCount; i ++)
		{
			MXScriptStringA text = m_ColumnNames[i];
			if( (ignore_case == true && _stricmp(name, text.c_str()) == 0) ||	//不区分大小写
				(ignore_case == false && strcmp(name, text.c_str()) == 0))		//区分大小写
			{
				result = i; break;
			}
		}
		return result;
	}
	virtual MXScriptBoolean		TextByColumnName(const MXScriptCharA* name, __mxscript_out MXScriptStringW& text, bool ignore_case = true)
	{
		MXScriptInteger col = IndexByColumnName(name, ignore_case);
		return this->TextByColumnIndex(col, text);
	}

private:
	virtual MXScriptBoolean		ParseColumnText();

	virtual bool		ParseText()
	{
		if(!this->ParseTextImpl(m_stringValues))
		{
			return false;
		}

		if(m_lColumnCount > (MXScriptInteger)m_stringValues.size())
		{
			return false;
		}
		return true;
	}

protected:
	MXScriptInteger					m_lFirstRow;

	MXScriptInteger					m_lColumnCount;
	std::vector<MXScriptStringA>	m_ColumnNames;		//列名根据系统环境改变
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
		m_Value		= L"";
		m_ValueA	= "";
	}

	bool			IsNull(){ return m_bNull; }
	bool			IsError(){ return m_bError; }

	void			operator = (const LoadScriptValue& value)
	{
		m_bError	= value.m_bError;
		m_bNull		= value.m_bNull;
		m_Value		= value.m_Value;
	}

	__inline operator			MXScriptBoolean()		{ return this->ToBool(); }
	__inline operator			MXScriptInteger()		{ return this->ToSignedDecimal(); }
	__inline operator			MXScriptUInteger()		{ return this->ToUnsignedDecimal(); }
	__inline operator			MXScriptInteger64()		{ return this->ToSignedDecimal64(); }
	__inline operator			MXScriptUInteger64()	{ return this->ToUnsignedDecimal64(); }
	__inline operator			MXScriptFloat()			{ return this->ToFloat(); }
	__inline operator			MXScriptNumber()		{ return this->ToNumber(); }
	__inline operator			const MXScriptCharW*()	{ return this->ToStringPtrUW(); }
	__inline operator			const MXScriptCharA*()	{ return this->ToStringPtrUA(); }

	MXScriptBoolean				ToBool();
	MXScriptFloat				ToFloat();
	MXScriptNumber				ToNumber();
	MXScriptInteger				ToSignedDecimal();
	MXScriptUInteger			ToUnsignedDecimal();
	MXScriptInteger64			ToSignedDecimal64();
	MXScriptUInteger64			ToUnsignedDecimal64();
	MXScriptUInteger			ToHexadecimal();
	MXScriptUInteger64			ToHexadecimal64();
	const MXScriptCharW*		ToStringPtrUW();
	const MXScriptCharA*		ToStringPtrUA();
	const MXScriptCharA*		ToStringPtrU8();

#ifdef _MX_LOADSCRIPT_USE_TIME_
	LoadScriptTime				StampTimeToTime10();
	LoadScriptTime				StampTimeToTime16();
#endif //_MX_LOADSCRIPT_USE_TIME_

public:
	MXScriptBoolean			m_bError;
	MXScriptBoolean			m_bNull;
	MXScriptStringW			m_Value;
	MXScriptStringA			m_ValueA;
};

//
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
	__inline LoadScriptValue	operator[] (MXScriptInteger index)
	{
		return this->ValueByIndex(index);
	}

	__inline LoadScriptValue	ValueByName (const MXScriptCharA* name)
	{
		LoadScriptValue	result;
		if(!m_pPointer->ColumnTextByName(name, result.m_Value))
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

	__inline LoadScriptValue	ValueByIndex (MXScriptInteger index)
	{
		LoadScriptValue	result;
		if(!m_pPointer->ValueText(index, result.m_Value))
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
	LoadScript() 
		: m_pStream (NULL)
	{ }
	virtual ~LoadScript(void)
	{
		if(m_pStream)
		{
			m_pStream->Close();
			m_pStream = NULL;
		}
	}

	operator LoadScriptStream*(){ return this->Stream(); }
	operator LoadScriptLinesEx*(){ return this->Lines(); }

	__inline LoadScriptLinesEx*	Lines(){ return m_pStream == NULL ? NULL: m_pStream->m_pLines; }
	__inline LoadScriptStream*	Stream(){ return m_pStream; }

	virtual bool		Load(){ return true; };

	virtual bool		Next(LoadScriptCacheLineT& cache_line);
	virtual bool		Next(LoadScriptCacheLineTH& cache_line);

protected:
	LoadScriptStream*		m_pStream;
};

//
template<class _TA>
class LoadScriptT : public LoadScript
{
public:
	LoadScriptT() { }
	virtual ~LoadScriptT(void)
	{ }

	bool		Load(const MXScriptStringT& stringFileName)
	{ 
		if(!m_StreamT.Load(stringFileName))
		{
			m_StreamT.Close();
			return false;
		}

		m_pStream = &m_StreamT;
		return LoadScript::Load(); 
	}
private:
	_TA			m_StreamT;
};

//
} //namespace MX

//
#endif //__MX_LoadScript_H__