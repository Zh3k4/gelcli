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

struct GelPost {
	const char *url;
	int urlLen;
	const char *filename;
	int filenameLen;
};

struct GelCtx gel_create(const char *const key, const char *const tags, int *ok);
void gel_destroy(struct GelCtx c);
int download_post(struct GelPost p);
struct GelPost get_post(struct GelCtx c, int *ok);

#endif /* GEL_H_ */
