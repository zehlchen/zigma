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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#include "zigma.h"

ZigmaContext* ZigmaCreate(ZigmaContext* context, const char* key, uint64 length)
{
  if (context == NULL)
    context = (ZigmaContext*) malloc(sizeof(ZigmaContext));

  if (key == NULL) {
    ZigmaCreateHash(context);

    return context;
  }

  uint8  toswap   = 0;
  uint8  swaptemp = 0;
  uint8  rsum     = 0;
  uint32 keypos   = 0;

  /* Populate the permutation vector. */
  for (int i = 0, j = 255; i < 256; i++, j--)
    context->state[i] = j;

  /* Randomize the permutation vector with the key. */
  for (int i = 255; i >= 0; i--) {
    toswap = ZigmaKeyRandom(context, i, key, length, &rsum, &keypos);

    swaptemp               = context->state[i];
    context->state[i]      = context->state[toswap];
    context->state[toswap] = swaptemp;
  }

  context->index_A = context->state[1];
  context->index_B = context->state[3];
  context->index_C = context->state[5];
  context->byte_X  = context->state[7];
  context->byte_Y  = context->state[rsum];

  return context;
}

ZigmaContext* ZigmaCreateHash(ZigmaContext* context)
{
  DEBUG_ASSERT(context != NULL);

  context->index_A = 1;
  context->index_B = 3;
  context->index_C = 5;
  context->byte_X  = 7;
  context->byte_Y  = 11;

  for (int i = 0, j = 255; i < 256; i++, j--) {
    context->state[i] = (uint8) j;
  }

  return context;
}

uint8 ZigmaKeyRandom(ZigmaContext* context, uint32 limit, uint8 const* key, uint32 length, uint8* rsum, uint32* keypos)
{
  uint32 u;
  uint32 retry_limiter = 0;
  uint32 mask          = 1;

  while (mask < limit)
    mask = (mask << 1) + 1;

  do {
    *rsum = context->state[(unsigned) *rsum] + key[(*keypos)++];

    if (*keypos >= length) {
      *keypos = 0;
      *rsum += length;
    }

    u = mask & *rsum;

    if (++retry_limiter > 11)
      u %= limit;

  } while (u > limit);

  return u;
}

void ZigmaHashFinal(ZigmaContext* context, uint8* data, uint32 length)
{
  /* Advance the permutation vector. */
  for (int i = 255; i >= 0; i--) {
    ZigmaEncodeByte(context, i);
  }

  /* Encrypt 0 to desired length to populate hash value */
  for (int i = 0; i < length; i++)
    data[i] = ZigmaEncodeByte(context, 0);
}

uint8 ZigmaEncodeByte(ZigmaContext* context, uint8 byte)
{
  uint8 swaptemp;

  context->index_B += context->state[context->index_A++];

  swaptemp                         = context->state[context->byte_Y];
  context->state[context->byte_Y]  = context->state[context->index_B];
  context->state[context->index_B] = context->state[context->byte_X];
  context->state[context->byte_X]  = context->state[context->index_A];
  context->state[context->index_A] = swaptemp;

  context->index_C += context->state[swaptemp];

  context->byte_Y = byte ^
                    context->state[(context->state[context->index_B] + context->state[context->index_A]) & 0xFF] ^
                    context->state[context->state[(context->state[context->byte_X] + context->state[context->byte_Y] +
                                                   context->state[context->index_C]) &
                                                  0xFF]];

  context->byte_X = byte;

  return context->byte_Y;
}

uint8 ZigmaDecodeByte(ZigmaContext* context, uint8 byte)
{
  uint8 swaptemp;

  context->index_B += context->state[context->index_A++];

  swaptemp                         = context->state[context->byte_Y];
  context->state[context->byte_Y]  = context->state[context->index_B];
  context->state[context->index_B] = context->state[context->byte_X];
  context->state[context->byte_X]  = context->state[context->index_A];
  context->state[context->index_A] = swaptemp;

  context->index_C += context->state[swaptemp];

  context->byte_X = byte ^
                    context->state[(context->state[context->index_B] + context->state[context->index_A]) & 0xFF] ^
                    context->state[context->state[(context->state[context->byte_X] + context->state[context->byte_Y] +
                                                   context->state[context->index_C]) &
                                                  0xFF]];

  context->byte_Y = byte;

  return context->byte_X;
}

void ZigmaEncodeBuffer(ZigmaContext* context, Buffer* buffer)
{
  DEBUG_ASSERT(context != NULL);
  DEBUG_ASSERT(buffer != NULL);

  for (int i = 0; i < buffer->length; i++)
    buffer->data[i] = ZigmaEncodeByte(context, buffer->data[i]);
}

void ZigmaDecodeBuffer(ZigmaContext* context, Buffer* buffer)
{
  DEBUG_ASSERT(context != NULL);
  DEBUG_ASSERT(buffer != NULL);

  for (int i = 0; i < buffer->length; i++)
    buffer->data[i] = ZigmaDecodeByte(context, buffer->data[i]);
}

void ZigmaPrint(ZigmaContext* context)
{
  DEBUG_ASSERT(context != NULL);

  fprintf(stderr, "! ZIGMA CONTEXT:\n");
  fprintf(stderr, "  index_A = %d\n", context->index_A);
  fprintf(stderr, "  index_B = %d\n", context->index_B);
  fprintf(stderr, "  index_C = %d\n", context->index_C);
  fprintf(stderr, "  byte_X  = %d\n", context->byte_X);
  fprintf(stderr, "  byte_Y  = %d\n", context->byte_Y);
  fprintf(stderr, "  state   = {\n");

  for (int i = 0; i < 256; i++) {
    fprintf(stderr, "%02x\n", context->state[i]);

    if ((i + 1) % 16 == 0)
      fprintf(stderr, "\n");
  }
  fprintf(stderr, "\r  }\n");
}