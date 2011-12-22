#define TEMPLAT_MAX 32
#define TEMPLAT_MAX_NUL (TEMPLAT_MAX + 1)
#define TEMPLAT_HUGE 4096

#define STR(x) #x
#define XSTR(x) STR(x)

struct templat_var_t {
    const char *key;
    char *val;
};

struct templat_row_t {
    struct templat_var_t *vars[TEMPLAT_MAX];
    struct templat_loop_t *loops[TEMPLAT_MAX];
    int nvars;
    int nloops;
};

struct templat_loop_t {
    const char *key;
    struct templat_row_t *rows[TEMPLAT_MAX];
    int nrows;
};

struct templat_t {
    char *filename;
    struct templat_row_t row;
};

void templat_init(struct templat_t *tmpl, const char *filename);
void templat_free(struct templat_t *tmpl);
void templat_loop_init(struct templat_loop_t *loop);
void templat_row_init(struct templat_row_t *row);
void templat_addvar(struct templat_t *tmpl, const char *key, const char *val);
void templat_row_addvar(struct templat_row_t *row, const char *key,
                        const char *val);
void templat_loop_addrow(struct templat_loop_t *loop,
                         struct templat_row_t *row);
void templat_row_addloop(struct templat_row_t *row, struct templat_loop_t *loop,
                         const char *key);
void templat_addloop(struct templat_t *tmpl, struct templat_loop_t *loop,
                     const char *key);
void templat_render(struct templat_t *tmpl);
