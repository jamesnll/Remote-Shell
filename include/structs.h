#ifndef REMOTE_SHELL_STRUCTS_H
#define REMOTE_SHELL_STRUCTS_H

#include <netinet/in.h>

struct arguments
{
    int         argc;
    const char *program_name;
    const char *message;
    const char *ip_address;
    const char *port_str;
    char      **argv;
};

struct settings
{
    const char *ip_address;
    in_port_t   port;
    struct sockaddr_storage addr;
};

struct context
{
    struct arguments *arguments;
    struct settings   settings;
    int               exit_code;
    char             *exit_message;
};

#endif    // REMOTE_SHELL_STRUCTS_H
