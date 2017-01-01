#include "dep/libnu/libnu.h"
// #include "fold_str.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

void FoldStr(char *str, char **foldedStr, uint32_t *newLen) {
	char *foldedBuffer = malloc(sizeof(str));
	char *folded = foldedBuffer;
	const char *inputChar  = str;

	uint32_t len = 0;
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

int main(void) {
	char *output;
	uint32_t *len;
	FoldStr("straßßE", &output, len);
	printf("%s %" PRIu32 "\n", output, *len);
}