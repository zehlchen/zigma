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

#pragma once
#ifndef _ZIGMATIQ_REGISTRY_H_
#define _ZIGMATIQ_REGISTRY_H_

#include "common.h"

#define ZQ_REGISTRY_KEY_MAX   256
#define ZQ_REGISTRY_VALUE_MAX 1024

typedef struct RegistryNode {
  char* key;
  char* value;

  struct RegistryNode* next;
  struct RegistryNode* prev;
} RegistryNode;

typedef void (*RegistryCallback)(RegistryNode* node);
typedef int (*RegistryCompare)(RegistryNode* node, const char* key);

/* Create a RegistryNode node
 * @param key   The key to store
 * @param value The value to store
 * @return      The new node
 */
RegistryNode* RegistryCreateNode(char* key, char* value);

/* Destroy a Registry node
 * @param node  The node to destroy
 * @return      The next node
 */
RegistryNode* RegistryDestroyNode(RegistryNode* node);

/* Search for a key in a Registry list
 * @param list  The list to search
 * @param key   The key to search for
 * @return      The node containing the key or NULL
 */
RegistryNode* RegistrySearch(RegistryNode** list, const char* key);

/* Add or update a key/value pair to a Registry list
 * @param list  The list to add to
 * @param key   The key to add
 * @param value The value to add
 * @return      The new node
 */
RegistryNode* RegistryUpdate(RegistryNode** head, const char* key, const char* value);

/* Apply `callback` to each node in `head`
 * @param head      The head of the list
 * @param callback  The callback to apply
 */
void RegistryForEach(RegistryNode* head, RegistryCallback callback);

#endif /* _ZIGMATIQ_REGISTRY_H_ */