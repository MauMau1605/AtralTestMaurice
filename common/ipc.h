/*
 * ipc.h
 *
 * Abstraction layer for the local IPC channel between sender and receiver.
 * This is the only place that knows the channel is a Linux Unix domain
 * socket -- sender.c and receiver.c only call the functions below.
 */
#ifndef IPC_H
#define IPC_H

/* Server side (sender): create, bind and listen on the IPC channel.
 * Returns a listening file descriptor, or -1 on error. */
int ipc_server_open(void);

/* Server side (sender): block until a client connects.
 * Returns a connected file descriptor, or -1 on error. */
int ipc_server_accept(int listen_fd);

/* Client side (receiver): connect to the IPC channel.
 * Returns a connected file descriptor, or -1 on error. */
int ipc_client_connect(void);

/* Closes a file descriptor returned by any of the functions above. */
void ipc_close(int fd);

#endif /* IPC_H */
