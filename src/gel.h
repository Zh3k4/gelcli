#ifndef GEL_H_
#define GEL_H_

#include "jsmn.h"
#include "typedef.h"

struct mem {
	usize size;
	char *data;
};

struct GelCtx {
	int ntok;
	struct mem json;
	jsmntok_t *tokens;
};

struct GelResult {
	int ok;
	union {
		const char *err;
		struct GelCtx ctx;
		struct GelPost *post;
		struct mem mem;
	} as;
};

struct GelResult gel_create(const char *const key, const char *const tags);
void gel_destroy(struct GelCtx c);

/* If returns 0 - failure,
 * If returns 1 - success,
 * If returns 2 - file exists */
int gel_post_download(struct GelPost p);

struct GelResult gel_post_get(struct GelCtx c);

#endif /* GEL_H_ */
