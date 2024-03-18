#ifndef REMOTE_SHELL_ARGUMENTS_H
#define REMOTE_SHELL_ARGUMENTS_H

#include "../include/structs.h"
#include <arpa/inet.h>
#include <inttypes.h>
#include <p101_c/p101_string.h>
#include <p101_posix/p101_string.h>
#include <p101_unix/p101_getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

#define UNKNOWN_OPTION_MESSAGE_LEN 24
#define BASE_TEN 10
#define REQUIRED_ARGS_NUM 5

void           parse_arguments(struct p101_env *env, struct p101_error *err, struct context *context);
void           check_arguments(struct p101_env *env, struct p101_error *err, struct context *context);
void           parse_in_port_t(struct p101_env *env, struct p101_error *err, struct context *context);
void           convert_address(const struct p101_env *env, struct p101_error *err, struct context *context);
_Noreturn void usage(struct p101_env *env, struct p101_error *err, struct context *context);

#endif    // REMOTE_SHELL_ARGUMENTS_H
