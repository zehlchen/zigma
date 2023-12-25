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
#include <unistd.h>

#ifdef __linux__
#include <termios.h>
#endif

#include "common.h"

#include "buffer.h"

void PrintVersion()
{
  fprintf(stderr, "ZIGMAtiq-%s/%s@%s\n", ZIGMATIQ_VERSION_STRING, ZIGMATIQ_GIT_BRANCH, ZIGMATIQ_GIT_COMMIT);
  fprintf(stderr, "  Copyright (C) 2024 Chase Zehl O'Byrne <zehl (at) live.com>\n\n");

#if 0
  fprintf(stderr, "  Built: %s (%s) - %s\n", __DATE__, __TIME__, __VERSION__);
#endif
}

void Nullify(void* ptr, uint64 size)
{
  volatile unsigned char* _ptr = ptr;

  while (size--)
    *_ptr++ = 0;
}

uint32 uint32_min(uint32 a, uint32 b)
{
  return (a < b) ? a : b;
}

uint32 LevenshteinDistance(const char* s, const char* t)
{
  int    ls = strlen(s), lt = strlen(t);
  uint32 matrix[ls + 1][lt + 1];

  for (int i = 0; i <= ls; i++)
    matrix[i][0] = i;
  for (int j = 0; j <= lt; j++)
    matrix[0][j] = j;

  for (int i = 1; i <= ls; i++) {
    for (int j = 1; j <= lt; j++) {
      uint32 cost  = (s[i - 1] == t[j - 1]) ? 0 : 1;
      matrix[i][j] = uint32_min(matrix[i - 1][j] + 1,                     // Deletion
                                uint32_min(matrix[i][j - 1] + 1,          // Insertion
                                           matrix[i - 1][j - 1] + cost)); // Substitution
    }
  }
  return matrix[ls][lt];
}

unsigned long CaptureKey(uint8* buffer, const uint8* prompt)
{
#ifdef __linux__
  struct termios old_term;
  struct termios new_term;

  tcgetattr(STDIN_FILENO, &old_term);
  new_term = old_term;

  new_term.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
#endif /* __linux__ */

  fprintf(stderr, "%s", prompt);

  int index = 0;

  while (1) {
    char ch = getchar();
    if (ch == '\n' || ch == '\r') { // Enter key
      buffer[index] = '\0';
      break;
    }
    else if (ch == 127 || ch == '\b') { // Backspace key
      if (index > 0) {
        index--;
        fputs("\b \b", stderr);
      }
    }
    else if (index < ZQ_MAX_KEY_SIZE - 1) {
      buffer[index++] = ch;
      fputc('*', stderr);
    }
  }

#ifdef __linux__
  tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
#endif /* __linux__ */

  fprintf(stderr, "\r\n");

  return index;
}

void PrintHex(Buffer* buffer)
{
  for (uint64 i = 0; i < buffer->length; i++) {
    fprintf(stderr, "%02x", buffer->data[i]);

    if ((i + 1) % 64 == 0)
      fprintf(stderr, "\n");
    else if ((i + 1) % 8 == 0)
      fprintf(stderr, "  ");
    else
      fprintf(stderr, " ");
  }
  fprintf(stderr, "\n");
}