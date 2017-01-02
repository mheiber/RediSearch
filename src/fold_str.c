#include "fold_str.h"

void FoldStr(const char *str, char **foldedStr, size_t *newLen) {
	char *foldedBuffer = malloc(sizeof(str));
	char *folded = foldedBuffer;
	const char *inputChar  = str;

	size_t len = 0;
	while (*inputChar  != 0) {
		uint32_t unicodeIn = 0;
		inputChar  = nu_utf8_read(inputChar, &unicodeIn);

		// nu_to_fold may return more than one codepoint
		const char *nextChar = nu_tofold(unicodeIn);

		if (nextChar == 0) {
			len++;
			folded = nu_utf8_write(unicodeIn, folded);
			continue;
		}

		uint32_t foldedUnicodeIn = 0;
		do {
			nextChar = nu_casemap_read(nextChar, &foldedUnicodeIn);
			if (foldedUnicodeIn == 0) break;
			len++;
			folded = nu_utf8_write(foldedUnicodeIn, folded);
		}
		while (foldedUnicodeIn != 0);
	}
	*newLen = len;
	*foldedStr = foldedBuffer;
}


void FoldRedisModuleString(RedisModuleCtx *ctx, RedisModuleString *redisStr, char **foldedStr, size_t *newLen) {
	const char *str = RedisModule_StringPtrLen(redisStr, NULL);

	size_t foldedStrLen;
	FoldStr(str, foldedStr, &foldedStrLen);

}
