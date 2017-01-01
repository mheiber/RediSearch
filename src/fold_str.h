#include <stdlib.h>
#include "dep/libnu/libnu.h"
#include "redismodule.h"

void FoldRedisModuleString(RedisModuleCtx *ctx, RedisModuleString *redisStr, RedisModuleString **foldedRedisStr, size_t *newLen);
