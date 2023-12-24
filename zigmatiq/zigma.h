/*
 * ZIGMAtiq, Copyright (C) 2024 Chase Zehl O'Byrne
 *   <mail: zehl@live.com> http://zehlchen.com/
 *
 * This file is part of ZIGMAtiq.
 *
 * ZIGMAtiq is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * ZIGMAtiq is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ZIGMAtiq; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef _ZIGMATIQ_ZIGMA_H_
#define _ZIGMATIQ_ZIGMA_H_

#include "common.h"

#include "buffer.h"

typedef struct ZigmaContext {
  uint8 index_A;
  uint8 index_B;
  uint8 index_C;

  uint8 byte_X;
  uint8 byte_Y;

  /* The permutation vector. */
  uint8 state[256];
} ZigmaContext;

/* Initialize a ZIGMA context with a key of the given length. This function accepts NULL as the context
 * argument, in which case it allocates a new context. Otherwise, it uses the context provided. If no key
 * is provided, the context is initialized to be used as a hash function.
 *   @param context The context to initialize, or NULL to allocate a new context.
 *   @param key The key to use, or NULL to initialize the context as a hash function.
 *   @param length The length of the key, or 0 if no key is provided.
 *   @return The initialized context, or NULL if the context could not be allocated.
 */
ZigmaContext* ZigmaCreate(ZigmaContext* context, const char* key, uint64 length);

/* Helper function used by `ZigmaCreate()` to initialize the context as a hash function.
 *   @param context The context to be used as a hash function.
 *   @return The initialized context.
 */
ZigmaContext* ZigmaCreateHash(ZigmaContext* context);

/* Helper function used by `ZigmaCreate()` to randomize and permute the context with a key. The function will return
 * a byte based on the key, the permutation vector, and an index running sum. The index running sum is used to prevent
 * a short key from being repeated and degrading security.
 *   @param context The context to be used as a generator.
 *   @param limit The maximum value to return.
 *   @param key The key being used to permute the context.
 *   @param length The length of the key.
 *   @param rsum The index running sum.
 *   @param keypos Position within the key
 *   @return A byte based on the key, the permutation vector, and an index running sum.
 */
uint8 ZigmaKeyRandom(ZigmaContext* context, uint32 limit, uint8 const* key, uint32 length, uint8* rsum, uint32* keypos);

/* Used to terminate a context to generate a hash value based on the permutation vector.
 *   @param context The context to be used as a hash function.
 *   @param data Pointer to location where the hash value will be stored.
 *   @param length The length of the hash value to generate.
 */
void ZigmaHashFinal(ZigmaContext* context, uint8* data, uint32 length);

/* Encode a byte.
 *   @param context The context to be used for encoding.
 *   @param byte The byte to encode.
 *   @return The encoded byte.
 */
uint8 ZigmaEncodeByte(ZigmaContext* context, uint8 byte);

/* Decode a byte.
 *   @param context The context to be used for decoding.
 *   @param byte The byte to decode.
 *   @return The decoded byte.
 */
uint8 ZigmaDecodeByte(ZigmaContext* context, uint8 byte);

/* Encode a buffer.
 *   @param context The context to be used for encoding.
 *   @param buffer The buffer to encode.
 */
void ZigmaEncodeBuffer(ZigmaContext* context, Buffer* buffer);

/* Decode a buffer.
 *   @param context The context to be used for decoding.
 *   @param buffer The buffer to decode.
 */
void ZigmaDecodeBuffer(ZigmaContext* context, Buffer* buffer);

#endif /* _ZIGMATIQ_ZIGMA_H_ */
