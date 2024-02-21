#ifndef REMOTE_SHELL_ARGUMENTS_H
#define REMOTE_SHELL_ARGUMENTS_H

#include <p101_fsm/fsm.h>

enum states
{
    PARSE_ARGS = P101_FSM_USER_START,    // 2
    CHECK_ARGS,
    PARSE_IN_PORT_T,
    //    CONVERT_ADDRESS,
    USAGE
};

p101_fsm_state_t parse_arguments(const struct p101_env *env, struct p101_error *err, void *arg);
p101_fsm_state_t check_arguments(const struct p101_env *env, struct p101_error *err, void *arg);
p101_fsm_state_t parse_in_port_t(const struct p101_env *env, struct p101_error *err, void *arg);
p101_fsm_state_t usage(const struct p101_env *env, struct p101_error *err, void *arg);

#endif    // REMOTE_SHELL_ARGUMENTS_H
