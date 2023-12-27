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

#ifndef _ZIGMATIQ_BASE64_H_
#define _ZIGMATIQ_BASE64_H_

/* Encode a buffer to base64.
 *  @param data The output buffer, or NULL to allocate one.
 *  @param buffer The input buffer.
 *  @param length The length of the input buffer.
 *  @return The length of the output buffer.
 */
unsigned int base64_encode(char* data, char const* buffer, unsigned long length);

/* Decode a base64 buffer.
 *  @param data The output buffer, or NULL to allocate one.
 *  @param buffer The input buffer.
 *  @param length The length of the input buffer.
 *  @return The length of the output buffer.
 */
unsigned int base64_decode(char* data, char const* buffer, unsigned long length);

/* Sanitize a buffer by removing comments and whitespace.
 *  @param output The output buffer, or NULL to allocate one.
 *  @param input The input buffer.
 *  @param length The length of the input buffer.
 *  @return The length of the output buffer.
 */
unsigned int base64_sanitize(char* output, char const* input, unsigned long length);

#endif /* _ZIGMATIQ_BASE64_H_ */