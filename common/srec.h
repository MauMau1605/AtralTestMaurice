/*
 * srec.h
 *
 * Minimal SREC (Motorola S-record) encoder/decoder.
 * Supports S0 (header/metadata), S1 (16-bit address data), S9 (end of file).
 */
#ifndef SREC_H
#define SREC_H

#include <stddef.h>
#include <stdint.h>

#define SREC_MAX_DATA     32   /* max data bytes per record */
#define SREC_MAX_LINE_LEN 128  /* max length of an encoded ASCII line, incl. '\0' */

typedef struct {
    char     type;               /* '0', '1', or '9' */
    uint16_t address;            /* 16-bit address (S1) or unused (S0/S9) */
    uint8_t  data[SREC_MAX_DATA];
    uint8_t  data_len;
} srec_record_t;

/*
 * Encode a record into an ASCII SREC line (no trailing newline).
 * Returns the line length on success, -1 on error (buffer too small, bad data_len).
 */
int srec_encode(const srec_record_t *rec, char *out_line, size_t out_size);

/*
 * Decode an ASCII SREC line into a record.
 * Returns 0 on success.
 * Returns -1 if the line is malformed (bad format, odd hex length, too short).
 * Returns -2 if the checksum does not match (data integrity failure).
 */
int srec_decode(const char *line, srec_record_t *rec);

#endif /* SREC_H */
