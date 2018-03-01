
//
#include <windows.h>

#include "LoadScript.h"

#include <functional> 

#ifdef _MX_LOADSCRIPT_USE_TIME_
	#include <time.h>
#endif //_MX_LOADSCRIPT_USE_TIME_


//
namespace MX{	


// 返回行数量
MXScriptInteger	LoadScriptLines::AppendBytes(unsigned char* bytes, MXScriptInteger length, MXScriptBoolean eof)
{
	MXScriptInteger		result			= 0;
	MXScriptInteger		temp_length		= m_tempOffset + length;
	if((MXScriptInteger)m_tempBytes.size() < temp_length)
	{ m_tempBytes.resize(temp_length); }

	//末尾追加字节
	m_tempLength		= temp_length;
	memcpy(&m_tempBytes[m_tempOffset], bytes, length);

	//
	MXScriptInteger		lines			= 0;
	while (true)
	{
		MXScriptInteger	parse_bytes		= this->ParseBytes(eof);
		if(parse_bytes < 0)
		{
			result = -1;
			break;
		}
		else if(parse_bytes == 0)
		{
			result = lines;
			break;
		}

		lines ++;
	}
	
	//
	return result;
}

MXScriptInteger	LoadScriptLines::ParseBytes(MXScriptBoolean eof)
{
	MXScriptInteger		result		= 0;
	MXScriptInteger		length		= m_tempLength;
	MXScriptInteger		offset		= m_tempOffset;
	MXScriptBoolean		new_line	= false;
	if(length == 0){ return result; }

	//
	while (offset < length && new_line == false)
	{
		if(m_tempBytes[offset] == '\r')
		{
			m_tempBytes[offset] = '\0';
		}
		else if(m_tempBytes[offset] == '\n')
		{
			m_tempBytes[offset] = '\0';

			new_line = true;
		}

		offset += this->SizeOfChar();
	}
	m_tempOffset			= offset;

	//
	if(new_line == true)
	{
		if(!this->AppendLine(&m_tempBytes[0], offset))
		{
			return -1;
		}

		if(length - offset > 0)
		{
			memcpy(&m_tempBytes[0], &m_tempBytes[offset], length - offset);

			m_tempLength = m_tempLength - offset;
			m_tempOffset = 0;
		}
		else if(length - offset == 0)
		{
			m_tempLength = 0;
			m_tempOffset = 0;
		}

		result			= offset;
	}
	
	//
	if(eof == true && new_line == false)
	{
		// 向末尾增加4个字节的结束符,兼容ANSI,UNICODE,UTF8,UTF32
		m_tempBytes.resize(m_tempBytes.size() + this->SizeOfChar());
		memset(&m_tempBytes[offset], 0, (MXScriptInteger)m_tempBytes.size() - offset);

		if(!this->AppendLine(&m_tempBytes[0], offset))
		{
			return -1;
		}

		m_tempLength	= 0;
		m_tempOffset	= 0;
		result			= offset;
	}

	//
	return result;
}


//
MXScriptBoolean	LoadScriptLinesEx::AppendLine(void* bytes, MXScriptInteger length)
{
#if defined(_MSC_VER)
	switch(m_Type)
	{
		case LoadScriptType_Ansi:
			{ 
				MXScriptInteger	text_lengthW = length;
				wchar_t*		text_bufferW = new wchar_t[text_lengthW + 1];
				text_lengthW = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)bytes, length, text_bufferW, text_lengthW);
				text_bufferW[text_lengthW] = '\0';
				
				this->AppendLine(text_bufferW);
				delete	[] text_bufferW;
				break;
			}
		case LoadScriptType_UTF16LE:
			{ 
				MXScriptInteger	text_lengthW = length / sizeof(wchar_t);
				wchar_t*		text_bufferW = new wchar_t[text_lengthW + 1];
				memcpy(text_bufferW, bytes, length);
				text_bufferW[text_lengthW] = '\0';

				this->AppendLine(text_bufferW);
				delete	[] text_bufferW;
				break;
			}
		case LoadScriptType_UTF8LE:
			{ 
				MXScriptInteger	text_lengthW = length;
				wchar_t*		text_bufferW = new wchar_t[text_lengthW + 1];
				text_lengthW = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)bytes, length, text_bufferW, text_lengthW);
				text_bufferW[text_lengthW] = '\0';
				
				this->AppendLine(text_bufferW);
				delete	[] text_bufferW;
			}
		case LoadScriptType_UTF32LE:
			{ return false; } //UTF32 原生不支持
		default:
			{ return false; }
	}
#else
	return false;
#endif
	return true;
}


//
LoadScriptFromFile::LoadScriptFromFile()
{
	m_pFile				= NULL;
	m_nFileLength		= 0;
	m_stringFileName	= _T("");
}

LoadScriptFromFile::~LoadScriptFromFile()
{
	this->Close();
}

bool	LoadScriptFromFile::Load(const MXScriptStringT& stringFileName)
{
	m_stringFileName = stringFileName;

	//
	return this->Load();
}

bool	LoadScriptFromFile::Load()
{
#if defined(_MSC_VER)
#ifdef _UNICODE
	_wfopen_s(&m_pFile, m_stringFileName.c_str(), _T("rb"));
#else
	fopen_s(&m_pFile, m_stringFileName.c_str(), _T("rb"));
#endif 
#else
	m_pFile = fopen(m_stringFileName.c_str(), "rb");
#endif
	if(m_pFile == NULL)
	{
		return false;
	}

	_fseeki64(m_pFile, 0, SEEK_END);
	m_nFileLength	= _ftelli64(m_pFile);
	_fseeki64(m_pFile, 0, SEEK_SET);

	m_Type		= LoadScriptType_Ansi;	//默认编码为ansi
	m_nLength	= m_nFileLength;
	if(m_nFileLength > 0)
	{
		unsigned long flag = 0;
		fread(&flag, 0x03, 0x01, m_pFile);

		if((flag & 0xFFFF) == 0xFEFF)
		{
			m_Type		= LoadScriptType_Unicode;
			m_nLength	= m_nFileLength - 2;
			_fseeki64(m_pFile, 2, SEEK_SET);
		}
		else if(flag == 0xBFBBEF)
		{
			m_Type		= LoadScriptType_UTF8;
			m_nLength	= m_nFileLength - 3;
			_fseeki64(m_pFile, 3, SEEK_SET);
		}
		else
		{
			m_nLength	= m_nFileLength;
			_fseeki64(m_pFile, 0, SEEK_SET);
		}
	}

	//
	if(m_Type == LoadScriptType_Unknow)
	{ return false; }

	//
	if(m_pLines == NULL)
	{ m_pLines = new LoadScriptLinesEx(m_Type); }

	//
	return LoadScriptStream::Load();
}

bool		LoadScriptFromFile::LoadImpl()
{
	if(m_pFile == NULL || m_pLines == NULL)
	{ return false; }

	m_pLines->Clear();
	m_pLines->Type(m_Type);

	if(!this->ReadImpl())
	{ return false; }

	return true;
}

MXScriptInteger64	LoadScriptFromFile::ReadImpl()
{
	//
	unsigned char	temp_buffer[4096]	= {0};
	int				temp_length			= 0;

	//
	MXScriptInteger64	read_total	= 0;
	MXScriptBoolean		read_eof	= 0;
	MXScriptInteger		read_lines	= 0;
	while (read_total < m_nLength)
	{
		temp_length = fread(temp_buffer, 1, 1000 * m_pLines->SizeOfChar(), m_pFile);

		if(read_total + temp_length >= m_nLength)
		{ read_eof = true; }

		MXScriptInteger line_count = m_pLines->AppendBytes(temp_buffer, (long)temp_length, read_eof);
		if(line_count < 0){ return false; }
		
		read_lines += line_count;
		read_total += temp_length;
	}

	return read_total;
}


//
MXScriptBoolean		LoadScriptCacheLineTH::ParseColumnText()
{
	std::vector<MXScriptStringW> values;
	if(!this->ParseTextImpl(values))
	{
		m_stringText		= L"";
		m_lColumnCount		= 0;
		return false;
	}

	m_stringText			= L"";
	m_lColumnCount			= (MXScriptInteger)values.size();
	m_ColumnNames.resize(m_lColumnCount);

	m_lFirstRow				= m_lRow;
	m_lRow ++;

	//检测是否有空的列名
	//设置默认列名
	for (int i = 0; i < m_lColumnCount; i++)
	{
		MXScriptCharA	name[50] = {0, };

		if(values[i].empty())
		{
			sprintf_s(name, "[NULL_%04d]", i);
			m_ColumnNames[i] = name;
		}
		else
		{
			MXScriptStringW			textW = values[i];

#if defined(_MSC_VER)
			//进行UNICODE转ANSI
			MXScriptInteger			text_lengthA = WideCharToMultiByte(CP_UTF8, 0, textW.c_str(), -1, NULL, 0, NULL, NULL);
			
			text_lengthA = WideCharToMultiByte(CP_ACP, 0, textW.c_str(), -1, 
				name, min(text_lengthA, 50 -1), NULL, NULL);

			//
			m_ColumnNames[i] = name;
#else
			//进行UNICODE转UTF8
			return false;
#endif
		}
	}
	return true;
}


//
MXScriptBoolean				LoadScriptValue::ToBool()
{
	//如果错误或者为null,默认返回false
	if(m_bError == true || m_bNull == true)
	{ return false; }

	MXScriptStringW	text = m_Value;
	text.erase(remove_if(text.begin(), text.end(),   
		std::bind2nd(std::equal_to<wchar_t>(), L' ')),   
		text.end()); 

	if(_wcsicmp(m_Value.c_str(), L"1") == 0 ||
		_wcsicmp(m_Value.c_str(), L"true") == 0)
	{
		return true;
	}
	return false;
}

MXScriptFloat			LoadScriptValue::ToFloat()
{
	//如果错误或者为null,默认返回false
	if(m_bError == true || m_bNull == true)
	{ return 0.0f; }

	MXScriptStringW	text = m_Value;
	MXScriptFloat result = (MXScriptFloat)wcstod(text.c_str(), NULL);
	return result;
}

MXScriptNumber			LoadScriptValue::ToNumber()
{
	//如果错误或者为null,默认返回false
	if(m_bError == true || m_bNull == true)
	{ return 0.0f; }

	MXScriptStringW	text = m_Value;
	MXScriptNumber result = (MXScriptNumber)wcstod(text.c_str(), NULL);
	return result;
}

MXScriptInteger			LoadScriptValue::ToSignedDecimal()
{
	//如果错误或者为null,默认返回false
	if(m_bError == true || m_bNull == true)
	{ return 0; }

	MXScriptStringW		text	= m_Value;
	MXScriptInteger		result	= (MXScriptInteger)wcstol(text.c_str(), NULL, 10);
	return result;
}

MXScriptUInteger			LoadScriptValue::ToUnsignedDecimal()
{
	//如果错误或者为null,默认返回false
	if(m_bError == true || m_bNull == true)
	{ return 0; }

	MXScriptStringW		text	= m_Value;
	MXScriptUInteger	result	= (MXScriptUInteger)wcstoul(text.c_str(), NULL, 10);
	return result;
}

MXScriptInteger64			LoadScriptValue::ToSignedDecimal64()
{
	//如果错误或者为null,默认返回false
	if(m_bError == true || m_bNull == true)
	{ return 0; }

	MXScriptStringW		text	= m_Value;
	MXScriptInteger		result	= (MXScriptInteger)_wcstoi64(text.c_str(), NULL, 10);
	return result;
}

MXScriptUInteger64			LoadScriptValue::ToUnsignedDecimal64()
{
	//如果错误或者为null,默认返回false
	if(m_bError == true || m_bNull == true)
	{ return 0; }

	MXScriptStringW		text	= m_Value;
	MXScriptUInteger	result	= (MXScriptUInteger)_wcstoui64(text.c_str(), NULL, 10);
	return result;
}

MXScriptUInteger	LoadScriptValue::ToHexadecimal()
{
	//如果错误或者为null,默认返回false
	if(m_bError == true || m_bNull == true)
	{ return 0; }

	MXScriptStringW		text	= m_Value;
	MXScriptUInteger	result	= (MXScriptUInteger)wcstoul(text.c_str(), NULL, 16);
	return result;
}

MXScriptUInteger64	LoadScriptValue::ToHexadecimal64()
{
	//如果错误或者为null,默认返回false
	if(m_bError == true || m_bNull == true)
	{ return 0; }

	MXScriptStringW		text	= m_Value;
	MXScriptUInteger	result	= (MXScriptUInteger)_wcstoui64(text.c_str(), NULL, 16);
	return result;
}

const MXScriptCharW* LoadScriptValue::ToStringPtrUW()
{
	//如果错误或者为null,默认返回false
	if(m_bError == true || m_bNull == true)
	{ return NULL; }

	return m_Value.c_str();
}

const MXScriptCharA* LoadScriptValue::ToStringPtrUA()
{
	//如果错误或者为null,默认返回false
	if(m_bError == true || m_bNull == true)
	{ return NULL; }
#if defined(_MSC_VER)
	//进行UNICODE转ANSI
	int		text_lengthA = WideCharToMultiByte(CP_ACP, 0, m_Value.c_str(), -1, NULL, 0, NULL, NULL);

	char*	text_bufferA = new char[text_lengthA + 1];
	text_lengthA = WideCharToMultiByte(CP_ACP, 0, m_Value.c_str(), -1, text_bufferA, text_lengthA, NULL, NULL);
	text_bufferA[text_lengthA] = '\0';

	//
	m_ValueA = text_bufferA;
	delete [] text_bufferA;
	return m_ValueA.c_str();
#else
	return NULL;
#endif
}

const MXScriptCharA* LoadScriptValue::ToStringPtrU8()
{
	//如果错误或者为null,默认返回false
	if(m_bError == true || m_bNull == true)
	{ return NULL; }
#if defined(_MSC_VER)
	//进行UNICODE转ANSI
	int		text_lengthA = WideCharToMultiByte(CP_UTF8, 0, m_Value.c_str(), -1, NULL, 0, NULL, NULL);

	char*	text_bufferA = new char[text_lengthA + 1];
	text_lengthA = WideCharToMultiByte(CP_UTF8, 0, m_Value.c_str(), -1, text_bufferA, text_lengthA, NULL, NULL);
	text_bufferA[text_lengthA] = '\0';

	//
	m_ValueA = text_bufferA;
	delete [] text_bufferA;
	return m_ValueA.c_str();
#else
	return NULL;
#endif
}


#ifdef _MX_LOADSCRIPT_USE_TIME_

LoadScriptTime		LoadScriptValue::StampTimeToTime10()
{
	LoadScriptTime		time;
	MXScriptUInteger64	value	= this->ToUnsignedDecimal64();

	tm			t		= {0};
	localtime_s(&t, (time_t*)&value);
	time.year			= t.tm_year + 1900;
	time.month			= t.tm_mon;
	time.day			= t.tm_mday;
	time.hour			= t.tm_hour;
	time.minute			= t.tm_min;
	time.second			= t.tm_sec;
	time.wday			= t.tm_wday;
	return time;
}

LoadScriptTime		LoadScriptValue::StampTimeToTime16()
{
	LoadScriptTime		time;
	MXScriptUInteger64	value	= this->ToHexadecimal64();

	tm			t		= {0};
	localtime_s(&t, (time_t*)&value);
	time.year			= t.tm_year + 1900;
	time.month			= t.tm_mon;
	time.day			= t.tm_mday;
	time.hour			= t.tm_hour;
	time.minute			= t.tm_min;
	time.second			= t.tm_sec;
	time.wday			= t.tm_wday;
	return time;
}

#endif // _MX_LOADSCRIPT_USE_TIME_


//
bool		LoadScript::Next(LoadScriptCacheLineT& cache_line)
{
	if(m_pStream == NULL || m_pStream->m_pLines == NULL)
	{ return false; }

	long	row		= cache_line.m_lRow;
	if(!m_pStream->m_pLines->LineText(row, cache_line.m_stringText))
	{
		return false;
	}
	
	if(!cache_line.ParseText())
	{
		return false;
	}

	return true;
}

bool		LoadScript::Next(LoadScriptCacheLineTH& cache_line)
{
	if(m_pStream == NULL || m_pStream->m_pLines == NULL)
	{ return false; }

	if(cache_line.m_lColumnCount == 0)
	{
		if(!cache_line.LineText(m_pStream->m_pLines))
		{
			return false;
		}

		if(!cache_line.ParseColumnText())
		{
			return false;
		}
	}

	if(!cache_line.LineText(m_pStream->m_pLines))
	{
		return false;
	}

	if(!cache_line.ParseText())
	{
		return false;
	}
	return true;
}

//
} //namespace MX
