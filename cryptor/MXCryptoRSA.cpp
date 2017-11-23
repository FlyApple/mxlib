
#include "MXCryptoRSA.h"

//
namespace MXCryptor
{
	//
	RSABase::RSABase()
	{
		//
		m_nPublicKeyLength	= 0;
		m_pPublicKeyData	= NULL;
		m_nPrivateKeyLength = 0;
		m_pPrivateKeyData	= NULL;
	}

	RSABase::~RSABase(void)
	{
		//
		if(m_pPublicKeyData)
		{
			free(m_pPublicKeyData); 
			m_pPublicKeyData	= NULL;
			m_nPublicKeyLength	= 0;
		}

		if(m_pPrivateKeyData)
		{
			free(m_pPrivateKeyData);
			m_pPrivateKeyData	= NULL;
			m_nPrivateKeyLength	= 0;
		}
	}

	bool			RSABase::LoadKey(const wchar_t* filename, long flag)
	{
		char filenameA[0x0100] = {0};
		WideCharToMultiByte(CP_ACP, 0, filename, -1, filenameA, 0x100, NULL, NULL);
		return this->LoadKey(filenameA, flag);
	}

	bool			RSABase::LoadKey(const char* filename, long flag)
	{
		long	length		= 0;
		void*	buffer		= NULL;
		if(!ReadKeyFile(filename, &buffer, &length))
		{
			return false;
		}

		//fiter text
		char*	text		= (char*)malloc(length);
		long	total		= 0;
		memset(text, 0, length);

		bool wordwrap = false;
		for(int i = 0; i < length; i ++)
		{
			if(wordwrap == false && ((char*)buffer)[i] == '-' &&
				i + 1 < length && ((char*)buffer)[i + 1] == '-')
			{
				wordwrap = true; i += 1;
				continue;
			}

			if(wordwrap)
			{
				if(((char*)buffer)[i] == '\n'){ wordwrap = false; continue; }
				else{ continue; }
			}

			if(((char*)buffer)[i] == '\r' || ((char*)buffer)[i] == '\n')
			{ continue; }

			if(((char*)buffer)[i] == ' ' || ((char*)buffer)[i] == '\t')
			{ continue; }

			if(((char*)buffer)[i] == '0' && i + 1 < length && 
				(((char*)buffer)[i+1] == 'X'|| ((char*)buffer)[i+1] == 'x'))
			{ i += 1; continue; }

			text[total] = ((char*)buffer)[i]; total ++;
		}


		//parse text
		memset(buffer, 0, length); length = 0; 
		for(int i = 0; i < total; i += 3)
		{
			char temp[10] = {0};
			temp[0] = text[i + 0];
			temp[1] = text[i + 1];

			((char*)buffer)[length] = (unsigned char)strtol(temp, NULL, 16);
			length ++;
		}

		//
		if(!this->LoadKeyData(buffer, length, flag))
		{
			//		
			free(text);
			free(buffer);
			return false;
		}

		//		
		free(text);
		free(buffer);
		return true;
	}

	bool			RSABase::ReadKeyFile(const char* filename, void** output_data, long* output_length)
	{
		FILE*	fr	= NULL;
		fopen_s(&fr, filename, "rb");
		if(fr == NULL)
		{
			_tprintf_s(_T("<%s> Create file fail.\n"), _T(__FUNCTION__));
			return false;
		}

		fseek(fr, 0, SEEK_END);
		long	length		= ftell(fr);
		fseek(fr, 0, SEEK_SET);

		void*	buffer		= malloc(length);
		fread(buffer, length, 1, fr);
		fclose(fr);

		if(output_data){ *output_data = buffer; }
		if(output_length){ *output_length = length; }
		return true;
	}

	bool			RSABase::SaveKey(const wchar_t* filename, const wchar_t* prefix)
	{
		char filenameA[0x0100] = {0};
		WideCharToMultiByte(CP_ACP, 0, filename, -1, filenameA, 0x100, NULL, NULL);
		char prefixA[0x100] = {0};
		WideCharToMultiByte(CP_ACP, 0, prefix, -1, prefixA, 0x100, NULL, NULL);
		return this->SaveKey(filenameA, prefixA);
	}

	bool			RSABase::SaveKey(const char* filename, const char* prefix)
	{
		if(m_nPublicKeyLength > 0)
		{
			char	name[200]; sprintf_s(name, "%s_public.key", filename);

			FILE*	fw	= NULL;
			fopen_s(&fw, name, "wb");
			if(fw == NULL)
			{
				_tprintf_s(_T("<%s> Create file fail.\n"), _T(__FUNCTION__));
				return false;
			}

			fprintf_s(fw, "---- [Public Key](Begin)(Length %d) ---- \r\n", m_nPublicKeyLength);

			char one[100] = {0};
			for(int i = 0; i < m_nPublicKeyLength; i ++)
			{
				char temp[10];
				sprintf_s(temp, "%s%02X, ", prefix, m_pPublicKeyData[i]);
				strcat_s(one, temp);

				if(i % 0x10 == 0x0F || i + 1 == m_nPublicKeyLength)
				{
					fprintf_s(fw, "%s\r\n", one);
					one[0] = _T('\0');
				}
			}

			fprintf_s(fw, "---- [Public Key](End) ---- \r\n");

			fclose(fw);
		}

		//
		if(m_nPrivateKeyLength > 0)
		{
			char	name[200]; sprintf_s(name, "%s_private.key", filename);

			FILE*	fw	= NULL;
			fopen_s(&fw, name, "wb");
			if(fw == NULL)
			{
				_tprintf_s(_T("<%s> Create file fail.\n"), _T(__FUNCTION__));
				return false;
			}

			fprintf_s(fw, "---- [Private Key](Begin)(Length %d) ---- \r\n", m_nPrivateKeyLength);

			char one[100] = {0};
			for(int i = 0; i < m_nPrivateKeyLength; i ++)
			{
				char temp[10];
				sprintf_s(temp, "%s%02X, ", prefix, m_pPrivateKeyData[i]);
				strcat_s(one, temp);

				if(i % 0x10 == 0x0F || i + 1 == m_nPrivateKeyLength)
				{
					fprintf_s(fw, "%s\r\n", one);
					one[0] = _T('\0');
				}
			}

			fprintf_s(fw, "---- [Private Key](End) ---- \r\n");

			fclose(fw);
		}
		return true;
	}

	//
	RSAApi::RSAApi(ALG_ID alg)
		:	m_nAlgId(alg), m_hPrev(NULL), m_hHash(NULL), m_hKey(NULL)
	{
		//
		CryptSetProvider(MS_DEF_PROV, PROV_RSA_FULL);

		//
		if(!CryptAcquireContext(&m_hPrev, alg == AT_SIGNATURE ? NULL : "MXCryptor_RSA_Provider", MS_DEF_PROV, PROV_RSA_FULL, 
			alg == AT_KEYEXCHANGE ? CRYPT_NEWKEYSET : CRYPT_VERIFYCONTEXT))
		{
			if(GetLastError() == NTE_EXISTS)
			{
				if(!CryptAcquireContext(&m_hPrev, "MXCryptor_RSA_Provider", MS_DEF_PROV, PROV_RSA_FULL, 0))
				{
					_tprintf_s(_T("<%s> CryptAcquireContext Fail.\n"), _T(__FUNCTION__));
					return ;
				}
			}

		}
	}

	RSAApi::~RSAApi(void)
	{
		if(m_hKey)
		{
			CryptDestroyKey(m_hKey);
			m_hKey	= NULL;
		}
		if(m_hHash)
		{
			CryptDestroyHash(m_hHash);
			m_hHash	= NULL;
		}
		if(m_hPrev)
		{
			CryptReleaseContext(m_hPrev, 0);
			m_hPrev	= NULL;
		}
	}

	bool			RSAApi::GenKey(UINT bit)
	{
		if(!CryptGenKey(m_hPrev, m_nAlgId, bit| CRYPT_EXPORTABLE, &m_hKey))
		{
			_tprintf_s(_T("<%s> CryptGenKey Fail, Code:%08X.\n"), _T(__FUNCTION__), GetLastError());
			return false;
		}

		return this->ExportKey(RSA_KEY_PRIVATE);
	}

	bool			RSAApi::GenHashKey(const wchar_t* buffer, long length, long hash, long key)
	{
		char key_stringA[0x0100] = {0};
		WideCharToMultiByte(CP_ACP, 0, buffer, length, key_stringA, 0x100, NULL, NULL);

		return this->GenHashKey(key_stringA, strlen(key_stringA), hash, key);
	}

	bool			RSAApi::GenHashKey(const char* buffer, long length, long hash, long key)
	{
		HCRYPTHASH	hHash	= NULL;
		if(!CryptCreateHash(m_hPrev, hash, NULL, 0, &hHash))
		{
			_tprintf_s(_T("<%s> CryptCreateHash Fail, Code:%X.\n"), _T(__FUNCTION__), GetLastError());
			goto _FAIL;
		}

		if(!CryptHashData(hHash, (BYTE*)buffer, length, 0))
		{
			_tprintf_s(_T("<%s> CryptHashData Fail.\n"), _T(__FUNCTION__));
			goto _FAIL;
		}

		BYTE	HashBuf[32]	= {0};
		DWORD	HashLen		= sizeof(HashBuf);
		CryptGetHashParam(hHash, HP_HASHVAL, HashBuf, &HashLen, 0  );

		HCRYPTKEY	hKey	= NULL;
		if(!CryptDeriveKey(m_hPrev, key, hHash, 0, &hKey))
		{
			_tprintf_s(_T("<%s> CryptDeriveKey Fail.\n"), _T(__FUNCTION__));
			goto _FAIL;
		}


		//
		m_hKey	= hKey;
		if(hHash)
		{
			CryptDestroyHash(hHash);
			hHash	= NULL;
		}

		return true;
_FAIL:
		if(hHash)
		{
			CryptDestroyHash(hHash);
			hHash	= NULL;
		}
		return false;
	}

	bool			RSAApi::LoadKeyData(void* buffer, long length, long flag)
	{
		HCRYPTKEY  hKey		= NULL;
		if(flag == RSA_KEY_PUBLIC)
		{
			if(!CryptImportKey(m_hPrev, (BYTE*)buffer, length, NULL, CRYPT_EXPORTABLE, &hKey))
			{
				_tprintf_s(_T("<%s> CryptImportKey Fail. %08X\n"), _T(__FUNCTION__), GetLastError());
				return false;
			}
		}

		//
		else if(flag == RSA_KEY_PRIVATE)
		{
			if(!CryptImportKey(m_hPrev, (BYTE*)buffer, length, NULL, CRYPT_EXPORTABLE, &hKey))
			{
				_tprintf_s(_T("<%s> CryptImportKey Fail. %08X\n"), _T(__FUNCTION__), GetLastError());
				return false;
			}
		}

		//
		else
		{
			return false;
		}

		if(m_hKey){ CryptDestroyKey(m_hKey); m_hKey = NULL; }
		m_hKey	= hKey;

		//
		return this->ExportKey(flag);
	}

	bool			RSAApi::ExportKey(long flag)
	{
		DWORD key_length	= 0;
		CryptExportKey(m_hKey, NULL, PUBLICKEYBLOB, 0, NULL, &key_length);
		if(key_length > 0)
		{
			void*	key_data	= malloc(key_length);
			if(CryptExportKey(m_hKey, NULL, PUBLICKEYBLOB, 0, (BYTE*)key_data, &key_length))
			{
				if(m_pPublicKeyData)
				{
					free(m_pPublicKeyData); 
					m_pPublicKeyData	= NULL;
					m_nPublicKeyLength	= 0;
				}

				m_nPublicKeyLength	= key_length;
				m_pPublicKeyData	= (BYTE*)key_data;
			}
			else
			{
				_tprintf_s(_T("<%s> [Public Key]CryptExportKey Fail. %08X\n"), _T(__FUNCTION__), GetLastError());
				free(key_data);
				return false;
			}
		}
		else
		{
			_tprintf_s(_T("<%s> [Public Key]CryptExportKey Fail, Code:%08X.\n"), _T(__FUNCTION__), GetLastError());
			return false;
		}

		if(flag == RSA_KEY_PRIVATE)
		{
			key_length	= 0;
			CryptExportKey(m_hKey, NULL, PRIVATEKEYBLOB, 0, NULL, &key_length);
			if(key_length > 0)
			{
				void*	key_data	= malloc(key_length);
				if(CryptExportKey(m_hKey, NULL, PRIVATEKEYBLOB, 0, (BYTE*)key_data, &key_length))
				{
					if(m_pPrivateKeyData)
					{
						free(m_pPrivateKeyData);
						m_pPrivateKeyData	= NULL;
						m_nPrivateKeyLength	= 0;
					}

					m_nPrivateKeyLength	= key_length;
					m_pPrivateKeyData	= (BYTE*)key_data;
				}
				else
				{
					_tprintf_s(_T("<%s> [Private Key]CryptExportKey Fail. %08X\n"), _T(__FUNCTION__), GetLastError());
					free(key_data);
					return false;
				}
			}
			else
			{
				_tprintf_s(_T("<%s> [Private Key]CryptExportKey Fail, Code:%08X.\n"), _T(__FUNCTION__), GetLastError());
				if(GetLastError() == NTE_BAD_KEY)
				{ }
				return false;
			}
		}
		return true;
	}

	bool			RSAApi::Encrypt(void* pInputData, long nInputLenght, void** pOutputData, long* nOutputLenght, long nBlockLenght)
	{
		if(!pInputData || !nInputLenght){ return false; }

		LONG		nEncryptLenght		= 0;
		void*		pEncryptData		= NULL;


		BYTE		nBlockData[0x200]	= {0};
		for(int i = 0; i < nInputLenght; i += nBlockLenght)
		{
			long	nTempLenght			= i + nBlockLenght > nInputLenght ? 
				(nInputLenght - i) % nBlockLenght : nBlockLenght;

			long	nCryptBlock			= nTempLenght;	//长度返回为加密块长度
			if(!CryptEncrypt(m_hKey, NULL, TRUE, 0, NULL, (DWORD*)&nCryptBlock, nTempLenght))
			{
				_tprintf_s(_T("<%s> CryptEncrypt Fail, Code:%08X.\n"), _T(__FUNCTION__), GetLastError());

				if(pEncryptData){ free(pEncryptData); }
				return false;
			}

			//
			nCryptBlock = nCryptBlock ? nCryptBlock : nTempLenght;

			// 这里需要指定加密块长度，而字符串长度也是返回长度
			memcpy(nBlockData, (unsigned char*)pInputData + i, nTempLenght);
			if(!CryptEncrypt(m_hKey, NULL, TRUE, 0, nBlockData, (DWORD*)&nTempLenght, nCryptBlock))
			{
				_tprintf_s(_T("<%s> CryptEncrypt Fail, Code:%08X.\n"), _T(__FUNCTION__), GetLastError());

				if(pEncryptData){ free(pEncryptData); }
				return false;
			}

			//
			if(pEncryptData == NULL)
			{
				pEncryptData = malloc(nTempLenght);
			}
			else
			{
				void*	pOriginData = pEncryptData;
				pEncryptData = malloc(nEncryptLenght + nTempLenght);
				memcpy(pEncryptData, pOriginData, nEncryptLenght);
				free(pOriginData);
			}
			memcpy((unsigned char*)pEncryptData + nEncryptLenght, 
				nBlockData, nTempLenght);

			//
			nEncryptLenght += nTempLenght;
		}

		//
		if(pOutputData){ *pOutputData = pEncryptData; }
		else{ free(pEncryptData); }
		if(nOutputLenght){ *nOutputLenght = nEncryptLenght; }
		return true;
	}

	bool			RSAApi::Decrypt(void* pInputData, long nInputLenght, void** pOutputData, long* nOutputLenght, long nBlockLenght)
	{
		if(!pInputData || !nInputLenght){ return false; }

		LONG		nDecryptLenght		= 0;
		void*		pDecryptData		= NULL;

		BYTE		nBlockData[0x200]	= {0};
		for(int i = 0; i < nInputLenght; i += nBlockLenght)
		{
			long	nCryptBlock		= i + nBlockLenght > nInputLenght ? 
				(nInputLenght - i) % nBlockLenght : nBlockLenght;
			long	nTempLenght		= nCryptBlock;

			//
			memcpy(nBlockData, (unsigned char*)pInputData + i, nCryptBlock);
			if(!CryptDecrypt(m_hKey, NULL, TRUE, 0, nBlockData, (DWORD*)&nTempLenght))
			{
				_tprintf_s(_T("<%s> CryptDecrypt Fail, Code:%08X.\n"), _T(__FUNCTION__), GetLastError());
								
				if(pDecryptData){ free(pDecryptData); }
				return false;
			}

			//
			if(pDecryptData == NULL)
			{
				pDecryptData = malloc(nTempLenght);
			}
			else
			{
				void*	pOriginData = pDecryptData;
				pDecryptData = malloc(nDecryptLenght + nTempLenght);
				memcpy(pDecryptData, pOriginData, nDecryptLenght);
				free(pOriginData);
			}
			memcpy((unsigned char*)pDecryptData + nDecryptLenght, 
				nBlockData, nTempLenght);

			//
			nDecryptLenght += nTempLenght;
		}

		//
		if(pOutputData){ *pOutputData = pDecryptData; }
		else{ free(pDecryptData); }
		if(nOutputLenght){ *nOutputLenght = nDecryptLenght; }
		return true;
	}


	bool			RSAApi::Signature(void* pInputData, long nInputLenght, void** pOutputData, long* nOutputLenght)
	{
		if(!pInputData || !nInputLenght){ return false; }

		HCRYPTHASH	hHash	= NULL;
		if(!CryptCreateHash(m_hPrev, CALG_MD5, NULL, 0, &hHash))
		{
			_tprintf_s(_T("<%s> CryptCreateHash Fail, Code:%X.\n"), _T(__FUNCTION__), GetLastError());
			goto _FAIL;
		}

		if(!CryptHashData(hHash, (BYTE*)pInputData, nInputLenght, 0))
		{
			_tprintf_s(_T("<%s> CryptHashData Fail.\n"), _T(__FUNCTION__));
			goto _FAIL;
		}

		BYTE	HashBuf[32]	= {0};
		DWORD	HashLen		= sizeof(HashBuf);
		CryptGetHashParam(hHash, HP_HASHVAL, HashBuf, &HashLen, 0);

		LONG		nSignLength		= 0;
		void*		pSignData		= NULL;
		if(!CryptSignHash(hHash, m_nAlgId, NULL, 0, NULL, (DWORD*)&nSignLength))
		{
			_tprintf_s(_T("<%s> CryptSignHash Fail, Code:%X.\n"), _T(__FUNCTION__), GetLastError());

			if(GetLastError() == NTE_BAD_KEYSET){ }
			goto _FAIL;
		}

		pSignData	= malloc(nSignLength);
		if(!CryptSignHash(hHash, m_nAlgId, NULL, 0, (BYTE*)pSignData, (DWORD*)&nSignLength))
		{
			_tprintf_s(_T("<%s> CryptSignHash Fail, Code:%X.\n"), _T(__FUNCTION__), GetLastError());

			free(pSignData);
			goto _FAIL;
		}

		//
		if(pOutputData){ *pOutputData = pSignData; }
		else{ free(pSignData); }
		if(nOutputLenght){ *nOutputLenght = nSignLength; }

		//
		if(hHash)
		{
			CryptDestroyHash(hHash);
			hHash	= NULL;
		}
		return true;
_FAIL:
		if(hHash)
		{
			CryptDestroyHash(hHash);
			hHash	= NULL;
		}
		return false;
	}

	bool			RSAApi::Verify(void* pInputData, long nInputLenght, void* pSignData, long nSignLenght)
	{
		if(!pInputData || !nInputLenght){ return false; }

		HCRYPTHASH	hHash	= NULL;
		if(!CryptCreateHash(m_hPrev, CALG_MD5, NULL, 0, &hHash))
		{
			_tprintf_s(_T("<%s> CryptCreateHash Fail, Code:%X.\n"), _T(__FUNCTION__), GetLastError());
			goto _FAIL;
		}

		if(!CryptHashData(hHash, (BYTE*)pInputData, nInputLenght, 0))
		{
			_tprintf_s(_T("<%s> CryptHashData Fail.\n"), _T(__FUNCTION__));
			goto _FAIL;
		}

		BYTE	HashBuf[32]	= {0};
		DWORD	HashLen		= sizeof(HashBuf);
		CryptGetHashParam(hHash, HP_HASHVAL, HashBuf, &HashLen, 0);

		if(!CryptVerifySignature(hHash, (BYTE*)pSignData, nSignLenght, m_hKey, NULL, 0))
		{
			switch(GetLastError())
			{
			case ERROR_INVALID_HANDLE:
				{
					_tprintf_s(_T("<%s> CryptVerifySignature ERROR: ERROR_INVALID_HANDLE.\n"), _T(__FUNCTION__));
					break;
				}
			case ERROR_INVALID_PARAMETER:
				{
					_tprintf_s(_T("<%s> CryptVerifySignature ERROR: ERROR_INVALID_PARAMETER.\n"), _T(__FUNCTION__));
					break;
				}
			case NTE_BAD_FLAGS:
				{
					_tprintf_s(_T("<%s> CryptVerifySignature ERROR: NTE_BAD_FLAGS.\n"), _T(__FUNCTION__));
					break;
				}
			case NTE_BAD_HASH:
				{
					_tprintf_s(_T("<%s> CryptVerifySignature ERROR: NTE_BAD_HASH.\n"), _T(__FUNCTION__));
					break;
				}
			case NTE_BAD_KEY:
				{
					_tprintf_s(_T("<%s> CryptVerifySignature ERROR: NTE_BAD_KEY.\n"), _T(__FUNCTION__));
					break;
				}
			case NTE_BAD_SIGNATURE:
				{
					_tprintf_s(_T("<%s> CryptVerifySignature ERROR: NTE_BAD_SIGNATURE.\n"), _T(__FUNCTION__));
					break;
				}
			default:
				{		
					_tprintf_s(_T("<%s> CryptVerifySignature ERROR: %08X\n"), _T(__FUNCTION__), GetLastError());
					break;
				}
			}
			goto _FAIL;
		}

		//
		if(hHash)
		{
			CryptDestroyHash(hHash);
			hHash	= NULL;
		}
		return true;

_FAIL:
		if(hHash)
		{
			CryptDestroyHash(hHash);
			hHash	= NULL;
		}
		return false;
	}


	//
	RSACert::RSACert(bool ignore_time, bool ignore_trust, bool ignore_link)
		:	m_bIgnoreTime(ignore_time),
			m_bIgnoreTrust(ignore_trust),
			m_bIgnoreLink(ignore_link)
	{
		m_hCertStore = NULL;
	}

	RSACert::~RSACert()
	{
		this->ClearStore();
	}

	void			RSACert::ClearStore()
	{
		m_mapCertContextByName.clear();

		for(std::vector<PCCERT_CONTEXT>::iterator iter = m_vCertContextList.begin();
			iter != m_vCertContextList.end(); iter ++)
		{
			if(*iter != NULL)
			{
				CertFreeCertificateContext(*iter);
				*iter = NULL;
			}
		}

		if(m_hCertStore)
		{
			CertCloseStore(m_hCertStore, CERT_CLOSE_STORE_CHECK_FLAG);
			m_hCertStore = NULL;
		}
	}

	bool			RSACert::LoadFromPFX(const wchar_t* filename, const wchar_t* password)
	{
		char filenameA[0x0100] = {0};
		WideCharToMultiByte(CP_ACP, 0, filename, -1, filenameA, 0x100, NULL, NULL);
		return this->LoadFromPFX(filenameA, password);
	}

	bool			RSACert::LoadFromPFX(const char* filename, const wchar_t* password)
	{
		long	length		= 0;
		void*	buffer		= NULL;
		if(!ReadKeyFile(filename, &buffer, &length))
		{
			return false;
		}

		CRYPT_DATA_BLOB	blob;
		blob.cbData = length;
		blob.pbData = (BYTE*)buffer;
		if(!PFXIsPFXBlob((CRYPT_DATA_BLOB*)&blob))
		{ 			
			free(buffer);
			return false; 
		}

		m_hCertStore = PFXImportCertStore((CRYPT_DATA_BLOB*)&blob, (LPCWSTR)password, CRYPT_USER_KEYSET| CRYPT_EXPORTABLE);
		if(m_hCertStore == NULL)
		{
			_tprintf_s(_T("<%s> PFXImportCertStore fail, Code:%X.\n"), _T(__FUNCTION__), GetLastError());
								
			free(buffer);
			return false; 
		}
		
		PCCERT_CONTEXT pCertContext = NULL;
		while((pCertContext = CertEnumCertificatesInStore(m_hCertStore, pCertContext)) != NULL)
		{
			if(VerifyContext(pCertContext, m_bIgnoreTime, m_bIgnoreTrust, m_bIgnoreLink))
			{
				m_vCertContextList.push_back(pCertContext);

				//
				DWORD	nNameType	= 0;
				TCHAR	szName[512] = {0};
				CertGetNameString(pCertContext, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, &nNameType, szName, 500);
				m_mapCertContextByName.insert(std::pair<std::basic_string<TCHAR>, PCCERT_CONTEXT>(szName, pCertContext));
			}
		}
		
		free(buffer);
		return !m_vCertContextList.empty();
	}

	bool			RSACert::VerifyContext(PCCERT_CONTEXT context, bool ignore_time, bool ignore_trust, bool ignore_link)
	{	
		//中间证书颁发机构
		HCERTSTORE hCACertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, CERT_SYSTEM_STORE_CURRENT_USER, L"CA");
		//个人证书
		HCERTSTORE hMYCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, CERT_SYSTEM_STORE_CURRENT_USER, L"MY");
		//受信任的根证书
		HCERTSTORE hROOTCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, CERT_SYSTEM_STORE_CURRENT_USER, L"ROOT");

		HCERTSTORE store_list[RSACertStoreIndex_MAX] = 
		{
			hROOTCertStore,
			hCACertStore,
			hMYCertStore,
			m_hCertStore
		};
		int store_count = RSACertStoreIndex_MAX;

		bool result = this->VerifyContextExt(store_list, store_count, context, ignore_time, ignore_trust, ignore_link);

		for(int i = 0; i < store_count; i ++)
		{
			if(store_list[i] != NULL && store_list[i] != m_hCertStore)
			{
				CertCloseStore(store_list[i], CERT_CLOSE_STORE_CHECK_FLAG);
				store_list[i] = NULL;
			}
		}

		return result;
	}

	bool	RSACert::VerifyContextExt(HCERTSTORE* store_list, int store_count, PCCERT_CONTEXT context, bool ignore_time, bool ignore_trust, bool ignore_link)
	{
		//
		DWORD	nNameType	= 0;
		TCHAR	szName[512] = {0};
		CertGetNameString(context, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, &nNameType, szName, 500);
		_tprintf_s(_T("<%s> Enum cert : Name : %s\n"), _T(__FUNCTION__), szName);

		nNameType = 0;
		TCHAR	szIssuer[256] = {0};
		CertGetNameString(context, CERT_NAME_SIMPLE_DISPLAY_TYPE, CERT_NAME_ISSUER_FLAG, &nNameType, szName, 500);
		_tprintf_s(_T("<%s> Enum cert : Root Name : %s\n"), _T(__FUNCTION__), szName);

		SYSTEMTIME sysTime;
		memset(&sysTime, 0, sizeof(sysTime));
		FileTimeToSystemTime(&context->pCertInfo->NotAfter, &sysTime);
		_tprintf_s(_T("<%s> Enum cert : Date End : %04d-%02d-%02d\n"), _T(__FUNCTION__), sysTime.wYear, sysTime.wMonth, sysTime.wDay);

		//认证证书和校验时间
		LONG result = 0;
		if((result = CertVerifyTimeValidity(NULL, context->pCertInfo)) != 0)
		{ 
			_tprintf_s(_T("<%s> Enum cert : Check Time : invalid\n"), _T(__FUNCTION__)); 
			
			// 如果不忽略时间校验,过期证书将被定为无效证书
			if(result < 0){ return false; }
			else if(result > 0 && !ignore_time){ return false; }
		}

		//验证证书链
		int store_index = -1;
		PCCERT_CONTEXT context_issuer = NULL;
		for(int i = 0; i < store_count; i ++)
		{
			if(store_list[i] != NULL)
			{		
				DWORD flags = CERT_STORE_SIGNATURE_FLAG;
				context_issuer = CertGetIssuerCertificateFromStore(store_list[i], context, NULL, &flags);
				if(context_issuer != NULL)
				{
					store_index = i;
					break;
				}
				else
				{
					DWORD error = GetLastError();
					if(error == CRYPT_E_SELF_SIGNED)
					{
						store_index = i;
						break;
					}
					else if(error == CRYPT_E_NOT_FOUND)
					{
						//nothing
					}
					else
					{
						break;
					}
				}
			}
		}
		
		if(context_issuer == NULL)
		{	
			if(store_index < 0) //根证书不存在
			{
				if(!ignore_link) //忽略证书链,就直接返回成功
				{ 
					_tprintf_s(_T("<%s> Enum cert : Check Link : fail\n"), _T(__FUNCTION__)); 
					return false;
				}
			}

		}
		else
		{
			DWORD	flags = CERT_STORE_SIGNATURE_FLAG;
			if(!CertVerifySubjectCertificateContext(context, context_issuer, &flags))
			{
				//证书并非来自根证书
				return false;
			}

			//继续接下来的根证书递归
			if(!this->VerifyContextExt(store_list, store_count, context_issuer, ignore_time, ignore_trust, ignore_link))
			{ return false; }

			if(!ignore_link && store_index >= 0)
			{
				if(!ignore_trust && store_index != RSACertStoreIndex_ROOT) //根证书并没有在受信任的颁发机构中
				{
					_tprintf_s(_T("<%s> Enum cert : Check Trust : no\n"), _T(__FUNCTION__)); 
					return false;
				}
			}
		}

		//
		return true;
	}

	//
	RSATrust::RSATrust(bool ignore_time, bool ignore_trust, bool ignore_link)
		: RSACert(ignore_time, ignore_trust, ignore_link)
	{
	}

	RSATrust::~RSATrust()
	{
	}

	bool	RSATrust::VerifyFile(const wchar_t* filename)
	{
		char filenameA[0x0100] = {0};
		WideCharToMultiByte(CP_ACP, 0, filename, -1, filenameA, 0x100, NULL, NULL);
		return this->VerifyFile(filenameA);
	}

	bool	RSATrust::VerifyFile(const char* filename)
	{
		long	length		= 0;
		void*	buffer		= NULL;
		if(!ReadKeyFile(filename, &buffer, &length))
		{
			return false;
		}
		
		CERT_BLOB	blob;
		blob.cbData = length;
		blob.pbData = (BYTE*)buffer;

		DWORD nEncoding, nContentType, nFormatType; 
		HCERTSTORE	hCertStore	= NULL;
		HCRYPTMSG	hCryptMsg	= NULL;
		if(!CryptQueryObject(CERT_QUERY_OBJECT_BLOB, &blob, 
			CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED,
			CERT_QUERY_FORMAT_FLAG_BINARY,
			0, &nEncoding, &nContentType, &nFormatType, &hCertStore, &hCryptMsg, NULL))
		{
			DWORD error = GetLastError();
			if(error == CRYPT_E_NOT_FOUND || error == CRYPT_E_NO_MATCH)
			{
				_tprintf_s(_T("<%s> Verify : Not found.\n"), _T(__FUNCTION__)); 
			}
			else
			{
				_tprintf_s(_T("<%s> Verify : Error Code %08X.\n"), _T(__FUNCTION__), error); 
			}
			free(buffer);
			return false;
		}
		
		// 枚举全部证书,并校验.
		m_hCertStore = hCertStore;

		//
		DWORD nSignerInfoCount = 0;
		if(!CryptMsgGetParam(hCryptMsg, CMSG_SIGNER_COUNT_PARAM, 0, NULL, &nSignerInfoCount))
		{
			free(buffer);

			CryptMsgClose(hCryptMsg);
			return false;
		}

		std::vector<std::vector<unsigned char>> SignerInfoList;
		for(int i = 0; i < (int)nSignerInfoCount; i ++)
		{
			DWORD nSignerInfoLength = 0;
			if(!CryptMsgGetParam(hCryptMsg, CMSG_SIGNER_INFO_PARAM, i, NULL, &nSignerInfoLength))
			{
				break;
			}
			
			SignerInfoList.resize(i + 1);
			SignerInfoList[i].resize(nSignerInfoLength, 0);
			PCMSG_SIGNER_INFO pSignerInfo = (PCMSG_SIGNER_INFO)SignerInfoList[i].data();
			if(!CryptMsgGetParam(hCryptMsg, CMSG_SIGNER_INFO_PARAM, i, pSignerInfo, &nSignerInfoLength))
			{
				SignerInfoList.pop_back();
				break;
			}
		}

		// 没有证书
		if(SignerInfoList.empty())
		{ 
			CryptMsgClose(hCryptMsg);

			free(buffer);
			return false; 
		}

		//
		if(!this->VerifySignInfo(SignerInfoList))
		{ 
			CryptMsgClose(hCryptMsg);

			free(buffer);
			return false; 
		}
		
		free(buffer);
		return true;
	}

	bool	RSATrust::VerifySignInfo(std::vector<std::vector<unsigned char>>& sign_list)
	{
		bool result = false;
		for(int i = 0; i < (int)sign_list.size(); i ++)
		{
			PCMSG_SIGNER_INFO pSignerInfo = (PCMSG_SIGNER_INFO)sign_list[i].data();

			CERT_INFO		cert_info; 
			ZeroMemory(&cert_info, sizeof(CERT_INFO));
			cert_info.Issuer		= pSignerInfo->Issuer;  
			cert_info.SerialNumber	= pSignerInfo->SerialNumber;  
					
			//
			TCHAR	szName[512] = {0};
			CertNameToStr(PKCS_7_ASN_ENCODING | X509_ASN_ENCODING, 
				&cert_info.Issuer, CERT_X500_NAME_STR, szName, 500);

			// 
			PCCERT_CONTEXT pCertContext = CertFindCertificateInStore(m_hCertStore, 
				PKCS_7_ASN_ENCODING| X509_ASN_ENCODING, 0, CERT_FIND_SUBJECT_CERT, &cert_info, NULL);
			if(pCertContext != NULL)
			{
				if(!this->VerifyContext(pCertContext, m_bIgnoreTime, m_bIgnoreTrust, m_bIgnoreLink))
				{
					_tprintf_s(_T("<%s> Check sign info : invalid\n"), _T(__FUNCTION__)); 
				}
				else 
				{
					// 校验证书成功,校验文件.
					if(result == false)
					{
						result = this->VerifySignInfo(pSignerInfo, pCertContext);
					}
				}
			}
		}
		return result;
	}

	bool	RSATrust::VerifySignInfo(PCMSG_SIGNER_INFO info, PCCERT_CONTEXT context)
	{

		return true;
	}
};
