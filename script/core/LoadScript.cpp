
//
#include <windows.h>

#include "LoadScript.h"

#include <functional> 

#ifdef _MX_LOADSCRIPT_USE_TIME_
	#include <time.h>
#endif //_MX_LOADSCRIPT_USE_TIME_


//
namespace MX{	

/*

// 返回行数量
long	LoadScriptLines::AppendBytes(unsigned char* bytes, long length, bool eof)
{
	long		result			= 0;
	long		temp_length		= m_tempOffset + length;
	if((long)m_tempBytes.size() < temp_length)
	{ m_tempBytes.resize(temp_length); }

	//末尾追加字节
	m_tempLength				= temp_length;
	memcpy(&m_tempBytes[m_tempOffset], bytes, length);

	//
	long		lines			= 0;
	while (true)
	{
		long	parse_bytes		= this->ParseBytes(eof);
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

long	LoadScriptLines::ParseBytes(bool eof)
{
	long		result		= 0;
	long		length		= m_tempLength;
	long		offset		= m_tempOffset;
	bool		new_line	= false;
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
		// 向末尾增加4个字节的结束符,兼容ANSI,UNICODE,UTF8
		m_tempBytes.resize(m_tempBytes.size() + 4);
		m_tempBytes[offset + 0] = 0;
		m_tempBytes[offset + 1] = 0;
		m_tempBytes[offset + 2] = 0;
		m_tempBytes[offset + 3] = 0;

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
bool		LoadScriptStream::LineText(long row, __loadscript_out ScriptString& text)
{
	if(m_pLines == NULL){ return false; }

	//
	if(this->m_Type == LoadScriptType_Ansi)
	{
		ScriptStringA		textA;
		if(!m_pLines->LineText(row, textA)){ return false; }

#ifdef _UNICODE
		//进行转换
		long			text_lengthW = (textA.length() + 1) * sizeof(wchar_t);
		wchar_t*		text_bufferW = new wchar_t[text_lengthW];
		ZeroMemory(text_bufferW, text_lengthW);
		text_lengthW = MultiByteToWideChar(CP_ACP, 0, textA.c_str(), textA.length(), text_bufferW, text_lengthW);
		text	= text_bufferW;

		delete	text_bufferW;
#else
		text	= textA;
#endif
	}
	else if(this->m_Type == LoadScriptType_Unicode)
	{
		ScriptStringW		textW;
		if(!m_pLines->LineText(row, textW)){ return false; }
#ifdef _UNICODE
		text	= textW;
#else
		//进行转换
		long			text_lengthA = (textW.length() + 1) * sizeof(wchar_t);
		char*			text_bufferA = new char[text_lengthA];
		ZeroMemory(text_bufferA, text_lengthA);
		text_lengthA = WideCharToMultiByte(CP_ACP, 0, textW.c_str(), textW.length(), text_bufferA, text_lengthA, NULL, NULL);
		text	= text_bufferA;

		delete	text_bufferA;
#endif
	}
	else if(this->m_Type == LoadScriptType_UTF8)
	{
		ScriptStringA		textA;
		if(!m_pLines->LineText(row, textA)){ return false; }

		//进行UTF8转UNICODE
		long			text_lengthW = (textA.length() + 1) * sizeof(wchar_t);
		wchar_t*		text_bufferW = new wchar_t[text_lengthW];
		ZeroMemory(text_bufferW, text_lengthW);
		text_lengthW = MultiByteToWideChar(CP_UTF8, 0, textA.c_str(), textA.length(), text_bufferW, text_lengthW);

#ifdef _UNICODE
		text	= text_bufferW;
#else
		//进行UNICODE转ANSI
		long			text_lengthA = (text_lengthW + 1) * sizeof(wchar_t);
		char*			text_bufferA = new char[text_lengthA];
		ZeroMemory(text_bufferA, text_lengthA);
		text_lengthA = WideCharToMultiByte(CP_ACP, 0, text_bufferW, text_lengthW, text_bufferA, text_lengthA, NULL, NULL);
		text	= text_bufferA;

		delete	text_bufferA;
#endif
		delete	text_bufferW;
	}
	else
	{
		return false;
	}

	//
	return true;
}


//
LoadScriptFromFile::LoadScriptFromFile()
{
	m_pFile				= NULL;
	m_nFileLength		= 0;
	m_stringFileName	= _T("");
}

LoadScriptFromFile::LoadScriptFromFile(const ScriptString& stringFileName)
{
	m_pFile				= NULL;
	m_nFileLength		= 0;
	m_stringFileName	= stringFileName;
}

LoadScriptFromFile::~LoadScriptFromFile()
{
	if(m_pFile)
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}
}

bool	LoadScriptFromFile::Load(const ScriptString& stringFileName)
{
	m_stringFileName = stringFileName;

	//
	return this->Load();
}

bool	LoadScriptFromFile::Load()
{
#ifdef _UNICODE
	_wfopen_s(&m_pFile, m_stringFileName.c_str(), _T("rb"));
#else
	fopen_s(&m_pFile, m_stringFileName.c_str(), _T("rb"));
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
	else if(m_Type == LoadScriptType_Unicode)
	{
		m_pLines	= new LoadScriptLinesW();
	}
	else
	{
		m_pLines	= new LoadScriptLinesA();
	}

	//
	return LoadScriptStream::Load();
}

bool		LoadScriptFromFile::LoadImpl()
{
	if(m_pFile == NULL || m_pLines == NULL){ return false; }

	//
	unsigned char	temp_buffer[512]	= {0};
	unsigned long	temp_length			= 0;

	//
	long	read_total	= 0;
	bool	read_eof	= 0;
	long	read_lines	= 0;
	while (read_total < m_nLength)
	{
		temp_length = fread(temp_buffer, 1, 100 * m_pLines->SizeOfChar(), m_pFile);
		if(read_total + temp_length >= m_nLength)
		{ read_eof = true; }

		long line_count = m_pLines->AppendBytes(temp_buffer, (long)temp_length, read_eof);
		if(line_count < 0){ return false; }
		
		read_lines += line_count;
		read_total += temp_length;
	}
	return true;
}

//
LoadScriptValue::operator bool()
{
	return this->ToBool();
}

LoadScriptValue::operator int()
{
	return this->ToSignedLongDec();
}

LoadScriptValue::operator long()
{
	return this->ToSignedLongDec();
}

LoadScriptValue::operator float()
{
	return this->ToFloat();
}

LoadScriptValue::operator const ScriptChar* ()
{
	return this->ToStringPtrC();
}

bool			LoadScriptValue::ToBool()
{
	//如果错误或者为null,默认返回false
	if(m_bError == true || m_bNull == true)
	{ return false; }

	ScriptString	text = m_Value;
	text.erase(remove_if(text.begin(), text.end(),   
		std::bind2nd(std::equal_to<char>(), ' ')),   
		text.end()); 
	if(_tcsicmp(m_Value.c_str(), _T("1")) == 0 ||
		_tcsicmp(m_Value.c_str(), _T("true")) == 0)
	{
		return true;
	}
	return false;
}

float			LoadScriptValue::ToFloat()
{
	//如果错误或者为null,默认返回false
	if(m_bError == true || m_bNull == true)
	{ return 0.0f; }

	ScriptString	text = m_Value;
	float result = (float)_tcstod(text.c_str(), NULL);
	return result;
}

long			LoadScriptValue::ToSignedLongDec()
{
	//如果错误或者为null,默认返回false
	if(m_bError == true || m_bNull == true)
	{ return 0; }

	ScriptString	text	= m_Value;
	long			result	= _tcstol(text.c_str(), NULL, 10);
	return result;
}

long			LoadScriptValue::ToSignedLongHex()
{
	//如果错误或者为null,默认返回false
	if(m_bError == true || m_bNull == true)
	{ return 0; }

	ScriptString	text	= m_Value;
	long			result	= _tcstol(text.c_str(), NULL, 16);
	return result;
}

unsigned long	LoadScriptValue::ToUnsignedLongDec()
{
	//如果错误或者为null,默认返回false
	if(m_bError == true || m_bNull == true)
	{ return 0; }

	ScriptString	text = m_Value;
	unsigned long result = _tcstoul(text.c_str(), NULL, 10);
	return result;
}

unsigned long	LoadScriptValue::ToUnsignedLongHex()
{
	//如果错误或者为null,默认返回false
	if(m_bError == true || m_bNull == true)
	{ return 0; }
	
	ScriptString	text = m_Value;
	unsigned long result = _tcstoul(text.c_str(), NULL, 16);
	return result;
}

const ScriptChar* LoadScriptValue::ToStringPtrC()
{
	//如果错误或者为null,默认返回false
	if(m_bError == true || m_bNull == true)
	{ return NULL; }

	return m_Value.c_str();
}

#ifdef LOADSCRIPT_USE_TIME

LoadScriptTime		LoadScriptValue::StampTimeToTime10()
{
	LoadScriptTime		time;
	unsigned __int64	value	= this->ToUnsignedLongDec();

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
	unsigned __int64	value	= this->ToUnsignedLongHex();

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

#endif // LOADSCRIPT_USE_TIME

//
bool		LoadScript::Next(LoadScriptCacheLineT& cache_line)
{
	if(m_pStream == NULL){ return false; }

	long	row		= cache_line.m_lRow;
	if(!m_pStream->LineText(row, cache_line.m_stringText))
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
	if(m_pStream == NULL){ return false; }

	if(cache_line.m_lColumnCount == 0)
	{
		if(!cache_line.LineText(m_pStream))
		{
			return false;
		}

		if(!cache_line.ParseColumnText())
		{
			return false;
		}
	}

	if(!cache_line.LineText(m_pStream))
	{
		return false;
	}

	if(!cache_line.ParseText())
	{
		return false;
	}
	return true;
}

*/

//
} //namespace MX
