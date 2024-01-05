## ❄ Frost OS

Frost OS is an async multi-task operating system for embedded hardware,  
which supports await/async function calling, and TLS(Task Local Storage).

Frost doesn't interrupt the task execution, so it's not a real-time operating system.

It owns an advanced task scheduler that can run two task types:  
 - **one shot**: task runs one shot, then deletes itself
 - **periodic**: task runs periodically, with a given period (setInterval)

It also supported a Golang-like "channel" feature which be used to communicate between tasks. (WIP)

[![frost](https://img.shields.io/badge/Frost%20OS-v0.1-68e8fd)](#)
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

## ❄ Porting Frost

Please port below functions to your platform to make Frost work:
```c
void __frost_log_print(const char* tag, const char* fmt, ...) {
  // Your platform implementation
}

uint64_t __frost_time_tick(uint64_t* tick) {
  // Your platform implementation
}
```

And markup `FROST_PORTED_LOG_PRINT` and `FROST_PORTED_TIME_TICK` before you include the Frost header.   
See more in [frost/port.h](frost/port.h)

## ❄ LICENSE
Frost is licensed under the MIT License with ❤.
