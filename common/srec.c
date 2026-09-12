/*
 * srec.c
 *
 * Minimal SREC (Motorola S-record) encoder/decoder.
 */

#include <stdio.h>
#include <string.h>
#include "srec.h"

static int hex_nibble(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

static int hex_byte(const char *s)
{
    int hi = hex_nibble(s[0]);
    int lo = hex_nibble(s[1]);
    if (hi < 0 || lo < 0)
        return -1;
    return (hi << 4) | lo;
}

int srec_encode(const srec_record_t *rec, char *out_line, size_t out_size)
{
    uint8_t payload[2 + SREC_MAX_DATA];
    uint8_t count = 0;
    uint8_t sum = 0;
    uint8_t checksum = 0;
    size_t pos = 0;
    int i = 0;
    if (rec->data_len > SREC_MAX_DATA)
        return -1;
    /* payload = address (2 bytes, big endian) + data */
    payload[0] = (rec->address >> 8) & 0xFF;
    payload[1] = rec->address & 0xFF;
    memcpy(&payload[2], rec->data, rec->data_len);
    count = (uint8_t)(2 + rec->data_len + 1); /* address + data + checksum byte */
    sum = count;
    for (i = 0; i < 2 + rec->data_len; i++)
        sum = (uint8_t)(sum + payload[i]);
    checksum = (uint8_t)(~sum);
    pos = 0;
    out_line[pos++] = 'S';
    out_line[pos++] = rec->type;
    pos += (size_t)snprintf(&out_line[pos], out_size - pos, "%02X", count);
    for (i = 0; i < 2 + rec->data_len; i++)
        pos += (size_t)snprintf(&out_line[pos], out_size - pos, "%02X", payload[i]);
    pos += (size_t)snprintf(&out_line[pos], out_size - pos, "%02X", checksum);
    return (int)pos;
}

int srec_decode(const char *line, srec_record_t *rec)
{
    size_t len = 0;
    int count = 0;
    uint8_t sum = 0;
    int addr_hi = 0, addr_lo = 0;
    int checksum_read = 0;
    uint8_t expected_checksum = 0;

    if (!line || !rec)
        return -1;

    len = strlen(line);
    /* trim trailing CR/LF */
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
        len--;

    /* Minimum SREC line: 'S' + type (1) + count (2) + addr (4) + checksum (2) = 10 chars */
    if (len < 10 || line[0] != 'S')
        return -1;

    char type = line[1];
    if (type != '0' && type != '1' && type != '9')
        return -1;
    rec->type = type;

    count = hex_byte(&line[2]);
    if (count < 3 || count > (int)(2 + SREC_MAX_DATA + 1))
        return -1;

    /* Total expected line length: 'S' (1) + type (1) + count (2) + count * 2 hex digits = 4 + count * 2 */
    if (len != (size_t)(4 + count * 2))
        return -1;

    sum = (uint8_t)count;

    addr_hi = hex_byte(&line[4]);
    addr_lo = hex_byte(&line[6]);
    if (addr_hi < 0 || addr_lo < 0)
        return -1;

    sum = (uint8_t)(sum + addr_hi + addr_lo);
    rec->address = (uint16_t)((addr_hi << 8) | addr_lo);
    rec->data_len = (uint8_t)(count - 3);

    for (size_t i = 0; i < rec->data_len; i++) {
        int val = hex_byte(&line[8 + i * 2]);
        if (val < 0)
            return -1;
        rec->data[i] = (uint8_t)val;
        sum = (uint8_t)(sum + val);
    }

    checksum_read = hex_byte(&line[8 + rec->data_len * 2]);
    if (checksum_read < 0)
        return -1;

    expected_checksum = (uint8_t)(~sum);
    if ((uint8_t)checksum_read != expected_checksum)
        return -2;

    return 0;
}
