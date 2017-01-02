#include <stdlib.h>
#include "dep/libnu/libnu.h"
#include "redismodule.h"

void FoldRedisModuleString(RedisModuleCtx *ctx, RedisModuleString *redisStr, char **foldedStr, size_t *newLen);
