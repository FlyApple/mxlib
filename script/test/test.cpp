// test.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include <LoadScript.h>

//
int _tmain(int argc, _TCHAR* argv[])
{
	MX::LoadScriptT<MX::LoadScriptFromFile>	script;
	if(script.Load("../data/text/JiNeng.txt"))
	{
		MX::LoadScriptCacheLineTH lines;
		while (script.Next(lines))
		{
			MX::LoadScriptCacheLinePtr<MX::LoadScriptCacheLineTH> cache(lines);

			printf_s("%s %s\r\n", (const char*)cache[0], (const char*)cache[2]);

			lines ++;
		}
	}

	return 0;
}

