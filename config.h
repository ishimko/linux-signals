#ifndef LINUX_SIGNALS_CONFIG_H
#define LINUX_SIGNALS_CONFIG_H

#include <signal.h>
#include <stdlib.h>

#define PROCESSES_COUNT 9
#define PARENTS_COUNT 3

typedef struct {
    int process_number;
    int children_count;
    int *children;
} forking_info_t;

static int RECEIVERS_IDS[PROCESSES_COUNT] = {-1, 2, -1, -1, -1, 6, -1, -1, 1};
static int SIGNALS_TO_SEND[PROCESSES_COUNT] = {-1, SIGUSR1, -1, -1, -1, SIGUSR1, -1, -1, SIGUSR1};

void create_forking_scheme(forking_info_t *forking_scheme);

#endif //LINUX_SIGNALS_CONFIG_H
