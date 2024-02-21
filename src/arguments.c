#include "../include/arguments.h"

p101_fsm_state_t parse_arguments(const struct p101_env *env, struct p101_error *err, void *arg)
{
    P101_TRACE(env);
    (void)err;
    (void)arg;

    return CHECK_ARGS;
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
