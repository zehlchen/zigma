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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

#include "base64.h"
#include "buffer.h"
#include "registry.h"
#include "zigma.h"

typedef enum OperationType { OP_UNKNOWN = 0, OP_ENCODE, OP_DECODE, OP_CHECK } OperationType;
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
  RegistryNode* input  = RegistrySearch(registry, "input");
  RegistryNode* output = RegistrySearch(registry, "output");
  RegistryNode* key    = RegistrySearch(registry, "key");
  RegistryNode* format = RegistrySearch(registry, "format");

  DEBUG_ASSERT(input != NULL);
  DEBUG_ASSERT(output != NULL);
  DEBUG_ASSERT(key != NULL);
  DEBUG_ASSERT(format != NULL);

  FILE* inputFile  = stdin;
  FILE* outputFile = stdout;

  fprintf(stderr, "> MODE = ENCODE / ENCRYPT\n");

  /* Setup the input. */
  if (*input->value != 0) {
    inputFile = fopen(input->value, "r");

    if (inputFile == NULL) {
      fprintf(stderr, "ERROR: fopen(): unable to open input file '%s': %s\n", input->value, strerror(errno));
      exit(EXIT_FAILURE);
    }

    fprintf(stderr, "# SUCCESS! OPEN INPUT FILE: '%s' ...!\n", input->value);
  }
  else {
    fprintf(stderr, "# SUCCESS! INPUT STREAM: <STDIN> ...!\n");
  }

  /* Setup the output. */
  if (*output->value != 0) {
    outputFile = fopen(output->value, "w");

    if (outputFile == NULL) {
      fprintf(stderr, "ERROR: fopen(): unable to open output file '%s': %s!\n", output->value, strerror(errno));
      exit(EXIT_FAILURE);
    }

    fprintf(stderr, "# SUCCESS! OPEN OUTPUT FILE: '%s' ...!\n", output->value);
  }
  else {
    fprintf(stderr, "# SUCCESS! OUTPUT STREAM: <STDOUT> ...!\n");
  }

  Buffer* passwordBuffer = BufferCreate(NULL, ZQ_MAX_KEY_SIZE);

  if (*key->value != 0) {
    FILE* keyFile = fopen(key->value, "r");

    if (keyFile == NULL) {
      fprintf(stderr, "ERROR: fopen(): unable to open key file '%s': %s!\n", key->value, strerror(errno));
      exit(EXIT_FAILURE);
    }

    passwordBuffer->length = fread(passwordBuffer->data, 1, ZQ_MAX_KEY_SIZE, keyFile);

    if (passwordBuffer->length == 0) {
      fprintf(stderr, "ERROR: fread(): unable to read key file '%s': %s!\n", key->value, strerror(errno));
      exit(EXIT_FAILURE);
    }

    fprintf(stderr, "# SUCCESS! READ KEY FILE! (%d bytes)\n", passwordBuffer->length);
  }
  else {
    Buffer* passwordRetryBuffer = BufferCreate(NULL, ZQ_MAX_KEY_SIZE);

    passwordBuffer->length      = CaptureKey(passwordBuffer->data, "Enter password: ");
    passwordRetryBuffer->length = CaptureKey(passwordRetryBuffer->data, "Re-enter password: ");

    if (passwordBuffer->length != passwordRetryBuffer->length ||
        memcmp(passwordBuffer->data, passwordRetryBuffer->data, passwordBuffer->length) != 0) {
      fprintf(stderr, "ERROR: Passwords do not match!\n");
      exit(EXIT_FAILURE);
    }

    fprintf(stderr, "# SUCCESS! READ PASSWORD! (%d bytes)\n", passwordBuffer->length);
  }

  ZigmaContext* cipher = ZigmaCreate(NULL, passwordBuffer->data, passwordBuffer->length);

  ZigmaPrint(cipher);

  uint32 outputBaseFormat = strtoul(format->value, NULL, 10);

  if (outputBaseFormat != 16 && outputBaseFormat != 64 && outputBaseFormat != 256) {
    fprintf(stderr, "ERROR: Invalid output format '%s'!\n", format->value);
    exit(EXIT_FAILURE);
  }

  Buffer* inputBuffer  = BufferCreate(NULL, ZQ_MAX_BUFFER_SIZE);
  Buffer* outputBuffer = BufferCreate(NULL, 0);

  uint64 count = 0;
  uint64 total = 0;

  while ((count = fread(inputBuffer->data, 1, ZQ_MAX_BUFFER_SIZE, inputFile)) > 0) {
    BufferResize(outputBuffer, count + total);

    memcpy(outputBuffer->data + total, inputBuffer->data, count);

    total += count;
  }

  ZigmaEncodeBuffer(cipher, outputBuffer);

  if (outputBaseFormat == 256) {
    fwrite(outputBuffer->data, 1, total, outputFile);
  }

  fprintf(stderr, "# SUCCESS! WROTE %d BYTES!\n", total);
}

void HandleDecode(RegistryNode** registry)
{
  RegistryNode* input  = RegistrySearch(registry, "input");
  RegistryNode* output = RegistrySearch(registry, "output");
  RegistryNode* key    = RegistrySearch(registry, "key");
  RegistryNode* format = RegistrySearch(registry, "format");

  DEBUG_ASSERT(input != NULL);
  DEBUG_ASSERT(output != NULL);
  DEBUG_ASSERT(key != NULL);
  DEBUG_ASSERT(format != NULL);

  FILE* inputFile  = stdin;
  FILE* outputFile = stdout;

  fprintf(stderr, "> MODE = DECODE / DECRYPT\n");

  /* Setup the input. */
  if (*input->value != 0) {
    inputFile = fopen(input->value, "r");

    if (inputFile == NULL) {
      fprintf(stderr, "ERROR: fopen(): unable to open input file '%s': %s\n", input->value, strerror(errno));
      exit(EXIT_FAILURE);
    }

    fprintf(stderr, "# SUCCESS! OPEN INPUT FILE: '%s' ...!\n", input->value);
  }
  else {
    fprintf(stderr, "# SUCCESS! INPUT STREAM: <STDIN> ...!\n");
  }

  /* Setup the output. */
  if (*output->value != 0) {
    outputFile = fopen(output->value, "w");

    if (outputFile == NULL) {
      fprintf(stderr, "ERROR: fopen(): unable to open output file '%s': %s!\n", output->value, strerror(errno));
      exit(EXIT_FAILURE);
    }

    fprintf(stderr, "# SUCCESS! OPEN OUTPUT FILE: '%s' ...!\n", output->value);
  }
  else {
    fprintf(stderr, "# SUCCESS! OUTPUT STREAM: <STDOUT> ...!\n");
  }

  Buffer* passwordBuffer = BufferCreate(NULL, ZQ_MAX_KEY_SIZE);

  if (*key->value != 0) {
    FILE* keyFile = fopen(key->value, "r");

    if (keyFile == NULL) {
      fprintf(stderr, "ERROR: fopen(): unable to open key file '%s': %s!\n", key->value, strerror(errno));
      exit(EXIT_FAILURE);
    }

    passwordBuffer->length = fread(passwordBuffer->data, 1, ZQ_MAX_KEY_SIZE, keyFile);

    if (passwordBuffer->length == 0) {
      fprintf(stderr, "ERROR: fread(): unable to read key file '%s': %s!\n", key->value, strerror(errno));
      exit(EXIT_FAILURE);
    }

    fprintf(stderr, "# SUCCESS! READ KEY FILE! (%d bytes)\n", passwordBuffer->length);
  }
  else {
    passwordBuffer->length = CaptureKey(passwordBuffer->data, "Enter password: ");

    fprintf(stderr, "# SUCCESS! READ PASSWORD! (%d bytes)\n", passwordBuffer->length);
  }

  ZigmaContext* cipher = ZigmaCreate(NULL, passwordBuffer->data, passwordBuffer->length);

  ZigmaPrint(cipher);

  uint32 outputBaseFormat = strtoul(format->value, NULL, 10);

  if (outputBaseFormat != 16 && outputBaseFormat != 64 && outputBaseFormat != 256) {
    fprintf(stderr, "ERROR: Invalid output format '%s'!\n", format->value);
    exit(EXIT_FAILURE);
  }

  Buffer* inputBuffer  = BufferCreate(NULL, ZQ_MAX_BUFFER_SIZE);
  Buffer* outputBuffer = BufferCreate(NULL, 0);

  uint64 count = 0;
  uint64 total = 0;

  while ((count = fread(inputBuffer->data, 1, ZQ_MAX_BUFFER_SIZE, inputFile)) > 0) {
    BufferResize(outputBuffer, count + total);

    memcpy(outputBuffer->data + total, inputBuffer->data, count);

    total += count;
  }

  ZigmaDecodeBuffer(cipher, outputBuffer);

  if (outputBaseFormat == 256) {
    fwrite(outputBuffer->data, 1, total, outputFile);
  }

  fprintf(stderr, "# SUCCESS! WROTE %d BYTES!\n", total);
}

void HandleCheck(RegistryNode** registry)
{
  fprintf(stderr, "> MODE = CHECK\n");
}
