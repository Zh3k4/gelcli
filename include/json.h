#ifndef JSON_H_
#define JSON_H_

#include <stddef.h>

enum json_err {
	JSONE_OK = 0,
	JSONE_BADJSON,
};

enum json_object_type {
	JSON_NONE = 0,
	JSON_ARRAY,
	JSON_BOOL,
	JSON_DOUBLE,
	JSON_INT,
	JSON_OBJECT,
	JSON_STRING,
};

struct json_ctx {
	const char *data;
	ptrdiff_t datalen;
	const char *cur;
	enum json_object_type type;
	int intboolval;
	double doubleval;
	const char *stringval;
};

const char *json_strerr(enum json_err err);
struct json_ctx json_ctx_init(const char *data, ptrdiff_t datalen);

#endif /* JSON_H_ */
