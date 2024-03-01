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

    while((opt = getopt(context->arguments->argc, context->arguments->argv, "hap:")) != -1)
    {
        switch(opt)
        {
            case 'a':    // IP address argument
            {
                context->arguments->ip_address = context->arguments->argv[optind];
                break;
            }
            case 'p':    // Port argument
            {
                context->arguments->port_str = context->arguments->argv[optind - 1];
                break;
            }
            case 'h':    // Help argument
            {
                goto usage;
            }
            case '?':
            {
                if(optopt == 'a')    // Passed -a without any argument
                {
                    context->exit_message = p101_strdup(env, err, "Option '-a' requires a value.");
                    goto usage;
                }
                else if(optopt == 'p')    // Passed -p without any argument
                {
                    context->exit_message = p101_strdup(env, err, "Option '-p' requires a value.");
                    goto usage;
                }
                else
                {
                    char message[UNKNOWN_OPTION_MESSAGE_LEN];

                    snprintf(message, sizeof(message), "Unknown option '-%c'.", optopt);
                    context->exit_message = p101_strdup(env, err, message);
                    goto usage;
                }
            }
            default:
            {
                context->exit_message = p101_strdup(env, err, "Unknown error with getopt.");
                goto usage;
            }
        }
    }
    if(optind >= context->arguments->argc)
    {
        context->exit_message = p101_strdup(env, err, "The message path is required");
        goto usage;
    }

    if(optind < context->arguments->argc - 1)
    {
        context->exit_message = p101_strdup(env, err, "Too many arguments.");
        goto usage;
    }

    return;

usage:
    usage(env, err, context);
}

// p101_fsm_state_t check_arguments(const struct p101_env *env, struct p101_error *err, void *arg)
//{
//     p101_fsm_state_t next_state;
//     struct context  *context;
//
//     P101_TRACE(env);
//     context    = arg;
//     next_state = PARSE_IN_PORT_T;
//
//     if(context->arguments->ip_address == NULL)
//     {
//         context->exit_message = p101_strdup(env, err, "<ip_address> must be passed.");
//         next_state            = USAGE;
//     }
//
//     if(context->arguments->port_str == NULL)
//     {
//         context->exit_message = p101_strdup(env, err, "<port> must be passed.");
//         next_state            = USAGE;
//     }
//
//     return next_state;
// }
//
// p101_fsm_state_t parse_in_port_t(const struct p101_env *env, struct p101_error *err, void *arg)
//{
//     char            *endptr;
//     p101_fsm_state_t next_state;
//     struct context  *context;
//     uintmax_t        parsed_value;
//
//     P101_TRACE(env);
//     context = arg;
//     next_state =
//
//         errno    = 0;
//     parsed_value = strtoumax(context->arguments->port_str, &endptr, BASE_TEN);
//
//     if(void)
//         err;
//     (void)arg;
//
//     return P101_FSM_EXIT;
// }
//
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
    exit(context->exit_code);
}
