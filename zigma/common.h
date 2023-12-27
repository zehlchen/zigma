/*
 * ZIGMA, Copyright (C) 2024 Chase Zehl O'Byrne
 *   <mail: zehl@live.com> http://zehlchen.com/
 *
 * This file is part of ZIGMA.
 *
 * ZIGMA is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * ZIGMA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ZIGMA; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#pragma once
#ifndef _ZIGMATIQ_COMMON_H_
#define _ZIGMATIQ_COMMON_H_

#include <stdio.h>

#include "typedef.h"

#ifndef ZIGMATIQ_GIT_BUILD
#define ZIGMATIQ_GIT_BUILD "unknown"
#endif

#ifndef ZIGMATIQ_GIT_COMMIT
#define ZIGMATIQ_GIT_COMMIT "unknown"
#endif

#ifndef ZIGMATIQ_GIT_BRANCH
#define ZIGMATIQ_GIT_BRANCH "unknown"
#endif

#ifndef ZIGMATIQ_GIT_TAG
#define ZIGMATIQ_GIT_TAG "unknown"
#endif

#ifndef ZIGMATIQ_VERSION_STRING
#define ZIGMATIQ_VERSION_STRING "unknown"
#endif

#define ZIGMATIQ_VERSION_STRING_SHORT "ZIGMA-" ZIGMATIQ_VERSION_STRING

void PrintVersion();

uint32 uint32_min(uint32 a, uint32 b);

uint32 LevenshteinDistance(const char* s, const char* t);

unsigned long CaptureKey(uint8* buffer, const uint8* prompt);

FILE* OpenFile(const char* filename, const char* mode);

/* Define the length in bytes of the checksum. */
#ifndef ZIGMA_CHECKSUM_SIZE
#define ZIGMA_CHECKSUM_SIZE 36 /* 288 bits */
#endif

/* Define the maximum length of a passkey. */
#ifndef ZQ_MAX_KEY_SIZE
#define ZQ_MAX_KEY_SIZE 256
#endif

#ifndef ZQ_MAX_BUFFER_SIZE
#define ZQ_MAX_BUFFER_SIZE 768
#endif

/*
 * Debug code ... respect no-debug requests.
 */
#ifndef NDEBUG
#define DEBUG_ASSERT(x)                                                       \
  if (!(x)) {                                                                 \
    fprintf(stderr, "%s:%d: assertion failed: %s\n", __FILE__, __LINE__, #x); \
    exit(EXIT_FAILURE);                                                       \
  }
#else
#define DEBUG_ASSERT(x)
#endif

/* Wipes a pointer to a memory location.
 * Write zeroes to the memory location pointed to by `ptr` (up to `size` bytes).
 *   @param ptr  The pointer to the memory location.
 *   @param size The size of the memory location.
 */
void Nullify(void* ptr, uint64 size);
#endif /* _ZIGMATIQ_COMMON_H_ */