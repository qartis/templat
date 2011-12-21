#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "templat.h"

void templat_row_addvar(struct templat_row_t *row, char *key, char *val)
{
    struct templat_var_t *var = malloc(sizeof(*var));
    var->key = key;
    var->val = strdup(val);
    row->vars[row->nkeys] = var;
    row->nkeys++;
}

void templat_row_addloop(struct templat_row_t *row, struct templat_loop_t *loop,
                         char *key)
{
    struct templat_loop_t *duploop = malloc(sizeof(*duploop));
    *duploop = *loop;
    duploop->key = key;
    row->loops[row->nloops] = duploop;
    row->nloops++;
}

void templat_addvar(struct templat_t *tmpl, char *key, char *val)
{
    struct templat_var_t *var = malloc(sizeof(*var));
    var->key = key;
    var->val = strdup(val);
    tmpl->vars[tmpl->nkeys] = var;
    tmpl->nkeys++;
}

void templat_loop_addrow(struct templat_loop_t *loop, struct templat_row_t *row)
{
    struct templat_row_t *duprow = malloc(sizeof(*duprow));
    *duprow = *row;
    loop->rows[loop->nrows] = duprow;
    loop->nrows++;
}

void templat_addloop(struct templat_t *tmpl, struct templat_loop_t *loop,
                     char *key)
{
    struct templat_loop_t *duploop = malloc(sizeof(*duploop));
    *duploop = *loop;
    duploop->key = key;
    tmpl->loops[tmpl->nloops] = duploop;
    tmpl->nloops++;
}

char *read_file(char *filename)
{
    struct stat st;
    char *buf;
    int fd;
    int n;

    fd = open(filename, O_RDONLY);
    if (fd == -1)
        return NULL;

    fstat(fd, &st);

    buf = malloc(st.st_size + 1);
    if (!buf)
        return NULL;

    n = read(fd, buf, st.st_size);
    if (n != st.st_size)
        return NULL;

    close(fd);

    buf[st.st_size] = '\0';

    return buf;
}

char *templat_get_var(struct templat_var_t **vars, int nkeys, char *key)
{
    int i;
    for (i = 0; i < nkeys; i++) {
        if (strcmp(vars[i]->key, key) == 0) {
            return vars[i]->val;
        }
    }
    return NULL;
}

char *templat_process_vars(struct templat_var_t **vars, int nkeys, char *data)
{
    char *output;
    int offset = 0;
    char key[TEMPLAT_MAX_NUL];
    char *pos;
    char *val;
    int n;
    int len;

    output = malloc(TEMPLAT_HUGE);
    output[0] = '\0';

    while ((pos = strstr(data + offset, "<TMPL_VAR"))) {
        n = sscanf(pos, "<TMPL_VAR %" XSTR(TEMPLAT_MAX) "[a-zA-Z_]>%n", key, &len);
        if (n != 1) {
            printf("malformed input sequence: %.20s\n", pos);
            goto err;
        }
        val = templat_get_var(vars, nkeys, key);
        if (!val) {
            printf("key not found: %s\n", key);
            goto err;
        }
        strncat(output, data + offset, pos - data - offset);
        strcat(output, val);
        offset = pos - data + len;
    }

err:
    strcat(output, data + offset);
    free(data);
    return output;
}

struct templat_loop_t *templat_get_loop(struct templat_loop_t **loops,
                                        int nloops, char *key)
{
    int i;
    for (i = 0; i < nloops; i++) {
        if (strcmp(loops[i]->key, key) == 0) {
            return loops[i];
        }
    }
    return NULL;
}

char *templat_process_loops(struct templat_loop_t **loops, int nloops,
                            char *data)
{
    char terminator[TEMPLAT_MAX];
    char *output;
    char *rendered_loop;
    int offset = 0;
    int i;
    int n;
    char *pos;
    char *end;
    char key[TEMPLAT_MAX_NUL];
    struct templat_loop_t *loop;
    int len;

    output = malloc(TEMPLAT_HUGE);
    output[0] = '\0';

    while ((pos = strstr(data + offset, "<TMPL_LOOP"))) {
        n = sscanf(pos, "<TMPL_LOOP %" XSTR(TEMPLAT_MAX) "[a-zA-Z_]>%n", key, &len);
        if (n != 1) {
            printf("malformed input sequence: '%.20s'\n", pos);
            goto err;
        }
        snprintf(terminator, sizeof(terminator), "</TMPL_LOOP %s>", key);
        end = strstr(data + offset + len, terminator);
        if (!end) {
            printf("unterminated loop: %.20s\n", pos);
            goto err;
        }
        strncat(output, data + offset, pos - data);
        loop = templat_get_loop(loops, nloops, key);
        if (loop) {
            for (i = 0; i < loop->nrows; i++) {
                rendered_loop = strndup(pos + len, end - pos - strlen(terminator));
                rendered_loop = templat_process_loops(loop->rows[i]->loops,
                                                      loop->rows[i]->nloops,
                                                      rendered_loop);
                rendered_loop = templat_process_vars(loop->rows[i]->vars,
                                                     loop->rows[i]->nkeys,
                                                     rendered_loop);
                strcat(output, rendered_loop);
                free(rendered_loop);
            }
        }

        offset = end + strlen(terminator) - data;
    }

err:
    strcat(output, data + offset);
    free(data);
    return output;
}

void templat_render(struct templat_t *tmpl, char *filename)
{
    char *data = read_file(filename);

    if (!data) {
        printf("error loading template file %s: %s\n", filename, strerror(errno));
        return;
    }

    data = templat_process_loops(tmpl->loops, tmpl->nloops, data);
    data = templat_process_vars(tmpl->vars, tmpl->nkeys, data);

    printf("%s", data);
}
