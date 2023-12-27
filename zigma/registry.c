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

#include "registry.h"

RegistryNode* RegistryCreateNode(char const* key, char const* value)
{
  RegistryNode* node = (RegistryNode*) malloc(sizeof(RegistryNode));

  DEBUG_ASSERT(node != NULL);

  node->key   = strndup(key, ZQ_REGISTRY_KEY_MAX);
  node->value = strndup(value, ZQ_REGISTRY_VALUE_MAX);

  node->prev = node->next = NULL;

  return node;
}

RegistryNode* RegistryDestroyNode(RegistryNode* node)
{
  DEBUG_ASSERT(node != NULL);
  DEBUG_ASSERT(node->key != NULL);
  DEBUG_ASSERT(node->value != NULL);

  free(node->key);
  free(node->value);

  free(node);

  return NULL;
}

RegistryNode* RegistrySearch(RegistryNode** list, const char* key)
{
  DEBUG_ASSERT(list != NULL);
  DEBUG_ASSERT(key != NULL);

  RegistryNode* node = *list;

  while (node != NULL) {
    if (strcmp(node->key, key) == 0)
      return node;

    node = node->next;
  }

  return NULL;
}

RegistryNode* RegistryUpdate(RegistryNode** head, const char* key, const char* value)
{
  RegistryNode* current = *head;

  /* Search for the node and update. */
  while (current != NULL) {
    if (strncmp(current->key, key, ZQ_REGISTRY_KEY_MAX) == 0) {
      free(current->value);
      current->value = strndup(value, ZQ_REGISTRY_VALUE_MAX);

      return current;
    }

    current = current->next;
  }

  /* Create and insert the node. */
  RegistryNode* node = RegistryCreateNode(key, value);

  DEBUG_ASSERT(node != NULL);

  /* Update the node structure.*/
  if (*head == NULL)
    *head = node;
  else {
    current = *head;

    while (current->next != NULL)
      current = current->next;

    current->next = node;
    node->prev    = current;
  }

  return current;
}

void RegistryForEach(RegistryNode** head, RegistryCallback callback)
{
  DEBUG_ASSERT(head != NULL);
  DEBUG_ASSERT(callback != NULL);

  RegistryNode* current = *head;

  while (current != NULL) {
    callback(current);

    current = current->next;
  }
}
