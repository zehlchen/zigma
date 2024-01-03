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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#include "base64.h"
#include "buffer.h"

Buffer* BufferCreate(Buffer* buffer, uint64 length)
{
  if (buffer == NULL)
    buffer = (Buffer*) malloc(sizeof(Buffer));

  uint64 toAllocate = length < ZQ_BUFFER_DEFAULT_CAPACITY ? ZQ_BUFFER_DEFAULT_CAPACITY : length;

  buffer->data     = (uint8*) malloc(toAllocate);
  buffer->length   = length;
  buffer->capacity = toAllocate;

  return buffer;
}

Buffer* BufferCreateCopy(Buffer* buffer, const uint8* data, uint64 length)
{
  if (buffer == NULL)
    buffer = (Buffer*) malloc(sizeof(Buffer));

  uint64 toAllocate = length < ZQ_BUFFER_DEFAULT_CAPACITY ? ZQ_BUFFER_DEFAULT_CAPACITY : length;

  buffer->data     = (uint8*) malloc(toAllocate);
  buffer->length   = length;
  buffer->capacity = toAllocate;

  memcpy(buffer->data, data, length);

  return buffer;
}

Buffer* BufferClone(Buffer* buffer, const Buffer* other)
{
  if (buffer == NULL)
    buffer = (Buffer*) malloc(sizeof(Buffer));

  uint64 toAllocate = other->capacity;

  buffer->data     = (uint8*) malloc(toAllocate);
  buffer->length   = other->length;
  buffer->capacity = toAllocate;

  memcpy(buffer->data, other->data, other->length);

  return buffer;
}

void BufferDestroy(Buffer* buffer)
{
  if (buffer == NULL)
    return;

  while (buffer->length--)
    buffer->data[buffer->length] = 0;

  free(buffer->data);
  free(buffer);
}

Buffer* BufferResize(Buffer* buffer, uint64 length)
{
  if (buffer == NULL)
    return BufferCreate(buffer, length);

  if (length <= buffer->capacity) {
    buffer->length = length;
    return buffer;
  }

  uint64 toAllocate = length < ZQ_BUFFER_DEFAULT_CAPACITY ? ZQ_BUFFER_DEFAULT_CAPACITY : length;

  buffer->data     = (uint8*) realloc(buffer->data, toAllocate);
  buffer->length   = length;
  buffer->capacity = toAllocate;

  return buffer;
}

void BufferDebugPrint(const Buffer* buffer)
{
  DEBUG_ASSERT(buffer != NULL);

  fprintf(stderr, "Buffer: %p\n", buffer);
  fprintf(stderr, "  Length:   %lu\n", buffer->length);
  fprintf(stderr, "  Capacity: %lu\n", buffer->capacity);
  fprintf(stderr, "  Data:     %p = {\n    ", buffer->data);

  for (int i = 0; i < buffer->length; i++) {
    fprintf(stderr, "%02x", buffer->data[i]);

    if ((i + 1) % 32 == 0)
      fprintf(stderr, "\n    ");
  }

  fprintf(stderr, "\r  }\n");
}

uint64 BufferPrintBase16(Buffer* buffer, FILE* stream)
{
  for (uint64 i = 0; i < buffer->length; i++) {
    fprintf(stream, "%02x", buffer->data[i]);
  }

  fflush(stream);

  return buffer->length * 2;
}

uint64 BufferPrintBase64(Buffer* buffer, FILE* stream)
{
  char*  encoded = malloc(2 * buffer->length); /* malloc(4 * ((buffer->length + 2) / 3)); */
  uint64 length  = base64_encode(encoded, (char*) buffer->data, buffer->length);

  for (int i = 0; i < length; i++) {
    fprintf(stream, "%c", encoded[i]);

    if ((i + 1) % 76 == 0)
      fprintf(stream, "\n");
  }

  fflush(stream);

  free(encoded);

  return length;
}

uint64 BufferReadBase256(Buffer* buffer, FILE* stream)
{
  DEBUG_ASSERT(buffer != NULL);
  DEBUG_ASSERT(stream != NULL);

  uint8* data = malloc(ZQ_MAX_BUFFER_SIZE);

  uint64 count = 0;
  uint64 total = 0;

  while ((count = fread(data, 1, ZQ_MAX_BUFFER_SIZE, stream)) > 0) {
    BufferResize(buffer, count + total);

    memcpy(buffer->data + total, data, count);

    total += count;
  }

  free(data);

  return total;
}

uint64 BufferReadBase16(Buffer* buffer, FILE* stream)
{
  DEBUG_ASSERT(buffer != NULL);
  DEBUG_ASSERT(stream != NULL);

  uint8* data = malloc(ZQ_MAX_BUFFER_SIZE);

  uint64 count = 0;
  uint64 total = 0;

  while ((count = fread(data, 1, ZQ_MAX_BUFFER_SIZE, stream)) > 0) {
    BufferResize(buffer, count + total);

    for (int i = 0; i < count; i += 2) {
      uint8 byte = 0;

      if (data[i] >= '0' && data[i] <= '9')
        byte = data[i] - '0';
      else if (data[i] >= 'a' && data[i] <= 'f')
        byte = data[i] - 'a' + 10;
      else if (data[i] >= 'A' && data[i] <= 'F')
        byte = data[i] - 'A' + 10;
      else
        continue;

      byte <<= 4;

      if (data[i + 1] >= '0' && data[i + 1] <= '9')
        byte |= data[i + 1] - '0';
      else if (data[i + 1] >= 'a' && data[i + 1] <= 'f')
        byte |= data[i + 1] - 'a' + 10;
      else if (data[i + 1] >= 'A' && data[i + 1] <= 'F')
        byte |= data[i + 1] - 'A' + 10;
      else
        continue;

      buffer->data[total++] = byte;
    }
  }

  free(data);

  return total;
}

uint64 BufferReadBase64(Buffer* buffer, FILE* stream)
{
  DEBUG_ASSERT(buffer != NULL);
  DEBUG_ASSERT(stream != NULL);

  Buffer* readBuffer = BufferCreate(NULL, ZQ_MAX_BUFFER_SIZE);

  char* data = malloc(ZQ_MAX_BUFFER_SIZE);

  uint64 count = 0;
  uint64 total = 0;

  while ((count = fread(data, 1, ZQ_MAX_BUFFER_SIZE, stream)) > 0) {
    BufferResize(readBuffer, count + total);
    memcpy(readBuffer->data + total, data, count);
    memset(data, 0, ZQ_MAX_BUFFER_SIZE);
    total += count;
  }

  uint8* sanitized        = malloc(total);
  uint64 sanitized_length = base64_sanitize(sanitized, readBuffer->data, total);

  fprintf(stderr, "\n\n");
  fprintf(stderr, "sanitized: %s\n", sanitized);
  fprintf(stderr, "sanitized_length: %lu\n", sanitized_length);

  buffer->length = base64_decode(buffer->data, sanitized, sanitized_length);

  BufferDebugPrint(buffer);

  free(sanitized);
  free(data);

  return total;
}