#include "RyanJsonFuzzer.h"

RyanJsonBool_e isEnableRandomMemFail = RyanJsonTrue;

int LLVMFuzzerTestOneInput(const char *data, uint32_t size)
{

	// for (int i = 0; i < size; i++) { printf("%c", size, data[i]); }
	// printf("\r\n");

	RyanJsonInitHooks(NULL, RyanJsonFuzzerFree, RyanJsonFuzzerRealloc);
	RyanJsonInitHooks(RyanJsonFuzzerMalloc, NULL, RyanJsonFuzzerRealloc);
	RyanJsonInitHooks(RyanJsonFuzzerMalloc, RyanJsonFuzzerFree, NULL);
	RyanJsonInitHooks(NULL, NULL, NULL);

	RyanJsonInitHooks(RyanJsonFuzzerMalloc, RyanJsonFuzzerFree, 0 != size % 2 ? NULL : RyanJsonFuzzerRealloc);

	RyanJsonAssert(NULL == RyanJsonParseOptions(NULL, 100, RyanJsonFalse, NULL));
	RyanJsonAssert(NULL == RyanJsonParseOptions(data, 0, RyanJsonFalse, NULL));

	const char *parseEndPtr = NULL;
	RyanJson_t pJson = RyanJsonParseOptions(data, size, 0 != size % 3 ? RyanJsonTrue : RyanJsonFalse, &parseEndPtr);
	if (NULL != pJson)
	{
		assert(NULL != parseEndPtr && parseEndPtr - data <= size);

		{
			isEnableRandomMemFail = RyanJsonFalse;
			RyanJson_t pJson2 = RyanJsonDuplicate(pJson);
			isEnableRandomMemFail = RyanJsonTrue;
			RyanJsonCheckCode(RyanJsonFuzzerTestDelete(pJson2, size), {
				RyanJsonDelete(pJson2);
				goto exit__;
			});
			RyanJsonDelete(pJson2);
		}

		{
			isEnableRandomMemFail = RyanJsonFalse;
			RyanJson_t pJson2 = RyanJsonDuplicate(pJson);
			isEnableRandomMemFail = RyanJsonTrue;
			RyanJsonCheckCode(RyanJsonFuzzerTestDetach(pJson2, size), {
				RyanJsonDelete(pJson2);
				goto exit__;
			});
			RyanJsonDelete(pJson2);
		}

		RyanJsonFuzzerTestMinify(data, size);
		RyanJsonFuzzerTestParse(pJson, data, size);
		RyanJsonFuzzerTestGet(pJson, size);

		RyanJsonFuzzerTestDuplicate(pJson);
		RyanJsonCheckCode(RyanJsonFuzzerTestModify(pJson, size), { goto exit__; });
		RyanJsonCheckCode(RyanJsonFuzzerTestCreate(pJson, size), { goto exit__; });
		RyanJsonCheckCode(RyanJsonFuzzerTestReplace(pJson, size), { goto exit__; });

		RyanJsonDelete(pJson);
	}

	return 0;

exit__:
	RyanJsonDelete(pJson);
	return 0;
}
