#include "../include/arguments.h"
#include "../include/network.h"
// #include "../include/signal-handler.h"
#include <p101_c/p101_string.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static volatile sig_atomic_t exit_flag = 0;    // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

void setup_signal_handler(void);
void sigint_handler(int signum);

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
    socket_bind(env, error, &context);
    if(p101_error_has_error(error))
    {
        ret_val = EXIT_FAILURE;
        goto free_env;
    }
    socket_start_listening(env, error, &context);
    if(p101_error_has_error(error))
    {
        ret_val = EXIT_FAILURE;
        goto free_env;
    }
    setup_signal_handler();
    while(!exit_flag)
    {
    }

    ret_val = EXIT_SUCCESS;
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

void setup_signal_handler(void)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));

#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#endif
    sa.sa_handler = sigint_handler;
#if defined(__clang__)
    #pragma clang diagnostic pop
#endif

    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if(sigaction(SIGINT, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

void sigint_handler(int signum)
{
    exit_flag = 1;
}

#pragma GCC diagnostic pop
