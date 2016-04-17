#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "processes.h"
#include "utils.h"

char *module_name;


int main(int argc, char* argv[]) {
    pid_t p1_pid = fork();
    switch (p1_pid) {
        case -1:
            print_error(module_name, strerror(errno), "fork");
            return 1;
        case 0:
            process1();
            break;
        default:
            wait(NULL);
    }
    return 0;
}