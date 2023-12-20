#ifndef _FROST_PORT_H
#define _FROST_PORT_H

#include <stdint.h>
#include <stdarg.h>

#ifndef FROST_PORTED_LOG_PRINT
  static void __frost_log_print(const char* tag, const char* fmt, ...) {}
#else
  extern void __frost_log_print(const char* tag, const char* fmt, ...);
#endif

#ifndef FROST_PORTED_TIME_TICK
  #warning "the __frost_time_tick function is not ported, \
            this will cause a frozen execution! \
            please port this function in your application before using Frost."
  static uint64_t __frost_time_tick(uint64_t* tick) { return 0; }
#else
  extern uint64_t __frost_time_tick(uint64_t* tick);
#endif

#endif /* _FROST_PORT_H */
