#ifndef _FROST_TLS_H
#define _FROST_TLS_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief create task local storage
 *
 * @param task current task context pointer, if NULL, get current task context
 * @return frost_errcode_t if success return orinus_ok
 */
frost_errcode_t tls_alloc_ex(frost_task_ctx_t* task);

/**
 * @brief destroy task local storage
 *
 * @param task current task context pointer, if NULL, get current task context
 * @return frost_errcode_t if success return orinus_ok
 */
frost_errcode_t tls_destroy_ex(frost_task_ctx_t* task);

/**
 * @brief check task local storage is allocated
 *
 * @param task current task context pointer, if NULL, get current task context
 * @return true / false
 */
bool tls_is_allocated_ex(frost_task_ctx_t* task);

/**
 * @brief put value to task local storage
 *
 * @param task current task context pointer, if NULL, get current task context
 * @param index key index
 * @param value value
 *
 * @return frost_errcode_t if success return orinus_ok
 */
frost_errcode_t tls_set_value_ex(frost_task_ctx_t* task, uint32_t index, size_t value);

/**
 * @brief get value from task local storage
 *
 * @param task current task context pointer, if NULL, get current task context
 * @param index key index
 * @param value value
 *
 * @return frost_errcode_t if success return orinus_ok
 */
frost_errcode_t tls_get_value_ex(frost_task_ctx_t* task, uint32_t index, size_t* value);

frost_errcode_t tls_alloc();
frost_errcode_t tls_destroy();
bool tls_is_allocated();
frost_errcode_t tls_set_value(uint32_t index, size_t value);
frost_errcode_t tls_get_value(uint32_t index, size_t* value);

#endif /* _FROST_TLS_H */
