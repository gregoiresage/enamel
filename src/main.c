#include <pebble.h>

#include "generated/enamel.h"

static Window *window;
static TextLayer *text_layer;


static void updateDisplay() {
  if(getEnablebackground()) {
    window_set_background_color(window, getBackground());
  }
  else {
    window_set_background_color(window,GColorWhite);
  }
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
  // Let Pebble Autoconfig handle received settings
  enamel_in_received_handler(iter, context);

  updateDisplay();
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create(GRect(0, 62, bounds.size.w, 60));
  text_layer_set_text(text_layer, "Change the background in the configuration page");
  text_layer_set_background_color(text_layer, GColorClear);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));

  updateDisplay();
}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void init(void) {
  // Initialize Pebble Autoconfig to register App Message handlers and restores settings
  enamel_init(0, 0);

  // Register our custom receive handler which in turn will call Pebble Autoconfigs receive handler
  app_message_register_inbox_received(in_received_handler);

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);
}

static void deinit(void) {
  window_destroy(window);
  app_message_deregister_callbacks();

  enamel_deinit();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
