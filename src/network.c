#include "../include/network.h"

void socket_create(const struct p101_env *env, struct p101_error *err, void *arg)
{
    struct context *context;

    P101_TRACE(env);
    context                  = (struct context *)arg;
    context->settings.sockfd = socket(context->settings.addr.ss_family, SOCK_STREAM | SOCK_CLOEXEC, 0);

    if(context->settings.sockfd == -1)
    {
        P101_ERROR_RAISE_USER(err, "Socket creation failed", EXIT_FAILURE);
    }
}

void socket_setsockopt(const struct p101_env *env, struct p101_error *err, const struct context *context)
{
    int enable;

    P101_TRACE(env);
    enable = 1;
    if(setsockopt(context->settings.sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1)
    {
        P101_ERROR_RAISE_USER(err, "setsockopt failed", EXIT_FAILURE);
        return;
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

    p101_listen(env, err, context->settings.sockfd, backlog);

    if(p101_error_has_error(err))
    {
        close(context->settings.sockfd);
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
    client_fd = p101_accept(env, err, context->settings.sockfd, (struct sockaddr *)&client->addr, &client->addr_len);
    if(p101_error_has_error(err))
    {
        close(context->settings.sockfd);
        return -1;
    }

    if(client_fd == -1)
    {
        if(errno != EINTR)
        {
            P101_ERROR_RAISE_USER(err, "Accept failed", EXIT_FAILURE);
        }

        return client_fd;
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

void socket_connect(const struct p101_env *env, struct p101_error *err, void *arg)
{
    char            addr_str[INET6_ADDRSTRLEN];
    in_port_t       net_port;
    socklen_t       addr_len;
    struct context *context;

    P101_TRACE(env);

    context = (struct context *)arg;

    if(inet_ntop(context->settings.addr.ss_family,
                 context->settings.addr.ss_family == AF_INET ? (void *)&(((struct sockaddr_in *)&context->settings.addr)->sin_addr) : (void *)&(((struct sockaddr_in6 *)&context->settings.addr)->sin6_addr),
                 addr_str,
                 sizeof(addr_str)) == NULL)
    {
        P101_ERROR_RAISE_USER(err, "inet_ntop failed", EXIT_FAILURE);
        return;
    }

    printf("Connecting to: %s:%u\n", addr_str, context->settings.port);
    net_port = htons(context->settings.port);

    if(context->settings.addr.ss_family == AF_INET)
    {
        struct sockaddr_in *ipv4_addr;

        ipv4_addr           = (struct sockaddr_in *)&context->settings.addr;
        ipv4_addr->sin_port = net_port;
        addr_len            = sizeof(struct sockaddr_in);
    }
    else if(context->settings.addr.ss_family == AF_INET6)
    {
        struct sockaddr_in6 *ipv6_addr;

        ipv6_addr            = (struct sockaddr_in6 *)&context->settings.addr;
        ipv6_addr->sin6_port = net_port;
        addr_len             = sizeof(struct sockaddr_in6);
    }
    else
    {
        P101_ERROR_RAISE_USER(err, "Invalid address family", EXIT_FAILURE);
        return;
    }

    if(connect(context->settings.sockfd, (struct sockaddr *)&context->settings.addr, addr_len) == -1)
    {
        P101_ERROR_RAISE_USER(err, "connect failed", EXIT_FAILURE);
        return;
    }

    printf("Connected to: %s:%u\n", addr_str, context->settings.port);
}

void socket_write(const struct p101_env *env, struct p101_error *err, void *arg, const char *message)
{
    const struct context *context;
    size_t                message_len;
    uint32_t              size;

    P101_TRACE(env);
    context     = (struct context *)arg;
    message_len = strlen(message);
    size        = (uint32_t)message_len;

    if(write(context->settings.sockfd, &size, sizeof(uint32_t)) == -1)
    {
        P101_ERROR_RAISE_USER(err, "write size failed", EXIT_FAILURE);
        return;
    }

    printf("Wrote size: %u\n", size);

    if(write(context->settings.sockfd, message, message_len) == -1)
    {
        P101_ERROR_RAISE_USER(err, "write message failed", EXIT_FAILURE);
        return;
    }

    printf("Wrote message: %s\n", message);
}

int socket_read(const struct p101_env *env, struct p101_error *err, struct client *client)
{
    uint32_t size;

    P101_TRACE(env);
    size = 0;

    while(size == 0)
    {
        ssize_t bytes_read;

        bytes_read = read(client->sockfd, &size, sizeof(uint32_t));
        if(bytes_read == 0)
        {
            printf("Connection closed, returning 0\n");
            return 0;
        }
    }

    if(read(client->sockfd, (char *)client->message_buffer, size) == -1)
    {
        P101_ERROR_RAISE_USER(err, "read message failed", EXIT_FAILURE);
        return -1;
    }

    client->message_buffer[size - 1] = '\0';    // Remove \n character
    printf("size: %u word: %s\n", size, client->message_buffer);
    return 1;
}
