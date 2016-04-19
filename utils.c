#include <unistd.h>
#include "utils.h"

void print_error(const char *module_name, const char *error_msg, const char *function_name) {
    fprintf(stderr, "%s: %s %s\n", module_name, error_msg, function_name ? function_name : "");
}

long long current_time() {
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_sec * 1000 + time.tv_usec / 1000;
}

char *signal_name(int signum) {
    switch (signum) {
        case SIGUSR1:
            return "USR1";
        case SIGUSR2:
            return "USR2";
        default:
            return "not found";
    }
}

void print_info(int process_number, char is_received, int signal_number) {
    printf("%d %d %d %s %s %lld\n", process_number, getpid(), getppid(), is_received ? "получил" : "послал",
           signal_name(signal_number), current_time());
}

void print_stat(int usr1_count, int usr2_count) {
    printf("%d %d завершил работу после %d-го сигнала SIGUSR1 и %d-го сигнала SIGUSR2\n", getpid(), getppid(), usr1_count,
           usr2_count);
}

