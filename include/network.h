#ifndef REMOTE_SHELL_NETWORK_H
#define REMOTE_SHELL_NETWORK_H

#include "../include/structs.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <inttypes.h>
#include <netdb.h>
#include <p101_env/env.h>
#include <p101_posix/sys/p101_socket.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MESSAGE_LENGTH 128
#define OUTPUT_LENGTH 1024

void socket_create(const struct p101_env *env, struct p101_error *err, void *arg);
void socket_setsockopt(const struct p101_env *env, struct p101_error *err, const struct context *context);
void socket_bind(const struct p101_env *env, struct p101_error *err, void *arg);
void socket_start_listening(const struct p101_env *env, struct p101_error *err, void *arg);
int  socket_accept_connection(const struct p101_env *env, struct p101_error *err, void *arg, struct client *client);
void socket_connect(const struct p101_env *env, struct p101_error *err, void *arg);
void socket_write(const struct p101_env *env, struct p101_error *err, void *arg, const char *message);
int  socket_read(const struct p101_env *env, struct p101_error *err, struct client *client);

#endif    // REMOTE_SHELL_NETWORK_H
