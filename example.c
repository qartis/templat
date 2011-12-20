#include "templat.h"

int main()
{
    struct templat_t tmpl = TEMPLAT_INIT;
    struct templat_loop_t people = TEMPLAT_LOOP_INIT;

    templat_addvar(&tmpl, "TOOL", "templat");

    {
        struct templat_loop_t menu = TEMPLAT_LOOP_INIT;

        struct templat_row_t row = TEMPLAT_ROW_INIT;
        templat_row_addvar(&row, "NAME", "Ronald");
        templat_row_addvar(&row, "EMAIL", "ronald@mcdonalds.com");

        {
            struct templat_row_t row = TEMPLAT_ROW_INIT;
            templat_row_addvar(&row, "NAME", "Big Mac");
            templat_row_addvar(&row, "PRICE", "3.25");
            templat_loop_addrow(&menu, &row);
        }

        {
            struct templat_row_t row = TEMPLAT_ROW_INIT;
            templat_row_addvar(&row, "NAME", "Filet o' Fish");
            templat_row_addvar(&row, "PRICE", "2.95");
            templat_loop_addrow(&menu, &row);
        }
        templat_row_addloop(&row, &menu, "MENU");
        templat_loop_addrow(&people, &row);
    }

    {
        struct templat_loop_t menu = TEMPLAT_LOOP_INIT;
        struct templat_row_t row = TEMPLAT_ROW_INIT;
        templat_row_addvar(&row, "NAME", "Col. Sanders");
        templat_row_addvar(&row, "EMAIL", "sanders@kentucky.mil");
        {
            struct templat_row_t row = TEMPLAT_ROW_INIT;
            templat_row_addvar(&row, "NAME", "Chicken Zinger");
            templat_row_addvar(&row, "PRICE", "3.00");
            templat_loop_addrow(&menu, &row);
        }
        templat_row_addloop(&row, &menu, "MENU");
        templat_loop_addrow(&people, &row);
    }
    templat_addloop(&tmpl, &people, "PEOPLE");

    templat_render(&tmpl, "example.tmpl");

    return 0;
}
