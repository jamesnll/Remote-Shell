#ifndef REMOTE_SHELL_NETWORK_H
#define REMOTE_SHELL_NETWORK_H

#include "../include/structs.h"
#include <arpa/inet.h>
#include <inttypes.h>
#include <p101_env/env.h>
#include <stdio.h>
#include <stdlib.h>

void socket_create(const struct p101_env *env, struct p101_error *err, void *arg);
void socket_bind(const struct p101_env *env, struct p101_error *err, void *arg);

#endif    // REMOTE_SHELL_NETWORK_H
