#include <pebble.h>

#include "enamel.h"

static Window *window;
static TextLayer *text_layer;

static void updateDisplay() {
  if(get_enableBackground()) {
    window_set_background_color(window, get_background());
  }
  else {
    window_set_background_color(window,GColorWhite);
  }
  switch(get_font_size()){
    case FONT_SIZE_SMALL : text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14)); break;
    case FONT_SIZE_NORMAL : text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD)); break;
    case FONT_SIZE_LARGE : text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD)); break;
  }
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
  updateDisplay();
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  text_layer = text_layer_create(bounds);
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
  // Initialize Enamel to register App Message handlers and restores settings
  enamel_init(0, 0);

  // Register our custom receive handler
  enamel_register_custom_inbox_received(in_received_handler);

  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(window, true);
}

static void deinit(void) {
  window_destroy(window);
  
  // Deinit Enamel to unregister App Message handlers and save settings
  enamel_deinit();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
