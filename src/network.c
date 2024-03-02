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

void socket_bind(const struct p101_env *env, struct p101_error *err, void *arg)
{
    char            addr_str[INET6_ADDRSTRLEN];
    socklen_t       addr_len;
    void           *vaddr;
    in_port_t       net_port;
    struct context *context;

    P101_TRACE(env);
    context  = (struct context *)arg;
    net_port = htons(context->settings.port);

    if(context->settings.addr.ss_family == AF_INET)
    {
        struct sockaddr_in *ipv4_addr;

        ipv4_addr           = (struct sockaddr_in *)&context->settings.addr;
        addr_len            = sizeof(*ipv4_addr);
        ipv4_addr->sin_port = net_port;
        vaddr               = (void *)&(((struct sockaddr_in *)&context->settings.addr)->sin_addr);
    }
    else if(context->settings.addr.ss_family == AF_INET6)
    {
        struct sockaddr_in6 *ipv6_addr;

        ipv6_addr            = (struct sockaddr_in6 *)&context->settings.addr;
        addr_len             = sizeof(*ipv6_addr);
        ipv6_addr->sin6_port = net_port;
        vaddr                = (void *)&(((struct sockaddr_in6 *)&context->settings.addr)->sin6_addr);
    }
    else
    {
        P101_ERROR_RAISE_USER(err, "addr->ss_family must be AF_INET or AF_INET6", EXIT_FAILURE);
        goto done;
    }

    if(inet_ntop(context->settings.addr.ss_family, vaddr, addr_str, sizeof(addr_str)) == NULL)
    {
        P101_ERROR_RAISE_USER(err, "inet_ntop failed", EXIT_FAILURE);
        goto done;
    }

    printf("Binding to: %s:%u\n", addr_str, context->settings.port);

    if(bind(context->settings.sockfd, (struct sockaddr *)&context->settings.addr, addr_len) == -1)
    {
        P101_ERROR_RAISE_USER(err, "Binding failed", EXIT_FAILURE);
        goto done;
    }

    printf("Bound to socket: %s:%u\n", addr_str, context->settings.port);

done:
    return;
}

void socket_start_listening(const struct p101_env *env, struct p101_error *err, void *arg)
{
    const struct context *context;
    const int             backlog = 4096;

    P101_TRACE(env);
    context = (struct context *)arg;

    if(listen(context->settings.sockfd, backlog) == -1)
    {
        close(context->settings.sockfd);
        P101_ERROR_RAISE_USER(err, "Listen failed", EXIT_FAILURE);
        return;
    }
    printf("Listening for incoming connections...\n");
}

int socket_accept_connection(const struct p101_env *env, struct p101_error *err, void *arg, struct client *client)
{
    const struct context *context;
    char                  client_host[NI_MAXHOST];
    char                  client_service[NI_MAXSERV];
    int                   client_fd;

    P101_TRACE(env);

    context   = (struct context *)arg;
    errno     = 0;
    client_fd = accept(context->settings.sockfd, (struct sockaddr *)&client->addr, &client->addr_len);

    if(client_fd == -1)
    {
        if(errno != EINTR)
        {
            P101_ERROR_RAISE_USER(err, "Accept failed", EXIT_FAILURE);
        }

        return -1;
    }

    if(getnameinfo((struct sockaddr *)&client->addr, client->addr_len, client_host, NI_MAXHOST, client_service, NI_MAXSERV, 0) == 0)
    {
        printf("Accepted a new connection from %s:%s\n", client_host, client_service);
    }
    else
    {
        printf("Unable to get client information\n");
    }

    return client_fd;
}
