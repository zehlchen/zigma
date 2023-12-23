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

#include "common.h"

void PrintVersion()
{
  fprintf(stderr, "ZIGMAtiq-%s/%s@%s\n", ZIGMATIQ_VERSION_STRING, ZIGMATIQ_GIT_BRANCH, ZIGMATIQ_GIT_COMMIT);
  fprintf(stderr, "  Copyright (C) 2024 Chase Zehl O'Byrne <zehl (at) live.com>\n");
  fprintf(stderr, "  Built: %s (%s) - %s\n", __DATE__, __TIME__, __VERSION__);
}

void Nullify(void* ptr, uint64 size)
{
  volatile unsigned char* _ptr = ptr;

  while (size--)
    *_ptr++ = 0;
}