#include "templat.h"

int main()
{
    struct templat_t tmpl = TEMPLAT_INIT;
    struct templat_loop_t users = TEMPLAT_LOOP_INIT;

    templat_addvar(&tmpl, "TOOL", "templat");

    {
        struct templat_row_t row = TEMPLAT_ROW_INIT;
        templat_row_addvar(&row, "NAME", "Ronald");
        templat_row_addvar(&row, "EMAIL", "ronald@mcdonalds.com");
        templat_loop_addrow(&users, &row);
    }

    {
        struct templat_row_t row = TEMPLAT_ROW_INIT;
        templat_row_addvar(&row, "NAME", "Sarah");
        templat_row_addvar(&row, "EMAIL", "xzqsarah@gmail.com");
        templat_loop_addrow(&users, &row);
    }

    {
        struct templat_row_t row = TEMPLAT_ROW_INIT;
        templat_row_addvar(&row, "NAME", "Col. Sanders");
        templat_row_addvar(&row, "EMAIL", "sanders@kentucky.mil");
        templat_loop_addrow(&users, &row);
    }
    templat_addloop(&tmpl, &users, "USERS");

    templat_render(&tmpl, "example.tmpl");

    return 0;
}
