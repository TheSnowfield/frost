// SPDX-License-Identifier: MIT
/*******************************************************************************
 * This file is the part of the Frost library
 *
 * (C) Copyright 2025 TheSnowfield.
 *
 * Authors: TheSnowfield <17957399+TheSnowfield@users.noreply.github.com>
 ****************************************************************************/

#ifndef _FROST_TLS_H
#define _FROST_TLS_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief create task local storage
 *
 * @param task task context pointer, pass NULL meant to use current task context
 * @return frost_errcode_t if success return frost_err_ok
 */
frost_errcode_t frost_tls_alloc_ex(frost_task_ctx_t* task);

/**
 * @brief destroy task local storage
 *
 * @param task task context pointer, pass NULL meant to use current task context
 * @return frost_errcode_t if success return frost_err_ok
 */
frost_errcode_t frost_tls_destroy_ex(frost_task_ctx_t* task);

/**
 * @brief check task local storage is allocated
 *
 * @param task task context pointer, pass NULL meant to use current task context
 * @return true / false
 */
bool frost_tls_is_allocated_ex(frost_task_ctx_t* task);

/**
 * @brief put value to task local storage
 *
 * @param task task context pointer, pass NULL meant to use current task context
 * @param index key index
 * @param value value
 *
 * @return frost_errcode_t if success return frost_err_ok
 */
frost_errcode_t frost_tls_set_value_ex(frost_task_ctx_t* task, uint32_t index, size_t value);

/**
 * @brief get value from task local storage
 *
 * @param task task context pointer, pass NULL meant to use current task context
 * @param index key index
 * @param value value
 *
 * @return frost_errcode_t if success return frost_err_ok
 */
frost_errcode_t frost_tls_get_value_ex(frost_task_ctx_t* task, uint32_t index, size_t* value);

/**
 * @brief allocate task local storage of the task
 *
 * @return frost_errcode_t if success return frost_err_ok
 */
frost_errcode_t frost_tls_alloc();

/**
 * @brief destroy the task local storage
 *
 * @return frost_errcode_t if success return frost_err_ok
 */
frost_errcode_t frost_tls_destroy();

/**
 * @brief is task local storage allocated
 *
 * @param task task context pointer, pass NULL meant to use current task context
 */
bool frost_tls_is_allocated(frost_task_ctx_t* task);

/**
 * @brief set value from task local storage
 *
 * @param index index
 * @param value value
 * @return frost_errcode_t
 */
frost_errcode_t frost_tls_set_value(uint32_t index, size_t value);

/**
 * @brief get value from task local storage
 *
 * @param index index
 * @param value value
 * @return frost_errcode_t
 */
frost_errcode_t frost_tls_get_value(uint32_t index, size_t* value);

#endif /* _FROST_TLS_H */
