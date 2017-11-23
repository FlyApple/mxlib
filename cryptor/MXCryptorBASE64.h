#ifndef __MX_CRYPTOR_H__
#define __MX_CRYPTOR_H__

#pragma once

//
#include <windows.h>
#include <wincrypt.h>
#include <stdlib.h>

#pragma comment(lib, "crypt32.lib")

//
namespace MXCryptor
{
	//
	#define BASE64_CRYPTTABLE_MAXLEN		64

	// 内存释放需要用delete []
	class Base64
	{
	public:
		Base64(CHAR* table = NULL, LONG index = 0);
		virtual ~Base64(void);

		virtual size_t	Encrypt(void* input_buf, size_t input_len, __out void** output_buf, __out size_t* output_len);
		virtual size_t	Decrypt(void* input_buf, size_t input_len, __out void** output_buf, __out size_t* output_len);

	protected:
		CHAR	m_szCryptTable[BASE64_CRYPTTABLE_MAXLEN];
	};
};

//
#endif //__MX_CRYPTOR_H__