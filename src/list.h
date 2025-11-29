// SPDX-License-Identifier: MIT
/*******************************************************************************
 * This file is the part of the Frost library
 *
 * (C) Copyright 2025 TheSnowfield.
 *
 * Authors: TheSnowfield <17957399+TheSnowfield@users.noreply.github.com>
 ****************************************************************************/

#ifndef _UTILS_LIST_H
#define _UTILS_LIST_H

#include <stddef.h>

typedef struct _list_node_t {
  struct _list_node_t* next;
  struct _list_node_t* prev;
  void* data;
} list_node_t;

typedef struct _list_ctx_t {
  list_node_t* head;
  list_node_t* tail;
  size_t size;
  bool closed;
} list_ctx_t;

/**
 * @brief create a new list
 *
 * @param ctx return list context if success
 * @return status_t
 */
frost_errcode_t list_create(list_ctx_t** ctx);

/**
 * @brief create a new list with a circle type
 *
 * @param ctx return list context if success
 * @return status_t
 */
frost_errcode_t list_create_ring(list_ctx_t** ctx);

/**
 * @brief put data into list
 *
 * @param ctx list context pointer
 * @param data data pointer
 * @param length data length
 * @param node return node pointer
 * @return status_t
 */
frost_errcode_t list_put(list_ctx_t* ctx, void* data, size_t length, list_node_t** node);

/**
 * @brief delete item from list
 *
 * @param ctx list context pointer
 * @param node node pointer
 * @return status_t
 */
frost_errcode_t list_delete(list_ctx_t* ctx, list_node_t* node);

/**
 * @brief destroy list
 *
 * @param ctx list context pointer
 * @return status_t
 */
frost_errcode_t list_destroy(list_ctx_t* ctx);

#endif /* _UTILS_LIST_H */
