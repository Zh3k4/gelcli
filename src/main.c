#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jsmn.h"

static int
run(void)
{
	int result = 0;
	curl_global_init(CURL_GLOBAL_ALL);

	/*
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

defer_tokens:
	free(tokens);
defer_mem:
	free(mem.memory);
	*/
defer:
	curl_global_cleanup();
	return result;
}

int
main(void)
{
	return run() ? EXIT_SUCCESS : EXIT_FAILURE;
}
