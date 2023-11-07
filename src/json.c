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
	case JSONE_BADJSON:
		return "Bad file format!";
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
	case '"': {
		struct slice_char slice = {0};
		json->cur++;
		slice.data = json->cur;
		while (*json->cur != '"') json->cur++;
		if (json->cur > &(json->data[json->datalen])) {
			return JSONE_BADJSON;
		}
		slice.len = json->cur - slice.data;
		if (json->type == JSON_KEY) {
			json->type = JSON_STRING;
			json->as.string = slice;
		} else {
			json->type = JSON_KEY;
			json->key = slice;
		}
	} break;
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	{
		char c = *json->cur;
		int is_float = 0;
		size_t len = 1;
		json->type = JSON_INT;
		while (isalpha(c) || c == '.') {
			if (c == '.') is_float = 1;
			len++;
		}
	} break;
	case '[':
		json->type = JSON_ARRAY_BEGIN;
		result = JSONE_OK;
		break;
	case ']':
		json->type = JSON_ARRAY_END;
		result = JSONE_OK;
		break;
	case '{':
		json->type = JSON_OBJECT_BEGIN;
		result = JSONE_OK;
		break;
	case '}':
		json->type = JSON_OBJECT_END;
		result = JSONE_OK;
		break;
	}

	json->cur++;
	return result;
}
