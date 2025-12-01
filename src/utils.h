// SPDX-License-Identifier: MIT
/*******************************************************************************
 * This file is the part of the Frost library
 *
 * (C) Copyright 2025 TheSnowfield.
 *
 * Authors: TheSnowfield <17957399+TheSnowfield@users.noreply.github.com>
 ****************************************************************************/

#ifndef _FROST_UTILS_H
#define _FROST_UTILS_H

static frost_task_ctx_t* __get_task_ctx(frost_task_ctx_t* task) {
  
  if(task != NULL) return task;

  frost_task_ctx_t* _task;
  frost_errcode_t _result;

  // get current task context
  if(!frost_ok(_result = frost_task_get_context(&_task))) {
    frost_log(TAG, "invalid task context");
    return NULL;
  }

  return _task;
}


#endif /* _FROST_UTILS_H */
