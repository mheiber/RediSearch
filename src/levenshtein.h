#ifndef __LEVENSHTEIN_H__
#define __LEVENSHTEIN_H__

#include <stdlib.h>


typedef struct trieNode {
	char b;
	struct trieNode *children;
    size_t numChildren;
	char terminal;
} TrieNode;

void TrieNode_Init(TrieNode *n, char c);
void TrieNode_Add(TrieNode *n, char *key, size_t len);
void TrieNode_FuzzyMatches(TrieNode *root, char *s, size_t len, int maxDist);

#endif