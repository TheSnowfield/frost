#ifndef _FROST_ENGINE_H
#define _FROST_ENGINE_H

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "common.h"
#include "log.h"
#include "list.h"

#define T frost_handle_t
typedef void (* frost_callback_t)();
typedef void (* frost_callback_arg1_t) (T);
typedef void (* frost_callback_arg2_t) (T,T);
typedef void (* frost_callback_arg3_t) (T,T,T);
typedef void (* frost_callback_arg4_t) (T,T,T,T);
typedef void (* frost_callback_arg5_t) (T,T,T,T,T);
typedef void (* frost_callback_arg6_t) (T,T,T,T,T,T);
typedef void (* frost_callback_arg7_t) (T,T,T,T,T,T,T);
typedef void (* frost_callback_arg8_t) (T,T,T,T,T,T,T,T);
typedef void (* frost_callback_arg9_t) (T,T,T,T,T,T,T,T,T);
typedef void (* frost_callback_arg10_t)(T,T,T,T,T,T,T,T,T,T);
typedef void (* frost_callback_arg11_t)(T,T,T,T,T,T,T,T,T,T,T);
typedef void (* frost_callback_arg12_t)(T,T,T,T,T,T,T,T,T,T,T,T);
typedef void (* frost_callback_arg13_t)(T,T,T,T,T,T,T,T,T,T,T,T,T);
typedef void (* frost_callback_arg14_t)(T,T,T,T,T,T,T,T,T,T,T,T,T,T);
typedef void (* frost_callback_arg15_t)(T,T,T,T,T,T,T,T,T,T,T,T,T,T,T);
#undef T

#define TLS_SIZE 8

#ifdef _MSC_VER
  #define TAG __FUNCTION__
#elif __GNUC__
  #define TAG __func__
#else
  #warning "Your compiler may not supported __func__ macro, so defined the TAG as '---'"
  #define TAG "---"
#endif

typedef enum {
  frost_flag_suspend = 1,
} frost_flag_t;

typedef struct _frost_awaiter_t {
  bool is_finished;
  frost_handle_t result;
  frost_errcode_t status;
  uint64_t timeout;
} frost_awaiter_t;

typedef struct _frost_tls_t {
  size_t table[TLS_SIZE];
} frost_tls_t;

typedef struct {
  uint32_t argc;
  frost_handle_t argv[16];
} frost_args_t;

typedef struct _frost_ctx_t {
  list_node_t* ref;
  const char* name;
  frost_args_t args;
  frost_tls_t* tls;
  frost_awaiter_t* awaiter;
  frost_callback_t callback;
  frost_flag_t flags;
  uint32_t interval;
  uint64_t tick;
  bool refill;
} frost_task_ctx_t;

typedef struct {
  bool initialized;
  struct {
    list_ctx_t* tasks;
    frost_task_ctx_t* context;
    uint64_t tick;
    bool is_dirty;
  } scheduler;
} frost_engine_t;

/**
 * @brief engine initialization
 *
 * @return frost_errcode_t if success return ok
 */
frost_errcode_t frost_init();

/**
 * @brief is ending initialized
 *
 * @return initialized return true
 */
bool frost_is_initialized();

/**
 * @brief engine uninitialization
 *
 * @return frost_errcode_t if success return ok
 */
frost_errcode_t frost_uninit();

/**
 * @brief engine is initialized
 *
 * @return bool if engine is initialized return true
*/
bool frost_is_initialized();

/**
 * @brief get engine instance
 *
 * @param instance pointer to storage engine pointer
 * @return frost_errcode_t if success return ok
 */
frost_errcode_t frost_get_instance(frost_engine_t** instance);

/**
 * @brief process tasks
 *
 * @return oricinus_frost_errcode_t if success return ok
 */
frost_errcode_t frost_schedule_tasks();

/**
 * @brief get current context
 *
 * @return frost_errcode_t if success return ok
 */
frost_errcode_t frost_task_get_context(frost_task_ctx_t** task);

/**
 * @brief run a task async
 *
 * @param func task callback
 * @return frost_awaiter_t* return an awaiter, please call @ref awaiter_destroy() to free it after task done
 */
// frost_awaiter_t* frost_task_run(task_callback_t func);
frost_awaiter_t* frost_task_run(void* func);

/**
 * @brief run a task async
 *
 * @param func task callback
 * @param argc argument count for task
 * @param ... arguments
 * @return frost_awaiter_t* return an awaiter, please call @ref awaiter_destroy() to free it after task done
*/
frost_awaiter_t* frost_task_run_ex(void* func, uint32_t argc, ...);

/**
 * @brief set task interval
 *
 * @param interval interval in milliseconds
 * @param func task callback
 * @param task pointer to task context
 * @return frost_errcode_t if success return ok
*/
frost_errcode_t frost_task_interval(uint32_t interval, void* func, frost_task_ctx_t** task);

/**
 * @brief delete a task
 *
 * @param task pointer to task context
 * @return frost_errcode_t if success return ok
*/
frost_errcode_t frost_task_delete(frost_task_ctx_t* task);

/**
 * @brief set task flag
 *
 * @param task pointer to task context
 * @param flag the flag to set
 * @return frost_errcode_t if success return ok
 */
frost_errcode_t frost_task_set_flag(frost_task_ctx_t* task, frost_flag_t flag);

/**
 * @brief get task flags
 *
 * @param task pointer to task context
 * @param flag the flag to get
 * @return frost_flag_t
 */
frost_errcode_t frost_task_get_flag(frost_task_ctx_t* task, frost_flag_t* flag);

/**
 * @brief sleep (ms). this function can grab the schedule rights from the main thread,
 * temporarily schedule another tasks and wait for timeout.
 *
 * @param duration_ms duration in millisecond
 * @return frost_errcode_t if success return ok
 */
frost_errcode_t frost_sleep(size_t duration_ms);
#endif /* _FROST_ENGINE_H */
