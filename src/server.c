#include "../include/arguments.h"
#include "../include/network.h"
#include "../include/signal-handler.h"
#include <p101_c/p101_string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

static void handle_server_connection(const struct p101_env *env, struct p101_error *err, struct client *client);
static void split_input(const struct p101_env *env, struct p101_error *err, char *message, char *args[]);
static void find_executable(const struct p101_env *env, struct p101_error *err, const char *command, char *full_path);
static void execute_command(const struct p101_env *env, struct p101_error *err, char *args[]);
static void redirect_output(const struct p101_env *env, struct p101_error *err, int old_fd, int new_fd);

#define ARGS_LIMIT 10
#define FULL_PATH_LENGTH 256

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
    socket_setsockopt(env, error, &context);
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
        struct client *client = (struct client *)malloc(sizeof(struct client));
        if(client == NULL)
        {
            continue;
        }

        client->addr_len = sizeof(client->addr);
        client->sockfd   = socket_accept_connection(env, error, &context, client);

        if(client->sockfd == -1)
        {
            if(exit_flag)
            {
                free(client);
                break;
            }

            continue;
        }

        handle_server_connection(env, error, client);
        if(p101_error_has_error(error))
        {
            ret_val = EXIT_FAILURE;
            free(client);
            goto free_env;
        }

        free(client);
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

static void handle_server_connection(const struct p101_env *env, struct p101_error *err, struct client *client)
{
    char *args[ARGS_LIMIT + 1];
    char  full_path[MESSAGE_LENGTH];
    int   output_fd;

    P101_TRACE(env);

    while(socket_read(env, err, client))
    {
        split_input(env, err, client->message_buffer, args);
        if(p101_error_has_error(err))
        {
            goto done;
        }
        find_executable(env, err, args[0], full_path);
        if(p101_error_has_error(err))
        {
            goto done;
        }
        args[0]   = full_path;
        output_fd = fcntl(STDOUT_FILENO, F_DUPFD_CLOEXEC, 0);
        redirect_output(env, err, client->sockfd, STDOUT_FILENO);
        if(p101_error_has_error(err))
        {
            goto done;
        }

        execute_command(env, err, args);
        if(p101_error_has_error(err))
        {
            goto done;
        }
        redirect_output(env, err, output_fd, STDOUT_FILENO);
        if(p101_error_has_error(err))
        {
            goto done;
        }
        write(client->sockfd, "", 1);

        memset(client->message_buffer, 0, sizeof(client->message_buffer));
    }
done:
    printf("Client disconnected\n");
    close(client->sockfd);
}

static void split_input(const struct p101_env *env, struct p101_error *err, char *message, char *args[])
{
    const char *delimiter;
    char       *savePtr;
    char       *token;
    int         args_index;

    P101_TRACE(env);
    delimiter  = " ";
    args_index = 0;

    token = strtok_r(message, delimiter, &savePtr);

    while(token != NULL)
    {
        if(args_index >= ARGS_LIMIT)
        {
            P101_ERROR_RAISE_USER(err, "exceeded maximum command args", EXIT_FAILURE);
            return;
        }

        args[args_index] = token;
        token            = strtok_r(NULL, delimiter, &savePtr);
        args_index++;
    }

    args[args_index] = NULL;
}

static void find_executable(const struct p101_env *env, struct p101_error *err, const char *command, char *full_path)
{
    const char  *delimiter;
    const char  *path;
    char         path_copy[FULL_PATH_LENGTH];
    const char  *path_token;
    char        *savePtr;
    const size_t line_length = 128;

    P101_TRACE(env);

    path = getenv("PATH");

    if(path == NULL)
    {
        P101_ERROR_RAISE_USER(err, "path environment variable not found", EXIT_FAILURE);
        return;
    }

    delimiter = ":";
    strncpy(path_copy, path, strlen(path));
    path_token = strtok_r(path_copy, delimiter, &savePtr);

    while(path_token != NULL)
    {
        snprintf(full_path, line_length, "%s/%s", path_token, command);
        if(access(full_path, X_OK) == 0)
        {
            return;
        }
        path_token = strtok_r(NULL, delimiter, &savePtr);
    }
}

static void execute_command(const struct p101_env *env, struct p101_error *err, char *args[])
{
    pid_t pid;

    P101_TRACE(env);

    pid = fork();

    if(pid == -1)
    {
        P101_ERROR_RAISE_USER(err, "fork failed", EXIT_FAILURE);
    }
    else if(pid == 0)
    {
        if(args != NULL)
        {
            execv(args[0], args);
        }
    }
    else
    {
        int code;
        waitpid(pid, &code, 0);
        if(WIFEXITED(code))
        {
            printf("Child process exited with status %d\n", WEXITSTATUS(code));
        }
    }
}

static void redirect_output(const struct p101_env *env, struct p101_error *err, int old_fd, int new_fd)
{
    P101_TRACE(env);

    if(dup2(old_fd, new_fd) == -1)
    {
        P101_ERROR_RAISE_USER(err, "dup2 failed", EXIT_FAILURE);
        close(old_fd);
        return;
    }
}
