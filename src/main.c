#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jsmn.h"

struct Mem {
	char *memory;
	size_t size;
};

struct RemoteFile {
	const char *url;
	int urlLen;
	const char *filename;
	int filenameLen;
};

static size_t
write_memory_func(void *content, size_t membsize, size_t nmemb, void *userp)
{
	size_t size = membsize * nmemb;
	struct Mem *mem = (struct Mem *)userp;

	char *re = realloc(mem->memory, mem->size + size + 1);
	if (re == NULL) {
		fprintf(stderr, "Error: out of memory");
		return 0;
	}

	mem->memory = re;

	memcpy(&(mem->memory[mem->size]), content, size);
	mem->size += size;
	mem->memory[mem->size] = '\0';

	return size;
}

static struct Mem
perform_api_call(void)
{
	struct Mem result = { 0 };
	CURL *curl;
	CURLcode curlcode;

	curl = curl_easy_init();
	if (!curl) goto out;

	struct Mem mem;
	mem.memory = malloc(1);
	mem.size = 0;

	if (!mem.memory) goto out_curl;

	char reqbuf[2048] = {0};
	const char *const api = "https://gelbooru.com/index.php?page=dapi&s=post&q=index&json=1&apikey=%s&tags=%s";
	snprintf(reqbuf, 2048, api, "", "1girl");
	curl_easy_setopt(curl, CURLOPT_URL, reqbuf);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory_func);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mem);

	curlcode = curl_easy_perform(curl);

	if (curlcode != CURLE_OK) {
		fprintf(stderr, "Error: %s\n",
			curl_easy_strerror(curlcode));
		goto out_mem;
	}

	result = mem;
	goto out_curl;

out_mem:
	free(mem.memory);
out_curl:
	curl_easy_cleanup(curl);
out:

	return result;
}

static int
iseq_tok_cstr(const char *const mem, jsmntok_t tok, const char *const s)
{
	if (tok.type != JSMN_STRING) return 0;

	int tlen = tok.end - tok.start;
	int slen = strlen(s);
	if (tlen != slen) return 0;

	const char *const t = &mem[tok.start];
	return strncmp(t, s, tlen) == 0;
}

struct RemoteFile
get_pic(const char *const mem, int ntok, jsmntok_t tokens[ntok])
{
	for (int i = 1; i < ntok; i += 1) {
		jsmntok_t t = tokens[i];
		if (!iseq_tok_cstr(mem, t, "post")) continue;

		jsmntok_t url = tokens[i + 2 + 44];
		jsmntok_t fn = tokens[i + 2 + 16];

		return (struct RemoteFile){
			.url = &mem[url.start],
			.urlLen = url.end - url.start,
			.filename = &mem[fn.start],
			.filenameLen = fn.end - fn.start,
		};
	}

	return (struct RemoteFile){ 0 };
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

static int
download_file(const char *const url, const char *const filepath)
{
	int result = 0;

	FILE *file = fopen(filepath, "wb");
	if (!file) goto out;

	CURL *curl;
	CURLcode curlcode;

	curl = curl_easy_init();
	if (!curl) goto out_file;

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

	curlcode = curl_easy_perform(curl);

	if (curlcode != CURLE_OK) {
		fprintf(stderr, "Error: %s\n",
			curl_easy_strerror(curlcode));
		goto out_curl;
	}

	result = 1;

out_curl:
	curl_easy_cleanup(curl);
out_file:
	fclose(file);
out:
	return result;
}

static int
run(void)
{
	int result = 0;
	curl_global_init(CURL_GLOBAL_ALL);

	struct Mem mem = perform_api_call();
	if (!mem.memory) goto out;

	jsmn_parser p;
	jsmn_init(&p);
	int ntok = jsmn_parse(&p, mem.memory, mem.size, NULL, 0);

	if (ntok < 1) {
		fprintf(stderr, "Error: could not parse json\n");
		result = 0; goto out_mem;
	}

	jsmntok_t *tokens = calloc(ntok, sizeof(jsmntok_t));
	jsmn_init(&p);
	if (!tokens) {
		fprintf(stderr, "Error: calloc\n");
		result = 0; goto out_mem;
	}
	ntok = jsmn_parse(&p, mem.memory, mem.size, tokens, ntok);

	if (ntok < 1 || tokens[0].type != JSMN_OBJECT) {
		fprintf(stderr, "Error: could not parse json\n");
		result = 0; goto out_tokens;
	}

	struct RemoteFile rf = get_pic(mem.memory, ntok, tokens);

	char *url = unescape_str(rf.urlLen, rf.url);
	char *filename = unescape_str(rf.filenameLen, rf.filename);
	int success = download_file(url, filename);
	free(filename);
	free(url);

	if (success) {
		printf("Downloaded file: %.*s\n", rf.filenameLen, rf.filename);
		result = 1;
	} else {
		printf("Couldn't download file\n");
	}

out_tokens:
	free(tokens);
out_mem:
	free(mem.memory);
out:
	curl_global_cleanup();
	return result;
}

int
main(void)
{
	return run() ? EXIT_SUCCESS : EXIT_FAILURE;
}
