#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gel.h"

static usize
write_memory_func(void *content, usize size, usize nmemb, void *userp)
{
	size *= nmemb;
	struct mem *m = (struct mem *)userp;

	char *re = realloc(m->data, m->size + size + 1);
	if (re == NULL) {
		fprintf(stderr, "Error: out of memory");
		return 0;
	}
	m->data = re;

	memcpy(&(m->data[m->size]), content, size);
	m->size += size;
	m->data[m->size] = '\0';

	return size;
}

static int
iseq_tok_cstr(const char *const json, jsmntok_t tok, const char *const s)
{
	if (tok.type != JSMN_STRING) return 0;

	int tlen = tok.end - tok.start;
	int slen = strlen(s);
	if (tlen != slen) return 0;

	const char *const t = &json[tok.start];
	return strncmp(t, s, tlen) == 0;
}

char *
unescape_str(int len, const char str[len])
{
	char *const new = calloc(len + 1, 1);
	if (!new) return NULL;

	const char *s = (const char *)str;
	const char *ss = s;
	char *n = new;

	while (s - ss < len) {
		if (*s == '\\') s++;
		*n++ = *s++;
	}

	return new;
}

static struct mem
perform_api_call(const char *const key, const char *const tags, int *ok)
{
	int status = 0;
	struct mem result = {0};
	CURL *curl;
	CURLcode curlcode;

	curl = curl_easy_init();
	if (!curl) goto defer;

	struct mem json = {
		.data = malloc(1),
		.size = 0,
	};

	if (!json.data) goto defer_curl;

	char reqbuf[2048] = {0};
	const char *const api = "https://gelbooru.com/index.php?page=dapi&s=post&q=index&json=1&apikey=%s&tags=%s";
	snprintf(reqbuf, 2048, api, key, tags);
	curl_easy_setopt(curl, CURLOPT_URL, reqbuf);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory_func);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &json);

	curlcode = curl_easy_perform(curl);
	if (curlcode != CURLE_OK) {
		fprintf(stderr, "Error: %s\n",
			curl_easy_strerror(curlcode));
		goto defer_json;
	}

	status = 1;
	result = json;
	goto defer_curl;

defer_json:
	free(json.data);
defer_curl:
	curl_easy_cleanup(curl);
defer:
	*ok = status;
	return result;
}

struct GelPost
gel_post_get(struct GelCtx c, int *ok)
{
	const char *const json = c.json.data;
	int ntok = c.ntok;
	jsmntok_t *tokens = c.tokens;

	for (int i = 1; i < ntok; i += 1) {
		jsmntok_t t = tokens[i];
		if (!iseq_tok_cstr(json, t, "post")) continue;

		jsmntok_t url = tokens[i + 2 + 44];
		jsmntok_t fn = tokens[i + 2 + 16];

		*ok = 1;
		return (struct GelPost){
			.url = &json[url.start],
			.urlLen = url.end - url.start,
			.filename = &json[fn.start],
			.filenameLen = fn.end - fn.start,
		};
	}

	*ok = 0;
	return (struct GelPost){0};
}

static int
download_file(const char *const url, const char *const filepath)
{
	int status = 0;

	FILE *file = fopen(filepath, "wb");
	if (!file) goto defer;

	CURL *curl;
	CURLcode curlcode;

	curl = curl_easy_init();
	if (!curl) goto defer_file;

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

	curlcode = curl_easy_perform(curl);

	if (curlcode != CURLE_OK) {
		fprintf(stderr, "Error: %s\n",
			curl_easy_strerror(curlcode));
		goto defer_curl;
	}

	status = 1;

defer_curl:
	curl_easy_cleanup(curl);
defer_file:
	fclose(file);
defer:
	return status;
}

int
gel_post_download(struct GelPost p)
{
	char *const url = unescape_str(p.urlLen, p.url);;
	char *const filename = unescape_str(p.filenameLen, p.filename);

	int status = download_file(url, filename);

	free(url);
	free(filename);

	return status;
}

struct GelCtx
gel_create(const char *const key, const char *const tags, int *ok)
{
	int status = 0;
	struct GelCtx result = {0};

	struct mem json = perform_api_call(key, tags, &status);
	if (!status) goto defer;

	jsmn_parser p;
	jsmn_init(&p);
	int ntok = jsmn_parse(&p, json.data, json.size, NULL, 0);

	if (ntok < 1) {
		fprintf(stderr, "Error: could not parse json\n");
		goto defer_json;
	}

	jsmntok_t *tokens = calloc(ntok, sizeof(jsmntok_t));
	jsmn_init(&p);
	if (!tokens) {
		fprintf(stderr, "Error: calloc\n");
		goto defer_json;
	}
	ntok = jsmn_parse(&p, json.data, json.size, tokens, ntok);

	if (ntok < 1 || tokens[0].type != JSMN_OBJECT) {
		fprintf(stderr, "Error: could not parse json\n");
		goto defer_tokens;
	}

	status = 1;
	result = (struct GelCtx){
		.ntok = ntok,
		.json = json,
		.tokens = tokens,
	};

	goto defer;

defer_tokens:
	free(tokens);
defer_json:
	free(json.data);
defer:
	*ok = status;
	return result;
}

void
gel_destroy(struct GelCtx c)
{
	if (c.json.data) free(c.json.data);
	if (c.tokens) free(c.tokens);
}
