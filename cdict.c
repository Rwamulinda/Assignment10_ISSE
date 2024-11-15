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
static void _CD_rehash(CDict dict) {
    assert(dict);
    assert(dict->capacity >0);
    unsigned int new_capacity = dict->capacity * 2;
    struct _hash_slot *new_slots = malloc(new_capacity * sizeof(struct _hash_slot));
    if (!new_slots) return;  // Allocation failed, skip rehashing

    // Initialize the new slots
    for (int i = 0; i < new_capacity; i++) {
        new_slots[i].status = SLOT_UNUSED;
    }

    // Reinsert elements from old slots into new slots
    for (int i = 0; i < dict->capacity; i++) {
        if (dict->slot[i].status == SLOT_IN_USE) {
            unsigned int index = _CD_hash(dict->slot[i].key, new_capacity);
            for (int j = 0; j < new_capacity; j++) {
                unsigned int probe = (index + j) % new_capacity;
                if (new_slots[probe].status == SLOT_UNUSED) {
                    new_slots[probe].key = dict->slot[i].key;
                    new_slots[probe].value = dict->slot[i].value;
                    new_slots[probe].status = SLOT_IN_USE;
                    break;
                }
            }
        }
    }

    // Free the old slots and update dictionary
    free(dict->slot);
    dict->slot = new_slots;
    dict->capacity = new_capacity;
    dict->num_deleted = 0; // Reset the deleted count after rehashing
}



// Documented in .h file
CDict CD_new() {
    CDict dict = malloc(sizeof(struct _dictionary));
    if (!dict) return NULL;

    dict->num_stored = 0;
    dict->num_deleted = 0;
    dict->capacity = DEFAULT_DICT_CAPACITY;
    dict->slot = malloc(DEFAULT_DICT_CAPACITY * sizeof(struct _hash_slot));
    if (!dict->slot) {
        free(dict);
        return NULL;
    }

    for (int i = 0; i < DEFAULT_DICT_CAPACITY; i++) {
        dict->slot[i].status = SLOT_UNUSED;
    }

    return dict;
}



// Documented in .h file
void CD_free(CDict dict) {
  assert(dict);
  if (dict) {
    free(dict->slot);
    free(dict);
  }
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
unsigned int CD_capacity(CDict dict) {
    return dict->capacity;
}



// Documented in .h file
bool CD_contains(CDict dict, CDictKeyType key) {
    unsigned int index = _CD_hash(key, dict->capacity);
    for (int i = 0; i < dict->capacity; i++) {
        unsigned int probe = (index + i) % dict->capacity;
        if (dict->slot[probe].status == SLOT_UNUSED) {
            return false; 
        }
        if (dict->slot[probe].status == SLOT_IN_USE && strcmp(dict->slot[probe].key, key) == 0) {
            return true; 
        }
    }
    return false;
}


// Documented in .h file
void CD_store(CDict dict, CDictKeyType key, CDictValueType value)
{
  assert(dict);
  assert(key);
  assert(value);


  if (CD_load_factor(dict) >= REHASH_THRESHOLD) {
        _CD_rehash(dict); // Rehash if load factor exceeds threshold
    }

  unsigned int index = _CD_hash(key, dict->capacity);
  for (int i = 0; i < dict->capacity; i++) {
        unsigned int probe = (index + i) % dict->capacity;
        if (dict->slot[probe].status == SLOT_UNUSED || dict->slot[probe].status == SLOT_DELETED) {
            dict->slot[probe].status = SLOT_IN_USE;
            dict->slot[probe].key = key;
            dict->slot[probe].value = value;
            dict->num_stored++;
            return;
        } else if (dict->slot[probe].status == SLOT_IN_USE && strcmp(dict->slot[probe].key, key) == 0) {
            dict->slot[probe].value = value;
            return;
        }
  }


  //
  // TODO: Add your code here
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
  for (int i = 0; i < dict->capacity; i++) {
        unsigned int probe = (index + i) % dict->capacity;
        if (dict->slot[probe].status == SLOT_UNUSED) {
            return INVALID_VALUE; // Key not found
        }
        if (dict->slot[probe].status == SLOT_IN_USE && strcmp(dict->slot[probe].key, key) == 0) {
            return dict->slot[probe].value;
        }
  }
  return INVALID_VALUE;
  //
}


// Documented in .h file
void CD_delete(CDict dict, CDictKeyType key)
{
  assert(dict);
  assert(key);


  unsigned int index = _CD_hash(key, dict->capacity);
  for (int i = 0; i < dict->capacity; i++) {
        unsigned int probe = (index + i) % dict->capacity;
        if (dict->slot[probe].status == SLOT_UNUSED) {
            return; // Key not found
        }
        if (dict->slot[probe].status == SLOT_IN_USE && strcmp(dict->slot[probe].key, key) == 0) {
            dict->slot[probe].status = SLOT_DELETED;
            dict->num_stored--;
            dict->num_deleted++;
            return;
        }
  }


  //
  // TODO: Add your code here
  //
}


// Documented in .h file
double CD_load_factor(CDict dict)
{
  assert(dict);

  return (double)(dict->num_stored + dict->num_deleted) / dict->capacity;


  //
  // TODO: Add your code here
  //
}
// Documented in .h file
void CD_print(CDict dict)
{
  assert(dict);
  for (int i = 0; i < dict->capacity; i++) {
        printf("Slot %d: ", i);
        if (dict->slot[i].status == SLOT_UNUSED) {
            printf("UNUSED\n");
        } else if (dict->slot[i].status == SLOT_DELETED) {
            printf("DELETED\n");
        } else {
            printf("IN USE - Key: %s, Value: %s\n", dict->slot[i].key, dict->slot[i].value);
        }
  }

  //
  // TODO: Add your code here
  //
}


void CD_foreach(CDict dict, CD_foreach_callback callback, void *cb_data)
{
  assert(dict);
  for (int i = 0; i < dict->capacity; i++) {
        if (dict->slot[i].status == SLOT_IN_USE) {
            callback(dict->slot[i].key, dict->slot[i].value, cb_data);
        }
  }

  //
  // TODO: Add your code here
  //
}
