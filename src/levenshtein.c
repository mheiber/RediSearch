#include <stdlib.h>
#include <stdio.h>
#include <sys/param.h>
#include <string.h>
#include "levenshtein.h"

typedef struct  {
	int idx, val;
}sparseVectorEntry;

// sparseVector is a crude implementation of a sparse vector for our needs
typedef struct {
    sparseVectorEntry *entries;
    size_t len;
    size_t cap;
} sparseVector; 


sparseVector newSparseVectorCap(size_t cap) {
    sparseVector v = {
        calloc(sizeof(sparseVectorEntry), cap),
        0, 
        cap
    };
    return v;
}
// newSparseVector creates a new sparse vector with the initial values of the dense int slice given to it
sparseVector newSparseVector(int *values, int len) {
	
    sparseVector v = {
        calloc(sizeof(sparseVectorEntry), len),
        len,
        len  
    };

	for (int i = 0; i < len; i++) {
		v.entries[i] = (sparseVectorEntry){i, values[i]};
	}

	return v;
}

// append appends another sparse vector entry with the given index and value. NOTE: We do not check
// that an entry with the same index is present in the vector
void sparseVector_append(sparseVector *v, int index, int value) {
    
    if (v->len == v->cap) {
        v->cap *= 2;
        v->entries = realloc(v->entries, v->cap * sizeof(sparseVectorEntry));
    }
    v->entries[v->len++] = (sparseVectorEntry){index, value};
}

void sparseVector_free(sparseVector *v) {
    free(v->entries);
    v->entries = NULL;
}

// SparseAutomaton is a naive Go implementation of a levenshtein automaton using sparse vectors, as described
// and implemented here: http://julesjacobs.github.io/2015/06/17/disqus-levenshtein-simple-and-fast.html
typedef struct   {
	const char *string;
    size_t len;
	int max;
} SparseAutomaton;

// NewSparseAutomaton creates a new automaton for the string s, with a given max edit distance check
SparseAutomaton NewSparseAutomaton(const char *s, size_t len, int maxEdits) {
	return (SparseAutomaton){ s, len, maxEdits };
}

// Start initializes the automaton's state vector and returns it for further iteration
sparseVector SparseAutomaton_Start(SparseAutomaton *a) {
    
	int vals[a->max+1];
	for (int i = 0; i < a->max+1; i++) {
		vals[i] = i;
	}

	return newSparseVector(vals, a->max+1);
}



// Step returns the next state of the automaton given a previous state and a character to check
sparseVector SparseAutomaton_Step(SparseAutomaton *a, sparseVector state, char c) {

	sparseVector newVec = newSparseVectorCap(state.len);

	if (state.len > 0 && state.entries[0].idx == 0 && state.entries[0].val < a->max) {
        sparseVector_append(&newVec, 0, state.entries[0].val+1);
	}
    
	for (int j = 0; j < state.len; j++) {
        
        sparseVectorEntry entry = state.entries[j];

		if (entry.idx == a->len) {
			break;
		}

		int cost = 0;
		if (a->string[entry.idx] != c) {
			cost = 1;
		}

		int val = state.entries[j].val + cost;

		if (newVec.len != 0 && newVec.entries[newVec.len - 1].idx == entry.idx) {
			val = MIN(val, newVec.entries[newVec.len-1].val+1);
		}

		if (state.len > j+1 && state.entries[j+1].idx == entry.idx+1) {
			val = MIN(val, state.entries[j+1].val+1);
		}

		if (val <= a->max) {
            sparseVector_append(&newVec, entry.idx+1, val);
		}

	}
	return newVec;
}

// IsMatch returns true if the current state vector represents a string that is within the max
// edit distance from the initial automaton string
int SparseAutomaton_IsMatch(SparseAutomaton *a, sparseVector v) {
	return v.len != 0 && v.entries[v.len-1].idx == a->len;
}

// CanMatch returns true if there is a possibility that feeding the automaton with more steps will
// yield a match. Once CanMatch is false there is no point in continuing iteration
int SparseAutomaton_CanMatch(SparseAutomaton *a, sparseVector v) {
	return v.len > 0;
}

// func (a *SparseAutomaton) Transitions(v sparseVector) []byte {

// 	set := map[byte]struct{}{}
// 	for _, entry := range v {

// 		if entry.idx < len(a.str) {
// 			set[a.str[entry.idx]] = struct{}{}
// 		}
// 	}

// 	ret := make([]byte, 0, len(set))
// 	for b, _ := range set {
// 		ret = append(ret, b)
// 	}

// 	return ret
// }




void TrieNode_Init(TrieNode *n, char c) {
    
    n->b = c;
    n->children = 0;
    n->numChildren = 0;
    n->terminal = 0;
    
}

inline TrieNode *TrieNode_Child(TrieNode *n, char c) {

	if (n->children == NULL) {
		return NULL;
	}
    for (int i = 0; i < n->numChildren; i++)  {
	
		if (n->children[i].b == c) {
			return &n->children[i];
		}
	}
	return NULL;
}

TrieNode *TrieNode_AddChild(TrieNode *n, char c) {

    n->children = realloc(n->children, (n->numChildren+1)*sizeof(TrieNode));
    TrieNode_Init(&n->children[n->numChildren], c);

	return &n->children[n->numChildren++];
}

///insert a new record into the index
void TrieNode_Add(TrieNode *n, char *key, size_t len) {

	TrieNode *current = n;
	if (n->b == key[0]) {
		key++;
        len--;
	}

	//find or create the node to put this record on
	for (int pos = 0; pos < len;  pos++) {

		TrieNode *next = TrieNode_Child(current, key[pos]);

		//we're iterating an existing node here
		if (next) {
			current = next;
		} else { //nothing for this prefix - create a new node
			current = TrieNode_AddChild(current, key[pos]);
		}

		if (pos == len-1) {
			current->terminal = 1;
		}
	}

}

typedef struct {
	sparseVector vec;
	char *str;
    size_t len;
	TrieNode *node;
}stackNode;

typedef struct {
    stackNode **nodes;
    size_t len;
    size_t cap;
} stack;

stackNode *stack_pop(stack *s) {
	//printf("stack size: %d\n", s->len);
    if (s->len == 0) {
        return NULL;
    }
    s->len--;
    return s->nodes[s->len];
}

void stack_push(stack *s, stackNode *node) {
    if (s->len == s->cap) {
        s->cap = s->cap ? s->cap*2 : 2;
        s->nodes = realloc(s->nodes, s->cap * sizeof(stackNode*));
    }
    s->nodes[s->len] = node;
	s->len++;
	
}

stackNode *newStackNode(sparseVector v, char *str, size_t len, TrieNode *tn) {
    stackNode *ret = malloc(sizeof(stackNode));
    ret->len = len;
    ret->str = str;
    ret->vec = v;
    ret->node = tn;
    return ret;
}

 

void TrieNode_Traverse(TrieNode *n, SparseAutomaton *a, sparseVector vec, char ***strs, size_t *nstrs) {

	// TODO: append to ret  

	stack st = { NULL,0,0 };
    
    stack_push(&st, newStackNode(vec,strdup(""), 0, n));
	

	stackNode *top = NULL;
	sparseVector newVec = vec;
	while (st.len > 0) {
		
		top = stack_pop(&st);
		
		n = top->node;
		if (n->b != 0) {
			newVec = SparseAutomaton_Step(a, top->vec, n->b);
		}
		// if this is a terminal node - just check if we have a match and add it to the results
		if (n->terminal && newVec.len > 0 && SparseAutomaton_IsMatch(a, newVec)) {
            printf("Found: %s%c\n", top->str,n->b);
			//ret = append(ret, top.str+string(n.b))
		}

		if (n->children && SparseAutomaton_CanMatch(a, newVec)) {
               
            int l = 0;
			if (n->b) {
                int l = strlen(top->str);
                top->str = realloc(top->str, l + 2);
				top->str[l] = n->b;
                top->str[l+1] = '\0';
			}
               
            for (int i = 0; i < n->numChildren; i++) {
                stack_push(&st, newStackNode(newVec, strdup(top->str), l + 2,&n->children[i]));
			}
		}
        
        free(top->str);

	}


}



// // Exists returns true if a string exists as it is in the trie
// func (t *Trie) Exists(s string) bool {

// 	current := t.root
// 	for i := 0; i < len(s); i++ {

// 		child := current.child(s[i])
// 		if child == nil {
// 			return false
// 		}
// 		current = child

// 	}

// 	return true
//}

// FuzzyMatches returns all the words in the trie that are with maxDist edit distance from s
void TrieNode_FuzzyMatches(TrieNode *root, char *s, size_t len, int maxDist) {
    
	SparseAutomaton a = NewSparseAutomaton(s, len, maxDist);

	sparseVector state =  SparseAutomaton_Start(&a);
	
    TrieNode_Traverse(root, &a, state, NULL, 0);
    

	//	for _, child := range t.root.children {

	//		matches := child.traverse(a, state)
	//		if len(matches) > 0 {
	//			ret = append(ret, matches...)
	//		}

	//	}
	//	return ret
}


