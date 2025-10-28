## ❄ frost

❄ Frost ❄ is an asynchronous multi-tasking library for embedded hardware,  
supporting await/async function calls and Task Local Storage (TLS).

Frost does not interfere with task execution, offering better cross-platform compatibility,  
it features an advanced task scheduler capable of running two types of tasks:  
 - **one shot**: task runs one shot, then deletes itself
 - **periodic**: task runs periodically, with a given period (setInterval)

Additionally, it supports a Golang-like "channel" for communication between tasks. (WIP)

[![frost](https://img.shields.io/badge/Frost-v0.1-b7fffd)](#)
[![license](https://img.shields.io/badge/LICENSE-MIT-blue)](./blob/main/LICENSE)

## ❄ Example

This is a simple example of toggling a LED every 1 second in Arduino:

```c
#include <frost/api.h>

void __task_led_toggle() {
  static int led_state = 0;
  digitalWrite(LED_BUILTIN, led_state);
  led_state = !led_state;
}

void setup() {
  
  // frost init
  frost_init();

  // create a periodic task as a 1000ms interval
  frost_task_interval(&__task_led_toggle, 1000, NULL);
}

void loop() {

  // run frost scheduler
  frost_schedule_tasks();
}

```

## ❄ Porting

Please port below functions to your platform to work with Frost:
```c
void __frost_log_print(const char* tag, const char* fmt, ...) {
  // Your platform implementation
}

uint64_t __frost_time_tick(uint64_t* tick) {
  // Your platform implementation
}
```

And pass `-DFROST_PORTED_LOG_PRINT` and `-DFROST_PORTED_TIME_TICK` to the compiler.   
See more in [frost/port.h](frost/port.h)

## ❄ LICENSE
Frost is licensed under the MIT License with ❤.
