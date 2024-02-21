#include "../include/arguments.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
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

    p101_fsm_run(fsm, &from_state, &to_state, NULL, transitions, sizeof(transitions));
    p101_fsm_info_destroy(env, &fsm);
    free(fsm_env);

    ret_val = EXIT_SUCCESS;

free_fsm_error:
    p101_error_reset(fsm_error);
    free(fsm_error);

free_env:
    free(env);

free_error:
    p101_error_reset(error);
    free(error);

done:
    return ret_val;
}
