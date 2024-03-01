#ifndef REMOTE_SHELL_ARGUMENTS_H
#define REMOTE_SHELL_ARGUMENTS_H

#include "../include/structs.h"
#include <inttypes.h>
#include <p101_posix/p101_string.h>
#include <p101_unix/p101_getopt.h>
#include <stdio.h>
#include <stdlib.h>

#define UNKNOWN_OPTION_MESSAGE_LEN 24
#define BASE_TEN 10

void parse_arguments(struct p101_env *env, struct p101_error *err, void *arg);
// p101_fsm_state_t check_arguments(const struct p101_env *env, struct p101_error *err, void *arg);
// p101_fsm_state_t parse_in_port_t(const struct p101_env *env, struct p101_error *err, void *arg);
_Noreturn void usage(struct p101_env *env, struct p101_error *err, void *arg);

#endif    // REMOTE_SHELL_ARGUMENTS_H
