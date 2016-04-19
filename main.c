#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <libgen.h>

#include "utils.h"
#include "config.h"

char *MODULE_NAME;
int *PROCESSES_PIDS;

extern int GROUPS_INFO[PROCESSES_COUNT];
extern int RECEIVERS_IDS[PROCESSES_COUNT];
extern int SIGNALS_TO_SEND[PROCESSES_COUNT];
forking_info_t *FORKING_SCHEME;

int PROCESS_NUMBER = 0;
int sig_received = 0;
int sig_usr1_sent = 0;
int sig_usr2_sent = 0;

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
        print_info(PROCESS_NUMBER, 1, signum);
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
            int signal_to_send = SIGNALS_TO_SEND[PROCESS_NUMBER];
            usleep(100);
            if (kill(-PROCESSES_PIDS[GROUPS_INFO[RECEIVERS_IDS[PROCESS_NUMBER]]], signal_to_send) == -1) {
                print_error(MODULE_NAME, strerror(errno), "kill");
                exit(1);
            } else {
                print_info(PROCESS_NUMBER, 0, signal_to_send);
                (signal_to_send == SIGUSR1) ? sig_usr1_sent++ : sig_usr2_sent++;
            };
        }
    } else {
        if (kill(-PROCESSES_PIDS[RECEIVERS_IDS[PROCESS_NUMBER]], SIGTERM) == -1) {
            print_error(MODULE_NAME, strerror(errno), "kill");
            exit(0);
        };
        while (wait(NULL) > 0);
        if (PROCESS_NUMBER != 1) {
            print_stat(sig_usr1_sent, sig_usr2_sent);
        }
        exit(0);
    }
}

void clear_forking_scheme(forking_info_t *forking_scheme) {
    for (int i = 0; i < PARENTS_COUNT; i++) {
        free(forking_scheme[i].children);
    }
    free(forking_scheme);
}

char is_all_set(pid_t *process_pids) {
    for (int i = 0; i < PROCESSES_COUNT; i++) {
        if (!process_pids[i]) {
            return 0;
        }
    }
    return 1;
}

void start_process() {
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

    while ((forking_info = get_fork_info(PROCESS_NUMBER, FORKING_SCHEME)) &&
           (child_number < forking_info->children_count)) {
        forked_process_number = forking_info->children[child_number];
        pid_t child = fork();
        switch (child) {
            case 0:
                child_number = 0;
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
    if (sigaction(SIGUSR1, &sig_handler, 0) == -1) {
        print_error(MODULE_NAME, strerror(errno), "sigaction");
    };
    if (sigaction(SIGUSR2, &sig_handler, 0) == -1) {
        print_error(MODULE_NAME, strerror(errno), "sigaction");
    };
    if (sigaction(SIGTERM, &sig_handler, 0) == -1) {
        print_error(MODULE_NAME, strerror(errno), "sigaction");
    };

    PROCESSES_PIDS[PROCESS_NUMBER] = getpid();
    if (PROCESS_NUMBER == 1) {
        while (!is_all_set(PROCESSES_PIDS));
        int signal_to_send = SIGNALS_TO_SEND[PROCESS_NUMBER];
        kill(-PROCESSES_PIDS[GROUPS_INFO[RECEIVERS_IDS[PROCESS_NUMBER]]], signal_to_send);
        print_info(PROCESS_NUMBER, 0, signal_to_send);
    }
    if (PROCESS_NUMBER == 0) {
        wait(NULL);
        return;
    }
    while (1){
        sleep(1);
    };
}


int main(int argc, char *argv[]) {
    MODULE_NAME = basename(argv[0]);
    FORKING_SCHEME = malloc(sizeof(forking_info_t) * PARENTS_COUNT);
    create_forking_scheme(FORKING_SCHEME);
    if (!(PROCESSES_PIDS = mmap(NULL, PROCESSES_COUNT * sizeof(pid_t), PROT_READ | PROT_WRITE,
                                MAP_SHARED | MAP_ANONYMOUS, -1, 0))) {
        print_error(MODULE_NAME, strerror(errno), "mmap");
        return 1;
    }

    start_process();

    if (munmap(PROCESSES_PIDS, sizeof(pid_t) * PROCESSES_COUNT) == -1) {
        print_error(MODULE_NAME, strerror(errno), "munmap");
    };
    clear_forking_scheme(FORKING_SCHEME);
    return 0;
}