/*
 * cdict.c
 * 
 * Dictionary based on a hash table utilizing open addressing to
 * resolve collisions.
 *
 * Author: <your name here>
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "cdict.h"


#define DEBUG

#define DEFAULT_DICT_CAPACITY 8
#define REHASH_THRESHOLD  0.6

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


/*
 * Return a pseudorandom hash of a key with reasonable distribution
 * properties. Based on Python's implementation before Python 3.4
 *
 * Parameters:
 *   str   The string to be hashed
 *   capacity  The capacity of the dictionary
 * 
 * Returns: The hash, in the range 0-(capacity-1) inclusive
 */
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



/*
 * Rehash the dictionary, doubling its capacity
 *
 * Parameters:
 *   dict     The dictionary to rehash
 * 
 * Returns: None
 */
static void _CD_rehash(CDict dict)
{
  //
  // TODO: Add your code here

  // Calculate new capacity (double the current capacity)
    unsigned int new_capacity = dict->capacity * 2;

    // Allocate new hash table with the new capacity
    struct _hash_slot *new_slots = malloc(sizeof(struct _hash_slot) * new_capacity);
    assert(new_slots != NULL);

    // Initialize all slots to unused
    for (unsigned int i = 0; i < new_capacity; i++) {
        new_slots[i].status = SLOT_UNUSED;
    }

    // Rehash all existing keys and move them to the new table
    for (unsigned int i = 0; i < dict->capacity; i++) {
        if (dict->slot[i].status == SLOT_IN_USE) {
            // Rehash each key to the new table
            unsigned int new_index = _CD_hash(dict->slot[i].key, new_capacity);
            while (new_slots[new_index].status == SLOT_IN_USE) {
                // Handle collision (linear probing)
                new_index = (new_index + 1) % new_capacity;
            }

            // Move the existing entry to the new table
            new_slots[new_index] = dict->slot[i];
        }
    }

    // Free the old table and update the dictionary
    free(dict->slot);
    dict->slot = new_slots;
    dict->capacity = new_capacity;
    dict->num_deleted = 0; // Reset the deleted count after rehashing
}


// Documented in .h file
CDict CD_new()
{

  //
  // TODO: Add your code here
  CDict dict = malloc(sizeof(struct _dictionary));
    assert(dict != NULL);
    dict->capacity = DEFAULT_DICT_CAPACITY;
    dict->num_stored = 0;
    dict->num_deleted = 0;
    dict->slot = malloc(dict->capacity * sizeof(struct _hash_slot));
    assert(dict->slot != NULL);

    for (int i = 0; i < dict->capacity; i++) {
        dict->slot[i].status = SLOT_UNUSED;
    }
    return dict;

  //
}


// Documented in .h file
void CD_free(CDict dict)
{
  assert(dict);
  //
  // TODO: Add your code here
  if (dict != NULL) {
  free(dict->slot);
  free(dict);

  }
  //
}


// documented in .h file
unsigned int CD_size(CDict dict)
{
#ifdef DEBUG
  // iterate across slots, counting number of keys found
  int used = 0;
  int deleted = 0;
  for (int i=0; i < dict->capacity; i++)
    if (dict->slot[i].status == SLOT_IN_USE)
      used++;
    else if (dict->slot[i].status == SLOT_DELETED)
      deleted++;

  assert(used == dict->num_stored);
  assert(deleted == dict->num_deleted);
#endif

  return dict->num_stored;
}


// documented in .h file
unsigned int CD_capacity(CDict dict)
{

  //
  // TODO: Add your code here
   assert(dict);
  //
  // TODO: Add your code here
   return dict->capacity;
  //
  //
}


// Documented in .h file
bool CD_contains(CDict dict, CDictKeyType key)
{

  //
  // TODO: Add your code here
  // TODO: Add your code here
    
   unsigned int index = _CD_hash(key, dict->capacity);

   while (dict->slot[index].status != SLOT_UNUSED) {
        if (dict->slot[index].status == SLOT_IN_USE && strcmp(dict->slot[index].key, key) == 0) {
            return true;
        }
        // Wrap around to index 0 when we reach the end of the array
        index = (index + 1) % dict->capacity;
    }
    return false; 
  //
}


// Documented in .h file
void CD_store(CDict dict, CDictKeyType key, CDictValueType value)
{
  assert(dict);
  assert(key);
  assert(value);


  //
  // TODO: Add your code here

  if (CD_load_factor(dict) > REHASH_THRESHOLD) {
        _CD_rehash(dict);
  }

    // Insert the new key-value pair
    unsigned int index = _CD_hash(key, dict->capacity);

    // Handle collision using linear probing
  while (dict->slot[index].status == SLOT_IN_USE) {
        if (strcmp(dict->slot[index].key, key) == 0) {
            // Overwrite the value if the key already exists
            dict->slot[index].value = value;
            return;
        }
        index = (index + 1) % dict->capacity;
    }

    // Insert the new key-value pair
    dict->slot[index].status = SLOT_IN_USE;
    dict->slot[index].key = key;
    dict->slot[index].value = value;
    dict->num_stored++;
  //
}


// Documented in .h file
CDictValueType CD_retrieve(CDict dict, CDictKeyType key)
{
  assert(dict);
  assert(key);


  //
  // TODO: Add your code here
  unsigned int index = _CD_hash(key, dict->capacity);
  while (dict->slot[index].status != SLOT_UNUSED) {
        if (dict->slot[index].status == SLOT_IN_USE && strcmp(dict->slot[index].key, key) == 0) {
            return dict->slot[index].value;
        }
        index = (index + 1) % dict->capacity;
  }
  return INVALID_VALUE;
  //
}


// Documented in .h file
void CD_delete(CDict dict, CDictKeyType key)
{
  assert(dict);
  assert(key);


  //
  // TODO: Add your code here
  unsigned int index = _CD_hash(key, dict->capacity);
  while (dict->slot[index].status != SLOT_UNUSED) {
    if (dict->slot[index].status == SLOT_IN_USE && strcmp(dict->slot[index].key, key) == 0) {
            dict->slot[index].status = SLOT_DELETED;
            dict->num_stored--;
            dict->num_deleted++;
            return;
    }
        index = (index + 1) % dict->capacity;
    }
  //
}


// Documented in .h file
double CD_load_factor(CDict dict)
{
  assert(dict);


  //
  // TODO: Add your code here
  return (double)(dict->num_stored + dict->num_deleted) / dict->capacity;
  //
}


// Documented in .h file
void CD_print(CDict dict)
{
  assert(dict);


  //
  // TODO: Add your code here
  for (int i = 0; i < dict->capacity; i++) {
        printf("%02d: ", i);
        switch (dict->slot[i].status) {
            case SLOT_UNUSED:
                printf("unused\n");
                break;
            case SLOT_DELETED:
                printf("DELETED\n");
                break;
            case SLOT_IN_USE:
                printf("IN_USE key=%s value=%s\n", dict->slot[i].key, dict->slot[i].value);
                break;
        }
    }
  //
}


void CD_foreach(CDict dict, CD_foreach_callback callback, void *cb_data)
{
  assert(dict);


  //
  // TODO: Add your code here

  for (int i = 0; i < dict->capacity; i++) {
        if (dict->slot[i].status == SLOT_IN_USE) {
            callback(dict->slot[i].key, dict->slot[i].value, cb_data);
        }
    }
  //
}
