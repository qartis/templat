#include "templat.h"

int main()
{
    struct templat_t tmpl;
    templat_init(&tmpl, "example.tmpl");

    struct templat_loop_t people;
    templat_loop_init(&people);
    templat_addvar(&tmpl, "TOOL", "templat");

    {
        struct templat_loop_t menu;
        templat_loop_init(&menu);

        struct templat_row_t person;
        templat_row_init(&person);
        templat_row_addvar(&person, "NAME", "Ronald");
        templat_row_addvar(&person, "EMAIL", "ronald@mcdonalds.com");

        {
            struct templat_row_t item;
            templat_row_init(&item);
            templat_row_addvar(&item, "NAME", "Big Mac");
            templat_row_addvar(&item, "PRICE", "3.25");
            templat_loop_addrow(&menu, &item);
        }

        {
            struct templat_row_t item;
            templat_row_init(&item);
            templat_row_addvar(&item, "NAME", "Filet o' Fish");
            templat_row_addvar(&item, "PRICE", "2.95");
            templat_loop_addrow(&menu, &item);
        }
        templat_row_addloop(&person, &menu, "MENU");
        templat_loop_addrow(&people, &person);
    }

    {
        struct templat_loop_t menu;
        templat_loop_init(&menu);

        struct templat_row_t person;
        templat_row_init(&person);
        templat_row_addvar(&person, "NAME", "Col. Sanders");
        templat_row_addvar(&person, "EMAIL", "sanders@kentucky.mil");
        {
            struct templat_row_t item;
            templat_row_init(&item);
            templat_row_addvar(&item, "NAME", "Chicken Zinger");
            templat_row_addvar(&item, "PRICE", "3.00");
            templat_loop_addrow(&menu, &item);
        }
        templat_row_addloop(&person, &menu, "MENU");
        templat_loop_addrow(&people, &person);
    }
    templat_addloop(&tmpl, &people, "PEOPLE");

    templat_render(&tmpl);
    templat_free(&tmpl);

    return 0;
}
