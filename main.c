#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>

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
int PROCESS_NUMBER = 0;
int sig_received = 0;
int sig_sent = 0;

int RECEIVERS_IDS[PROCESSES_COUNT] = {-1, 2, -1, -1, -1, 6, -1, -1, 1};

forking_info_t *get_fork_info(int process_number, forking_info_t *forking_scheme) {
    for (int i = 0; i < PARENTS_COUNT; i++) {
        if (forking_scheme[i].process_number == process_number) {
            return &(forking_scheme[i]);
        }
    }
    return NULL;
}

void signal_handler(int signum) {
    if (signum != SIGTERM) {
        print_info(PROCESS_NUMBER, getpid(), getppid(), 1, signum);
        int receiver_number = RECEIVERS_IDS[PROCESS_NUMBER];
        sig_received++;
        if (PROCESS_NUMBER == 1) {
            if (sig_received == 101) {
                if (kill(-PROCESSES_PIDS[RECEIVERS_IDS[PROCESS_NUMBER]], SIGTERM) == -1) {
                    print_error(MODULE_NAME, strerror(errno), "kill");
                    exit(0);
                };
                while (wait(NULL) > 0);
                exit(0);
            }
        }
        if (receiver_number != -1) {
            if (kill(-PROCESSES_PIDS[GROUPS_INFO[RECEIVERS_IDS[PROCESS_NUMBER]]], SIGUSR1) == -1) {
                print_error(MODULE_NAME, strerror(errno), "kill");
                exit(1);
            } else {
                print_info(PROCESS_NUMBER, getpid(), getppid(), 0, signum);
                sig_sent++;
            };
        }
    } else {
        if (kill(-PROCESSES_PIDS[RECEIVERS_IDS[PROCESS_NUMBER]], SIGTERM) == -1) {
            print_error(MODULE_NAME, strerror(errno), "kill");
            exit(0);
        };
        while (wait(NULL) > 0);
        if (PROCESS_NUMBER != 1) {
            printf("%d завершен после %d сигналов\n", PROCESS_NUMBER, sig_sent);
        }
        exit(0);
    }
}


void create_groups_info(forking_info_t *forking_scheme, int *groups_info) {
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

char is_all_set(pid_t *process_pids) {
    for (int i = 0; i < PROCESSES_COUNT; i++) {
        if (!process_pids[i]) {
            return 0;
        }
    }
    return 1;
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
    int child_number = 0;
    pid_t group_leader;
    forking_info_t *forking_info;

    struct sigaction sig_handler;
    sig_handler.sa_handler = &signal_handler;
    sig_handler.sa_flags = 0;
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);
    sigaddset(&sigset, SIGTERM);

    struct sigaction old_handler;
    while ((forking_info = get_fork_info(PROCESS_NUMBER, FORKING_SCHEME)) &&
           (child_number < forking_info->children_count)) {
        forked_process_number = forking_info->children[child_number];
        pid_t child = fork();
        switch (child) {
            case 0:
                child_number = 0;

//                printf("forked %d, parent %d, pid %d, ppid %d\n", forked_process_number, PROCESS_NUMBER,
//                       getpid(), getppid());
                PROCESS_NUMBER = forked_process_number;
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
    if (sigaction(SIGUSR1, &sig_handler, &old_handler) == -1) {
        print_error(MODULE_NAME, strerror(errno), "sigaction");
    };
    if (sigaction(SIGTERM, &sig_handler, &old_handler) == -1) {
        print_error(MODULE_NAME, strerror(errno), "sigaction");
    };

    PROCESSES_PIDS[PROCESS_NUMBER] = getpid();
    if (PROCESS_NUMBER == 1) {
        while (!is_all_set(PROCESSES_PIDS));
        kill(-PROCESSES_PIDS[2], SIGUSR1);
    }
    if (PROCESS_NUMBER == 0){
        wait(NULL);
        exit(0);
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