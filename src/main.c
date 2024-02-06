#include <curl/curl.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "gel.h"
#include "jsmn.h"

static void
usage(const char *const program)
{
	printf("Usage: %s <how many to download> <tags>\n", program);
}

static int
create_dir(const char *const path)
{
	errno = 0;
	DIR *dir = opendir(path);
	if (dir) {
		closedir(dir);
		return 1;
	}

	if (errno != ENOENT) {
		perror("opendir");
		return 0;
	}

	errno = 0;
	if (mkdir(path, 0700) == -1 && errno != EEXIST) {
		perror("opendir");
		return 0;
	}

	return 1;
}

static int
run(const int nImages, char *const tags)
{
	int ok = 0, result = 0;
	curl_global_init(CURL_GLOBAL_ALL);

	for (char *c = tags; *c; c++) if (*c == ' ') *c = '+';

	struct GelCtx gel = gel_create("", tags, &ok);
	if (!ok) goto defer;

	struct GelPost *post = gel_post_get(gel);
	if (!post) goto defer_gel;

	for (struct GelPost *p = post; p && p - post < nImages; p++) {
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
main(int argc, char **argv)
{
	if (argc > 1 && strcmp(argv[1], "--help") == 0) {
		usage(argv[0]);
		return EXIT_SUCCESS;
	}
	if (argc != 3) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	char *endptr, *str = argv[1];
	errno = 0;
	long val = strtol(str, &endptr, 0);
	if (errno != 0 || str == endptr || val < 1) {
		usage(argv[0]);
		return EXIT_FAILURE;
	}

	if (!create_dir("gelbooru")) return EXIT_FAILURE;
	if (chdir("gelbooru") == -1) {
		perror("chdir");
		return EXIT_FAILURE;
	}

	return run(val, argv[2]) ? EXIT_SUCCESS : EXIT_FAILURE;
}
