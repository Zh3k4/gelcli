#ifndef GEL_H_
#define GEL_H_

struct GelCtx;
struct GelPost;

struct GelCtx gel_create(const char *const key, const char *const tags);
void gel_destroy(struct GelCtx c);
struct GelPost get_post(struct GelCtx c);

#endif /* GEL_H_ */
