#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "common/srec.h"
#include "common/protocol.h"
#include "common/ipc.h"

static int send_line(int fd, const char *line)
{
    printf("[TX] %s\n", line);
    size_t len = strlen(line);
    char buf[SREC_MAX_LINE_LEN + 1];
    if (len + 1 >= sizeof(buf))
        return -1;
    memcpy(buf, line, len);
    buf[len] = '\n';
    return write(fd, buf, len + 1) == (ssize_t)(len + 1) ? 0 : -1;
}

int main(int argc, char *argv[])
{
    const char *input_path;
    int cipher = 0;
    uint8_t *msg;
    long msg_len;
    int listen_fd, conn_fd;
    char line[SREC_MAX_LINE_LEN];
    srec_record_t rec = {0};
    long offset;
    int counter = 0;
	
    /* Usage: ./sender <input_file> [cipher_type]
     * cipher_type: 0 = none (default), 1 = xor_fixed , 2 = add_mod */
    if (argc < 2) {
        printf("usage: %s <input_file> [cipher_type: 0|1|2]\n", argv[0]);
        return 1;
    }
    input_path = argv[1];
    if (argc >= 3)
        cipher = atoi(argv[2]);
    FILE *f = fopen(input_path, "rb");
    /* Get file size */	
    fseek(f, 0, SEEK_END);
    msg_len = ftell(f);
    rewind(f);
    msg = malloc(msg_len);
    
    /* Read file size */
    fread(msg, 1, msg_len, f);
    fclose(f);
    printf("Loaded %ld bytes from %s\n", msg_len, input_path);
 
    /*set up listening socket */
    listen_fd = ipc_server_open();
    if (listen_fd < 0) {
        free(msg);
        return 1;
    }
    printf("Waiting for a receiver to connect...\n");
    conn_fd = ipc_server_accept(listen_fd);
    if (conn_fd < 0) {
        ipc_close(listen_fd);
        free(msg);
        return 1;
    }
    printf("Receiver connected. Sending...\n");
    /* S0 : header, carries CIPHER_TYPE in the address field */
    memset(&rec, 0, sizeof(rec));
    rec.type = '0';
    rec.address = cipher;
    rec.data_len = 0;
    srec_encode(&rec, line, sizeof(line));
    send_line(conn_fd, line);

    
    /* S1 : one record per [address|payload] entry of the source file. */
    for (offset = 0; offset < msg_len; offset += SOURCE_RECORD_SIZE) 
    {
        uint16_t src_addr = (uint16_t)((msg[offset] << 8) | msg[offset + 1]);
        uint8_t payload[SOURCE_PAYLOAD_SIZE];
        memcpy(payload, &msg[offset + SOURCE_ADDR_SIZE], SOURCE_PAYLOAD_SIZE);
	if (cipher == 1) 
    {
		 for (int i = 0; i < SOURCE_PAYLOAD_SIZE; i++)
			payload[i] = (uint8_t)(payload[i] ^ PRESHARED_KEY);
	}
	else
	{ 
        if (cipher !=0)
        {
            for (int i = 0; i < SOURCE_PAYLOAD_SIZE; i++) {
                payload[i] = (uint8_t)(payload[i] + PRESHARED_KEY);
                
            }
        }
	}
        memset(&rec, 0, sizeof(rec));
        rec.type = '1';
        rec.address = src_addr;
        rec.data_len = SOURCE_PAYLOAD_SIZE;
        memcpy(rec.data, payload, SOURCE_PAYLOAD_SIZE);
        srec_encode(&rec, line, sizeof(line));
        send_line(conn_fd, line);
    }
    /* --- S9 : end of transmission --- */
    memset(&rec, 0, sizeof(rec));
    rec.type = '9';
    rec.address = 0x0000;
    rec.data_len = 0;
    srec_encode(&rec, line, sizeof(line));
    send_line(conn_fd, line);
    printf("Done. %ld bytes sent in %ld record(s).\n",
           msg_len, (msg_len / SOURCE_RECORD_SIZE) + 2);
    ipc_close(conn_fd);
    ipc_close(listen_fd);
    free(msg);
    return 0;
}
