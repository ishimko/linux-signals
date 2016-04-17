#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/time.h>

#include "processes.h"
#include "utils.h"

#define PROCESSES_COUNT 8

pid_t *processes_pids = NULL;

char is_all_started();

void process1() {
    if (!(processes_pids = mmap(NULL, PROCESSES_COUNT * sizeof(pid_t), PROT_READ | PROT_WRITE,
                                MAP_SHARED | MAP_ANONYMOUS, -1, 0))) {
        print_error(module_name, strerror(errno), "mmap");
        return;
    }

    processes_pids[0] = getpid();
    printf("1 %d\n", processes_pids[0]);

    processes_pids[1] = fork();
    switch (processes_pids[1]) {
        case -1:
            print_error(module_name, strerror(errno), "fork");
            return;
        case 0:
            process2();
            return;
        default:
            processes_pids[2] = fork();
            switch (processes_pids[2]) {
                case -1:
                    print_error(module_name, strerror(errno), "fork");
                    return;
                case 0:
                    process3();
                    return;
                default:
                    processes_pids[3] = fork();
                    switch (processes_pids[3]) {
                        case -1:
                            print_error(module_name, strerror(errno), "fork");
                            return;
                        case 0:
                            process4();
                            return;
                        default:
                            processes_pids[4] = fork();
                            switch (processes_pids[4]) {
                                case -1:
                                    print_error(module_name, strerror(errno), "fork");
                                    return;
                                case 0:
                                    process5();
                                    return;
                                default:
                                    while (!is_all_started());
                                    if (munmap(processes_pids, sizeof(pid_t)*PROCESSES_COUNT) == -1){
                                        print_error(module_name, strerror(errno), "munmap");
                                    };
                            }
                    }
            }
    }


}

void process5() {
    printf("5 %d\n", getpid());
    processes_pids[4] = getpid();
    pid_t child;
    child = fork();
    switch (child) {
        case -1:
            print_error(module_name, strerror(errno), "fork");
            return;
        case 0:
            process6();
            return;
        default:
            child = fork();
            switch (child) {
                case -1:
                    print_error(module_name, strerror(errno), "fork");
                    return;
                case 0:
                    process7();
                    return;
                default:
                    child = fork();
                    switch (child) {
                        case -1:
                            print_error(module_name, strerror(errno), "fork");
                            return;
                        case 0:
                            process8();
                            return;
                        default:
                            break;
                    }

            }
    }
}

void process8() {
    printf("8 %d\n", getpid());
    processes_pids[7] = getpid();
}

void process2() {
    printf("2 %d\n", getpid());
    processes_pids[1] = getpid();
}

void process3() {
    printf("3 %d\n", getpid());
    processes_pids[2] = getpid();
}

void process4() {
    printf("4 %d\n", getpid());
    processes_pids[3] = getpid();
}

void process6() {
    printf("6 %d\n", getpid());
    processes_pids[5] = getpid();
}

void process7() {
    printf("7 %d\n", getpid());
    processes_pids[6] = getpid();
}

int get_process_number(const pid_t pid) {
    for (int i = 0; i < PROCESSES_COUNT; i++) {
        if (processes_pids[i] == pid) {
            return i;
        }
    }
    return -1;
}

char is_all_started() {
    for (int i = 0; i < PROCESSES_COUNT; i++) {
        if (processes_pids[i] == 0) {
            return 0;
        }
    }
    return 1;
}
