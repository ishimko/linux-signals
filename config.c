#include "config.h"



void create_forking_scheme(forking_info_t *forking_scheme) {

    forking_scheme[0].process_number = 0;
    forking_scheme[0].children_count = 1;
    forking_scheme[0].children = malloc(sizeof(int));
    forking_scheme[0].children[0] = 1;

    forking_scheme[1].process_number = 1;
    forking_scheme[1].children_count = 4;
    forking_scheme[1].children = malloc(sizeof(int) * 4);
    forking_scheme[1].children[0] = 2;
    forking_scheme[1].children[1] = 3;
    forking_scheme[1].children[2] = 4;
    forking_scheme[1].children[3] = 5;

    forking_scheme[2].process_number = 5;
    forking_scheme[2].children_count = 3;
    forking_scheme[2].children = malloc(sizeof(int) * 3);
    forking_scheme[2].children[0] = 6;
    forking_scheme[2].children[1] = 7;
    forking_scheme[2].children[2] = 8;
}
