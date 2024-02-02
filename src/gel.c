#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gel.h"

enum {
	GEL_POST = 9,
	GEL_FIRST_POST = 11,
	GEL_POST_SIZE = 59,
	GEL_IMAGE = 16,
	GEL_URL = 44,
};

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

struct GelPost *
gel_post_get(struct GelCtx c)
{
	const char *const json = c.json.data;
	int ntok = c.ntok;
	jsmntok_t *tokens = c.tokens;

	int count = 0;
	for (int i = GEL_FIRST_POST; i < ntok; i += GEL_POST_SIZE) count++;

	struct GelPost *posts = calloc(count + 1, sizeof(struct GelPost));
	if (!posts) return NULL;

	struct GelPost *p = posts;
	for (int i = GEL_FIRST_POST; i < ntok; i += GEL_POST_SIZE, p++) {
		jsmntok_t fn = tokens[i + GEL_IMAGE];
		jsmntok_t url = tokens[i + GEL_URL];

		p->url = &json[url.start];
		p->urlLen = url.end - url.start;
		p->filename = &json[fn.start];
		p->filenameLen = fn.end - fn.start;
	}

	return posts;
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
	char *const url = unescape_str(p.urlLen, p.url);
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
	if (!iseq_tok_cstr(json.data, tokens[GEL_POST], "post")) {
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
