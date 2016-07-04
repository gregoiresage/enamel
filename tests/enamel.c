#include <pebble.h>
#include "unit.h"
#include "enamel.h"
#include <pebble-events/pebble-events.h>
#include "constants.h"

// Colour code definitions to make the output all pretty.
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

// Keep track of how many tests have run, and how many have passed.
int tests_run = 0;
int tests_passed = 0;

static AppMessageInboxReceived s_received_callback;

EventHandle events_app_message_register_inbox_received(AppMessageInboxReceived received_callback, void *context){
  s_received_callback = received_callback;
}

static void before_each(void) {
  enamel_init();
}

static void after_each(void) {
  enamel_deinit();
}

static char* default_values(void) {
  printf("default_values\n");
  mu_assert(enamel_get_enable_background(), "enamel_get_enable_background wrong default value");
  mu_assert(GColorFromHEX(0xFF0000).argb == enamel_get_background().argb, "enamel_get_background wrong default value");
  mu_assert(1 == enamel_get_font_size(), "enamel_get_font_size wrong default value");
  mu_assert(enamel_get_favoritefood(FAVORITEFOOD_SUSHI), "enamel_get_favoritefood FAVORITEFOOD_SUSHI wrong default value");
  mu_assert(enamel_get_favoritefood(FAVORITEFOOD_PIZZA) == false, "enamel_get_favoritefood FAVORITEFOOD_PIZZA wrong default value");
  mu_assert(enamel_get_favoritefood(FAVORITEFOOD_BURGERS), "enamel_get_favoritefood FAVORITEFOOD_BURGERS wrong default value");
  mu_assert(strcmp("coca", enamel_get_favorite_drink()) == 0, "enamel_get_favorite_drink wrong default value");
  mu_assert(strcmp("grape", enamel_get_flavor()) == 0, "enamel_get_flavor wrong default value");
  mu_assert(1500 == enamel_get_slider(), "enamel_get_slider wrong default value");
  mu_assert(strcmp("gregoire@test.fr", enamel_get_email()) == 0, "enamel_get_email wrong default value");
  mu_assert(125 == enamel_get_slider_nostep(), "enamel_get_slider_nostep wrong default value");

  mu_assert(!enamel_get_enable_background_no_default(), "enamel_get_enable_background_no_default wrong default value");
  mu_assert(GColorFromHEX(0x0).argb == enamel_get_background_no_default().argb, "enamel_get_background_no_default wrong default value");
  mu_assert(0 == enamel_get_font_size_no_default(), "enamel_get_font_size_no_default wrong default value");
  mu_assert(strcmp("water", enamel_get_favorite_drink_no_default()) == 0, "enamel_get_favorite_drink_no_default wrong default value");
  mu_assert(0 == enamel_get_slider_no_default(), "enamel_get_slider_no_default wrong default value");
  mu_assert(strcmp("", enamel_get_email_no_default()) == 0, "enamel_get_email_no_default wrong default value");

  return 0;
}

static char* save_load_no_change(void) {
  printf("save_load_no_change\n");
  enamel_deinit();
  enamel_init();
  mu_assert(enamel_get_enable_background(), "enamel_get_enable_background wrong default value");
  mu_assert(GColorFromHEX(0xFF0000).argb == enamel_get_background().argb, "enamel_get_background wrong default value");
  mu_assert(1 == enamel_get_font_size(), "enamel_get_font_size wrong default value");
  mu_assert(enamel_get_favoritefood(FAVORITEFOOD_SUSHI), "enamel_get_favoritefood FAVORITEFOOD_SUSHI wrong default value");
  mu_assert(enamel_get_favoritefood(FAVORITEFOOD_PIZZA) == false, "enamel_get_favoritefood FAVORITEFOOD_PIZZA wrong default value");
  mu_assert(enamel_get_favoritefood(FAVORITEFOOD_BURGERS), "enamel_get_favoritefood FAVORITEFOOD_BURGERS wrong default value");
  mu_assert(strcmp("coca", enamel_get_favorite_drink()) == 0, "enamel_get_favorite_drink wrong default value");
  mu_assert(strcmp("grape", enamel_get_flavor()) == 0, "enamel_get_flavor wrong default value");
  mu_assert(1500 == enamel_get_slider(), "enamel_get_slider wrong default value");
  mu_assert(strcmp("gregoire@test.fr", enamel_get_email()) == 0, "enamel_get_email wrong default value");
  mu_assert(125 == enamel_get_slider_nostep(), "enamel_get_slider_nostep wrong default value");

  mu_assert(!enamel_get_enable_background_no_default(), "enamel_get_enable_background_no_default wrong default value");
  mu_assert(GColorFromHEX(0x0).argb == enamel_get_background_no_default().argb, "enamel_get_background_no_default wrong default value");
  mu_assert(0 == enamel_get_font_size_no_default(), "enamel_get_font_size_no_default wrong default value");
  mu_assert(strcmp("water", enamel_get_favorite_drink_no_default()) == 0, "enamel_get_favorite_drink_no_default wrong default value");
  mu_assert(0 == enamel_get_slider_no_default(), "enamel_get_slider_no_default wrong default value");
  mu_assert(strcmp("", enamel_get_email_no_default()) == 0, "enamel_get_email_no_default wrong default value");
  return 0;
}

static char* changes(void) {
  printf("changes\n");
  DictionaryIterator iterator;
  iterator.dictionary = 0;

  uint8_t dict_buffer[TUPLE_SIZE * 20];
  dict_write_begin(&iterator, dict_buffer, sizeof(dict_buffer));
  dict_write_int32(&iterator, MESSAGE_KEY_enable_background, 0);
  dict_write_int32(&iterator, MESSAGE_KEY_background, 0xFFAA00);
  dict_write_cstring(&iterator, MESSAGE_KEY_font_size, "2");
  dict_write_int32(&iterator, MESSAGE_KEY_favoritefood, false);
  dict_write_int32(&iterator, MESSAGE_KEY_favoritefood+1, true);
  dict_write_int32(&iterator, MESSAGE_KEY_favoritefood+2, false);
  dict_write_cstring(&iterator, MESSAGE_KEY_favorite_drink, "water");
  dict_write_cstring(&iterator, MESSAGE_KEY_flavor, "banana");
  dict_write_int32(&iterator, MESSAGE_KEY_slider, 2000);
  dict_write_cstring(&iterator, MESSAGE_KEY_email, "hello you");
  dict_write_int32(&iterator, MESSAGE_KEY_slider_nostep, 1250);

  dict_write_int32(&iterator, MESSAGE_KEY_enable_background_no_default, 1);
  dict_write_int32(&iterator, MESSAGE_KEY_background_no_default, 0xFFAAAA);
  dict_write_cstring(&iterator, MESSAGE_KEY_font_size_no_default, "2");
  dict_write_cstring(&iterator, MESSAGE_KEY_favorite_drink_no_default, "coca");
  dict_write_int32(&iterator, MESSAGE_KEY_slider_no_default, 5432);
  dict_write_cstring(&iterator, MESSAGE_KEY_email_no_default, "coucou");
  dict_write_end(&iterator);

  s_received_callback(&iterator, NULL);

  mu_assert(enamel_get_enable_background() == false, "enamel_get_enable_background wrong changed value");
  mu_assert(GColorFromHEX(0xFFAA00).argb == enamel_get_background().argb, "enamel_get_background wrong changed value");
  mu_assert(2 == enamel_get_font_size(), "enamel_get_font_size wrong changed value");
  mu_assert(enamel_get_favoritefood(FAVORITEFOOD_SUSHI) == false, "enamel_get_favoritefood FAVORITEFOOD_SUSHI wrong changed value");
  mu_assert(enamel_get_favoritefood(FAVORITEFOOD_PIZZA), "enamel_get_favoritefood FAVORITEFOOD_PIZZA wrong changed value");
  mu_assert(enamel_get_favoritefood(FAVORITEFOOD_BURGERS) == false, "enamel_get_favoritefood FAVORITEFOOD_BURGERS wrong changed value");
  mu_assert(strcmp("water", enamel_get_favorite_drink()) == 0, "enamel_get_favorite_drink wrong changed value");
  mu_assert(strcmp("banana", enamel_get_flavor()) == 0, "enamel_get_flavor wrong changed value");
  mu_assert(2000 == enamel_get_slider(), "enamel_get_slider wrong changed value");
  mu_assert(strcmp("hello you", enamel_get_email()) == 0, "enamel_get_email wrong changed value");
  mu_assert(1250 == enamel_get_slider_nostep(), "enamel_get_slider_nostep wrong default value");

  mu_assert(enamel_get_enable_background_no_default(), "enamel_get_enable_background_no_default wrong default value");
  mu_assert(GColorFromHEX(0xFFAAAA).argb == enamel_get_background_no_default().argb, "enamel_get_background_no_default wrong default value");
  mu_assert(2 == enamel_get_font_size_no_default(), "enamel_get_font_size_no_default wrong default value");
  mu_assert(strcmp("coca", enamel_get_favorite_drink_no_default()) == 0, "enamel_get_favorite_drink_no_default wrong default value");
  mu_assert(5432 == enamel_get_slider_no_default(), "enamel_get_slider_no_default wrong default value");
  mu_assert(strcmp("coucou", enamel_get_email_no_default()) == 0, "enamel_get_email_no_default wrong default value");

  return 0;
}

static char* load_changes(void) {
  printf("load_changes\n");

  mu_assert(enamel_get_enable_background() == false, "enamel_get_enable_background wrong loaded value");
  mu_assert(GColorFromHEX(0xFFAA00).argb == enamel_get_background().argb, "enamel_get_background wrong loaded value");
  mu_assert(2 == enamel_get_font_size(), "enamel_get_font_size wrong loaded value");
  mu_assert(enamel_get_favoritefood(FAVORITEFOOD_SUSHI) == false, "enamel_get_favoritefood FAVORITEFOOD_SUSHI wrong loaded value");
  mu_assert(enamel_get_favoritefood(FAVORITEFOOD_PIZZA), "enamel_get_favoritefood FAVORITEFOOD_PIZZA wrong loaded value");
  mu_assert(enamel_get_favoritefood(FAVORITEFOOD_BURGERS) == false, "enamel_get_favoritefood FAVORITEFOOD_BURGERS wrong loaded value");
  mu_assert(strcmp("water", enamel_get_favorite_drink()) == 0, "enamel_get_favorite_drink wrong loaded value");
  mu_assert(strcmp("banana", enamel_get_flavor()) == 0, "enamel_get_flavor wrong loaded value");
  mu_assert(2000 == enamel_get_slider(), "enamel_get_slider wrong loaded value");
  mu_assert(strcmp("hello you", enamel_get_email()) == 0, "enamel_get_email wrong loaded value");
  mu_assert(1250 == enamel_get_slider_nostep(), "enamel_get_slider_nostep wrong default value");

  mu_assert(enamel_get_enable_background_no_default(), "enamel_get_enable_background_no_default wrong default value");
  mu_assert(GColorFromHEX(0xFFAAAA).argb == enamel_get_background_no_default().argb, "enamel_get_background_no_default wrong default value");
  mu_assert(2 == enamel_get_font_size_no_default(), "enamel_get_font_size_no_default wrong default value");
  mu_assert(strcmp("coca", enamel_get_favorite_drink_no_default()) == 0, "enamel_get_favorite_drink_no_default wrong default value");
  mu_assert(5432 == enamel_get_slider_no_default(), "enamel_get_slider_no_default wrong default value");
  mu_assert(strcmp("coucou", enamel_get_email_no_default()) == 0, "enamel_get_email_no_default wrong default value");
  
  return 0;
}

static char* all_tests(void) {
  mu_run_test(default_values);
  mu_run_test(save_load_no_change);
  mu_run_test(changes);
  mu_run_test(load_changes);
  return 0;
}

// Test application entry point.
// Executes all the tests and prints the results in pretty colours.
int main(int argc, char **argv) {
  printf("%s------------------------\n", KCYN);
  printf("| Running Enamel Tests |\n");
  printf("------------------------\n%s", KNRM);
  char* result = all_tests();
  if (0 != result) {
    printf("%s- Failed Test:%s %s\n", KRED, KNRM, result);
  }
  printf("- Tests Run: %s%d%s\n", (tests_run == tests_passed) ? KGRN : KRED, tests_run, KNRM);
  printf("- Tests Passed: %s%d%s\n", (tests_run == tests_passed) ? KGRN : KRED, tests_passed, KNRM);

  printf("%s------------------------%s\n", KCYN, KNRM);
  return result != 0;
}