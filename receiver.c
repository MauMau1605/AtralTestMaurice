#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "common/srec.h"
#include "common/protocol.h"
#include "common/ipc.h"

#define MAX_MESSAGE_SIZE 65536

static int print_reconstructed_message(const char *path)
{
    uint8_t record[SOURCE_RECORD_SIZE];
    FILE *in = fopen(path, "rb");
    if (!in) {
        return -1;
    }

    printf("=== Reconstructed message ===\n");
    while (fread(record, 1, SOURCE_RECORD_SIZE, in) == SOURCE_RECORD_SIZE) {
        fwrite(&record[SOURCE_ADDR_SIZE], 1, SOURCE_PAYLOAD_SIZE, stdout);
    }
    printf("\n");

    fclose(in);
    return 0;
}

static int read_line(int fd, char *buf, size_t bufsize)
{
    size_t pos = 0;
    char c;
    while (pos + 1 < bufsize) {
        ssize_t n = read(fd, &c, 1);
        if (n < 0)
            return -1;
        if (n == 0)
            return pos == 0 ? 0 : (int)pos;
        if (c == '\n') {
            buf[pos] = '\0';
            return (int)pos;
        }
        buf[pos++] = c;
    }
    return -1;
}

int main(void)
{
    const char *output_path = DEFAULT_OUTPUT_PATH;
    int fd;
    char line[SREC_MAX_LINE_LEN];
    srec_record_t rec = {0};
    size_t total_len = 0;
    int cipher = 0;
    FILE *out = fopen(output_path, "wb");
    fd = ipc_client_connect();
    if (fd < 0) {
        return 1;
    }
    printf("Connected to sender.\n");
    while (1) {
        if (read_line(fd, line, sizeof(line)) == 0)
            break;
        if (srec_decode(line, &rec)!= 0)
        {
            printf("error on line: %s\n", line);
            ipc_close(fd);
            return 1;
        }
        if (rec.type == '0') {
            cipher = rec.address;
	    /* Debug traces uncomment for debug*/
            /* printf("Header received: CIPHER_TYPE=0x%04X\n", rec.address); */
        }
        if (rec.type == '1') {
            size_t end = (size_t)rec.address + rec.data_len;
            uint8_t addr_bytes[SOURCE_ADDR_SIZE];
            if (cipher == CIPHER_XOR) {
                /* Decrypt 3-byte payload */
                for (int i = 0; i < SOURCE_PAYLOAD_SIZE; i++)
                    rec.data[i] = (uint8_t)(rec.data[i] ^ PRESHARED_KEY);
            }
            else if (cipher == CIPHER_MOD) {
                for (int i = 0; i < SOURCE_PAYLOAD_SIZE; i++) {
                    rec.data[i] = (uint8_t)(rec.data[i] - PRESHARED_KEY);
                }
            }
            if (end > total_len)
                total_len = end;
            addr_bytes[0] = (uint8_t)((rec.address >> 8) & 0xFF);
            addr_bytes[1] = (uint8_t)(rec.address & 0xFF);
            fwrite(addr_bytes, 1, sizeof(addr_bytes), out);
            fwrite(rec.data, 1, rec.data_len, out);
	    /* Debug traces uncomment for debug*/
            /* printf("Data record: %u byte(s) at address 0x%04X (checksum OK)\n",
                   rec.data_len, rec.address);*/
        }
       	if (rec.type == '9') {
            printf("End of transmission.\n");
	    break;
        }
    }
    
    ipc_close(fd);
    fclose(out);
    printf("\nReconstructed memory image written to '%s'.\n", output_path);
    print_reconstructed_message(output_path); 
    return 0;

}
