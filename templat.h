#define MAX 8
#define HUGE 4096

struct templat_var_t {
    char *key;
    char *val;
};
#define TEMPLAT_VAR_INIT {0, 0}

struct templat_row_t {
    struct templat_var_t *vars[MAX];
    int nkeys;
};
#define TEMPLAT_ROW_INIT {{0}, 0}

struct templat_loop_t {
    char *key;
    struct templat_loop_t *loops[MAX];
    struct templat_row_t *rows[MAX];
    int nloops;
    int nrows;
};
#define TEMPLAT_LOOP_INIT {0, {0}, {0}, 0, 0}

struct templat_t {
    char *filename;
    struct templat_loop_t *loops[MAX];
    struct templat_var_t *vars[MAX];
    int nloops;
    int nkeys;
};
#define TEMPLAT_INIT {0, {0}, {0}, 0, 0}

void templat_addvar(struct templat_t *tmpl, char *key, char *val);
void templat_row_addvar(struct templat_row_t *row, char *key, char *val);
void templat_loop_addrow(struct templat_loop_t *loop,
                         struct templat_row_t *row);
void templat_loop_addloop(struct templat_loop_t *outer,
                         struct templat_loop_t *inner);
void templat_addloop(struct templat_t *tmpl, struct templat_loop_t *loop,
                     char *key);
void templat_render(struct templat_t *tmpl, char *filename);
