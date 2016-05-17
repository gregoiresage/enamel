# Enamel
Enamel is a python script that generates C helpers from a [Clay](https://github.com/pebble/clay) configuration file to easily get the value of your settings.

Enamel will :
* handle AppMessages automatically (app_message_open, handler registration, ...)
* save/load the value of the settings in the persistant storage automatically
* provide a getter for each of your settings

You can focus on your watchapp/face, Enamel will do the rest !

---

# Demo

You can find a basic demo project using Enamel [here](https://github.com/gregoiresage/demo-enamel)

---

# Getting Started (pebble wscript)
1. You project must contain a valid configuration file in `src/js/config.json` (see https://github.com/pebble/clay)
2. Extract [Enamel](https://github.com/gregoiresage/enamel/releases/latest) at the root of your Pebble project or create a git submodule
3. Copy and paste the following line into the top of your `wscript` : 
  
  ``` python
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

# Getting Started (python)
If you can't modify the wscript (Cloudpebble development) you can call directly the python script.

Enamel needs the Jinja2 module for the code generation.
Install the dependency with 
```
pip install Jinja2
```

The following command will generate 2 files (enamel.c and enamel.h), you just need to copy them in your project
```
python enamel.py --appinfo /path/to/your/appinfo.json --config /path/to/your/config.json 
```

Call `python enamel.py --help` for help

---

# Using Enamel
1. Setup your project correctly for Clay : https://github.com/pebble/clay
2. Include `enamel.h` in your c file : 
  
  ``` c
  #include "enamel.h"
  ```
3. Initialize enamel in your `init` function : 
  
  ``` c
  static void init(void) {
    // Initialize Enamel to register App Message handlers and restores settings
    enamel_init();
    
    // Let enamel init the appmessage framework
    enamel_app_message_open(0, 0, NULL); 
    
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
5. (Optional) Register a custom `in_received_handler` in your `init`. <br>This handler will be automatically called by enamel when a setting is received.

  ``` c
  static void in_received_handler(DictionaryIterator *iter, void *context) {
    APP_LOG(0, "Settings received %d", enamel_get_myinteger());
    window_set_background_color(window, enamel_get_background());
    // do what you want here 
    // you will probably udpate your textlayers, colors, ... with the new settings
    // and mark your layers dirty
  }
  
  ...
  
  static void init(void) {
    // Initialize Enamel to register App Message handlers and restores settings
    enamel_init();

    // Register our custom receive handler
    enamel_app_message_open(0, 0, in_received_handler);
    
    ...
  }
  ```
6. Get the value of your setting with :
  
  ``` c
  enamel_get_Mysetting(); // where 'Mysetting' is an appKey in your configuration file
  ```

6. (Advanced usage) You can manually manage your appmessage and call `enamel_inbox_received_handle` to notify enamel when settings are received :
  
  ``` c
  static void in_received_handler(DictionaryIterator *iter, void *context) {
    // Do some stuff with the dictionary
    
    // Call enamel to parse the dictionary if it contains settings
    enamel_inbox_received_handle(iter, context);
  }
  
  ...
  
  static void init(void) {
    // Initialize Enamel to register App Message handlers and restores settings
    enamel_init();

    // handle appmessages manually
    app_message_register_inbox_received(in_received_handler);
    app_message_open(500, 500);
    
    ...
  }
  ```
---

# Enamel API

## Methods

| Method | Description |
|--------|---------|
| `void enamel_init()` | Initialize Enamel and read settings from persistant storage |
| `void enamel_app_message_open(const uint32_t size_inbound, const uint32_t size_outbound, AppMessageInboxReceived received_callback)` | Let Enamel manages the appmessages. <br>If `size_inbound` is `0`, Enamel will calculate the inbound size automatically for you. The `received_callback` will be called by Enamel when a message is received |
| `void enamel_deinit()` | Deinitialize Enamel and save the settings in the persistant storage |
| `<type> enamel_get_<appKeyId>()` | Return the value for the setting `appKeyId` |
| `<type> enamel_get_<appKeyId>(uint16_t index_)` | *Only relevant for `checkboxgroup`*. <br>Return the value at given index for the setting `appKeyId` |
| `uint16_t enamel_get_<appKeyId>_count()` | *Only relevant for `checkboxgroup`*. <br>Return the number of values for the setting `appKeyId` |

## Type mapping

| Clay Type | Type returned by the getter |
|--------|---------|
| `input` | `char*` |
| `toggle` | `bool` |
| `color` | `GColor` |
| `select/radiogroup` | `char*` or `enum` |
| `checkboxgroup` | `char*` or `enum` |
| `slider` | `int32_t` |

### Special case for `select`, `radiogroup` and `checkboxgroup`

If the value of the options are `string` in the `config.json`, Enamel will generate a `char*` getter

If the value of the options are `integer`, Enamel will generate an `enum` mapping all the possible values for this setting and the getter will return this `enum`

For the given setting :
``` json
{
  "type": "radiogroup",
  "appKey": "favorite_food",
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

