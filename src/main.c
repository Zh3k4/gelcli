#include <curl/curl.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "gel.h"

struct Jobs {
	int err;
	int help;

	int changeDir;
	const char *dir;

	long nImages;
	char *tags;
};

static void
usage(const char *const program)
{
	printf("Usage: %s [-d <dir>] <download count> <tags>\n", program);
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

static void
parse_args(struct Jobs *j, int argc, char **argv)
{
	if (argc > 0 && !strcmp(argv[0], "-h")) {
		j->help = 1;
		return;
	}

	if (argc < 2) {
		j->err = 1;
		return;
	}

	const char *arg = shift(&argc, &argv);

	for (; argc > 0; arg = shift(&argc, &argv)) {
		if (!strcmp(arg, "-d")) {
			const char *const dir = shift(&argc, &argv);
			if (!dir) {
				j->err = 1;
				return;
			}
			j->changeDir = 1;
			j->dir = dir;
		} else if (!strcmp(arg, "--")) {
			break;
		} else {
			break;
		}
	}

	/*
	 * Positional arguments
	 */

	if (argc < 1) {
		j->err = 1;
		return;
	}

	char *endptr;
	errno = 0;
	long val = strtol(arg, &endptr, 0);
	if (errno != 0 || arg == endptr || val < 1) {
		j->err = 1;
		return;
	}
	j->nImages = val;

	j->tags = argv[0];
}

static int
run(const int nImages, char *const tags)
{
	int result = 0;
	struct GelResult ret;

	struct GelCtx gel;
	curl_global_init(CURL_GLOBAL_ALL);

	for (char *c = tags; *c; c++) if (*c == ' ') *c = '+';

	ret = gel_create("", tags);
	if (!ret.ok) {
		fprintf(stderr, "Error: %s\n", ret.as.err);
		goto defer;
	}
	gel = ret.as.ctx;

	ret = gel_post_get(gel);
	if (!ret.ok) {
		fprintf(stderr, "Error: %s\n", ret.as.err);
		goto defer;
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

defer:
	if (gel.tokens) gel_destroy(gel);
	curl_global_cleanup();
	return result;
}

int
main(int argc, char **argv)
{
	const char *dir = "gelbooru";
	const char *const program = shift(&argc, &argv);

	struct Jobs jobs = {0};
	parse_args(&jobs, argc, argv);

	if (jobs.err) {
		usage(program);
		return EXIT_FAILURE;
	}
	if (jobs.help) {
		usage(program);
		return EXIT_SUCCESS;
	}
	if (jobs.changeDir) {
		dir = jobs.dir;
	}

	if (!create_dir(dir)) return EXIT_FAILURE;
	if (chdir(dir) == -1) {
		perror("chdir");
		return EXIT_FAILURE;
	}

	return run(jobs.nImages, jobs.tags) ? EXIT_SUCCESS : EXIT_FAILURE;
}
