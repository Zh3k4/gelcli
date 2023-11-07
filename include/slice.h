#ifndef SLICE_H_
#define SLICE_H_

#include <stdlib.h>

struct slice_char {
	const char *data;
	size_t len;
};

/* allocates memory, it's up to the user to free it */
char *slice_to_cstr(struct slice_char);

#endif /* SLICE_H_ */
