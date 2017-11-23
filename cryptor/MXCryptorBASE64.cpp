
#include "MXCryptorBASE64.h"


namespace MXCryptor
{
	//--------------------------------------------------------------------------------------------
	// class Base64
	//
	Base64::Base64(CHAR* table, LONG index)
	{
		if(!table){ table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"; }
		
		if(index == 1){ table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-"; }
		else if(index == 2){ table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789._"; }
		else if(index == 3){ table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.-"; }
		else if(index == 4){ table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_:"; }
		else if(index == 5){ table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789^*"; }
		
		memcpy(m_szCryptTable, table, BASE64_CRYPTTABLE_MAXLEN);
	}

	//
	Base64::~Base64(void)
	{
	}

	//
	size_t	Base64::Encrypt(void* input_buf, size_t input_len, __out void** output_buf, __out size_t* output_len)
	{
		// 計算長度,每三字節轉換為四字節
		size_t length = input_len / 3 * 4;
		// 如果不能整除,計算餘數
		length += input_len % 3 == 0 ? 0 : 4;
		
		//
		BYTE*	output = new BYTE[length];
		memset(output, 0, length);

		//
		BYTE	temp[4] = {0};

		//
		BYTE*	input = (BYTE*)input_buf;
		for (size_t i = 0, n = 0;  i < input_len; i ++)
		{
			switch(i % 3)
			{
			case 0:
				{
					temp[0] = 0x00;
					temp[1] = 0x00;
					temp[3] = 0x00;
					temp[2] = 0x00;

					temp[1] = (input[i] & 0x03) << 0x04;
					temp[0] |= (input[i] >> 0x02);
					break;
				}
			case 1:
				{
					temp[2] = (input[i] & 0x0F) << 0x02;
					temp[1] |= (input[i] >> 0x04);
					break;
				}
			case 2:
				{
					temp[3] = (input[i] & 0x3F) << 0x00;
					temp[2] |= (input[i] >> 0x06); 
					break;
				}
			}

			if(i % 3 == 2 || i + 1 == input_len)
			{
				output[n + 0] = m_szCryptTable[temp[0] & 0x3F];
				output[n + 1] = m_szCryptTable[temp[1] & 0x3F];
				output[n + 2] = m_szCryptTable[temp[2] & 0x3F];
				output[n + 3] = m_szCryptTable[temp[3] & 0x3F];
				n += 4;
			}
		}

		// 補兩個符號
		if(input_len % 3 == 1)
		{
			output[length - 1] = '=';
			output[length - 2] = '=';
		}
		else if(input_len % 3 == 2)
		{
			output[length - 1] = '=';
		}
		else
		{
			//nothing
		}

		if(output_len){ *output_len = length; }
		if(output_buf){ *output_buf = output; }
		return length;
	}

	//
	size_t	Base64::Decrypt(void* input_buf, size_t input_len, __out void** output_buf, __out size_t* output_len)
	{
		// 四字節轉換為三字節
		if(input_len % 4 > 0){ return 0; }

		size_t length = input_len / 4 * 3;
		BYTE*	input = (BYTE*)input_buf;
		if(input[input_len - 1] ==  '='){ length -= 1; }
		if(input[input_len - 2] ==  '='){ length -= 1; }

		//
		BYTE*	output = new BYTE[length];
		memset(output, 0, length);
		
		//
		BYTE	temp[3] = {0};
		for (size_t i = 0, n = 0;  i < input_len; i ++)
		{
			int l = 0;
			if(input[i] == '=')
			{
				l = -1;
			}
			else
			{
				for ( ; l < BASE64_CRYPTTABLE_MAXLEN; l ++)
				{ if(m_szCryptTable[l] == input[i]){ break; } }
			}

			if( l >= 0 )
			{
				switch(i % 4)
				{
				case 0:
					{
						temp[0] = 0x00;
						temp[1] = 0x00;
						temp[2] = 0x00;

						temp[0] = (l << 0x02);
						break;
					}
				case 1:
					{
						temp[0] |= (l >> 0x04) & 0x03;
						temp[1] = (l & 0x0F) << 0x04;
						break;
					}
				case 2:
					{
						temp[1] |= (l >> 0x02) & 0x0F;
						temp[2] = (l & 0x03) << 0x06;
						break;
					}
				case 3:
					{
						temp[2] |= (l >> 0x00) & 0x3F;
						break;
					}
				}
			}

			if(i % 4 == 3)
			{
				for (int m = 0 ; m < 3 && n < length; m ++, n ++)
				{
					output[n] = temp[m] & 0xFF;
				}
			}
		}

		//
		if(output_len){ *output_len = length; }
		if(output_buf){ *output_buf = output; }
		return length;
	}
}

