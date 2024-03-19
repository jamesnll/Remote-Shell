#ifndef REMOTE_SHELL_STRUCTS_H
#define REMOTE_SHELL_STRUCTS_H

#include <netinet/in.h>
#include <stdbool.h>

#define MESSAGE_LENGTH 128

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
    const char             *ip_address;
    int                     sockfd;
    in_port_t               port;
    struct sockaddr_storage addr;
};

struct context
{
    struct arguments *arguments;
    struct settings   settings;
    int               exit_code;
    char             *exit_message;
};

struct client
{
    char                    message_buffer[MESSAGE_LENGTH];
    int                     sockfd;
    struct sockaddr_storage addr;
    socklen_t               addr_len;
};

struct redirections
{
    bool input_redirection;
    bool output_redirection;
    bool output_append_redirection;
    int  redirection_index;
};

#endif    // REMOTE_SHELL_STRUCTS_H
