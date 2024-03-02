#ifndef REMOTE_SHELL_NETWORK_H
#define REMOTE_SHELL_NETWORK_H

#include "../include/structs.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <inttypes.h>
#include <netdb.h>
#include <p101_env/env.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void socket_create(const struct p101_env *env, struct p101_error *err, void *arg);
void socket_bind(const struct p101_env *env, struct p101_error *err, void *arg);
void socket_start_listening(const struct p101_env *env, struct p101_error *err, void *arg);
int  socket_accept_connection(const struct p101_env *env, struct p101_error *err, void *arg, struct client *client);

#endif    // REMOTE_SHELL_NETWORK_H
