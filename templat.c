#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <regex.h>

#include "templat.h"

void templat_row_addvar(struct templat_row_t *row, char *key, char *val)
{
    int n = row->nkeys;
    row->vars[n].key = key;
    row->vars[n].val = strdup(val);
    row->nkeys++;
}

void templat_addvar(struct templat_t *tmpl, char *key, char *val)
{
    int n = tmpl->nkeys;
    tmpl->vars[n].key = key;
    tmpl->vars[n].val = strdup(val);
    tmpl->nkeys++;
}

void templat_loop_addrow(struct templat_loop_t *loop, struct templat_row_t *row)
{
    int n = loop->nrows;
    loop->rows[n] = *row;
    loop->nrows++;
}

void templat_addloop(struct templat_t *tmpl, struct templat_loop_t *loop,
                     char *key)
{
    int n = tmpl->nloops;
    tmpl->loops[n] = *loop;
    tmpl->loops[n].key = key;
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

    buf = malloc(st.st_size);
    
    n = read(fd, buf, st.st_size);
    if (n != st.st_size)
        return NULL;

    close(fd);

    return buf;
}

char *templat_get_var(struct templat_var_t *vars, int nkeys, char *key)
{
    int i;
    for (i = 0; i < nkeys; i++) {
        if (strcmp(vars[i].key, key) == 0) {
            return vars[i].val;
        }
    }
    return NULL;
}

char *templat_process_vars(struct templat_var_t *vars, int nkeys, char *data)
{
    size_t rm;
    char *output;
    regex_t preg;
    regmatch_t pmatch[3];
    int offset = 0;

    output = malloc(HUGE);
    output[0] = '\0';

    regcomp(&preg, "<TMPL_VAR[[:space:]]+([[:alpha:]]+)>", REG_EXTENDED);

    while ((rm = regexec(&preg, data + offset, 3, pmatch, 0)) != REG_NOMATCH) {
        char *key =
            strndup(data + offset + pmatch[1].rm_so,
                    pmatch[1].rm_eo - pmatch[1].rm_so);
        char *val = templat_get_var(vars, nkeys, key);
        if (!val) {
            printf("key not found: %s\n", key);
            free(data);
            return output;
        }
        strncat(output, data + offset, pmatch[0].rm_so);
        strcat(output, val);
        offset += pmatch[0].rm_eo;
    }
    strcat(output, data + offset);
    free(data);

    return output;
}

struct templat_loop_t *templat_get_loop(struct templat_t *tmpl, char *key)
{
    int i;
    for (i = 0; i < tmpl->nloops; i++) {
        if (strcmp(tmpl->loops[i].key, key) == 0) {
            return &tmpl->loops[i];
        }
    }
    return NULL;
}

char *templat_process_loops(struct templat_t *tmpl, char *data)
{
    regex_t preg;
    regmatch_t pmatch[3];
    char *output;
    char *rendered_loop;
    size_t rm;
    int offset = 0;
    int i;

    output = malloc(HUGE);
    output[0] = '\0';

    /* never parse html with regular expressions */
    regcomp(&preg,
            "<TMPL_LOOP[[:space:]]+([[:alpha:]]+)[[:space:]]*>"
            "(.*)"
            "</TMPL_LOOP[[:space:]]+\\1[[:space:]]*>",
            REG_EXTENDED);

    while ((rm = regexec(&preg, data + offset, 3, pmatch, 0)) != REG_NOMATCH) {
        char *key = strndup(data + offset + pmatch[1].rm_so,
                            pmatch[1].rm_eo - pmatch[1].rm_so);
        struct templat_loop_t *loop = templat_get_loop(tmpl, key);
        if (!loop) {
            printf("key not found: %s\n", key);
            return data;
        }
        strncat(output, data + offset, pmatch[0].rm_so);
        for (i = 0; i < loop->nrows; i++) {
            rendered_loop = strndup(data + offset + pmatch[2].rm_so,
                                    pmatch[2].rm_eo - pmatch[2].rm_so);
            rendered_loop = templat_process_vars(loop->rows[i].vars,
                                                 loop->rows[i].nkeys,
                                                 rendered_loop);
            strcat(output, rendered_loop);
        }

        offset += pmatch[0].rm_eo;
    }

    strcat(output, data + offset);
    free(data);
    return output;
}

void templat_render(struct templat_t *tmpl, char *filename)
{
    char *data = read_file(filename);

    if (!data){
        printf("template file not found: %s\n", filename);
        return;
    }

    data = templat_process_loops(tmpl, data);
    data = templat_process_vars(tmpl->vars, tmpl->nkeys, data);

    printf("%s", data);
}
