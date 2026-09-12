/*
 * cipher.c
 *
 * Implementation of cipher registry and transformations.
 */
#include <string.h>
#include "cipher.h"

typedef void (*cipher_transform_fn)(const uint8_t *in, uint8_t *out, size_t len, uint8_t key);

typedef struct {
    uint16_t            type;
    const char         *name;
    cipher_transform_fn encrypt;
    cipher_transform_fn decrypt;
} cipher_descriptor_t;

/* --- CIPHER_NONE implementation --- */
static void cipher_none_transform(const uint8_t *in, uint8_t *out, size_t len, uint8_t key)
{
    (void)key;
    memcpy(out, in, len);
}

/* --- CIPHER_XOR implementation --- */
static void cipher_xor_transform(const uint8_t *in, uint8_t *out, size_t len, uint8_t key)
{
    for (size_t i = 0; i < len; i++) {
        out[i] = (uint8_t)(in[i] ^ key);
    }
}

/* --- CIPHER_MOD implementation (Addition modulo 256) --- */
static void cipher_mod_encrypt(const uint8_t *in, uint8_t *out, size_t len, uint8_t key)
{
    for (size_t i = 0; i < len; i++) {
        out[i] = (uint8_t)(in[i] + key);
    }
}

static void cipher_mod_decrypt(const uint8_t *in, uint8_t *out, size_t len, uint8_t key)
{
    for (size_t i = 0; i < len; i++) {
        out[i] = (uint8_t)(in[i] - key);
    }
}

/*
 * Dispatch cypher table
 */
static const cipher_descriptor_t CIPHER_REGISTRY[] = {
    { CIPHER_NONE, "NONE", cipher_none_transform, cipher_none_transform },
    { CIPHER_XOR,  "XOR",  cipher_xor_transform,  cipher_xor_transform  },
    { CIPHER_MOD,  "MOD",  cipher_mod_encrypt,    cipher_mod_decrypt    }
};

#define CIPHER_REGISTRY_SIZE (sizeof(CIPHER_REGISTRY) / sizeof(CIPHER_REGISTRY[0]))

static const cipher_descriptor_t *cipher_find(uint16_t cipher_type)
{
    for (size_t i = 0; i < CIPHER_REGISTRY_SIZE; i++) {
        if (CIPHER_REGISTRY[i].type == cipher_type) {
            return &CIPHER_REGISTRY[i];
        }
    }
    return NULL;
}

bool cipher_is_supported(uint16_t cipher_type)
{
    return cipher_find(cipher_type) != NULL;
}

const char *cipher_get_name(uint16_t cipher_type)
{
    const cipher_descriptor_t *desc = cipher_find(cipher_type);
    return desc ? desc->name : "UNKNOWN";
}

int cipher_encrypt(uint16_t cipher_type, const uint8_t *in, uint8_t *out, size_t len, uint8_t key)
{
    if (!in || !out) {
        return -1;
    }
    const cipher_descriptor_t *desc = cipher_find(cipher_type);
    if (!desc || !desc->encrypt) {
        return -1;
    }
    desc->encrypt(in, out, len, key);
    return 0;
}

int cipher_decrypt(uint16_t cipher_type, const uint8_t *in, uint8_t *out, size_t len, uint8_t key)
{
    if (!in || !out) {
        return -1;
    }
    const cipher_descriptor_t *desc = cipher_find(cipher_type);
    if (!desc || !desc->decrypt) {
        return -1;
    }
    desc->decrypt(in, out, len, key);
    return 0;
}
