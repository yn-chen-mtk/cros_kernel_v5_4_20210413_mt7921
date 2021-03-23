/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright 2014-2015, Qualcomm Atheros, Inc.
 */

#ifndef AES_GCM_H
#define AES_GCM_H

#include <linux/crypto.h>

static inline void
ieee80211_aes_gcm_encrypt(struct crypto_aead *tfm, u8 *j_0, u8 *aad,
			  u8 *data, size_t data_len, u8 *mic)
{
}

static inline int
ieee80211_aes_gcm_decrypt(struct crypto_aead *tfm, u8 *j_0, u8 *aad,
			  u8 *data, size_t data_len, u8 *mic)
{
    return -EOPNOTSUPP;
}

static inline struct crypto_aead *
ieee80211_aes_gcm_key_setup_encrypt(const u8 key[], size_t key_len)
{
    return NULL;
}

static inline void
ieee80211_aes_gcm_key_free(struct crypto_aead *tfm)
{
}

#endif /* AES_GCM_H */
