#include "dep/libnu/libnu.h"
#include "fold_str.h"

void FoldStr(char *str, char **foldedStr) {
	char *foldedBuffer = malloc(sizeof(str));
	char *folded = foldedBuffer;
	const char *inputChar  = str;
	
	while (*inputChar  != 0) {
		uint32_t unicodeIn = 0;
		inputChar  = nu_utf8_read(inputChar, &unicodeIn);

		// nu_to_fold may return more than one codepoint
		const char *nextChar = nu_tofold(unicodeIn);

		if (nextChar == 0) {
			folded = nu_utf8_write(unicodeIn, folded);
			break;
		}

		uint32_t foldedUnicodeIn = 0;
		do {
			nextChar = nu_casemap_read(nextChar, &foldedUnicodeIn);
			if (foldedUnicodeIn == 0) break;
			folded = nu_utf8_write(foldedUnicodeIn, folded);
		}
		while (foldedUnicodeIn != 0);
	}

	*foldedStr = foldedBuffer;
}
