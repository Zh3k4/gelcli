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

static char *
shift(int *argc, char ***argv)
{
	if (*argc < 1) return NULL;
	char *arg = **argv;
	*argc -= 1;
	*argv += 1;
	return arg;
}

static int
create_dir(const char *const path)
{
	DIR *dir = opendir(path);
	if (dir) {
		closedir(dir);
		return 1;
	}
	if (errno != ENOENT) {
		perror("opendir");
		return 0;
	}
	if (mkdir(path, 0700) == -1 && errno != EEXIST) {
		perror("opendir");
		return 0;
	}
	return 1;
}

static int
run(const int nImages, char *const tags)
{
	int result = 0;
	struct GelResult ret;

	curl_global_init(CURL_GLOBAL_ALL);

	for (char *c = tags; *c; c++) if (*c == ' ') *c = '+';

	ret = gel_create("", tags);
	if (!ret.ok) {
		fprintf(stderr, "Error: %s\n", ret.as.err);
		goto defer;
	}
	struct GelCtx gel = ret.as.ctx;

	ret = gel_post_get(gel);
	if (!ret.ok) {
		fprintf(stderr, "Error: %s\n", ret.as.err);
		goto defer_gel;
	}
	struct GelPost *post = ret.as.post;

	for (struct GelPost *p = post; p && p - post < nImages; p++) {
		int status = gel_post_download(*p);
		switch (status) {
		case 1:
			printf("Downloaded file: %.*s\n", p->filenameLen, p->filename);
			break;
		case 2: break;
		default:
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
	const char *const program = shift(&argc, &argv);

	if (argc > 0 && strcmp(argv[0], "--help") == 0) {
		usage(program);
		return EXIT_SUCCESS;
	}
	if (argc != 2) {
		usage(program);
		return EXIT_FAILURE;
	}

	char *endptr, *str = argv[0];
	errno = 0;
	long val = strtol(str, &endptr, 0);
	if (errno != 0 || str == endptr || val < 1) {
		usage(program);
		return EXIT_FAILURE;
	}

	if (!create_dir("gelbooru")) return EXIT_FAILURE;
	if (chdir("gelbooru") == -1) {
		perror("chdir");
		return EXIT_FAILURE;
	}

	return run(val, argv[1]) ? EXIT_SUCCESS : EXIT_FAILURE;
}
