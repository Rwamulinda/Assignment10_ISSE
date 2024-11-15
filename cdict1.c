#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include "cdict.h"

#define DEBUG
#define DEFAULT_DICT_CAPACITY 8
#define REHASH_THRESHOLD 0.6

typedef enum {
  SLOT_UNUSED = 0,
  SLOT_IN_USE,
  SLOT_DELETED
} CDictSlotStatus;

struct _hash_slot {
  CDictSlotStatus status;
  CDictKeyType    key;
  CDictValueType  value;
};

struct _dictionary {
  unsigned int num_stored;
  unsigned int num_deleted;
  unsigned int capacity;
  struct _hash_slot *slot;
};

// Hash function (provided)
static unsigned int _CD_hash(CDictKeyType str, unsigned int capacity)
{
  unsigned int x;
  unsigned int len = 0;

  for (const char *p = str; *p; p++) 
    len++;

  if (len == 0)
    return 0;

  const char *p = str;
  x = (unsigned int)*p << 7;

  for (int i=0; i < len; i++)
    x = (1000003 * x) ^ (unsigned int) *p++;

  x ^= (unsigned int) len;

  return x % capacity;
}

// Rehashing the dictionary when the load factor exceeds REHASH_THRESHOLD
static void _CD_rehash(CDict dict)
{
  unsigned int new_capacity = dict->capacity * 2;
  struct _hash_slot *new_slots = malloc(sizeof(struct _hash_slot) * new_capacity);
  assert(new_slots);

  // Initialize new slots
  for (unsigned int i = 0; i < new_capacity; i++) {
    new_slots[i].status = SLOT_UNUSED;
  }

  // Rehash all items from old slots
  for (unsigned int i = 0; i < dict->capacity; i++) {
    if (dict->slot[i].status == SLOT_IN_USE) {
      unsigned int new_index = _CD_hash(dict->slot[i].key, new_capacity);
      while (new_slots[new_index].status == SLOT_IN_USE) {
        new_index = (new_index + 1) % new_capacity;
      }
      new_slots[new_index] = dict->slot[i];
    }
  }

  free(dict->slot);
  dict->slot = new_slots;
  dict->capacity = new_capacity;
  dict->num_deleted = 0; // Reset deleted count after rehash
}

// Create a new empty dictionary
CDict CD_new()
{
  CDict dict = malloc(sizeof(struct _dictionary));
  assert(dict);

  dict->num_stored = 0;
  dict->num_deleted = 0;
  dict->capacity = DEFAULT_DICT_CAPACITY;
  dict->slot = malloc(sizeof(struct _hash_slot) * dict->capacity);
  assert(dict->slot);

  // Initialize all slots as unused
  for (unsigned int i = 0; i < dict->capacity; i++) {
    dict->slot[i].status = SLOT_UNUSED;
  }

  return dict;
}

// Free the memory used by the dictionary
void CD_free(CDict dict)
{
  assert(dict);
  free(dict->slot);
  free(dict);
}

// Return the size of the dictionary (number of stored elements)
unsigned int CD_size(CDict dict)
{
#ifdef DEBUG
  int used = 0;
  int deleted = 0;
  for (int i = 0; i < dict->capacity; i++) {
    if (dict->slot[i].status == SLOT_IN_USE)
      used++;
    else if (dict->slot[i].status == SLOT_DELETED)
      deleted++;
  }
  assert(used == dict->num_stored);
  assert(deleted == dict->num_deleted);
#endif

  return dict->num_stored;
}

// Return the capacity of the dictionary
unsigned int CD_capacity(CDict dict)
{
  return dict->capacity;
}

// Check if a key is in the dictionary
bool CD_contains(CDict dict, CDictKeyType key)
{
  unsigned int index = _CD_hash(key, dict->capacity);
  unsigned int original_index = index;

  while (dict->slot[index].status != SLOT_UNUSED) {
    if (dict->slot[index].status == SLOT_IN_USE && strcmp(dict->slot[index].key, key) == 0)
      return true;
    index = (index + 1) % dict->capacity;
    if (index == original_index) // We have looped through all slots
      break;
  }

  return false;
}

// Store a key-value pair in the dictionary
void CD_store(CDict dict, CDictKeyType key, CDictValueType value)
{
  assert(dict);
  assert(key);
  assert(value);

  if (CD_load_factor(dict) >= REHASH_THRESHOLD) {
    _CD_rehash(dict);
  }

  unsigned int index = _CD_hash(key, dict->capacity);
  unsigned int original_index = index;

  while (dict->slot[index].status == SLOT_IN_USE) {
    if (strcmp(dict->slot[index].key, key) == 0) {
      dict->slot[index].value = value; // Overwrite existing value
      return;
    }
    index = (index + 1) % dict->capacity;
    if (index == original_index) break; // We've looped around
  }

  dict->slot[index].key = key;
  dict->slot[index].value = value;
  dict->slot[index].status = SLOT_IN_USE;
  dict->num_stored++;
}

// Retrieve the value associated with a key
CDictValueType CD_retrieve(CDict dict, CDictKeyType key)
{
  assert(dict);
  assert(key);

  unsigned int index = _CD_hash(key, dict->capacity);
  unsigned int original_index = index;

  while (dict->slot[index].status != SLOT_UNUSED) {
    if (dict->slot[index].status == SLOT_IN_USE && strcmp(dict->slot[index].key, key) == 0) {
      return dict->slot[index].value;
    }
    index = (index + 1) % dict->capacity;
    if (index == original_index) break; // We've looped through all slots
  }

  return NULL; // Key not found
}

// Delete a key from the dictionary
void CD_delete(CDict dict, CDictKeyType key)
{
  assert(dict);
  assert(key);

  unsigned int index = _CD_hash(key, dict->capacity);
  unsigned int original_index = index;

  while (dict->slot[index].status != SLOT_UNUSED) {
    if (dict->slot[index].status == SLOT_IN_USE && strcmp(dict->slot[index].key, key) == 0) {
      dict->slot[index].status = SLOT_DELETED;
      dict->num_stored--;
      dict->num_deleted++;
      return;
    }
    index = (index + 1) % dict->capacity;
    if (index == original_index) break; // We've looped through all slots
  }
}

// Return the current load factor of the dictionary
double CD_load_factor(CDict dict)
{
  return (double)(dict->num_stored) / dict->capacity;
}

// Print the contents of the dictionary (for debugging)
void CD_print(CDict dict)
{
  assert(dict);

  printf("*** capacity: %d stored: %d deleted: %d load_factor: %.2f\n",
         dict->capacity, dict->num_stored, dict->num_deleted, CD_load_factor(dict));

  for (unsigned int i = 0; i < dict->capacity; i++) {
    printf("%02d: ", i);
    if (dict->slot[i].status == SLOT_UNUSED) {
      printf("unused\n");
    } else if (dict->slot[i].status == SLOT_DELETED) {
      printf("DELETED\n");
    } else {
      printf("IN_USE key=%s hash=%u value=%s\n", dict->slot[i].key,
             _CD_hash(dict->slot[i].key, dict->capacity), dict->slot[i].value);
    }
  }
}

// Apply a callback function to each element in the dictionary
void CD_foreach(CDict dict, CD_foreach_callback callback, void *cb_data)
{
  assert(dict);

  for (unsigned int i = 0; i < dict->capacity; i++) {
    if (dict->slot[i].status == SLOT_IN_USE) {
      callback(dict->slot[i].key, dict->slot[i].value, cb_data);
    }
  }
}
