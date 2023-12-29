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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base64.h"
#include "common.h"

static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

unsigned int base64_encode(char* data, char const* buffer, unsigned long length)
{
  unsigned long output_length = 4 * ((length + 2) / 3);

  DEBUG_ASSERT(data != NULL);

  for (int i = 0, j = 0; i < length;) {
    unsigned long octet_a = i < length ? (unsigned char) buffer[i++] : 0;
    unsigned long octet_b = i < length ? (unsigned char) buffer[i++] : 0;
    unsigned long octet_c = i < length ? (unsigned char) buffer[i++] : 0;

    unsigned long triple = (octet_a << 16) + (octet_b << 8) + octet_c;

    data[j++] = base64_chars[(triple >> 18) & 0x3F];
    data[j++] = base64_chars[(triple >> 12) & 0x3F];
    data[j++] = base64_chars[(triple >> 6) & 0x3F];
    data[j++] = base64_chars[(triple >> 0) & 0x3F];
  }

  static const int mod_table[] = {0, 2, 1};

  for (int i = 0; i < mod_table[length % 3]; i++)
    data[output_length - 1 - i] = '=';

  data[output_length] = '\0';

  return output_length;
}

/* Convert a base64 character to its integer value.
 *  @param c The base64 character to convert.
 *  @return The integer value.
 */
unsigned char base64_char_value(char c)
{
  if (c >= 'A' && c <= 'Z')
    return c - 'A';
  if (c >= 'a' && c <= 'z')
    return c - 'a' + 26;
  if (c >= '0' && c <= '9')
    return c - '0' + 52;
  if (c == '+')
    return 62;
  if (c == '/')
    return 63;

  return -1;
}

unsigned int base64_sanitize(char* output, char const* input, unsigned long length)
{
  DEBUG_ASSERT(input != NULL);

  if (length == 0)
    return 0;

  if (output == NULL)
    output = malloc(length);

  DEBUG_ASSERT(output != NULL);

  unsigned int output_length = 0;
  bool         in_comment    = false;

  for (unsigned long i = 0; i < length; i++) {
    char ch = input[i];

    if (ch == '#' && (i == 0 || input[i - 1] == '\n' || input[i - 1] == '\r')) {
      in_comment = true;
    }

    if (ch == '\n' || ch == '\r') {
      in_comment = false;
      continue;
    }

    if (!in_comment && ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r') {
      output[output_length++] = ch;
    }
  }

  output[output_length] = '\0';

  return output_length;
}

unsigned int base64_decode(char* data, char const* buffer, unsigned long length)
{
  if (length % 4 != 0)
    return 0;

  unsigned int output_length = length / 4 * 3;

  if (buffer[length - 1] == '=')
    output_length--;
  if (buffer[length - 2] == '=')
    output_length--;

  if (data == NULL)
    data = malloc(output_length);

  DEBUG_ASSERT(data != NULL);

  for (unsigned long i = 0, j = 0; i < length;) {
    unsigned long sextet_a = buffer[i] == '=' ? 0 & i++ : base64_char_value(buffer[i++]);
    unsigned long sextet_b = buffer[i] == '=' ? 0 & i++ : base64_char_value(buffer[i++]);
    unsigned long sextet_c = buffer[i] == '=' ? 0 & i++ : base64_char_value(buffer[i++]);
    unsigned long sextet_d = buffer[i] == '=' ? 0 & i++ : base64_char_value(buffer[i++]);

    unsigned long triple = (sextet_a << 3 * 6) + (sextet_b << 2 * 6) + (sextet_c << 1 * 6) + (sextet_d << 0 * 6);

    if (j < output_length)
      data[j++] = (triple >> 2 * 8) & 0xFF;
    if (j < output_length)
      data[j++] = (triple >> 1 * 8) & 0xFF;
    if (j < output_length)
      data[j++] = (triple >> 0 * 8) & 0xFF;
  }

  return output_length;
}
