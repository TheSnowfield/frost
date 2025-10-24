// SPDX-License-Identifier: MIT
/*******************************************************************************
 * This file is the part of the Frost library
 *
 * (C) Copyright 2025 TheSnowfield.
 *
 * Authors: TheSnowfield <17957399+TheSnowfield@users.noreply.github.com>
 ****************************************************************************/

#ifndef _FROST_COMMON_H
#define _FROST_COMMON_H

#include <stdint.h>

/**
 * @brief frost native handle
 */
typedef uintptr_t* frost_handle_t;

/**
 * @brief frost error code
 */
typedef enum {
  frost_err_ok,
  frost_err_fatal_error            = -1,
  frost_err_task_timeout           = -2,
  frost_err_invalid_parameter      = -3,
  frost_err_task_canceled          = -4,
  frost_err_out_of_memory          = -5,
  frost_err_need_initialize        = -6,
} frost_errcode_t;

#define frost_ok(x) ((x) == frost_err_ok)

#endif /* _FROST_COMMON_H */
