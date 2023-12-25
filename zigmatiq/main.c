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

#include "base64.h"
#include "buffer.h"
#include "registry.h"

typedef enum OperationType { OP_UNKNOWN = 0, OP_ENCODE, OP_DECODE, OP_SIGN, OP_CHECK } OperationType;
typedef void (*OperationFunction)(RegistryNode** registry);

OperationFunction ParseRegistry(RegistryNode** registry, int argc, char* argv[]);

struct Command {
  const char*       name;
  OperationType     op;
  OperationFunction func;
};

void HandleEncode(RegistryNode** registry);
void HandleDecode(RegistryNode** registry);
void HandleCheck(RegistryNode** registry);

struct Command commands[] = {{"encode", OP_ENCODE, &HandleEncode},
                             {"decode", OP_DECODE, &HandleDecode},
                             {"check", OP_CHECK, &HandleCheck},
                             {NULL, OP_UNKNOWN, NULL}};

int main(int argc, char* argv[])
{
  PrintVersion();

  if (argc < 2) {
    fprintf(stderr, "error: no operation specified!\n");
    exit(EXIT_FAILURE);
  }

  RegistryNode* registry = NULL;

  RegistryUpdate(&registry, "input", "");    /* NULL = stdin */
  RegistryUpdate(&registry, "output", "");   /* NULL = stdout */
  RegistryUpdate(&registry, "key", "");      /* NULL = stdin */
  RegistryUpdate(&registry, "format", "64"); /* Base 16, 64, 256 */

  OperationFunction op = ParseRegistry(&registry, argc, argv);

  if (op != NULL) {
    op(&registry);
  }
  else {
    fprintf(stderr, "error: unknown operation!\n");
    exit(EXIT_FAILURE);
  }

  return 0;
}

OperationFunction ParseRegistry(RegistryNode** registry, int argc, char* argv[])
{
  for (int i = 2; i < argc; i++) {
    char* dupl    = strndup(argv[i], ZQ_REGISTRY_KEY_MAX);
    char* delimit = strchr(dupl, '=');
    char* key     = dupl;
    char* value   = "";

    if (delimit != NULL) {
      *delimit = '\0';
      value    = delimit + 1;
    }

    // Add or update the key-value pair.
    RegistryUpdate(registry, key, value);

    free(dupl);
  }

  const char* input = argv[1];

  OperationFunction closestMatchOperation = NULL;
  uint32            closestMatchDistance  = -1;

  for (int i = 0; commands[i].name != NULL; i++) {
    uint32 distance = LevenshteinDistance(commands[i].name, input);

    fprintf(stderr, "distance (%s, %s): %d\n", commands[i].name, input, distance);

    if (distance == 0) {
      return commands[i].func;
    }

    // Prioritize command if it starts with the input substring
    if (strncmp(commands[i].name, input, strlen(input)) == 0) {
      return commands[i].func;
    }

    if (distance < closestMatchDistance) {
      closestMatchDistance  = distance;
      closestMatchOperation = commands[i].func;
    }
  }

  return closestMatchOperation;
}

void HandleEncode(RegistryNode** registry)
{
  fprintf(stderr, "> MODE = ENCODE / ENCRYPT\n");
}

void HandleDecode(RegistryNode** registry)
{
  fprintf(stderr, "> MODE = DECODE / DECRYPT\n");
}

void HandleCheck(RegistryNode** registry)
{
  fprintf(stderr, "> MODE = CHECK\n");
}
