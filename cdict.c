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

static unsigned int _CD_hash(CDictKeyType str, unsigned int capacity)
{
    unsigned int x;
    unsigned int len = 0;

    if (!str) return 0;  // Handle NULL input

    for (const char *p = str; *p; p++)
        len++;

    if (len == 0)
        return 0;

    const char *p = str;
    x = (unsigned int)*p << 7;

    for (int i = 0; i < len; i++)
        x = (1000003 * x) ^ (unsigned int)*p++;

    x ^= (unsigned int)len;

    return x % capacity;
}

static void _CD_rehash(CDict dict)
{
    assert(dict);
    assert(dict->capacity > 0);
   
    unsigned int new_capacity = dict->capacity * 2;
    struct _hash_slot *new_slots = malloc(new_capacity * sizeof(struct _hash_slot));
    if (!new_slots) return;  // Allocation failed, skip rehashing

    // Initialize the new slots
    for (unsigned int i = 0; i < new_capacity; i++) {
        new_slots[i].status = SLOT_UNUSED;
        new_slots[i].key = NULL;
        new_slots[i].value = NULL;
    }

    // Reinsert elements from old slots into new slots
    for (unsigned int i = 0; i < dict->capacity; i++) {
        if (dict->slot[i].status == SLOT_IN_USE) {
            unsigned int index = _CD_hash(dict->slot[i].key, new_capacity);
           
            // Linear probing with guaranteed termination
            for (unsigned int j = 0; j < new_capacity; j++) {
                unsigned int probe = (index + j) % new_capacity;
                if (new_slots[probe].status == SLOT_UNUSED) {
                    new_slots[probe].key = strdup(dict->slot[i].key);    // Make a copy of the key
                    new_slots[probe].value = strdup(dict->slot[i].value);// Make a copy of the value
                    new_slots[probe].status = SLOT_IN_USE;
                    break;
                }
            }
            // Free old key and value
            free((void*)dict->slot[i].key);
            free((void*)dict->slot[i].value);
        }
    }

    free(dict->slot);
    dict->slot = new_slots;
    dict->capacity = new_capacity;
    dict->num_deleted = 0;  // Reset deleted count after rehashing
}

CDict CD_new()
{
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

    for (unsigned int i = 0; i < DEFAULT_DICT_CAPACITY; i++) {
        dict->slot[i].status = SLOT_UNUSED;
        dict->slot[i].key = NULL;
        dict->slot[i].value = NULL;
    }

    return dict;
}

void CD_free(CDict dict)
{
    if (!dict) return;
   
    // Free all stored keys and values
    for (unsigned int i = 0; i < dict->capacity; i++) {
        if (dict->slot[i].status == SLOT_IN_USE) {
            free((void*)dict->slot[i].key);
            free((void*)dict->slot[i].value);
        }
    }
   
    free(dict->slot);
    free(dict);
}

unsigned int CD_size(CDict dict)
{
    assert(dict);
   
#ifdef DEBUG
    // iterate across slots, counting number of keys found
    unsigned int used = 0;
    unsigned int deleted = 0;
    for (unsigned int i = 0; i < dict->capacity; i++) {
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

unsigned int CD_capacity(CDict dict)
{
    assert(dict);
    return dict->capacity;
}

bool CD_contains(CDict dict, CDictKeyType key)
{
    assert(dict);
    assert(key);
   
    unsigned int index = _CD_hash(key, dict->capacity);
    for (unsigned int i = 0; i < dict->capacity; i++) {
        unsigned int probe = (index + i) % dict->capacity;
        if (dict->slot[probe].status == SLOT_UNUSED) {
            return false;
        }
        if (dict->slot[probe].status == SLOT_IN_USE &&
            strcmp(dict->slot[probe].key, key) == 0) {
            return true;
        }
    }
    return false;
}

void CD_store(CDict dict, CDictKeyType key, CDictValueType value)
{
    assert(dict);
    assert(key);
    assert(value);

    if (CD_load_factor(dict) >= REHASH_THRESHOLD) {
        _CD_rehash(dict);
    }

    unsigned int index = _CD_hash(key, dict->capacity);
    for (unsigned int i = 0; i < dict->capacity; i++) {
        unsigned int probe = (index + i) % dict->capacity;
       
        if (dict->slot[probe].status != SLOT_IN_USE ||
            (dict->slot[probe].status == SLOT_IN_USE &&
             strcmp(dict->slot[probe].key, key) == 0)) {
           
            // If updating existing key
            if (dict->slot[probe].status == SLOT_IN_USE) {
                free((void*)dict->slot[probe].value);
            } else {
                // New key insertion
                dict->slot[probe].status = SLOT_IN_USE;
                dict->slot[probe].key = strdup(key);
                dict->num_stored++;
                if (dict->slot[probe].status == SLOT_DELETED) {
                    dict->num_deleted--;
                }
            }
            dict->slot[probe].value = strdup(value);
            return;
        }
    }
}

CDictValueType CD_retrieve(CDict dict, CDictKeyType key)
{
    assert(dict);
    assert(key);

    unsigned int index = _CD_hash(key, dict->capacity);
    for (unsigned int i = 0; i < dict->capacity; i++) {
        unsigned int probe = (index + i) % dict->capacity;
        if (dict->slot[probe].status == SLOT_UNUSED) {
            return INVALID_VALUE;
        }
        if (dict->slot[probe].status == SLOT_IN_USE &&
            strcmp(dict->slot[probe].key, key) == 0) {
            return dict->slot[probe].value;
        }
    }
    return INVALID_VALUE;
}

void CD_delete(CDict dict, CDictKeyType key)
{
    assert(dict);
    assert(key);

    unsigned int index = _CD_hash(key, dict->capacity);
    for (unsigned int i = 0; i < dict->capacity; i++) {
        unsigned int probe = (index + i) % dict->capacity;
        if (dict->slot[probe].status == SLOT_UNUSED) {
            return;
        }
        if (dict->slot[probe].status == SLOT_IN_USE &&
            strcmp(dict->slot[probe].key, key) == 0) {
            free((void*)dict->slot[probe].key);
            free((void*)dict->slot[probe].value);
            dict->slot[probe].status = SLOT_DELETED;
            dict->slot[probe].key = NULL;
            dict->slot[probe].value = NULL;
            dict->num_stored--;
            dict->num_deleted++;
            return;
        }
    }
}

double CD_load_factor(CDict dict)
{
    assert(dict);
    return (double)(dict->num_stored + dict->num_deleted) / dict->capacity;
}

void CD_print(CDict dict)
{
    assert(dict);
   
    printf("Dictionary contents (capacity=%u, stored=%u, deleted=%u):\n",
           dict->capacity, dict->num_stored, dict->num_deleted);
           
    for (unsigned int i = 0; i < dict->capacity; i++) {
        printf("Slot %u: ", i);
        switch (dict->slot[i].status) {
            case SLOT_UNUSED:
                printf("UNUSED\n");
                break;
            case SLOT_DELETED:
                printf("DELETED\n");
                break;
            case SLOT_IN_USE:
                printf("IN USE - Key: %s, Value: %s\n",
                       dict->slot[i].key, dict->slot[i].value);
                break;
        }
    }
}

void CD_foreach(CDict dict, CD_foreach_callback callback, void *cb_data)
{
    assert(dict);
    assert(callback);
   
    for (unsigned int i = 0; i < dict->capacity; i++) {
        if (dict->slot[i].status == SLOT_IN_USE) {
            callback(dict->slot[i].key, dict->slot[i].value, cb_data);
        }
    }
}