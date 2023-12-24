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
#ifndef _ZIGMATIQ_BUFFER_H_
#define _ZIGMATIQ_BUFFER_H_

#include "typedef.h"

#define ZQ_BUFFER_DEFAULT_CAPACITY (1024 * 1024) /* 1MB */

/* Unified "binary-string" object for manipulation.
 * The buffer object is a simple wrapper around a uint8 array.
 */
typedef struct Buffer {
  /* The data array. */
  uint8* data;

  /* The length of the data array. */
  uint64 length;

  /* The capacity of the data array. */
  uint64 capacity;
} Buffer; /* Buffer */

/* Initialize a buffer object.
 * Will always allocate at least ZQ_BUFFER_DEFAULT_CAPACITY bytes.
 *   @param buffer The buffer object.
 *   @param length The requested length.
 *   @return The buffer object.
 *   @note If buffer is NULL, a new buffer object will be allocated.
 */
Buffer* BufferCreate(Buffer* buffer, uint64 length);

/* Create a new buffer from a uint8 array.
 *   @param buffer The buffer object.
 *   @param data The data array.
 *   @param length The length of the data array.
 *   @return The buffer object.
 *   @note If buffer is NULL, a new buffer object will be allocated.
 */
Buffer* BufferCreateCopy(Buffer* buffer, const uint8* data, uint64 length);

/* Clone a buffer object.
 *   @param buffer The buffer object.
 *   @param other The buffer to clone.
 *   @return The buffer object.
 *   @note If buffer is NULL, a new buffer object will be allocated.
 */
Buffer* BufferClone(Buffer* buffer, const Buffer* other);

/* Destroy a buffer object.
 *   @param buffer The buffer object.
 */
void BufferDestroy(Buffer* buffer);

/* Resize a buffer object.
 *   @param buffer The buffer object.
 *   @param length The new length.
 *   @return The buffer object.
 */
Buffer* BufferResize(Buffer* buffer, uint64 length);

/* Print a buffer object.
 *   @param buffer The buffer object.
 */
void BufferPrint(const Buffer* buffer);

#endif /* _ZIGMATIQ_BUFFER_H_ */
