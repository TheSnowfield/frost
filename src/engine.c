// SPDX-License-Identifier: MIT
/*******************************************************************************
 * This file is the part of the Frost library
 *
 * (C) Copyright 2025 TheSnowfield.
 *
 * Authors: TheSnowfield <17957399+TheSnowfield@users.noreply.github.com>
 ****************************************************************************/

#include <stdarg.h>

#include "engine.h"
#include "tls.h"
#include "chan.h"
#include "await.h"
#include "callback.h"

static frost_engine_t engine = { 0 };

/**
 * @brief test frost task flag
 *
 * @param ctx task ctx
 * @param flag flag to test
 * @return flag set return non zero
 */
static int __fflag(frost_task_ctx_t* ctx, frost_flag_t flag) {
  return ctx->flags & flag;
}

frost_errcode_t frost_init() {

  if(engine.initialized)
    return frost_err_ok;

  frost_errcode_t _result;

  // create task list
  if(!frost_ok(_result = list_create(&engine.scheduler.tasks))) {
    frost_log(TAG, "go to failure procedure");
    frost_uninit();
    return frost_err_fatal_error;
  }

  // okay all done!
  engine.initialized = true;
  frost_log(TAG, "global initialization finished");

  return frost_err_ok;
}

bool frost_is_initialized() {
  return engine.initialized;
}

frost_errcode_t frost_uninit() {

  // delete all tasks
  if(engine.scheduler.tasks != NULL)
    list_destroy(engine.scheduler.tasks);

  engine.initialized = false;
  frost_log(TAG, "global uninit");

  return frost_err_ok;
}

frost_errcode_t frost_get_engine(frost_engine_t** instance) {

  if(instance == NULL)
    return frost_err_invalid_parameter;

  *instance = &engine;
  return frost_err_ok;
}

frost_errcode_t frost_schedule_tasks() {

  if(!engine.initialized)
    return frost_err_need_initialize;

  bool _is_realtime = true;
  int64_t _last_score = 0;
  uint64_t _time_measure_start = 0;

  list_node_t* _node = engine.scheduler.tasks->head;
  while(_node != NULL) {

    frost_task_ctx_t* _curctx = *(frost_task_ctx_t **)_node->data; {

      // do not invoke itself
      if(engine.scheduler.context == _curctx)
        goto next;

      // for frozen task
      else if (__fflag(_curctx, frost_flag_freeze)) {

        // if not set chan write unfreeze, jump into next
        if(!__fflag(_curctx, frost_flag_unfreeze_by_chan_write)) {
          goto next;
        }

        // dont unfreeze case
        if(!_curctx->chan.ref || _curctx->chan.ref->notify_cnt <= 0) {
          if(!_curctx->chan.ref) {
            frost_log(TAG, "task[%p] set 'unfreeze_by_chan_write' but no channel allocated", _curctx);
          }
          goto next;
        }

        // sync the tick to scheduler main tick to fire the task immediately
        _curctx->tick = engine.scheduler.tick;
      }

      // get current tick time
      uint64_t _time = __frost_time_tick(NULL); {
        engine.scheduler.tick = _time; {
          _time_measure_start = engine.scheduler.tick;
        }
      }

      // it's time to do something? :p
      if(_curctx->interval == 0 || engine.scheduler.tick >= _curctx->tick) {

        #ifdef FROST_DEBUG
        _curctx->fire++;
        #endif /* FROST_DEBUG */

        // update the new context then run the task,
        // and restore the old context finally
        frost_task_ctx_t* _oldctx = engine.scheduler.context; {
          engine.scheduler.context = _curctx;
          __invoke_task_callback(_curctx);
          engine.scheduler.context = _oldctx;

          // refill the tick time
          if (_curctx->refill) {

            if(_curctx->score > 0)
              _curctx->tick += _curctx->interval;
            else
              _curctx->tick = engine.scheduler.tick - _curctx->exec_time + _curctx->interval;

            // calculate score
            engine.scheduler.tick = __frost_time_tick(NULL); {
              _curctx->exec_time = engine.scheduler.tick - _time_measure_start;
              _curctx->score = _curctx->tick - engine.scheduler.tick;
            }
          }

          // if this task marked as one-shot task, remove it from the list
          else {
            frost_task_delete(_curctx);
          }
        }

        // if the task list is changed in the previous task (add task / del task)
        // drop current context and re-run it next time
        if(engine.scheduler.is_dirty) {
          frost_log(TAG, "scheduler has been marked as 'dirty' state, reset context");
          engine.scheduler.context = NULL;
          engine.scheduler.is_dirty = false;
          return frost_err_ok;
        }
      }

      // raise priority
      // preemptive control
      if (_curctx->score < _last_score) {
        list_move_forward(engine.scheduler.tasks, _node);
      }

      // record score for next use
      _last_score = _curctx->score;
      if (_is_realtime && _curctx->score < 0) {
        _is_realtime = false;
      }
    }

    // next task
  next:

    // get current tick time
    engine.scheduler.is_realtime = _is_realtime;
    _node = _node->next;
  }

  engine.scheduler.context = NULL;
  return frost_err_ok;
}

frost_errcode_t frost_task_get_context(frost_task_ctx_t** task) {

  if(task == NULL)
    return frost_err_invalid_parameter;
  else if(!engine.initialized)
    return frost_err_need_initialize;

  // get current task context
  *task = engine.scheduler.context;

  return frost_err_ok;
}

frost_awaiter_t* frost_task_run_ex(void* func, uint32_t argc, ...) {

  if(!engine.initialized)
    return awaiter_from_value(NULL, frost_err_need_initialize);

  // create an awaiter for task
  frost_awaiter_t* _awaiter = awaiter_create(); {
    if(_awaiter == NULL) return awaiter_from_value(NULL, frost_err_out_of_memory);
  }

  // create a new task
  frost_handle_t _task = malloc(sizeof(frost_task_ctx_t)); {
    if(_task == NULL) {
      awaiter_destroy(_awaiter);
      return awaiter_from_value(NULL, frost_err_out_of_memory);
    }
  }

  frost_log(TAG, "create async task using callback address [%p]", func);

  // setup task information
  frost_task_ctx_t* _task_ptr = (frost_task_ctx_t *)_task; {
    memset(_task_ptr, 0x00, sizeof(frost_task_ctx_t));
    _task_ptr->callback = func;
    _task_ptr->awaiter = _awaiter;
    _task_ptr->refill = false;
    _task_ptr->name = "<async task>";

    va_list _args;
    va_start(_args, argc);

    // copy arguments
    _task_ptr->args.argc = argc; {
      for(size_t i = 0; i < argc; ++i) {
        _task_ptr->args.argv[i] = va_arg(_args, void *);
      }
    }

    va_end(_args);
  }

  frost_errcode_t _result;

  // append new task to scheduler
  if(!frost_ok(_result = list_put(engine.scheduler.tasks, &_task_ptr,
     sizeof(frost_task_ctx_t *), &_task_ptr->ref))) {
    free(_task);
    awaiter_destroy(_awaiter);
    return awaiter_from_value(NULL, _result);
  }

  // request update scheduler context
  engine.scheduler.is_dirty = true;
  frost_log(TAG, "mark scheduler context as 'dirty' state");
  frost_log(TAG, "current task size => %zu", engine.scheduler.tasks->size);

  return _awaiter;
}

frost_awaiter_t* frost_task_run(void* func) {
  return frost_task_run_ex(func, 0);
}

frost_errcode_t frost_task_interval(uint32_t interval, void* func, frost_task_ctx_t** task) {

  if(!engine.initialized)
    return frost_err_need_initialize;

  // create a new task
  frost_handle_t _task = malloc(sizeof(frost_task_ctx_t)); {
    if(_task == NULL) return frost_err_out_of_memory;
  }

  frost_log(TAG, "create interval task using callback address"
                 "[%p], interval %zu ms", func, interval);

  // setup task information
  frost_task_ctx_t* _task_ptr = (frost_task_ctx_t *)_task; {
    memset(_task_ptr, 0x00, sizeof(frost_task_ctx_t));
    _task_ptr->name = "<interval>";
    _task_ptr->callback = func;
    _task_ptr->refill = true;
    _task_ptr->interval = interval;
    _task_ptr->tick = __frost_time_tick(NULL) + interval;
    _task_ptr->score = interval;
  }

  frost_errcode_t _result;

  // append new task to scheduler
  if(!frost_ok(_result = list_put(engine.scheduler.tasks, &_task_ptr,
     sizeof(frost_task_ctx_t *), &_task_ptr->ref))) {
    free(_task);
    return _result;
  }

  // if task not NULL then return task pointer
  if(task != NULL) *task = _task_ptr;

  // request update scheduler context
  engine.scheduler.is_dirty = true;
  frost_log(TAG, "mark scheduler context as 'dirty' state");
  frost_log(TAG, "current task size => %zu", engine.scheduler.tasks->size);

  return frost_err_ok;
}

frost_errcode_t frost_task_delete(frost_task_ctx_t* task) {

  if(!engine.initialized)
    return frost_err_need_initialize;

  frost_log(TAG, "perform task '%s'[%p] deletion", task->name, task);

  frost_errcode_t _result;

  // remove task from scheduler
  if(!frost_ok(_result = list_delete(engine.scheduler.tasks, task->ref))) {
    return _result;
  }

  // why not delete awaiter here?

  // because users always use an await function to wait task to finish,
  // if we delete the awaiter right here
  // can cause an invalid pointer and data from the caller.

  // so awaiters should be managed by the caller manually only

  if(task->awaiter) {

    if(!task->awaiter->is_finished) {
      frost_log(TAG, "awaiter is not finished, force marked as cancel state");

      // let caller to cleanup
      awaiter_cancel(task->awaiter);
    }

    // awaiter_destroy(task->awaiter);
    // task->awaiter = NULL;
  }

  // clean up tls storage
  if(task->tls) {

    frost_log(TAG, "destroying a task that has not destroyed tls storage yet, "
                      "this may cause a memory leak");

    frost_tls_destroy_ex(task);
    task->tls = NULL;
  }

  // clean up channel
  if(task->chan.ref || task->chan.bind) {
    frost_log(TAG, "destroying a task that has not destroyed chan yet, "
                      "this may cause a memory leak");

    // destroy channel with automatic unbind
    frost_chan_destroy_ex(task);
    task->chan.ref = NULL;
    task->chan.bind = NULL;
  }

  // request update scheduler context
  engine.scheduler.is_dirty = true;
  frost_log(TAG, "mark scheduler context as 'dirty' state");
  frost_log(TAG, "current task size => %zu", engine.scheduler.tasks->size);

  return frost_err_ok;
}

frost_errcode_t frost_task_set_flag(frost_task_ctx_t* task, frost_flag_t flag) {

  if(task == NULL)
    return frost_err_invalid_parameter;
  else if(!engine.initialized)
    return frost_err_need_initialize;

  task->flags = flag;
  return frost_err_ok;
}

frost_errcode_t frost_task_get_flag(frost_task_ctx_t* task, frost_flag_t* flag) {

  if(task == NULL || flag == NULL)
    return frost_err_invalid_parameter;
  else if(!engine.initialized)
    return frost_err_need_initialize;

  *flag = task->flags;
  return frost_err_ok;
}

frost_errcode_t frost_sleep(size_t duration_ms) {

  uint64_t _local_time = __frost_time_tick(NULL);
  frost_errcode_t _ret = frost_err_ok;

  while(!(__frost_time_tick(NULL) - _local_time >= duration_ms)) {
    _ret = frost_schedule_tasks();
  }

  return _ret;
}

uint64_t frost_get_timetick(uint64_t* tick) {
  return __frost_time_tick(tick);
}

/**
 * @brief enumerate task list
 *
 * @return frost_errcode_t if reach the end return frost_err_eof
 */
frost_errcode_t frost_enumerate_tasks(frost_task_enum_t* e) {

  #define _to_handle(x) ((frost_handle_t)(x))
  #define _from_handle(x) ((list_node_t*)(x))

  if(e == NULL) {
    return frost_err_invalid_parameter;
  }

  list_node_t* _node = NULL;
  
  if(!e->__inited) {
    e->__inited = true;

    if(!engine.scheduler.tasks->head) {
      return frost_err_eof;
    }

    _node = engine.scheduler.tasks->head;
    e->__next = _to_handle(_node->next);
    e->index = 0;
    e->task = *(frost_task_ctx_t **)_node->data;
    return frost_err_ok;
  }
  
  else {

    if(!e->__next) {
      return frost_err_eof;
    }

    _node = ((list_node_t *)e->__next);
    e->__next = _to_handle(_node->next);
    e->index++;
    e->task = *(frost_task_ctx_t **)_node->data;
    return frost_err_ok;
  }

  #undef _to_handle
  #undef _from_handle
}
