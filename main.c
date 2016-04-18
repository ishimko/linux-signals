#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "utils.h"

#define PROCESSES_COUNT 9
#define PARENTS_COUNT 3


typedef struct {
    int process_number;
    int children_count;
    int *children;
} forking_info_t;

char *MODULE_NAME;
int *GROUPS_INFO;
int *PROCESSES_PIDS;
forking_info_t *FORKING_SCHEME;

int RECEIVERS_IDS[PROCESSES_COUNT] = {-1, 2, -1, -1, -1, 6, -1, -1, 1};

forking_info_t *get_fork_info(int process_number, forking_info_t *forking_scheme) {
    for (int i = 0; i < PARENTS_COUNT; i++) {
        if (forking_scheme[i].process_number == process_number) {
            return &(forking_scheme[i]);
        }
    }
    return NULL;
}

int *create_groups_info(forking_info_t *forking_scheme, int *groups_info) {
    for (int i = 0; i < PARENTS_COUNT; i++) {
        groups_info[forking_scheme[i].process_number + 1] = forking_scheme[i].process_number + 1;
    }
    int parent = 0;
    for (int i = 0; i < PROCESSES_COUNT; i++) {
        if (groups_info[i]) {
            parent = groups_info[i];
        } else {
            groups_info[i] = parent;
        }
    }
}


int get_process_number(const pid_t pid, const pid_t *processes_pids) {
    for (int i = 0; i < PROCESSES_COUNT; i++) {
        if (processes_pids[i] == pid) {
            return i;
        }
    }
    return -1;
}


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

void start_process() {
    if (!(PROCESSES_PIDS = mmap(NULL, PROCESSES_COUNT * sizeof(pid_t), PROT_READ | PROT_WRITE,
                                MAP_SHARED | MAP_ANONYMOUS, -1, 0))) {
        print_error(MODULE_NAME, strerror(errno), "mmap");
        return;
    }
    PROCESSES_PIDS[0] = getpid();
    int forked_process_number;
    int process_number = 0;
    int child_number = 0;
    pid_t group_leader;
    forking_info_t *forking_info;
    while ((forking_info = get_fork_info(process_number, FORKING_SCHEME)) &&
           (child_number < forking_info->children_count)) {
        forked_process_number = forking_info->children[child_number];
        pid_t child = fork();
        switch (child) {
            case 0:
                child_number = 0;

                PROCESSES_PIDS[forked_process_number] = getpid();
                printf("forked %d, parent %d, pid %d, ppid %d\n", forked_process_number, process_number,
                       getpid(), getppid());
                process_number = forked_process_number;
                break;
            case -1:
                print_error(MODULE_NAME, strerror(errno), "fork");
                exit(1);
            default:
                while (!(group_leader = PROCESSES_PIDS[GROUPS_INFO[forked_process_number]]));
                if (setpgid(child, group_leader) == -1) {
                    print_error(MODULE_NAME, strerror(errno), "setpgid");
                    exit(1);
                };
                child_number++;
        }
    }

    while (1);
//    while (wait(NULL) > 0);
//    if (process_number == 0) {
//        if (munmap(processes_pids, sizeof(pid_t) * PROCESSES_COUNT) == -1) {
//            print_error(MODULE_NAME, strerror(errno), "munmap");
//        };
//    };
}


int main(int argc, char *argv[]) {
    FORKING_SCHEME = malloc(sizeof(forking_info_t) * PARENTS_COUNT);
    create_forking_scheme(FORKING_SCHEME);
    GROUPS_INFO = malloc(sizeof(int) * PROCESSES_COUNT);
    create_groups_info(FORKING_SCHEME, GROUPS_INFO);
    start_process();
    return 0;
}