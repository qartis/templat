#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "templat.h"

char *templat_process(struct templat_row_t *row, char *data);

void templat_init(struct templat_t *tmpl, const char *filename)
{
    templat_row_init(&tmpl->row);
    tmpl->filename = strdup(filename);
}

void templat_row_init(struct templat_row_t *row)
{
    row->nloops = 0;
    row->nvars = 0;
}

void templat_loop_init(struct templat_loop_t *loop)
{
    loop->nrows = 0;
    loop->key = NULL;
}

void templat_row_free(struct templat_row_t *row)
{
    int i, j;

    for (i = 0; i < row->nloops; i++) {
        for (j = 0; j < row->loops[i]->nrows; j++) {
            templat_row_free(row->loops[i]->rows[j]);
            free(row->loops[i]->rows[j]);
        }
        free(row->loops[i]);
    }

    for (i = 0; i < row->nvars; i++) {
        free(row->vars[i]->val);
        free(row->vars[i]);
    }
}

void templat_free(struct templat_t *tmpl)
{
    templat_row_free(&tmpl->row);
    free(tmpl->filename);
}

void templat_row_addvar(struct templat_row_t *row, const char *key,
                        const char *val)
{
    struct templat_var_t *var = malloc(sizeof(*var));
    var->key = key;
    var->val = strdup(val);
    row->vars[row->nvars] = var;
    row->nvars++;
}

void templat_row_addloop(struct templat_row_t *row, struct templat_loop_t *loop,
                         const char *key)
{
    struct templat_loop_t *duploop = malloc(sizeof(*duploop));
    *duploop = *loop;
    duploop->key = key;
    row->loops[row->nloops] = duploop;
    row->nloops++;
}

void templat_loop_addrow(struct templat_loop_t *loop, struct templat_row_t *row)
{
    struct templat_row_t *duprow = malloc(sizeof(*duprow));
    *duprow = *row;
    loop->rows[loop->nrows] = duprow;
    loop->nrows++;
}

void templat_addvar(struct templat_t *tmpl, const char *key, const char *val)
{
    templat_row_addvar(&tmpl->row, key, val);
}

void templat_addloop(struct templat_t *tmpl, struct templat_loop_t *loop,
                     const char *key)
{
    templat_row_addloop(&tmpl->row, loop, key);
}

char *read_file(const char *filename)
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

const char *templat_get_var(struct templat_row_t *row, const char *key)
{
    int i;
    for (i = 0; i < row->nvars; i++) {
        if (strcmp(row->vars[i]->key, key) == 0) {
            return row->vars[i]->val;
        }
    }
    return NULL;
}

char *templat_process_vars(struct templat_row_t *row, char *data)
{
    char *output;
    int offset = 0;
    char key[TEMPLAT_MAX_NUL];
    const char *pos;
    const char *val;
    int n;
    int len;

    output = malloc(TEMPLAT_HUGE);
    output[0] = '\0';

    while ((pos = strstr(data + offset, "<TMPL_VAR"))) {
        n = sscanf(pos, "<TMPL_VAR %" XSTR(TEMPLAT_MAX) "[a-zA-Z_]>%n",
                   key, &len);
        if (n != 1) {
            printf("malformed input sequence: %.20s\n", pos);
            goto err;
        }
        val = templat_get_var(row, key);
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

struct templat_loop_t *templat_get_loop(struct templat_row_t *row,
                                        const char *key)
{
    int i;
    for (i = 0; i < row->nloops; i++) {
        if (strcmp(row->loops[i]->key, key) == 0) {
            return row->loops[i];
        }
    }
    return NULL;
}

char *templat_process_loops(struct templat_row_t *row, char *data)
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
        n = sscanf(pos, "<TMPL_LOOP %" XSTR(TEMPLAT_MAX) "[a-zA-Z_]>%n",
                   key, &len);
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
        strncat(output, data + offset, pos - data - offset);
        loop = templat_get_loop(row, key);
        if (loop) {
            for (i = 0; i < loop->nrows; i++) {
                rendered_loop = strndup(pos + len,
                                        end - pos - strlen(terminator));
                rendered_loop = templat_process(loop->rows[i],
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

char *templat_process(struct templat_row_t *row, char *data)
{
    data = templat_process_loops(row, data);
    data = templat_process_vars(row, data);
    return data;
}

void templat_render(struct templat_t *tmpl)
{
    char *data = read_file(tmpl->filename);

    if (!data) {
        printf("error loading template file %s: %s\n",
               tmpl->filename, strerror(errno));
        return;
    }

    data = templat_process(&tmpl->row, data);

    printf("%s", data);

    free(data);
}
