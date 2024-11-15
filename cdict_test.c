/*
 * dict_test.c
 * 
 * Test code for ISSE dictionary
 *
 * Author: <Pauline Uwase>
 */

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "cdict.h"

// Checks that value is true; if not, prints a failure message and
// returns 0 from this function
#define test_assert(value) {                                            \
    if (!(value)) {                                                     \
      printf("FAIL %s[%d]: %s\n", __FUNCTION__, __LINE__, #value);      \
      goto test_error;                                                  \
    }                                                                   \
  }

typedef struct {
  const char *city;
  const char *team;
} team_data_t;

team_data_t team_data[] = { {"Atlanta", "Hawks"}, {"Boston", "Celtics"},
                            {"Brooklyn", "Nets"}, {"Charlotte", "Hornets"},
                            {"Chicago", "Bulls"}, {"Cleveland", "Cavaliers"},
                            {"Dallas", "Mavericks"}, {"Denver", "Nuggets"},
                            {"Detroit", "Pistons"}, {"Golden State", "Warriors"},
                            {"Houston", "Rockets"}, {"Indiana", "Pacers"},
                            {"Los Angeles", "Lakers"}, {"Memphis", "Grizzlies"},
                            {"Miami", "Heat"}, {"Milwaukee", "Bucks"},
                            {"Minnesota", "Timberwolves"}, {"New Orleans", "Pelicans"},
                            {"New York", "Knicks"}, {"Oklahoma City", "Thunder"},
                            {"Orlando", "Magic"}, {"Philadelphia", "76ers"},
                            {"Phoenix", "Suns"}, {"Portland", "Trail Blazers"},
                            {"Sacramento", "Kings"}, {"San Antonio", "Spurs"},
                            {"Toronto", "Raptors"}, {"Utah", "Jazz"},
                            {"Washington", "Wizards"} };

const int team_data_len = sizeof (team_data) / sizeof(team_data[0]);


/*
 * Demonstrates basic operation of the dictionary. Returns 1 if all
 * tests, pass, 0 otherwise.
 */
int demonstrate_dict()
{
  CDict dict = CD_new();

  test_assert( CD_size(dict) == 0 );
  test_assert( CD_load_factor(dict) == 0.0 );

  CD_store(dict, "Atlanta", "Hawks");
  CD_store(dict, "Boston", "Celtics");
  CD_store(dict, "Los Angeles", "Lakers");
  CD_store(dict, "Denver", "Nuggets");
    
  printf("The Denver team is called the %s\n", CD_retrieve(dict, "Denver"));
  printf("  current load factor: %.2f\n", CD_load_factor(dict) );

  CD_delete(dict, "Boston");
  printf("  current size: %d\n", CD_size(dict));

  printf("Here is the entire dictionary:\n");
  CD_print(dict);

  // overwrite an existing value
  CD_store(dict, "Denver", "Broncos");
  printf("After updating Denver to 'Broncos':\n");
  CD_print(dict);
  
  // add two more elements to force a rehash
  CD_store(dict, "New York", "Knicks");
  CD_store(dict, "Miami", "Head");
  
  printf("After adding New York and Miami:\n");
  CD_print(dict);

  CD_free(dict);
  
  return 1;

 test_error:
  CD_free(dict);
  return 0;
}

// Adds a test case for retrieving a non-existing key.
int test_retrieve_non_existent_key()
{
  CDict dict = CD_new();
  
  // Store some elements in the dictionary
  CD_store(dict, "Los Angeles", "Lakers");
  CD_store(dict, "Boston", "Celtics");

  // Try to retrieve a non-existing key
  const char *team = CD_retrieve(dict, "Chicago");
  test_assert(team == NULL);  // Assert that the result is NULL
  
  CD_free(dict);
  return 1;

test_error:
  CD_free(dict);
  return 0;
}

// Adds a test case for updating an existing key.
int test_update_existing_key()
{
  CDict dict = CD_new();

  // Store some elements in the dictionary
  CD_store(dict, "Chicago", "Bulls");

  // Retrieve and verify the original value
  const char *team = CD_retrieve(dict, "Chicago");
  test_assert(strcmp(team, "Bulls") == 0);

  // Update the value for the same key
  CD_store(dict, "Chicago", "Blackhawks");

  // Retrieve and verify the updated value
  team = CD_retrieve(dict, "Chicago");
  test_assert(strcmp(team, "Blackhawks") == 0);
  
  CD_free(dict);
  return 1;

test_error:
  CD_free(dict);
  return 0;
}

// Adds a test case for dictionary resizing
int test_dictionary_resizing()
{
  CDict dict = CD_new();

  // Initially, dictionary size should be 0
  test_assert(CD_size(dict) == 0);
  test_assert(CD_load_factor(dict) == 0.0);

  // Add elements to force a resize
  for (int i = 0; i < 50; i++) {
    char key[10];
    snprintf(key, sizeof(key), "team%d", i);
    CD_store(dict, key, "Some Team");
  }

  // After adding many elements, check the load factor and size
  test_assert(CD_size(dict) == 50);
  test_assert(CD_load_factor(dict) > 0.0);  // Load factor should be non-zero

  // Check if dictionary still functions properly after resizing
  test_assert(CD_retrieve(dict, "team10") != NULL);
  
  CD_free(dict);
  return 1;

test_error:
  CD_free(dict);
  return 0;
}

// Adds a test case for deleting a key
int test_delete_key()
{
  CDict dict = CD_new();

  // Store some elements in the dictionary
  CD_store(dict, "Dallas", "Mavericks");
  CD_store(dict, "Miami", "Heat");

  // Delete one element and check if it was removed
  CD_delete(dict, "Dallas");
  const char *team = CD_retrieve(dict, "Dallas");
  test_assert(team == NULL);  // Assert that the result is NULL
  
  // Check that the other key still exists
  team = CD_retrieve(dict, "Miami");
  test_assert(strcmp(team, "Heat") == 0);
  
  CD_free(dict);
  return 1;

test_error:
  CD_free(dict);
  return 0;
}

int main()
{
  int passed = 0;
  int num_tests = 0;

  num_tests++; passed += demonstrate_dict(); 
  num_tests++; passed += test_retrieve_non_existent_key();
  num_tests++; passed += test_update_existing_key();
  num_tests++; passed += test_dictionary_resizing();
  num_tests++; passed += test_delete_key();

  printf("Passed %d/%d test cases\n", passed, num_tests);
  fflush(stdout);
  return 0;
}