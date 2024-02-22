#include "../include/arguments.h"
#include <p101_c/p101_stdlib.h>
#include <p101_c/p101_string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    static struct p101_fsm_transition transitions[] = {
        {P101_FSM_INIT,   PARSE_ARGS,      parse_arguments},
        {PARSE_ARGS,      CHECK_ARGS,      check_arguments},
        {PARSE_ARGS,      USAGE,           usage          },
        {CHECK_ARGS,      PARSE_IN_PORT_T, parse_in_port_t},
        {CHECK_ARGS,      USAGE,           usage          },
        {PARSE_IN_PORT_T, P101_FSM_EXIT,   NULL           },
        {USAGE,           P101_FSM_EXIT,   NULL           }
    };
    bool                  bad;     // notify if any bad state change occurs
    bool                  will;    // notify that the state is going to do this thing
    bool                  did;     // notify that the state has done this thing
    int                   ret_val;
    struct p101_error    *error;
    struct p101_env      *env;
    struct p101_error    *fsm_error;
    struct p101_env      *fsm_env;
    struct p101_fsm_info *fsm;
    p101_fsm_state_t      from_state;
    p101_fsm_state_t      to_state;
    struct arguments      arguments;
    struct context        context;

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

    fsm_error = p101_error_create(false);

    if(fsm_error == NULL)
    {
        ret_val = EXIT_FAILURE;
        goto free_env;
    }

    fsm_env = p101_env_create(error, true, NULL);

    if(p101_error_has_error(error))
    {
        ret_val = EXIT_FAILURE;
        goto free_fsm_error;
    }

    fsm  = p101_fsm_info_create(env, error, "client-fsm", fsm_env, fsm_error, NULL);
    bad  = true;
    will = false;
    did  = true;

    if(bad)
    {
        p101_fsm_info_set_bad_change_state_notifier(fsm, p101_fsm_info_default_bad_change_state_notifier);
    }

    if(will)
    {
        p101_fsm_info_set_will_change_state_notifier(fsm, p101_fsm_info_default_will_change_state_notifier);
    }

    if(did)
    {
        p101_fsm_info_set_did_change_state_notifier(fsm, p101_fsm_info_default_did_change_state_notifier);
    }

    p101_memset(env, &arguments, 0, sizeof(arguments));    // Set memory of arguments to 0
    p101_memset(env, &context, 0, sizeof(context));        // Set memory of context to 0
    context.arguments       = &arguments;
    context.arguments->argc = argc;
    context.arguments->argv = argv;

    p101_fsm_run(fsm, &from_state, &to_state, &context, transitions, sizeof(transitions));
    p101_fsm_info_destroy(env, &fsm);
    free(fsm_env);

    ret_val = EXIT_SUCCESS;

free_fsm_error:
    p101_error_reset(fsm_error);
    p101_free(env, fsm_error);

free_env:
    p101_free(env, env);

free_error:
    p101_error_reset(error);
    free(error);

done:
    return ret_val;
}

/*
 * Parent Process
 *      | // fork here before setting up server
 *      |
 *      |
 * Process Manager
 *      |
 *      |
 * Child processes
 *
 * use domain sockets to transfer fds from one process to another, d'arcy might be lying ):
 * create the domain socket, then fork (check out socket pair example)
 *
 * call socketpair()
 * then call fork() // child process has access to the other side of the socketpair
 * from that child process call fork()
 *
 */
