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

  fprintf(stderr, "> MODE = ENCODE\n");

  /* Setup the input. */
  if (*input->value != 0) {
    inputFile = OpenFile(input->value, "r");

    fprintf(stderr, "> INPUT STREAM = '%s' ...\n", input->value);
  }
  else {
    fprintf(stderr, "> INPUT STREAM = <STDIN> ...\n");
  }

  /* Setup the output. */
  if (*output->value != 0) {
    outputFile = OpenFile(output->value, "w");

    fprintf(stderr, "> OUTPUT STREAM = '%s' ...\n", output->value);
  }
  else {
    fprintf(stderr, "> OUTPUT STREAM = <STDOUT> ...\n");
  }

  Buffer* passwordBuffer = BufferCreate(NULL, ZQ_MAX_KEY_SIZE);

  if (*key->value != 0) {
    FILE* keyFile = OpenFile(key->value, "r");

#if 0
    passwordBuffer->length = fread(passwordBuffer->data, 1, ZQ_MAX_KEY_SIZE, keyFile);
#endif

    BufferRead(passwordBuffer, keyFile);

    if (passwordBuffer->length > ZQ_MAX_KEY_SIZE) {
      fprintf(stderr, "ERROR: Key file '%s' is too large!\n", key->value);
      exit(EXIT_FAILURE);
    }

    fprintf(stderr, "> KEY = '%s' ... %d/%d (%f%%) bytes\n", key->value, passwordBuffer->length, ZQ_MAX_KEY_SIZE,
            (float) passwordBuffer->length / (float) ZQ_MAX_KEY_SIZE * 100.0f);
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

    fprintf(stderr, "> KEY = PASSPHRASE! %d/%d (%f%%) bytes\n", passwordBuffer->length, ZQ_MAX_KEY_SIZE,
            (float) passwordBuffer->length / (float) ZQ_MAX_KEY_SIZE * 100.0f);
  }

  ZigmaContext* cipher = ZigmaCreate(NULL, passwordBuffer->data, passwordBuffer->length);

  uint32 outputBaseFormat = strtoul(format->value, NULL, 10);

  if (outputBaseFormat != 16 && outputBaseFormat != 64 && outputBaseFormat != 256) {
    fprintf(stderr, "ERROR: Invalid output format '%s'!\n", format->value);
    exit(EXIT_FAILURE);
  }

  Buffer* outputBuffer = BufferCreate(NULL, 0);

  uint64 total = BufferRead(outputBuffer, inputFile);

  ZigmaEncodeBuffer(cipher, outputBuffer);

  if (outputBaseFormat == 256) {
    fwrite(outputBuffer->data, 1, total, outputFile);
  }
  else if (outputBaseFormat == 64) {
    BufferPrintBase64(outputBuffer, outputFile);
  }
  else if (outputBaseFormat == 16) {
    BufferPrintBase16(outputBuffer, outputFile);
  }
  fprintf(stderr, "!COMPLETE! ENCODED %d BYTES!\n", total);
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

  fprintf(stderr, "> MODE = DECODE\n");

  /* Setup the input. */
  if (*input->value != 0) {
    inputFile = OpenFile(input->value, "r");

    fprintf(stderr, "> INPUT STREAM = '%s' ...\n", input->value);
  }
  else {
    fprintf(stderr, "> INPUT STREAM = <STDIN> ...\n");
  }

  /* Setup the output. */
  if (*output->value != 0) {
    outputFile = OpenFile(output->value, "w");

    fprintf(stderr, "> OUTPUT STREAM = '%s' ...\n", output->value);
  }
  else {
    fprintf(stderr, "> OUTPUT STREAM = <STDOUT> ...\n");
  }

  Buffer* passwordBuffer = BufferCreate(NULL, ZQ_MAX_KEY_SIZE);

  if (*key->value != 0) {
    FILE* keyFile = OpenFile(key->value, "r");

#if 0
    passwordBuffer->length = fread(passwordBuffer->data, 1, ZQ_MAX_KEY_SIZE, keyFile);
#endif

    BufferRead(passwordBuffer, keyFile);

    if (passwordBuffer->length > ZQ_MAX_KEY_SIZE) {
      fprintf(stderr, "ERROR: Key file '%s' is too large!\n", key->value);
      exit(EXIT_FAILURE);
    }

    fprintf(stderr, "> KEY = '%s' ... %d/%d (%f%%) bytes\n", key->value, passwordBuffer->length, ZQ_MAX_KEY_SIZE,
            (float) passwordBuffer->length / (float) ZQ_MAX_KEY_SIZE * 100.0f);
  }
  else {
    passwordBuffer->length = CaptureKey(passwordBuffer->data, "Enter password: ");

    fprintf(stderr, "> KEY = PASSPHRASE! %d/%d (%f%%) bytes\n", passwordBuffer->length, ZQ_MAX_KEY_SIZE,
            (float) passwordBuffer->length / (float) ZQ_MAX_KEY_SIZE * 100.0f);
  }

  ZigmaContext* cipher = ZigmaCreate(NULL, passwordBuffer->data, passwordBuffer->length);

  uint32 outputBaseFormat = strtoul(format->value, NULL, 10);

  if (outputBaseFormat != 16 && outputBaseFormat != 64 && outputBaseFormat != 256) {
    fprintf(stderr, "ERROR: Invalid output format '%s'!\n", format->value);
    exit(EXIT_FAILURE);
  }

  Buffer* outputBuffer = BufferCreate(NULL, 0);

  uint64 total = BufferRead(outputBuffer, inputFile);

  ZigmaDecodeBuffer(cipher, outputBuffer);

  if (outputBaseFormat == 256) {
    fwrite(outputBuffer->data, 1, total, outputFile);
  }

  fprintf(stderr, "!COMPLETE! DECODED %d BYTES!\n", total);
}

void HandleCheck(RegistryNode** registry)
{
  RegistryNode* input  = RegistrySearch(registry, "input");
  RegistryNode* format = RegistrySearch(registry, "format");

  DEBUG_ASSERT(input != NULL);
  DEBUG_ASSERT(format != NULL);

  FILE* inputFile = stdin;

  fprintf(stderr, "> MODE = CHECKSUM\n");

  /* Setup the input. */
  if (*input->value != 0) {
    inputFile = OpenFile(input->value, "r");

    fprintf(stderr, "> INPUT STREAM = '%s' ...\n", input->value);
  }
  else {
    fprintf(stderr, "> INPUT STREAM = <STDIN> ...\n");
  }

  ZigmaContext* cipher       = ZigmaCreate(NULL, NULL, 0);
  Buffer*       outputBuffer = BufferCreate(NULL, 0);
  uint64        total        = BufferRead(outputBuffer, inputFile);

  ZigmaEncodeBuffer(cipher, outputBuffer);

  Buffer* checksumBuffer = BufferCreate(NULL, ZIGMA_CHECKSUM_SIZE);

  ZigmaHashFinal(cipher, checksumBuffer->data, ZIGMA_CHECKSUM_SIZE);

  uint32 outputBaseFormat = strtoul(format->value, NULL, 10);

  fprintf(stderr, "> CHECKSUM = ");
  if (outputBaseFormat == 16) {
    for (int i = 0; i < ZIGMA_CHECKSUM_SIZE; i++) {
      fprintf(stderr, "%02x", checksumBuffer->data[i]);
    }
  }
  else if (outputBaseFormat == 64) {
    char*  encoded = malloc(4 * ((checksumBuffer->length + 2) / 3));
    uint64 length  = base64_encode(encoded, (char*) checksumBuffer->data, ZIGMA_CHECKSUM_SIZE);

    for (int i = 0; i < length; i++) {
      fprintf(stderr, "%c", encoded[i]);
    }

    fprintf(stderr, " (%d bytees)\n", total);
  }
}