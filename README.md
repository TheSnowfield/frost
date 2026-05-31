## ❄ frost

❄ Frost ❄ is a cooperative asynchronous task scheduler, uses a deadline-derived urgency
score to gradually promote tasks that are closer to, or already past, their scheduled execution time.  
This makes it similar in spirit to a lightweight soft-EDF scheduler,
but implemented as a portable cooperative event loop.  
 - cooperative scheduling
 - deadline/urgency-based task promotion
 - channel-triggered task wakeup
 - task local storage
 - lightweight awaiter primitives
 
Frost does not interfere with task execution, offering better cross-platform compatibility,  
it features an advanced task scheduler capable of running three types of tasks:  
 - **one-shot**: task runs one shot, then deletes itself
 - **periodic**: task runs periodically, with a given period (setInterval)
 - **awaken-by-chan-write**: awake the task when channel is not empty, otherwise turn into freeze state

[![frost](https://img.shields.io/badge/Frost-v0.2-b7fffd)](#)
[![license](https://img.shields.io/badge/LICENSE-MIT-blue)](./blob/main/LICENSE)

## ❄ ToDo
- [x] EDF scheduling
- [x] unfreeze task by channel write
- [] Fix await

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
