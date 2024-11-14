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
  assert(dict);

    // Step 1: Create a new dictionary with double the old capacity
    unsigned int new_capacity = dict->capacity * 2;
    CDict new_dict = malloc(sizeof(struct _dictionary));
    assert(new_dict);  // Make sure the allocation succeeded
    new_dict->capacity = new_capacity;
    new_dict->num_stored = 0;
    new_dict->num_deleted = 0;
    new_dict->slot = malloc(sizeof(struct _hash_slot) * new_capacity);
    assert(new_dict->slot);  // Ensure the slots array was allocated

    // Initialize the slots of the new dictionary to SLOT_UNUSED
    for (unsigned int i = 0; i < new_capacity; i++) {
        new_dict->slot[i].status = SLOT_UNUSED;
    }

    // Step 2: Rehash and move all the key-value pairs from the old dictionary to the new dictionary
    for (unsigned int i = 0; i < dict->capacity; i++) {
        if (dict->slot[i].status == SLOT_IN_USE) {
            CDictKeyType key = dict->slot[i].key;
            CDictValueType value = dict->slot[i].value;
            unsigned int new_index = _CD_hash(key, new_capacity);

            // Insert the key-value pair into the new dictionary
            while (new_dict->slot[new_index].status == SLOT_IN_USE) {
                new_index = (new_index + 1) % new_capacity;  // Linear probing if necessary
            }

            // Place the key-value pair into the new dictionary
            new_dict->slot[new_index].key = key;
            new_dict->slot[new_index].value = value;
            new_dict->slot[new_index].status = SLOT_IN_USE;
            new_dict->num_stored++;
        }
    }

    // Step 3: Update the original dictionary to the new dictionary's state
    free(dict->slot);  // Free the old slots array
    dict->num_deleted = 0;  // Copy the contents of the new dictionary into the old dictionary
    dict->num_stored = new_dict->num_stored;  
    dict->slot = new_dict->slot;
    new_dict->slot = NULL;  

    
    // Step 4: Free the memory for the new dictionary
    CD_free(new_dict);
}


// Documented in .h file
CDict CD_new()
{
  CDict dict = malloc(sizeof(struct _dictionary));
  if (dict == NULL) return NULL;

  dict->num_stored = 0;
  dict->num_deleted = 0;
  dict->capacity = DEFAULT_DICT_CAPACITY;

  dict->slot = malloc(DEFAULT_DICT_CAPACITY * sizeof(struct _hash_slot));
  if (dict->slot == NULL) {
      free(dict);
      return NULL;  // Handle allocation failure
  }

    // Initialize each slot in the dictionary
  for (unsigned int i = 0; i < DEFAULT_DICT_CAPACITY; i++) {
      dict->slot[i].status = SLOT_UNUSED;
      dict->slot[i].key ="";
      dict->slot[i].value ="";
  }
 
  return dict;
}



// Documented in .h file
void CD_free(CDict dict)
{

  //
  // TODO: Add your code here

    //if (dict != NULL) {
        free(dict->slot);
        free(dict);
    }
  //
//}


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
  assert(dict && key);
  //
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
// Documented in .h file
void CD_store(CDict dict, CDictKeyType key, CDictValueType value)
{
    assert(dict);
    assert(key);
    assert(value);

    unsigned int index = _CD_hash(key, dict->capacity);

    while (dict->slot[index].status == SLOT_IN_USE) {
        if (strcmp(dict->slot[index].key, key) == 0) {
            // If the key exists, update the value
            dict->slot[index].value = value;
            return;
        }
        // Wrap around to index 0 when we reach the end of the array
        index = (index + 1) % dict->capacity;
    }

    // If we find an unused or deleted slot, store the key-value pair
    dict->slot[index].key = key;
    dict->slot[index].value = value;
    dict->slot[index].status = SLOT_IN_USE;
    dict->num_stored++;

    // Rehash if the load factor exceeds the threshold
    if (CD_load_factor(dict) > REHASH_THRESHOLD) {
        _CD_rehash(dict);
    }
}


// Documented in .h file
CDictValueType CD_retrieve(CDict dict, CDictKeyType key)
{
    assert(dict);
    assert(key);

    unsigned int index = _CD_hash(key, dict->capacity);

    while (dict->slot[index].status != SLOT_UNUSED) {
        if (dict->slot[index].status == SLOT_IN_USE && strcmp(dict->slot[index].key, key) == 0) {
            return dict->slot[index].value;
        }
        // Wrap around to index 0 when we reach the end of the array
        index = (index + 1) % dict->capacity;
    }

    // If the key was not found, return a null or default value
    return INVALID_VALUE;
}


// Documented in .h file
void CD_delete(CDict dict, CDictKeyType key)
{
    assert(dict);
    assert(key);

    unsigned int index = _CD_hash(key, dict->capacity);

    while (dict->slot[index].status != SLOT_UNUSED) {
        if (dict->slot[index].status == SLOT_IN_USE && strcmp(dict->slot[index].key, key) == 0) {
            // Mark the slot as deleted
            dict->slot[index].status = SLOT_DELETED;
            dict->num_stored--;
            dict->num_deleted++;
            return;
        }
        // Wrap around to index 0 when we reach the end of the array
        index = (index + 1) % dict->capacity;
    }
}


// Documented in .h file
double CD_load_factor(CDict dict)
{
    assert(dict);

    return (double)(dict->num_stored + dict->num_deleted) / dict->capacity;
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
            printf("IN USE (Key: %s, Value: %s)\n", dict->slot[i].key, dict->slot[i].value);
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

