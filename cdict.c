/*
 * cdict.c
 * 
 * Dictionary based on a hash table utilizing open addressing to
 * resolve collisions.
 *
 * Author: <Pauline Uwase>
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

    // Save old slots and capacity
    unsigned int old_capacity = dict->capacity;
    struct _hash_slot *old_slots = dict->slot;

    // Double the capacity and reset counters
    dict->capacity *= 2;
    dict->num_stored = 0;
    dict->num_deleted = 0;
    dict->slot = calloc(dict->capacity, sizeof(struct _hash_slot));

    // Rehash each slot
    for (unsigned int i = 0; i < old_capacity; i++) {
        if (old_slots[i].status == SLOT_IN_USE) {
            CD_store(dict, old_slots[i].key, old_slots[i].value);
        }
    }

    // Free old slots memory
    free(old_slots);
  //
}


// Documented in .h file
CDict CD_new()
{

  //
  // TODO: Add your code here
    CDict dict = malloc(sizeof(struct _dictionary));
    dict->capacity = DEFAULT_DICT_CAPACITY;
    dict->num_stored = 0;
    dict->num_deleted = 0;
    dict->slot = calloc(dict->capacity, sizeof(struct _hash_slot));
    return dict;

  //
}


// Documented in .h file
void CD_free(CDict dict)
{

  //
  // TODO: Add your code here

    if (dict) {
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
    return dict->capacity;
  //
}


// Documented in .h file
bool CD_contains(CDict dict, CDictKeyType key)
{

  //
  // TODO: Add your code here
    unsigned int hash = _CD_hash(key, dict->capacity);
    for (unsigned int i = 0; i < dict->capacity; i++) {
        unsigned int idx = (hash + i) % dict->capacity;
        if (dict->slot[idx].status == SLOT_UNUSED) {
            return false;
        }
        if (dict->slot[idx].status == SLOT_IN_USE && strcmp(dict->slot[idx].key, key) == 0) {
            return true;
        }
    }
    return false;
  //
}


// Documented in .h file
// Documented in .h file
void CD_store(CDict dict, CDictKeyType key, CDictValueType value)
{
    assert(dict);
    assert(key);
    assert(value);

    // Check if rehash is needed
    if (CD_load_factor(dict) >= REHASH_THRESHOLD) {
        _CD_rehash(dict);
    }

    unsigned int hash = _CD_hash(key, dict->capacity);

    // Attempt to find an available slot or update an existing key
    for (unsigned int i = 0; i < dict->capacity; i++) {
        unsigned int idx = (hash + i) % dict->capacity;

        // If slot is in use with the same key, overwrite the value
        if (dict->slot[idx].status == SLOT_IN_USE && strcmp(dict->slot[idx].key, key) == 0) {
            dict->slot[idx].value = value;
            return;
        }

        // Use an empty or deleted slot for new entry
        if (dict->slot[idx].status != SLOT_IN_USE) {
            dict->slot[idx].status = SLOT_IN_USE;
            dict->slot[idx].key = key;
            dict->slot[idx].value = value;
            dict->num_stored++;
            return;
        }
    }
}


// Documented in .h file
CDictValueType CD_retrieve(CDict dict, CDictKeyType key)
{
    assert(dict);
    assert(key);

    unsigned int hash = _CD_hash(key, dict->capacity);

    // Search for the key in the table
    for (unsigned int i = 0; i < dict->capacity; i++) {
        unsigned int idx = (hash + i) % dict->capacity;

        if (dict->slot[idx].status == SLOT_UNUSED) {
            return INVALID_VALUE; // Key not found
        }

        if (dict->slot[idx].status == SLOT_IN_USE && strcmp(dict->slot[idx].key, key) == 0) {
            return dict->slot[idx].value; // Key found
        }
    }
    return INVALID_VALUE; // Key not found
}


// Documented in .h file
void CD_delete(CDict dict, CDictKeyType key)
{
    assert(dict);
    assert(key);

    unsigned int hash = _CD_hash(key, dict->capacity);

    // Locate the key to delete
    for (unsigned int i = 0; i < dict->capacity; i++) {
        unsigned int idx = (hash + i) % dict->capacity;

        if (dict->slot[idx].status == SLOT_UNUSED) {
            return; // Key not found
        }

        if (dict->slot[idx].status == SLOT_IN_USE && strcmp(dict->slot[idx].key, key) == 0) {
            dict->slot[idx].status = SLOT_DELETED;
            dict->num_stored--;
            dict->num_deleted++;
            return; // Key deleted
        }
    }
}


// Documented in .h file
double CD_load_factor(CDict dict)
{
    assert(dict);

    return (double)(dict->num_stored) / dict->capacity;
}


// Documented in .h file
void CD_print(CDict dict)
{
    assert(dict);

    for (unsigned int i = 0; i < dict->capacity; i++) {
        printf("Slot %u: ", i);
        
        if (dict->slot[i].status == SLOT_UNUSED) {
            printf("UNUSED\n");
        } else if (dict->slot[i].status == SLOT_DELETED) {
            printf("DELETED\n");
        } else if (dict->slot[i].status == SLOT_IN_USE) {
            printf("Key: %s, Value: %s\n", dict->slot[i].key, dict->slot[i].value);
        }
    }
}


// Documented in .h file
void CD_foreach(CDict dict, CD_foreach_callback callback, void *cb_data)
{
    assert(dict);

    for (unsigned int i = 0; i < dict->capacity; i++) {
        if (dict->slot[i].status == SLOT_IN_USE) {
            // Execute the callback function on each element
            callback(dict->slot[i].key, dict->slot[i].value, cb_data);
        }
    }
}

