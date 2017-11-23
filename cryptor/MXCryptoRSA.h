#ifndef __MX_CRYPTOR_RSA_H__
#define __MX_CRYPTOR_RSA_H__


#pragma once

//
#include <windows.h>
#include <wincrypt.h>
#include <stdio.h>
#include <tchar.h>

#include <vector>
#include <map>
#include <string>

//
namespace MXCryptor
{
	//
	#define RSA_KEY_PRIVATE		0x00
	#define RSA_KEY_PUBLIC		0x01

	//
	#ifndef RSA0256BIT_KEY
		#define RSA0256BIT_KEY	0x01000000L
	#endif //RSA256BIT_KEY
	#ifndef RSA0512BIT_KEY
		#define RSA0512BIT_KEY	0x02000000L
	#endif //RSA512BIT_KEY
	#ifndef RSA1024BIT_KEY
		#define RSA1024BIT_KEY	0x04000000L
	#endif //RSA1024BIT_KEY
	#ifndef RSA2048BIT_KEY
		#define RSA2048BIT_KEY	0x08000000L
	#endif //RSA2048BIT_KEY

	// RSA是暫時無法和WINAPI的RSA共用KEY的
	// RSA是暫時無法進行簽名和認證簽名的
	class RSABase
	{
	public:
		RSABase(void);
		virtual ~RSABase(void);
		
		virtual bool			GenKey(UINT bit = RSA1024BIT_KEY) = 0;

		bool			LoadKey(const wchar_t* filename, long flag = RSA_KEY_PUBLIC);
		bool			LoadKey(const char* filename, long flag = RSA_KEY_PUBLIC);


		bool			SaveKey(const wchar_t* filename, const wchar_t* prefix = L"");
		bool			SaveKey(const char* filename, const char* prefix = "");

	protected:
		virtual bool			ReadKeyFile(const char* filename, void** output_data, long* output_length);
		virtual bool			LoadKeyData(void* buffer, long length, long flag = RSA_KEY_PUBLIC) = 0;

	protected:

		//
		BYTE*		m_pPublicKeyData;
		LONG		m_nPublicKeyLength;
		BYTE*		m_pPrivateKeyData;
		LONG		m_nPrivateKeyLength;
	};

	// 微软RSA加密为公开是公有密钥，解密是私有密钥。
	// 只能用公有密钥加密,如:1024位key,加密块长度为0x20，解密块长度为0x80
	// 加密解密用AT_KEYEXCHANGE，不能与AT_SIGNATURE混合
	// 需要两组密钥，来完成加密和认证。
	// HASH加密解密支持算法
	// 支持RC2,RC4,DES
	// 内存释放需要用free
	class RSAApi : public RSABase
	{
	public:
		RSAApi(ALG_ID alg = AT_KEYEXCHANGE);
		virtual ~RSAApi(void);

		bool			GenKey(UINT bit = RSA1024BIT_KEY);
		bool			GenHashKey(const wchar_t* buffer, long length, long hash = CALG_MD5, long key = CALG_RC4);
		bool			GenHashKey(const char* buffer, long length, long hash = CALG_MD5, long key = CALG_RC4);

		bool			Encrypt(void* pInputData, long nInputLenght, void** pOutputData, long* nOutputLenght, long nBlockLenght = 0x20);
		bool			Decrypt(void* pInputData, long nInputLenght, void** pOutputData, long* nOutputLenght, long nBlockLenght = 0x20);

		bool			Signature(void* pInputData, long nInputLenght, void** pOutputData, long* nOutputLenght);
		bool			Verify(void* pInputData, long nInputLenght, void* pSignData, long nSignLenght);

	protected:
		bool			LoadKeyData(void* buffer, long length, long flag = RSA_KEY_PUBLIC);
		bool			ExportKey(long flag);

	private:
		ALG_ID			m_nAlgId;

		//
		HCRYPTPROV		m_hPrev;
		HCRYPTHASH		m_hHash;
		HCRYPTKEY		m_hKey;
	};

	//
	enum RSACertStoreIndex
	{
		RSACertStoreIndex_ROOT,
		RSACertStoreIndex_CA,
		RSACertStoreIndex_MY,
		RSACertStoreIndex_LOCAL,
		RSACertStoreIndex_MAX
	};

	//
	class RSACert : public RSAApi
	{
	public:
		RSACert(bool ignore_time = false, bool ignore_trust = false, bool ignore_link = false);
		virtual ~RSACert();
		  
		void			ClearStore();

		bool			LoadFromPFX(const wchar_t* filename, const wchar_t* password);
		bool			LoadFromPFX(const char* filename, const wchar_t* password);

	protected:
		bool			VerifyContext(PCCERT_CONTEXT context, bool ignore_time = false, 
									bool ignore_trust = false, bool ignore_link = false);
		bool			VerifyContextExt(HCERTSTORE* store_list, int store_count, PCCERT_CONTEXT context, bool ignore_time = false, 
									bool ignore_trust = false, bool ignore_link = false);

	protected:
		bool			m_bIgnoreTime;
		bool			m_bIgnoreTrust;
		bool			m_bIgnoreLink;

		HCERTSTORE					m_hCertStore;
		std::vector<PCCERT_CONTEXT>	m_vCertContextList;
		std::map<std::basic_string<TCHAR>, PCCERT_CONTEXT>	m_mapCertContextByName;
	};

	//
	class RSATrust : public RSACert
	{
	public:
		RSATrust(bool ignore_time = false, bool ignore_trust = false, bool ignore_link = false);
		virtual ~RSATrust();

		bool			VerifyFile(const wchar_t* filename);
		bool			VerifyFile(const char* filename);

	protected:
		bool			VerifySignInfo(std::vector<std::vector<unsigned char>>& sign_list);
		bool			VerifySignInfo(PCMSG_SIGNER_INFO info, PCCERT_CONTEXT context);
	};
};

#endif //__MX_CRYPTOR_RSA_H__