#include "../include/arguments.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

p101_fsm_state_t parse_arguments(const struct p101_env *env, struct p101_error *err, void *arg)
{
    int              opt;
    p101_fsm_state_t next_state;
    struct context  *context;

    P101_TRACE(env);

    context                          = arg;    // Convert arg to a context struct
    context->arguments->program_name = context->arguments->argv[0];
    opterr                           = 0;
    next_state                       = CHECK_ARGS;

    while((opt = getopt(context->arguments->argc, context->arguments->argv, "hap:")) != -1)
    {
        switch(opt)
        {
            case 'a':    // IP address argument
            {
                context->arguments->ip_address = optarg;
                break;
            }
            case 'p':    // Port argument
            {
                context->arguments->port_str = optarg;
                break;
            }
            case 'h':    // Help argument
            {
                next_state = USAGE;
                break;
            }
            case '?':
            {
                if(optopt == 'a')    // Passed -a without any argument
                {
                    context->exit_message = p101_strdup(env, err, "Option '-a' requires a value.");
                }
                else if(optopt == 'p')    // Passed -p without any argument
                {
                    context->exit_message = p101_strdup(env, err, "Option '-p' requires a value.");
                }
                else
                {
                    char message[UNKNOWN_OPTION_MESSAGE_LEN];

                    snprintf(message, sizeof(message), "Unknown option '-%c'.", optopt);
                    context->exit_message = p101_strdup(env, err, message);
                }
                next_state = USAGE;
                break;
            }
            default:
            {
                context->exit_message = p101_strdup(env, err, "Unknown error with getopt.");
                next_state            = USAGE;
            }
        }
    }
    if(next_state != USAGE)
    {
        if(optind >= context->arguments->argc)
        {
            context->exit_message = p101_strdup(env, err, "The message path is required");
            next_state            = USAGE;
        }

        if(optind < context->arguments->argc - 1)
        {
            context->exit_message = p101_strdup(env, err, "Too many arguments.");
        }

        if(next_state != USAGE)
        {
            context->arguments->message = context->arguments->argv[optind];
        }
    }

    return next_state;
}

p101_fsm_state_t check_arguments(const struct p101_env *env, struct p101_error *err, void *arg)
{
    P101_TRACE(env);
    (void)err;
    (void)arg;

    return PARSE_IN_PORT_T;
}

p101_fsm_state_t parse_in_port_t(const struct p101_env *env, struct p101_error *err, void *arg)
{
    P101_TRACE(env);
    (void)err;
    (void)arg;

    return P101_FSM_EXIT;
}

p101_fsm_state_t usage(const struct p101_env *env, struct p101_error *err, void *arg)
{
    P101_TRACE(env);
    (void)err;
    (void)arg;

    return P101_FSM_EXIT;
}
