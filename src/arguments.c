#include "../include/arguments.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

void parse_arguments(struct p101_env *env, struct p101_error *err, void *arg)
{
    int             opt;
    struct context *context;

    P101_TRACE(env);

    context                          = (struct context *)arg;
    context->arguments->program_name = context->arguments->argv[0];
    opterr                           = 0;

    while((opt = getopt(context->arguments->argc, context->arguments->argv, "ha:p:")) != -1)
    {
        switch(opt)
        {
            case 'a':    // IP address argument
            {
                printf("-a: %s\n", optarg);
                context->arguments->ip_address = optarg;
                break;
            }
            case 'p':    // Port argument
            {
                printf("-p: %s\n", optarg);
                context->arguments->port_str = optarg;
                break;
            }
            case 'h':    // Help argument
            {
                goto usage;
            }
            case '?':
            {
                char message[UNKNOWN_OPTION_MESSAGE_LEN];

                snprintf(message, sizeof(message), "Unknown option '-%c'.", optopt);
                context->exit_message = p101_strdup(env, err, message);
                goto usage;
            }
            default:
            {
                context->exit_message = p101_strdup(env, err, "Unknown error with getopt.");
                goto usage;
            }
        }
    }

    if(optind > REQUIRED_ARGS_NUM)
    {
        context->exit_message = p101_strdup(env, err, "Too many arguments.");
        goto usage;
    }

    return;

usage:
    usage(env, err, context);
}

void check_arguments(struct p101_env *env, struct p101_error *err, void *arg)
{
    struct context *context;

    P101_TRACE(env);
    context = (struct context *)arg;

    if(context->arguments->ip_address == NULL)
    {
        context->exit_message = p101_strdup(env, err, "<ip_address> must be passed.");
        goto usage;
    }

    if(context->arguments->port_str == NULL)
    {
        context->exit_message = p101_strdup(env, err, "<port> must be passed.");
        goto usage;
    }

        context->settings.ip_address = context->arguments->ip_address;
    return;

usage:
    usage(env, err, context);
}

void parse_in_port_t(struct p101_env *env, struct p101_error *err, void *arg)
{
    char           *endptr;
    struct context *context;
    uintmax_t       parsed_value;

    P101_TRACE(env);
    context = (struct context *)arg;
    errno   = 0;
    parsed_value = strtoumax(context->arguments->port_str, &endptr, BASE_TEN);

    if(errno != 0)
    {
        context->exit_message = p101_strdup(env, err, "Error parsing in_port_t.");
        goto usage;
    }

    if(*endptr != '\0')
    {
        context->exit_message = p101_strdup(env, err, "Invalid characters in input.");
        goto usage;
    }

    if(parsed_value > UINT16_MAX)
    {
        context->exit_message = p101_strdup(env, err, "in_port_t value out of range.");
        goto usage;
    }

    printf("Port num: %ju\n", parsed_value);

    context->settings.port = (in_port_t)parsed_value;
    return;

usage:
    usage(env, err, context);
}

_Noreturn void usage(struct p101_env *env, struct p101_error *err, void *arg)
{
    struct context *context;
    P101_TRACE(env);

    context = (struct context *)arg;

    if(context->exit_message != NULL)
    {
        fprintf(stderr, "%s\n", context->exit_message);
    }

    fprintf(stderr, "Usage: %s [-h] -a <ip_address> -p <port>\n", context->arguments->program_name);
    fputs("Options:\n", stderr);
    fputs("  -h Display this help message\n", stderr);
    fputs("  -a <ip_address>  Option 'a' (required) with an IP Address.\n", stderr);
    fputs("  -p <port>        Option 'p' (required) with a port.\n", stderr);
    free(context->exit_message);
    free(env);
    p101_error_reset(err);
    free(err);
    printf("Exit code: %d\n", context->exit_code);
    exit(context->exit_code);
}
