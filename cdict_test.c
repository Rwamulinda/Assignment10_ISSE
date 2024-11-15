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
      return 0;                                                         \
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

const int team_data_len = sizeof(team_data) / sizeof(team_data[0]);

/*
 * Demonstrates basic operation of the dictionary. Returns 1 if all
 * tests pass, 0 otherwise.
 */
int demonstrate_dict()
{
  CDict dict = CD_new();

  test_assert(CD_size(dict) == 0);
  test_assert(CD_load_factor(dict) == 0.0);

  CD_store(dict, "Atlanta", "Hawks");
  CD_store(dict, "Boston", "Celtics");
  CD_store(dict, "Los Angeles", "Lakers");
  CD_store(dict, "Denver", "Nuggets");

  printf("The Denver team is called the %s\n", CD_retrieve(dict, "Denver"));
  printf("  current load factor: %.2f\n", CD_load_factor(dict));

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
  CD_print(dict);
  CD_store(dict, "Miami", "Heat");

  printf("After adding New York and Miami:\n");
  CD_print(dict);

  CD_free(dict);

  return 1;
}

int test_print() {
    CDict dict = CD_new();
    CD_store(dict, "Atlanta", "Hawks");
    CD_store(dict, "Boston", "Celtics");
    CD_store(dict, "Los Angeles", "Lakers");

    printf("Testing print function:\n");
    CD_print(dict);
    CD_free(dict);
    return 1;
}

int test_load_factor() {
    CDict dict = CD_new();
    test_assert(CD_load_factor(dict) == 0.0);
    CD_store(dict, "Atlanta", "Hawks");
    test_assert(CD_load_factor(dict) > 0.0);
    CD_free(dict);
    return 1;
}

int test_update_value() {
    CDict dict = CD_new();
    CD_store(dict, "Atlanta", "Hawks");
    CD_store(dict, "Atlanta", "Falcons");
    test_assert(strcmp(CD_retrieve(dict, "Atlanta"), "Falcons") == 0);
    CD_free(dict);
    return 1;
}

int test_delete_key() {
    CDict dict = CD_new();
    CD_store(dict, "Atlanta", "Hawks");
    CD_store(dict, "Boston", "Celtics");
    test_assert(CD_size(dict) == 2);
    CD_delete(dict, "Atlanta");
    test_assert(CD_size(dict) == 1);
    const char* retrieved = CD_retrieve(dict, "Atlanta");
    test_assert(retrieved == NULL);  // Handling invalid retrieval
    test_assert(strcmp(CD_retrieve(dict, "Boston"), "Celtics") == 0);
    CD_free(dict);
    return 1;
}

int test_store_and_retrieve() {
    CDict dict = CD_new();
    CD_store(dict, "Atlanta", "Hawks");
    test_assert(strcmp(CD_retrieve(dict, "Atlanta"), "Hawks") == 0);
    test_assert(CD_retrieve(dict, "Boston") == NULL);  // Check for NULL when not found
    CD_free(dict);
    return 1;
}

int test_create_dict() {
    CDict dict = CD_new();
    test_assert(dict != NULL);
    test_assert(CD_size(dict) == 0);
    test_assert(CD_capacity(dict) == 8);
    test_assert(CD_load_factor(dict) == 0.0);
    CD_free(dict);
    return 1;
}

int test_contains() {
    CDict dict = CD_new();
    CD_store(dict, "Atlanta", "Hawks");
    test_assert(CD_contains(dict, "Atlanta"));
    test_assert(!CD_contains(dict, "Boston"));
    CD_free(dict);
    return 1;
}

int main()
{
  int passed = 0;
  int num_tests = 0;

  num_tests++; passed += demonstrate_dict();
  num_tests++; passed += test_create_dict();
  num_tests++; passed += test_store_and_retrieve();
  num_tests++; passed += test_delete_key();
  num_tests++; passed += test_update_value();
  num_tests++; passed += test_load_factor();
  num_tests++; passed += test_contains();
  num_tests++; passed += test_print();

  printf("Passed %d/%d test cases\n", passed, num_tests);
  fflush(stdout);
  return 0;
}
