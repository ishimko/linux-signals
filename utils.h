//
// Created by ivan on 17.04.16.
//

#ifndef LINUX_SIGNALS_UTILS_H
#define LINUX_SIGNALS_UTILS_H

#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>

void print_error(const char *module_name, const char *error_msg, const char *function_name);
void print_info(int process_number, pid_t pid, pid_t ppid, char is_received, int signal_number);
long long current_time();

#endif //LINUX_SIGNALS_UTILS_H
