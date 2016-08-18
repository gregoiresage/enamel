# Enamel [![Build Status](https://travis-ci.org/gregoiresage/enamel.svg?branch=master)](https://travis-ci.org/gregoiresage/enamel)&nbsp;[![MIT License](http://img.shields.io/badge/license-MIT-lightgray.svg)](./LICENSE)

Enamel is a python script that generates C helpers from a [Clay](https://github.com/pebble/clay) configuration file to easily get the value of your settings.

Enamel will :
* handle AppMessages automatically (app_message_open, handler registration, ...)
* save/load the value of the settings in the persistant storage automatically
* provide a getter for each of your settings
* notify subscribers when settings are received from Clay

You can focus on your watchapp/face, Enamel will do the rest !

---

# Getting Started (SDK 3.13+)
1. You project must contain a valid configuration file in `src/js/config.json` (see https://github.com/pebble/clay)
2. Install enamel with `pebble package install enamel`
3. Copy and paste the following line into the top of your `wscript` : 
  
  ``` python
  import sys
  sys.path.append('node_modules')
  from enamel.enamel import enamel
  ```
4. Change the `build` of your wscript from 

  ``` python
  ctx.pbl_program(source=ctx.path.ant_glob('src/**/*.c'), target=app_elf)
  ```
  to
  ``` python
  ctx(rule = enamel, source='src/js/config.json', target=['enamel.c', 'enamel.h'])
  ctx.pbl_program(source=ctx.path.ant_glob('src/**/*.c') + ['enamel.c'], target=app_elf)
  ```
5. Launch your Pebble build : 2 files (enamel.c and enamel.h) should be generated in `build` and compiled

>:warning:<br>
>The first time you launch a build, you will get an error message because Jinja2 module is missing.<br>
>Just follow the instructions to fix your environment.

# Getting Started (CloudPebble)
In the CloudPebble environment, you can't modify the wscript so you need to call directly the python script.

1. Under Dependencies in the project navigation, enter `enamel` as the Package Name and ^1.0.0 for the Version. You may use any specific version you like, however using ^1.0.0 will ensure you receive all minor version updates.

2. Copy the content of your Clay's `config.js` file into a local file (`local_config.js`)

3. Install the python dependencies for the code generation

  ```
  pip install -r requirements.txt
  ```

4. Call the script to generate the enamel files

  ```
  python enamel.py --config /path/to/local_config.js 
  ```

5. Copy the 2 generated files in your CloudPebble project

---

# Using Enamel
1. Setup your project correctly for Clay : https://github.com/pebble/clay
2. Include `enamel.h` in your c file : 
  
  ``` c
  #include "enamel.h"
  #include <pebble-events/pebble-events.h>
  ```
3. Initialize enamel in your `init` function and call `events_app_message_open()` after any other libraries you need to init.
  
  ``` c
  static void init(void) {
    // Initialize Enamel to register App Message handlers and restores settings
    enamel_init();
    
    // call pebble-events app_message_open function
    events_app_message_open(); 
    
    ...
  }
  ```
4. Deinitialize enamel in your `deinit` function :
  
  ``` c
  static void deinit(void) {
    ...
  
    // Deinit Enamel to unregister App Message handlers and save settings
    enamel_deinit();
  }
  ```
5. (Optional) Subscribe with a handler after `enamel_init` that will be automatically called when the settings are received. Multiple subscribers are supported. Do not forget to unsubscribe before calling `enamel_deinit`!

  ``` c
  static EventHandle s_window_event_handle;
  static EventHandle s_text_layer_event_handle;

  ...

  static void enamel_settings_received_window_handler(void *context){
    APP_LOG(0, "Settings received %d", enamel_get_myinteger());
    Window *window = (Window *) context;
    window_set_background_color(window, enamel_get_background());
  }

  static void enamel_settings_received_text_layer_handler(void *context){
    APP_LOG(0, "Settings received %d", enamel_get_myinteger());
    TextLayer *text_layer = (TextLayer *) context;
    text_layer_set_text_color(text_layer, enamel_get_foreground());
  }
  
  ...
  
  static void init(void) {
    // Initialize Enamel to register App Message handlers and restores settings
    enamel_init();

    // Subscribe a handler for a window
    s_window_event_handle = enamel_settings_received_subscribe(enamel_settings_received_window_handler, window);

    // Subscribe a handler for a text layer
    s_text_layer_event_handle = enamel_settings_received_subscribe(enamel_settings_received_text_layer_handler, text_layer);
    
    // call pebble-events app_message_open function
    events_app_message_open(); 
    
    ...
  }

  static void deinit(void) {
      // Unsubscribe from Enamel events
      enamel_settings_received_unsubscribe(s_window_event_handle);
      enamel_settings_received_unsubscribe(s_text_layer_event_handle);

      enamel_deinit();

      ...
  }
  ```
6. Get the value of your setting with :
  
  ``` c
  enamel_get_Mysetting(); // where 'Mysetting' is a messageKey in your configuration file
  ```
---

# Enamel API

## Methods

| Method | Description |
|--------|---------|
| `void enamel_init()` | Initialize Enamel and read settings from persistant storage |
| `void enamel_deinit()` | Deinitialize Enamel and save the settings in the persistant storage |
| `<type> enamel_get_<messageKeyId>()` | Return the value for the setting `messageKeyId` |
| `bool enamel_get_<messageKeyId>(uint16_t index_)` | *Only relevant for `checkboxgroup`*. <br>Return the value at given index for the setting `messageKeyId` |

## Type mapping

| Clay Type | Type returned by the getter |
|--------|---------|
| `input` | `char*` |
| `toggle` | `bool` |
| `color` | `GColor` |
| `select/radiogroup` | `char*` or `enum` |
| `checkboxgroup` | `bool` |
| `slider` | `int32_t` |

### Special case for `select`, `radiogroup`

If the value of the options are `string` in the `config.json`, Enamel will generate a `char*` getter

If the value of the options are `integer`, Enamel will generate an `enum` mapping all the possible values for this setting and the getter will return this `enum`

For the given setting :
``` json
{
  "type": "radiogroup",
  "messageKey": "favorite_food",
  "label": "Favorite Food",
  "defaultValue": "1",
  "options": [
    { 
      "label": "Sushi", 
      "value": 0 
    },
    { 
      "label": "Pizza", 
      "value": 1 
    },
    { 
      "label": "Burgers", 
      "value": 2 
    }
  ]
}
```

Enamel will generate

``` c
typedef enum {
	FAVORITE_FOOD_SUSHI = 0,
	FAVORITE_FOOD_PIZZA = 1,
	FAVORITE_FOOD_BURGER = 2,
} FAVORITE_FOODValue;
FAVORITE_FOODValue enamel_get_favorite_food();
```

You can then easily code switch case for this setting
``` c
switch(enamel_get_favorite_food()){
 case FAVORITE_FOOD_SUSHI : break; //do something
 case FAVORITE_FOOD_PIZZA : break; //do something
 case FAVORITE_FOOD_BURGER : break; //do something
}
```

### Special case for `slider`

Enamel will also generate a constant for your slider containing the 'precision' of your slider, e.g.
```
#define MY_SLIDER_PRECISION 100
```

