#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jsmn.h"

struct Mem {
	char *memory;
	size_t size;
};

const char *const gel = "https://gelbooru.com/index.php?page=dapi&s=post&q=index&json=1&apikey=%s&tags=%s";
char reqbuf[2048] = {0};

static size_t write_memory_func(void *content, size_t membsize,
	size_t nmemb, void *userp);

static int
run(void)
{
	int result;
	CURL *curl;
	CURLcode curlcode;

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if (!curl) {
		result = 0;
		goto out;
	}

	struct Mem mem;
	mem.memory = malloc(1);
	mem.size = 0;

	snprintf(reqbuf, 2048, gel, "", "futanari");
	curl_easy_setopt(curl, CURLOPT_URL, reqbuf);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_memory_func);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &mem);

	curlcode = curl_easy_perform(curl);

	if (curlcode != CURLE_OK) {
		fprintf(stderr, "Error: %s\n",
			curl_easy_strerror(curlcode));
		result = 0;
		goto out;
	}

	jsmn_parser p;
	jsmn_init(&p);
	int ntok = jsmn_parse(&p, mem.memory, mem.size, NULL, 0);

	if (ntok < 1) {
		fprintf(stderr, "Error: could not parse json\n");
		result = 0; goto out;
	}

	jsmntok_t *tokens = calloc(ntok, sizeof(jsmntok_t));
	jsmn_init(&p);
	if (!tokens) {
		fprintf(stderr, "Error: calloc\n");
		result = 0; goto out;
	}
	ntok = jsmn_parse(&p, mem.memory, mem.size, tokens, ntok);

	if (ntok < 1 || tokens[0].type != JSMN_OBJECT) {
		fprintf(stderr, "Error: could not parse json\n");
		free(tokens);
		result = 0; goto out;
	}

	/* We've got the tokens! */

	free(tokens);
	result = 1;

out:
	if (mem.memory) free(mem.memory);
	if (curl) curl_easy_cleanup(curl);
	curl_global_cleanup();
	return result;
}

int
main(void)
{
	return !run();
}

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
