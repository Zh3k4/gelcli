#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gel.h"
#include "jsmn.h"

static int
run(void)
{
	int ok = 0, result = 0;
	curl_global_init(CURL_GLOBAL_ALL);

	struct GelCtx gel = gel_create("", "1girl", &ok);
	if (!ok) goto defer;

	struct GelPost post = gel_post_get(gel, &ok);
	if (!ok) goto defer_gel;

	ok = gel_post_download(post);
	if (ok) {
		printf("Downloaded file: %.*s\n", post.filenameLen, post.filename);
		result = 1;
	} else {
		printf("Couldn't download file\n");
	}

defer_gel:
	gel_destroy(gel);
defer:
	curl_global_cleanup();
	return result;
}

int
main(void)
{
	return run() ? EXIT_SUCCESS : EXIT_FAILURE;
}
