#include "utils.h"

void print_error(const char *module_name, const char *error_msg, const char *function_name) {
    fprintf(stderr, "%s: %s %s\n", module_name, error_msg, function_name ? function_name : "");
}

long long current_time() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec * 1000 + time.tv_usec / 1000;
}

void print_info(int process_number, pid_t pid, pid_t ppid, char is_received, int signal_number) {
    if (signal_number == SIGUSR1) {
        printf("%d %d %d %s %s %lld", process_number, pid, ppid, is_received ? "получил" : "послал",
               strsignal(signal_number), current_time());
    }
}


