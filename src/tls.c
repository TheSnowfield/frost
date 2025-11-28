// SPDX-License-Identifier: MIT
/*******************************************************************************
 * This file is the part of the Frost library
 *
 * (C) Copyright 2025 TheSnowfield.
 *
 * Authors: TheSnowfield <17957399+TheSnowfield@users.noreply.github.com>
 ****************************************************************************/

#include <stdlib.h>

#include "common.h"
#include "engine.h"
#include "tls.h"
#include "log.h"
#include "utils.h"

frost_errcode_t frost_tls_alloc_ex(frost_task_ctx_t* task) {
  
  frost_task_ctx_t* _task = __get_task_ctx(task); {
    if(_task == NULL) return frost_err_invalid_parameter;
  }

  frost_log(TAG, "allocating tls for task '%s'[%p]", _task->name, _task);

  // if tls context has been already allocated, skip
  if(_task->tls != NULL) {
    frost_log(TAG, "this task already allocated tls, skipping");
    return frost_err_ok;
  }

  // allocate tls context
  frost_handle_t _tls = malloc(sizeof(frost_tls_t)); {
    
    // if allocation failed, return error
    if(_tls == NULL)
      return frost_err_out_of_memory;

    // okay for tls
    _task->tls = (frost_tls_t *)_tls;
    frost_log(TAG, "tls[%p] has allocated for task '%s'[%p]", _tls, _task->name, _task);
  }

  return frost_err_ok;
}

frost_errcode_t frost_tls_alloc() {
  return frost_tls_alloc_ex(NULL);
}

frost_errcode_t frost_tls_destroy_ex(frost_task_ctx_t* task) {

  frost_task_ctx_t* _task = __get_task_ctx(task); {
    if(_task == NULL) return frost_err_invalid_parameter;
  }
  
  // skip if task has no tls allocated
  if(_task->tls == NULL) {
    frost_log(TAG, "this task has not allocated tls, skipping");
    return frost_err_ok;
  }

  // clear tls context
  free(_task->tls); {
    _task->tls = NULL;
  }

  frost_log(TAG, "clearing tls for task '%s'[%p]", _task->name, _task);
  return frost_err_ok;
}

frost_errcode_t frost_tls_destroy() {
  return frost_tls_destroy_ex(NULL);
}

bool frost_tls_is_allocated_ex(frost_task_ctx_t* task) {

  frost_task_ctx_t* _task = __get_task_ctx(task); {
    if(_task == NULL) return frost_err_invalid_parameter;
  }

  return _task->tls != NULL;
}

bool frost_tls_is_allocated(frost_task_ctx_t* task) {
  return frost_tls_is_allocated_ex(NULL);
}

frost_errcode_t frost_tls_set_value_ex(frost_task_ctx_t* task, uint32_t index, size_t value) {

  frost_task_ctx_t* _task = __get_task_ctx(task); {
    if(_task == NULL) return frost_err_invalid_parameter;
  }

  // validate arguments
  if(index > FROST_TLS_SIZE || index < 0)
    return frost_err_invalid_parameter;

  // setup value
  _task->tls->table[index] = value;
  return frost_err_ok;
}

frost_errcode_t frost_tls_set_value(uint32_t index, size_t value) {
  return frost_tls_set_value_ex(NULL, index, value);
}

frost_errcode_t frost_tls_get_value_ex(frost_task_ctx_t* task, uint32_t index, size_t* value) {

  frost_task_ctx_t* _task = __get_task_ctx(task); {
    if(_task == NULL) return frost_err_invalid_parameter;
  }

  // validate arguments
  if(value == NULL || index > FROST_TLS_SIZE || index < 0)
    return frost_err_invalid_parameter;

  // read value
  *value = _task->tls->table[index];
  return frost_err_ok;
}

frost_errcode_t frost_tls_get_value(uint32_t index, size_t* value) {
  return frost_tls_get_value_ex(NULL, index, value);
}
