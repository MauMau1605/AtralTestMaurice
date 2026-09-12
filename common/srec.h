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

/**
 * @brief Encode a record into an ASCII SREC line without trailing newline.
 * @param rec Input record to encode.
 * @param out_line Output buffer that receives the encoded ASCII line.
 * @param out_size Size of out_line in bytes.
 * @return Encoded line length on success, -1 on error.
 */
int srec_encode(const srec_record_t *rec, char *out_line, size_t out_size);

/**
 * @brief Decode an ASCII SREC line into a record.
 * @param line Null-terminated ASCII SREC input line.
 * @param rec Output record filled with decoded content.
 * @return 0 on success, -1 on malformed input, -2 on checksum mismatch.
 */
int srec_decode(const char *line, srec_record_t *rec);

#endif /* SREC_H */
