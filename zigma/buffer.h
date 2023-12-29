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
#ifndef _ZIGMATIQ_BUFFER_H_
#define _ZIGMATIQ_BUFFER_H_

#include "buffer.h"

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

void BufferDebugPrint(const Buffer* buffer);

/* Print a buffer to a stream in base16.
 *   @param buffer The buffer object.
 *   @param stream The stream to print to.
 *   @return The number of bytes printed.
 */
uint64 BufferPrintBase16(Buffer* buffer, FILE* stream);

/* Print a buffer to a stream in base64.
 *   @param buffer The buffer object.
 *   @param stream The stream to print to.
 *   @return The number of bytes printed.
 */
uint64 BufferPrintBase64(Buffer* buffer, FILE* stream);

/* Read a buffer from a stream.
 *   @param buffer The buffer object.
 *   @param stream The stream to read from.
 *   @return The number of bytes read.
 */
uint64 BufferReadBase256(Buffer* buffer, FILE* stream);
uint64 BufferReadBase64(Buffer* buffer, FILE* stream);
uint64 BufferReadBase16(Buffer* buffer, FILE* stream);

#endif /* _ZIGMATIQ_BUFFER_H_ */
