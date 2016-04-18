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
    int* children;
} forking_info_t;

char *module_name;

forking_info_t *get_fork_info(int process_number, forking_info_t *forking_scheme){
    for (int i = 0; i < PARENTS_COUNT; i++){
        if (forking_scheme[i].process_number == process_number){
            return &(forking_scheme[i]);
        }
    }
    return NULL;
}


void create_forking_scheme(forking_info_t **forking_scheme){
    *forking_scheme = malloc(sizeof(forking_info_t) * PARENTS_COUNT);
    forking_info_t* result = *forking_scheme;

    result[0].process_number = 0;
    result[0].children_count = 1;
    result[0].children = malloc(sizeof(int));
    result[0].children[0] = 1;

    result[1].process_number = 1;
    result[1].children_count = 4;
    result[1].children = malloc(sizeof(int) * 4);
    result[1].children[0] = 2;
    result[1].children[1] = 3;
    result[1].children[2] = 4;
    result[1].children[3] = 5;

    result[2].process_number = 5;
    result[2].children_count = 3;
    result[2].children = malloc(sizeof(int) * 3);
    result[2].children[0] = 6;
    result[2].children[1] = 7;
    result[2].children[2] = 8;
}

void start_process(forking_info_t* forking_scheme){
    int *processes_pids;
    if (!(processes_pids = mmap(NULL, PROCESSES_COUNT * sizeof(pid_t), PROT_READ | PROT_WRITE,
                               MAP_SHARED | MAP_ANONYMOUS, -1, 0))) {
        print_error(module_name, strerror(errno), "mmap");
        return;
    }
    processes_pids[0] = getpid();
    int forked_process_number;
    int process_number = 0;
    int child_number = 0;

    forking_info_t *forking_info;
    while ((forking_info = get_fork_info(process_number, forking_scheme)) && (child_number < forking_info->children_count)){
        forked_process_number = forking_info->children[child_number];
        pid_t child = fork();
        switch (child) {
            case 0:
                child_number = 0;
                printf("forked %d, parent %d, pid %d, ppid %d\n", forked_process_number, process_number, getpid(), getppid());
                processes_pids[forked_process_number] = getpid();
                process_number = forked_process_number;
                break;
            case -1:
                print_error(module_name, strerror(errno), "fork");
                return;
            default:
                child_number++;
        }
    }
    while (wait(NULL) > 0);
    if (process_number == 0){
        for (int i = 0; i < PROCESSES_COUNT; i++){
            printf("%d %d\n", i, processes_pids[i]);
        }
        if (munmap(processes_pids, sizeof(pid_t)*PROCESSES_COUNT) == -1){
            print_error(module_name, strerror(errno), "munmap");
        };

    }
}


int main(int argc, char* argv[]) {
    forking_info_t *forking_scheme;
    create_forking_scheme(&forking_scheme);
    start_process(forking_scheme);
    return 0;
}