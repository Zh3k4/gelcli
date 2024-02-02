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

	struct GelPost *post = gel_post_get(gel);
	if (!post) goto defer_gel;

	for (struct GelPost *p = post; p - post < 10; p++) {
		ok = gel_post_download(*p);
		if (ok) {
			printf("Downloaded file: %.*s\n", p->filenameLen, p->filename);
		} else {
			printf("Couldn't download file\n");
		}
	}

	free(post);
	result = 1;

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
