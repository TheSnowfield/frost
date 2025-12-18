// SPDX-License-Identifier: MIT
/*******************************************************************************
 * This file is the part of the Frost library
 *
 * (C) Copyright 2025 TheSnowfield.
 *
 * Authors: TheSnowfield <17957399+TheSnowfield@users.noreply.github.com>
 ****************************************************************************/

#include <stdlib.h>

#include "engine.h"
#include "await.h"

static frost_awaiter_t* awaiter_create_ex(bool is_finished,
frost_handle_t result, frost_errcode_t status) {

  // memory allocation failed.. nothing to do. oops
  frost_handle_t _awaiter = malloc(sizeof(frost_awaiter_t)); {
    if(_awaiter == NULL) {
      frost_log(TAG, "memory allocation failed for task awaiter");
      return NULL;
    }
  }

  // initialize awaiter
  frost_awaiter_t* _awaiter_ptr = (frost_awaiter_t *)_awaiter; {
    _awaiter_ptr->is_finished = is_finished;
    _awaiter_ptr->result = result;
    _awaiter_ptr->status = status;
  }

  frost_log(TAG, "awaiter created %p", _awaiter_ptr);

  return _awaiter_ptr;
}

frost_awaiter_t* awaiter_from_value(frost_handle_t value, frost_errcode_t status) {
  return awaiter_create_ex(true, value, status);
}

frost_awaiter_t* awaiter_create() {
  return awaiter_create_ex(false, NULL, frost_err_ok);
}

frost_errcode_t awaiter_destroy(frost_awaiter_t* awaiter) {
  
  if(awaiter == NULL)
    return frost_err_invalid_parameter;

  frost_log(TAG, "awaiter destroyed %p", awaiter);
  free(awaiter);

  return frost_err_ok;
}

frost_awaiter_t* awaiter_await(frost_awaiter_t* awaiter) {
  
  if(awaiter == NULL)
    return awaiter_from_value(NULL, frost_err_invalid_parameter);

  frost_engine_t* _engine;
  frost_errcode_t _result;

  if(!frost_ok(_result = frost_get_engine(&_engine))) {
    awaiter->status = frost_err_invalid_parameter;
    awaiter->is_finished = true;
    awaiter->result = NULL;
    return awaiter;
  }

  uint64_t _start = _engine->scheduler.tick;
  while(frost_schedule_tasks() == frost_err_ok) {
    
    // the task is finished
    if(awaiter->is_finished)
      return awaiter;

    // check if task timed out
    if(awaiter->timeout != 0 &&
      (_engine->scheduler.tick- _start >= awaiter->timeout)) {

      frost_log(TAG, "task timed out, force to break");

      awaiter->status = frost_err_task_timeout;
      awaiter->is_finished = true;
      awaiter->result = NULL;

      return awaiter;
    }

  }

  frost_log(TAG, "awaiting task failure, frost_schedule_tasks() does not return frost_err_ok");
  awaiter->status = frost_err_fatal_error;
  awaiter->is_finished = true;
  awaiter->result = NULL;
  return awaiter;
}

frost_errcode_t awaiter_finish(frost_awaiter_t* awaiter, frost_handle_t result) {

  if(awaiter == NULL)
    return frost_err_invalid_parameter;

  awaiter->result = result;
  awaiter->status = frost_err_ok;
  awaiter->is_finished = true;
  return frost_err_ok;
}

frost_errcode_t awaiter_cancel(frost_awaiter_t* awaiter) {

  if(awaiter == NULL)
    return frost_err_invalid_parameter;

  awaiter->result = NULL;
  awaiter->status = frost_err_task_canceled;
  awaiter->is_finished = true;
  return frost_err_ok;
}
