#ifndef JSON_H_
#define JSON_H_

#include <stddef.h>

#include "slice.h"

enum json_err {
	JSONE_OK = 0,
	JSONE_BADJSON,
	JSONE_EOF,
	JSONE_MARKER,
};

enum json_object_type {
	JSON_NONE = 0,
	JSON_ARRAY_BEGIN,
	JSON_ARRAY_END,
	JSON_ASSIGN,
	JSON_BOOL,
	JSON_COMMA,
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
	struct slice_char key;
};

struct json_ctx {
	const char *data;
	size_t datalen;
	const char *cur;
	enum json_object_type type;
	union json_union as;
};

const char *json_strerr(enum json_err err);
const char *json_strtype(enum json_object_type type);
void json_str(struct json_ctx *json);
struct json_ctx json_ctx_init(const char *data, ptrdiff_t datalen);
enum json_err json_next(struct json_ctx *json);

#endif /* JSON_H_ */

#ifdef JSON_IMPL

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>

const char *
json_strerr(enum json_err err)
{
	switch(err) {
	case JSONE_OK: return "OK!";
	case JSONE_BADJSON: return "Bad file format!";
	case JSONE_EOF: return "End of file!";
	case JSONE_MARKER: return "Here it is!";
	default: assert(0 && "unreachable");
	}
}

const char *
json_strtype(enum json_object_type type)
{
	switch (type) {
	case JSON_NONE: return "NONE";
	case JSON_ARRAY_BEGIN: return "ARRAY_BEGIN";
	case JSON_ARRAY_END: return "ARRAY_END";
	case JSON_ASSIGN: return "ASSIGN";
	case JSON_BOOL: return "BOOL";
	case JSON_COMMA: return "COMMA";
	case JSON_DOUBLE: return "DOUBLE";
	case JSON_INT: return "INT";
	case JSON_KEY: return "KEY";
	case JSON_OBJECT_BEGIN: return "OBJECT_BEGIN";
	case JSON_OBJECT_END: return "OBJECT_END";
	case JSON_STRING: return "STRING";
	default: assert(0 && "unreachable");
	}
}

void
json_str(struct json_ctx *json)
{
	switch (json->type) {
	case JSON_NONE: assert(0 && "todo");
	case JSON_ARRAY_BEGIN: printf("%c", '['); break;
	case JSON_ARRAY_END: printf("%c", ']'); break;
	case JSON_ASSIGN: printf("%c", ':'); break;
	case JSON_BOOL:
		if (json->as._int) puts("true");
		else puts("false");
		break;
	case JSON_COMMA: printf ("%c", ','); break;
	case JSON_DOUBLE: printf("%f", json->as._double); break;
	case JSON_INT: printf("%d", json->as._int); break;
	case JSON_KEY:
		printf("\"%.*s\"", (int)json->as.key.len, json->as.key.data);
		break;
	case JSON_OBJECT_BEGIN: printf("%c", '{'); break;
	case JSON_OBJECT_END: printf("%c", '}'); break;
	case JSON_STRING:
		printf("\"%.*s\"", (int)json->as.string.len, json->as.string.data);
		break;
	default: assert(0 && "unreachable");
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
		json->cur += 1;
		if (json->cur > json->data + json->datalen) {
			result = JSONE_EOF;
			goto out;
		}
	}
	if (json->cur > json->data + json->datalen) {
		result = JSONE_EOF;
		goto out;
	}

	switch (*json->cur) {
	case '"': {
		struct slice_char slice = {0};
		json->cur += 1;
		slice.data = json->cur;
		slice.len = 0;
		while (*json->cur != '"') {
			json->cur += 1;
			slice.len += 1;
		}
		if (json->cur > json->data + json->datalen) {
			goto out;
		}
		if (json->type == JSON_ASSIGN) {
			json->type = JSON_STRING;
			json->as.string = slice;
		} else {
			json->type = JSON_KEY;
			json->as.key = slice;
		}
		result = JSONE_OK;
	} break;

	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9': {
		size_t len = 1;
		char c = json->cur[1];
		int is_float = 0;
		json->type = JSON_INT;
		while (isalpha(c) || c == '.') {
			if (c == '.' && is_float) {
				goto out;
			}
			if (c == '.') is_float = 1;
			len++;
			c = json->cur[len];
		}

		char *buf = calloc(1, len + 1);
		strncpy(buf, json->cur, len);
		if (is_float) {
			json->type = JSON_DOUBLE;
			json->as._double = atof(buf);
		} else {
			json->as._int = atoi(buf);
		}
		free(buf);
		json->cur += len;
		result = JSONE_OK;
	} break;

	case 't':
		if (strncmp(json->cur, "true", 4) == 0) {
			json->type = JSON_BOOL;
			json->as._int = 1;
			result = JSONE_OK;
		}
		break;
	case 'f':
		if (strncmp(json->cur, "false", 5) == 0) {
			json->type = JSON_BOOL;
			json->as._int = 0;
			result = JSONE_OK;
		}
		break;

	case ':':
		json->type = JSON_ASSIGN;
		result = JSONE_OK;
		break;
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
	case ',':
		json->type = JSON_COMMA;
		result = JSONE_OK;
		break;
	case '\0':
		json->type = JSON_NONE;
		result = JSONE_EOF;
		break;
	default:
		fprintf(stderr, "unhandled: '%c'\n", *json->cur);
	}
	
	json->cur += 1;

out:
	return result;
}

#endif /* JSON_IMPL */
