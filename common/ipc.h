/*
 * ipc.h
 *
 * Abstraction layer for the local IPC channel between sender and receiver.
 * This is the only place that knows the channel is a Linux Unix domain
 * socket -- sender.c and receiver.c only call the functions below.
 */
#ifndef IPC_H
#define IPC_H

/**
 * @brief Create, bind, and listen on the IPC channel (server side).
 * @return Listening file descriptor on success, -1 on error.
 */
int ipc_server_open(void);

/**
 * @brief Block until a client connects to the IPC server.
 * @param listen_fd Listening file descriptor returned by ipc_server_open.
 * @return Connected file descriptor on success, -1 on error.
 */
int ipc_server_accept(int listen_fd);

/**
 * @brief Connect to the IPC channel (client side).
 * @return Connected file descriptor on success, -1 on error.
 */
int ipc_client_connect(void);

/**
 * @brief Close an IPC file descriptor.
 * @param fd File descriptor to close.
 * @return void
 */
void ipc_close(int fd);

#endif /* IPC_H */
