#include <assert.h>
#include <ctype.h>
#include <stddef.h>

#include "json.h"

const char *
json_strerr(enum json_err err)
{
	switch(err) {
	case JSONE_OK:
		return "OK!";
	default:
		assert(0 && "unreachable");
	}
}

struct json_ctx
json_ctx_init(const char *data, ptrdiff_t datalen)
{
	struct json_ctx json = {0};
	json.data = data;
	json.datalen = datalen;
	json.cur = data;
	return json;
}

enum json_err
json_next(struct json_ctx *json)
{
	enum json_err result = JSONE_BADJSON;

	while (isspace(*json->cur)) {
		json->cur++;
		if (json->cur > &(json->data[json->datalen])) {
			return JSONE_BADJSON;
		}
	}

	switch (*json->cur) {
	case '{':
		json->type = JSON_OBJECT;
		result = JSONE_OK;
		break;
	}

	json->cur++;
	return result;
}
