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

#pragma once
#ifndef _ZIGMATIQ_COMMON_H_
#define _ZIGMATIQ_COMMON_H_

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

#define ZIGMATIQ_VERSION_STRING_SHORT "ZIGMAtiq-" ZIGMATIQ_VERSION_STRING

void PrintVersion();

/* Define the length in bytes of the checksum. */
#ifndef ZIGMA_CHECKSUM_SIZE
#define ZIGMA_CHECKSUM_SIZE 32 /* 256 bits */
#endif

/*
 * Debug code ... respect no-debug requests.
 */
#ifndef NDEBUG
#define DEBUG_ASSERT(x)                            \
  if (!(x)) {                                      \
    fprintf(stderr, "assertion failed: %s\n", #x); \
    exit(EXIT_FAILURE);                            \
  }
#else
#define DEBUG_ASSERT(x)
#endif

#endif /* _ZIGMATIQ_COMMON_H_ */