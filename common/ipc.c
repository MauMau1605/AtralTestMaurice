/*
 * ipc.c
 *
 * Unix domain socket implementation of the IPC abstraction.
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "ipc.h"
#include "protocol.h"

int ipc_server_open(void)
{
    int listen_fd;
    struct sockaddr_un addr;

    listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, IPC_SOCKET_PATH, sizeof(addr.sun_path) - 1);
    unlink(IPC_SOCKET_PATH);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listen_fd);
        return -1;
    }

    if (listen(listen_fd, 1) < 0) {
        perror("listen");
        close(listen_fd);
        return -1;
    }

    return listen_fd;
}

int ipc_server_accept(int listen_fd)
{
    int conn_fd = accept(listen_fd, NULL, NULL);
    if (conn_fd < 0) {
        perror("accept");
        return -1;
    }
    return conn_fd;
}

int ipc_client_connect(void)
{
    int fd;
    struct sockaddr_un addr;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, IPC_SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(fd);
        return -1;
    }

    return fd;
}

void ipc_close(int fd)
{
    close(fd);
}
