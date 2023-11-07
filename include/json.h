#ifndef JSON_H_
#define JSON_H_

#include <stddef.h>

#include "slice.h"

enum json_err {
	JSONE_OK = 0,
	JSONE_BADJSON,
};

enum json_object_type {
	JSON_NONE = 0,
	JSON_ARRAY_BEGIN,
	JSON_ARRAY_END,
	JSON_BOOL,
	JSON_DOUBLE,
	JSON_INT,
	JSON_KEY,
	JSON_OBJECT_BEGIN,
	JSON_OBJECT_END,
	JSON_STRING,
};

union json_union {
	int _int;
	double _double;
	struct slice_char string;
};

struct json_ctx {
	const char *data;
	size_t datalen;
	const char *cur;
	struct slice_char key;
	enum json_object_type type;
	union json_union as;
};

const char *json_strerr(enum json_err err);
struct json_ctx json_ctx_init(const char *data, ptrdiff_t datalen);
enum json_err json_next(struct json_ctx *json);

#endif /* JSON_H_ */
