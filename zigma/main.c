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

typedef enum OperationType { OP_UNKNOWN = 0, OP_ENCODE, OP_DECODE, OP_CHECK, OP_HELP, OP_VERSION } OperationType;
typedef void (*OperationFunction)(RegistryNode** registry);

OperationFunction DetermineOperation(const char* input);

void ParseRegistry(RegistryNode** registry, int argc, char* argv[]);

struct Command {
  const char*       name;
  OperationType     op;
  OperationFunction func;
};

void HandleEncode(RegistryNode** registry);
void HandleDecode(RegistryNode** registry);
void HandleCheck(RegistryNode** registry);
void HandleHelp(RegistryNode** registry);
void HandleVersion(RegistryNode** registry);

void PrintVersion();

struct Command commands[] = {{"encode", OP_ENCODE, &HandleEncode},    {"decode", OP_DECODE, &HandleDecode},
                             {"check", OP_CHECK, &HandleCheck},       {"help", OP_HELP, &HandleHelp},
                             {"version", OP_VERSION, &HandleVersion}, {NULL, OP_UNKNOWN, NULL}};

int main(int argc, char* argv[])
{
  PrintVersion();

  if (argc < 2) {
    fprintf(stderr, "error: no operation specified!\n");
    exit(EXIT_FAILURE);
  }

  RegistryNode* registry = NULL;

  OperationFunction op = DetermineOperation(argv[1]);

  /* Load the defaults. */
  if (op == HandleEncode) {
    RegistryUpdate(&registry, "in", "");         /* NULL = stdin */
    RegistryUpdate(&registry, "in.fmt", "256");  /* 256 = binary */
    RegistryUpdate(&registry, "out", "");        /* NULL = stdout */
    RegistryUpdate(&registry, "out.fmt", "64");  /* 64 = base64 */
    RegistryUpdate(&registry, "key", "");        /* NULL = stdin */
    RegistryUpdate(&registry, "key.fmt", "256"); /* 256 = binary */
  }
  else if (op == HandleDecode) {
    RegistryUpdate(&registry, "in", "");         /* NULL = stdin */
    RegistryUpdate(&registry, "in.fmt", "64");   /* 64 = binary */
    RegistryUpdate(&registry, "out", "");        /* NULL = stdout */
    RegistryUpdate(&registry, "out.fmt", "256"); /* 256 = binary */
    RegistryUpdate(&registry, "key", "");        /* NULL = stdin */
    RegistryUpdate(&registry, "key.fmt", "256"); /* 256 = binary */
  }
  else if (op == HandleCheck) {
    RegistryUpdate(&registry, "in", "");        /* NULL = stdin */
    RegistryUpdate(&registry, "in.fmt", "256"); /* 256 = binary */
    RegistryUpdate(&registry, "out", "");       /* NULL = stdout */
    RegistryUpdate(&registry, "out.fmt", "16"); /* 16 = base16 */
  }

  ParseRegistry(&registry, argc, argv);

  if (op != NULL) {
    op(&registry);
  }
  else {
    fprintf(stderr, "error: unknown operation!\n");
    exit(EXIT_FAILURE);
  }

  return 0;
}

OperationFunction DetermineOperation(const char* input)
{
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

void ParseRegistry(RegistryNode** registry, int argc, char* argv[])
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
}

void HandleEncode(RegistryNode** registry)
{
  RegistryNode* input        = RegistrySearch(registry, "in");
  RegistryNode* inputFormat  = RegistrySearch(registry, "in.fmt");
  RegistryNode* output       = RegistrySearch(registry, "out");
  RegistryNode* outputFormat = RegistrySearch(registry, "out.fmt");
  RegistryNode* key          = RegistrySearch(registry, "key");
  RegistryNode* keyFormat    = RegistrySearch(registry, "key.fmt");

  uint32 inputBaseFormat  = strtoul(inputFormat->value, NULL, 10);
  uint32 outputBaseFormat = strtoul(outputFormat->value, NULL, 10);
  uint32 keyBaseFormat    = strtoul(keyFormat->value, NULL, 10);

#define IS_VALID_FORMAT(x) ((x) == 16 || (x) == 64 || (x) == 256)
  if (!IS_VALID_FORMAT(inputBaseFormat) || !IS_VALID_FORMAT(outputBaseFormat) || !IS_VALID_FORMAT(keyBaseFormat)) {
    fprintf(stderr, "ERROR: Invalid format '%s'!\n", keyFormat->value);
    exit(EXIT_FAILURE);
  }
#undef IS_VALID_FORMAT

  FILE* inputFile  = *input->value != 0 ? OpenFile(input->value, "r") : stdin;
  FILE* outputFile = *output->value != 0 ? OpenFile(output->value, "w") : stdout;

  Buffer* passwordBuffer = BufferCreate(NULL, ZQ_MAX_KEY_SIZE);

  if (*key->value != 0) {
    FILE* keyFile = OpenFile(key->value, "r");

    BufferReadBase256(passwordBuffer, keyFile);

    if (passwordBuffer->length > ZQ_MAX_KEY_SIZE) {
      fprintf(stderr, "ERROR: Key file '%s' is too large!\n", key->value);
      exit(EXIT_FAILURE);
    }
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
  }

  fprintf(stderr, "   mode            = ENCODING\n");
  fprintf(stderr, "  input (fmt: %3d) = %s\n", inputBaseFormat, *input->value != 0 ? input->value : "<STDIN>");
  fprintf(stderr, " output (fmt: %3d) = %s\n", outputBaseFormat, *output->value != 0 ? output->value : "<STDOUT>");
  fprintf(stderr, "    key (fmt: %3d) = %s -> %d/%d (%f%%) bytes\n\n", keyBaseFormat,
          *key->value != 0 ? key->value : "<PASSPHRASE>", passwordBuffer->length, ZQ_MAX_KEY_SIZE,
          (float) passwordBuffer->length / (float) ZQ_MAX_KEY_SIZE * 100.0f);

  ZigmaContext* cipher = ZigmaCreate(NULL, passwordBuffer->data, passwordBuffer->length);

  Buffer* outputBuffer = BufferCreate(NULL, 0);

  uint64 total;

  if (inputBaseFormat == 16)
    total = BufferReadBase16(outputBuffer, inputFile);
  else if (inputBaseFormat == 64)
    total = BufferReadBase64(outputBuffer, inputFile);
  else
    total = BufferReadBase256(outputBuffer, inputFile);

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
  RegistryNode* input        = RegistrySearch(registry, "in");
  RegistryNode* inputFormat  = RegistrySearch(registry, "in.fmt");
  RegistryNode* output       = RegistrySearch(registry, "out");
  RegistryNode* outputFormat = RegistrySearch(registry, "out.fmt");
  RegistryNode* key          = RegistrySearch(registry, "key");
  RegistryNode* keyFormat    = RegistrySearch(registry, "key.fmt");

  uint32 inputBaseFormat  = strtoul(inputFormat->value, NULL, 10);
  uint32 outputBaseFormat = strtoul(outputFormat->value, NULL, 10);
  uint32 keyBaseFormat    = strtoul(keyFormat->value, NULL, 10);

#define IS_VALID_FORMAT(x) ((x) == 16 || (x) == 64 || (x) == 256)
  if (!IS_VALID_FORMAT(inputBaseFormat) || !IS_VALID_FORMAT(outputBaseFormat) || !IS_VALID_FORMAT(keyBaseFormat)) {
    fprintf(stderr, "ERROR: Invalid format '%s'!\n", keyFormat->value);
    exit(EXIT_FAILURE);
  }
#undef IS_VALID_FORMAT

  FILE* inputFile  = *input->value != 0 ? OpenFile(input->value, "r") : stdin;
  FILE* outputFile = *output->value != 0 ? OpenFile(output->value, "w") : stdout;

  Buffer* passwordBuffer = BufferCreate(NULL, ZQ_MAX_KEY_SIZE);

  if (*key->value != 0) {
    FILE* keyFile = OpenFile(key->value, "r");

    BufferReadBase256(passwordBuffer, keyFile);

    if (passwordBuffer->length > ZQ_MAX_KEY_SIZE) {
      fprintf(stderr, "ERROR: Key file '%s' is too large!\n", key->value);
      exit(EXIT_FAILURE);
    }
  }
  else {
    passwordBuffer->length = CaptureKey(passwordBuffer->data, "Enter password: ");
  }

  fprintf(stderr, "   mode            = DECODING\n");
  fprintf(stderr, "  input (fmt: %3d) = %s\n", inputBaseFormat, *input->value != 0 ? input->value : "<STDIN>");
  fprintf(stderr, " output (fmt: %3d) = %s\n", outputBaseFormat, *output->value != 0 ? output->value : "<STDOUT>");
  fprintf(stderr, "    key (fmt: %3d) = %s -> %d/%d (%f%%) bytes\n\n", keyBaseFormat,
          *key->value != 0 ? key->value : "<PASSPHRASE>", passwordBuffer->length, ZQ_MAX_KEY_SIZE,
          (float) passwordBuffer->length / (float) ZQ_MAX_KEY_SIZE * 100.0f);

  ZigmaContext* cipher = ZigmaCreate(NULL, passwordBuffer->data, passwordBuffer->length);

  Buffer* outputBuffer = BufferCreate(NULL, 0);

  uint64 total;

  if (inputBaseFormat == 16)
    total = BufferReadBase16(outputBuffer, inputFile);
  else if (inputBaseFormat == 64)
    total = BufferReadBase64(outputBuffer, inputFile);
  else
    total = BufferReadBase256(outputBuffer, inputFile);

  ZigmaDecodeBuffer(cipher, outputBuffer);

  if (outputBaseFormat == 256) {
    fwrite(outputBuffer->data, 1, total, outputFile);
  }
  else if (outputBaseFormat == 64) {
    BufferPrintBase64(outputBuffer, outputFile);
  }
  else if (outputBaseFormat == 16) {
    BufferPrintBase16(outputBuffer, outputFile);
  }

  fprintf(stderr, "!COMPLETE! DECODED %d BYTES!\n", total);
}

void HandleCheck(RegistryNode** registry)
{
  RegistryNode* input        = RegistrySearch(registry, "in");
  RegistryNode* inputFormat  = RegistrySearch(registry, "in.fmt");
  RegistryNode* output       = RegistrySearch(registry, "out");
  RegistryNode* outputFormat = RegistrySearch(registry, "out.fmt");

  uint32 inputBaseFormat  = strtoul(inputFormat->value, NULL, 10);
  uint32 outputBaseFormat = strtoul(outputFormat->value, NULL, 10);

#define IS_VALID_FORMAT(x) ((x) == 16 || (x) == 64 || (x) == 256)
  if (!IS_VALID_FORMAT(inputBaseFormat) || !IS_VALID_FORMAT(outputBaseFormat)) {
    fprintf(stderr, "ERROR: Invalid format!\n");
    exit(EXIT_FAILURE);
  }
#undef IS_VALID_FORMAT

  FILE* inputFile = *input->value != 0 ? OpenFile(input->value, "r") : stdin;

  ZigmaContext* cipher       = ZigmaCreate(NULL, NULL, 0);
  Buffer*       outputBuffer = BufferCreate(NULL, 0);
  uint64        total        = BufferReadBase256(outputBuffer, inputFile);

  ZigmaEncodeBuffer(cipher, outputBuffer);

  Buffer* checksumBuffer = BufferCreate(NULL, ZIGMA_CHECKSUM_SIZE);

  ZigmaHashFinal(cipher, checksumBuffer->data, ZIGMA_CHECKSUM_SIZE);

  if (outputBaseFormat == 16) {
    for (int i = 0; i < ZIGMA_CHECKSUM_SIZE; i++) {
      fprintf(stdout, "%02x", checksumBuffer->data[i]);
    }
  }
  else if (outputBaseFormat == 64) {
    char*  encoded = malloc(4 * ((checksumBuffer->length + 2) / 3));
    uint64 length  = base64_encode(encoded, (char*) checksumBuffer->data, ZIGMA_CHECKSUM_SIZE);

    for (int i = 0; i < length; i++) {
      fprintf(stdout, "%c", encoded[i]);
    }

    free(encoded);
  }

  fprintf(stdout, "  %s (%d)\n", *input->value != 0 ? input->value : "-", total);
}

void HandleHelp(RegistryNode** registry)
{
  fprintf(stderr, "Usage: zigma OPERATION [OPERAND...]\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "OPERATION must be one one of the following:\n");
  fprintf(stderr, "  encode, decode, check, help, version\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "OPERAND must be in the form of <KEY[.SUBKEY]>[=VALUE]\n");
  fprintf(stderr, "  KEY must be one of the following:\n");
  fprintf(stderr, "    in=FILE    read from FILE instead, or omit for:  <STDIN>\n");
  fprintf(stderr, "    out=FILE   write to FILE instead, or omit for:   <STDOUT>\n");
  fprintf(stderr, "    key=FILE   use FILE as master key, or omit for:  <CAPTURE>\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  SUBKEY must be one of the following:\n");
  fprintf(stderr, "    .fmt=BASE   the base encoding of the data (16, 64, 256)\n");
  fprintf(stderr, "\n");
}

void HandleVersion(RegistryNode** registry)
{
  fprintf(stderr, "  Copyright (C) 2024 Chase Zehl O'Byrne <zehl (at) live.com>\n\n");
  fprintf(stderr, "  NOTICE: This program comes with ABSOLUTELY NO WARRANTY.\n");
}

void PrintVersion()
{
  fprintf(stderr, "ZIGMA %s/%s@%s (%s)\n", ZIGMATIQ_VERSION_STRING, ZIGMATIQ_GIT_BRANCH, ZIGMATIQ_GIT_COMMIT,
          ZIGMATIQ_GIT_TAG);
}