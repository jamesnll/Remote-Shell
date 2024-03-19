#include "../include/arguments.h"
#include "../include/network.h"
#include "../include/signal-handler.h"
#include <p101_c/p101_string.h>
#include <p101_posix/p101_unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

static void handle_connection(const struct p101_env *env, struct p101_error *err, struct client *client);
static void split_input(const struct p101_env *env, struct p101_error *err, char *message, char *args[], struct redirections *ioRedirections);
static void find_executable(const struct p101_env *env, struct p101_error *err, const char *command, char *full_path);
static void execute_command(const struct p101_env *env, struct p101_error *err, char *args[], const struct redirections *ioRedirections, int client_sockfd);
static void redirect_io(const struct p101_env *env, struct p101_error *err, int old_fd, int new_fd);

#define ARGS_LIMIT 10
#define FULL_PATH_LENGTH 256
#define MAX_CONNECTIONS 5

int main(int argc, char *argv[])
{
    int                connection_count;
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
    connection_count = 0;
    while(!exit_flag)
    {
        if(connection_count < MAX_CONNECTIONS)
        {
            int pid;
            pid = fork();

            if(pid == -1)
            {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            else if(pid == 0)
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

                handle_connection(env, error, client);
                if(p101_error_has_error(error))
                {
                    ret_val = EXIT_FAILURE;
                    free(client);
                    goto free_env;
                }

                free(client);
            }
            else
            {
                connection_count++;
            }
        }
        else
        {
            int code;
            wait(&code);
            connection_count--;
        }
    }

    ret_val = EXIT_SUCCESS;
    close(context.settings.sockfd);

free_env:
    free(context.exit_message);
    free(env);

free_error:
    //    if(p101_error_has_error(error))
    //    {
    //        fprintf(stderr, "Error: %s\n", p101_error_get_message(error));
    //    }
    p101_error_reset(error);
    free(error);

done:
    //    printf("Exit code: %d\n", ret_val);
    return ret_val;
}

static void handle_connection(const struct p101_env *env, struct p101_error *err, struct client *client)
{
    int   output_fd;
    char *args[ARGS_LIMIT + 1];
    char  full_path[MESSAGE_LENGTH];

    P101_TRACE(env);

    while(socket_read(env, err, client))
    {
        struct redirections ioRedirections = {false, false, false, 0};
        split_input(env, err, client->message_buffer, args, &ioRedirections);
        if(p101_error_has_error(err))
        {
            goto error;
        }
        find_executable(env, err, args[0], full_path);
        if(p101_error_has_error(err))
        {
            goto error;
        }
        args[0]   = full_path;
        output_fd = fcntl(STDOUT_FILENO, F_DUPFD_CLOEXEC, 0);
        execute_command(env, err, args, &ioRedirections, client->sockfd);
        if(p101_error_has_error(err))
        {
            goto error;
        }
        // redirect stdout back to stdout
        redirect_io(env, err, output_fd, STDOUT_FILENO);

    error:
        if(p101_error_has_error(err))
        {
            write(client->sockfd, p101_error_get_message(err), strlen(p101_error_get_message(err)));
            //            write(client->sockfd, "", 1);
            p101_error_reset(err);
        }
        write(client->sockfd, "", 1);

        memset(client->message_buffer, 0, sizeof(client->message_buffer));
    }
    // done:
    printf("Client disconnected\n");
    //    if(p101_error_has_error(err))
    //    {
    //        write(client->sockfd, p101_error_get_message(err), strlen(p101_error_get_message(err)));
    //        write(client->sockfd, "", 1);
    //    }
    close(client->sockfd);
}

static void split_input(const struct p101_env *env, struct p101_error *err, char *message, char *args[], struct redirections *ioRedirections)
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
        if(strcmp(token, "<") == 0)
        {
            ioRedirections->input_redirection = true;
            ioRedirections->redirection_index = args_index;
            printf("input is true\n");
        }
        else if(strcmp(token, ">") == 0)
        {
            ioRedirections->output_redirection = true;
            ioRedirections->redirection_index  = args_index;
            printf("truncate is true\n");
        }
        else if(strcmp(token, ">>") == 0)
        {
            ioRedirections->output_append_redirection = true;
            ioRedirections->redirection_index         = args_index;
            printf("append is true\n");
        }
        token = strtok_r(NULL, delimiter, &savePtr);
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
            printf("Executable path: %s\n", full_path);
            return;
        }
        path_token = strtok_r(NULL, delimiter, &savePtr);
    }
    P101_ERROR_RAISE_USER(err, "bash: command not found...\n", EXIT_FAILURE);
}

static void execute_command(const struct p101_env *env, struct p101_error *err, char *args[], const struct redirections *ioRedirections, int client_sockfd)
{
    pid_t     pid;
    int       file_fd;
    const int file_permissions = 0666;

    P101_TRACE(env);

    pid = fork();

    if(pid == -1)
    {
        P101_ERROR_RAISE_USER(err, "fork failed", EXIT_FAILURE);
    }
    else if(pid == 0)
    {
        // if > or >>, redirect stdout to file
        if(ioRedirections->output_redirection)
        {
            file_fd = open(args[ioRedirections->redirection_index + 1], O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, file_permissions);
            if(file_fd == -1)
            {
                exit(EXIT_FAILURE);
            }
            redirect_io(env, err, file_fd, STDOUT_FILENO);

            args[ioRedirections->redirection_index]     = NULL;
            args[ioRedirections->redirection_index + 1] = NULL;
        }
        else if(ioRedirections->output_append_redirection)
        {
            file_fd = open(args[ioRedirections->redirection_index + 1], O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, file_permissions);
            if(file_fd == -1)
            {
                exit(EXIT_FAILURE);
            }

            redirect_io(env, err, file_fd, STDOUT_FILENO);

            args[ioRedirections->redirection_index]     = NULL;
            args[ioRedirections->redirection_index + 1] = NULL;
        }
        else if(ioRedirections->input_redirection)
        {
            file_fd = open(args[ioRedirections->redirection_index + 1], O_RDONLY | O_CLOEXEC);
            if(file_fd == -1)
            {
                exit(EXIT_FAILURE);
            }

            redirect_io(env, err, file_fd, STDIN_FILENO);
            redirect_io(env, err, client_sockfd, STDOUT_FILENO);
            args[ioRedirections->redirection_index]     = NULL;
            args[ioRedirections->redirection_index + 1] = NULL;
        }
        else
        {
            redirect_io(env, err, client_sockfd, STDOUT_FILENO);
        }
        if(args != NULL)
        {
            execv(args[0], args);
        }
    }
    else
    {
        // redirect stdout to client
        int code;
        redirect_io(env, err, client_sockfd, STDOUT_FILENO);
        waitpid(pid, &code, 0);
        if(WIFEXITED(code))
        {
            printf("Child process exited with status %d\n", WEXITSTATUS(code));
        }
    }
}

static void redirect_io(const struct p101_env *env, struct p101_error *err, int old_fd, int new_fd)
{
    P101_TRACE(env);

    if(p101_dup2(env, err, old_fd, new_fd) == -1)
    {
        P101_ERROR_RAISE_USER(err, "dup2 failed", EXIT_FAILURE);
        close(old_fd);
        return;
    }
}
