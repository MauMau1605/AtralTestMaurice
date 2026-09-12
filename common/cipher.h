/*
 * cipher.h
 *
 * Cryptographic/obfuscation abstraction layer.
*/
#ifndef CIPHER_H
#define CIPHER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Standard cipher IDs transmitted in the S0 address field */
#define CIPHER_NONE 0x0000
#define CIPHER_XOR  0x0001
#define CIPHER_MOD  0x0002

/**
 * @brief Check whether a cipher algorithm is supported.
 * @param cipher_type Cipher identifier to validate.
 * @return true if supported, false otherwise.
 */
bool cipher_is_supported(uint16_t cipher_type);

/**
 * @brief Get the human-readable name of a cipher algorithm.
 * @param cipher_type Cipher identifier.
 * @return Pointer to a static string containing the cipher name.
 */
const char *cipher_get_name(uint16_t cipher_type);

/**
 * @brief Encrypt a buffer using the specified cipher type and key.
 * @param cipher_type Cipher identifier.
 * @param in Input buffer to encrypt.
 * @param out Output buffer that receives encrypted bytes.
 * @param len Number of bytes to process.
 * @param key Cipher key.
 * @return 0 on success, -1 on invalid parameter or unsupported cipher.
 */
int cipher_encrypt(uint16_t cipher_type, const uint8_t *in, uint8_t *out, size_t len, uint8_t key);

/**
 * @brief Decrypt a buffer using the specified cipher type and key.
 * @param cipher_type Cipher identifier.
 * @param in Input buffer to decrypt.
 * @param out Output buffer that receives decrypted bytes.
 * @param len Number of bytes to process.
 * @param key Cipher key.
 * @return 0 on success, -1 on invalid parameter or unsupported cipher.
 */
int cipher_decrypt(uint16_t cipher_type, const uint8_t *in, uint8_t *out, size_t len, uint8_t key);

#endif /* CIPHER_H */
