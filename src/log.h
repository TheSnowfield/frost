#ifndef _FROST_LOG_H
#define _FROST_LOG_H

#ifdef _MSC_VER
  #define TAG __FUNCTION__
#elif __GNUC__
  #define TAG __func__
#else
  #warning "Your compiler may not supported __func__ macro, so defined the TAG as '---'"
  #define TAG "---"
#endif

#include "frost/port.h"
#define frost_log __frost_log_print

#endif /* _FROST_LOG_H */
