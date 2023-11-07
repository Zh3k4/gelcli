#include "slice.h"

char *
slice_to_cstr(struct slice_char slice)
{
	char *cstr = malloc(slice.len + 1);
	if (cstr == NULL) {
		return NULL;
	}

	memcpy(cstr, slice.data, slice.len);
	cstr[slice.len] = '\0';
	return cstr;
}
