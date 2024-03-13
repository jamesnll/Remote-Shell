#include "../include/arguments.h"
#include "../include/network.h"
#include "../include/signal-handler.h"
#include <p101_c/p101_string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static void socket_read_from_server(const struct p101_env *env, struct p101_error *err, int sockfd);

#define INPUT_LENGTH 128

int main(int argc, char *argv[])
{
    int                ret_val;
    struct p101_error *error;
    struct p101_env   *env;
    struct arguments   arguments;
    struct context     context;

    error = p101_error_create(false);

    if(error == NULL)
    {
        ret_val = EXIT_FAILURE;
        goto done;
    }

    env = p101_env_create(error, true, NULL);
    if(p101_error_has_error(error))
    {
        ret_val = EXIT_FAILURE;
        goto free_error;
    }

    p101_memset(env, &arguments, 0, sizeof(arguments));    // Set memory of arguments to 0
    p101_memset(env, &context, 0, sizeof(context));        // Set memory of context to 0
    context.arguments       = &arguments;
    context.arguments->argc = argc;
    context.arguments->argv = argv;

    parse_arguments(env, error, &context);
    check_arguments(env, error, &context);
    parse_in_port_t(env, error, &context);
    if(p101_error_has_error(error))
    {
        ret_val = EXIT_FAILURE;
        goto free_env;
    }
    convert_address(env, error, &context);
    if(p101_error_has_error(error))
    {
        ret_val = EXIT_FAILURE;
        goto free_env;
    }
    socket_create(env, error, &context);
    if(p101_error_has_error(error))
    {
        ret_val = EXIT_FAILURE;
        goto free_env;
    }
    socket_connect(env, error, &context);
    if(p101_error_has_error(error))
    {
        ret_val = EXIT_FAILURE;
        goto socket_close;
    }

    setup_signal_handler();
    while(!exit_flag)
    {
        char input_buffer[INPUT_LENGTH];

        fgets(input_buffer, INPUT_LENGTH, stdin);
        // write to server
        socket_write(env, error, &context, input_buffer);
        if(p101_error_has_error(error))
        {
            ret_val = EXIT_FAILURE;
            goto socket_close;
        }
        socket_read_from_server(env, error, context.settings.sockfd);
        if(p101_error_has_error(error))
        {
            ret_val = EXIT_FAILURE;
            goto socket_close;
        }
    }

    ret_val = EXIT_SUCCESS;

socket_close:
    close(context.settings.sockfd);

free_env:
    free(context.exit_message);
    free(env);

free_error:
    if(p101_error_has_error(error))
    {
        fprintf(stderr, "Error: %s\n", p101_error_get_message(error));
    }
    p101_error_reset(error);
    free(error);

done:
    printf("Exit code: %d\n", ret_val);
    return ret_val;
}

static void socket_read_from_server(const struct p101_env *env, struct p101_error *err, int sockfd)
{
    ssize_t bytes_read;
    char    buffer[OUTPUT_LENGTH];

    P101_TRACE(env);

    while((bytes_read = read(sockfd, buffer, sizeof(buffer))) > 0)
    {
        if(bytes_read == 1)
        {
            break;
        }
        if(write(STDOUT_FILENO, buffer, strlen(buffer)) == -1)
        {
            P101_ERROR_RAISE_USER(err, "write to console failed", EXIT_FAILURE);
            return;
        }
        if(strstr(buffer, "Child process exited with status") != NULL)
        {
            memset(buffer, 0, sizeof(buffer));
            break;
        }

        memset(buffer, 0, sizeof(buffer));
    }
}
