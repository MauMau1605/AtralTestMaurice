/*
 * protocol.h
 *
 * Shared constants between sender and receiver.
 */
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

/* Unix domain socket path used for the IPC channel between the two
 * processes. Detail of implementation: a Unix socket was chosen over a
 * named pipe because it gives a clean connect()/accept() handshake, which
 * doubles as the "receiver ready" signal without needing an extra message.
 */
#define IPC_SOCKET_PATH "/tmp/srec_ipc.sock"

/* Pre-shared key: known in advance by both sides, never transmitted over
 * the wire. In a real product this would come from a provisioning step,
 * not a compile-time constant -- see the oral restitution discussion.
 */
#define PRESHARED_KEY 0xA5

#include "cipher.h"

/* Bytes of address per source memory record (16-bit). */
#define SOURCE_ADDR_SIZE 2

/* Bytes of payload per source memory record (24-bit). */
#define SOURCE_PAYLOAD_SIZE 3

/* Total size of one source memory record: [address(2)][payload(3)]. */
#define SOURCE_RECORD_SIZE (SOURCE_ADDR_SIZE + SOURCE_PAYLOAD_SIZE)

/* Default path of the reconstructed memory-image file (receiver output). */
#define DEFAULT_OUTPUT_PATH "received_image.bin"

#endif /* PROTOCOL_H */
