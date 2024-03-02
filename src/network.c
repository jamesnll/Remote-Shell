#include "../include/network.h"

void socket_create(const struct p101_env *env, struct p101_error *err, void *arg)
{
    struct context *context;

    P101_TRACE(env);
    context                  = (struct context *)arg;
    context->settings.sockfd = socket(context->settings.addr.ss_family, SOCK_STREAM, 0);

    if(context->settings.sockfd == -1)
    {
        P101_ERROR_RAISE_USER(err, "Socket creation failed", EXIT_FAILURE);
    }
}
