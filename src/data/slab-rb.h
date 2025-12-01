// SPDX-License-Identifier: MIT
/*******************************************************************************
 * This file is the part of the Frost library
 *
 * (C) Copyright 2025 TheSnowfield.
 *
 * Authors: TheSnowfield <17957399+TheSnowfield@users.noreply.github.com>
 ****************************************************************************/

#ifndef _FROST_DATA_SLAB_RB_H
#define _FROST_DATA_SLAB_RB_H

#include <stddef.h>

typedef struct _rb_node_t {
  struct _rb_node_t* next;
  size_t size;
} rb_node_t;

typedef struct _rb_header_t {
  bool self_increase;
  rb_node_t* rb_start;
  rb_node_t* rb_end;
  rb_node_t* init_rb_body;
  size_t init_rb_body_size;
  size_t size;
  size_t capacity;
  size_t block_size;
} rb_header_t;

/**
 * @brief create ring buffer
 *
 * @param capacity initialize size
 * @param blocksz block size every nodes
 * @param rb ring buffer context
 */
frost_errcode_t rb_create(size_t capacity, size_t blocksz, rb_header_t** rb);

/**
 * @brief ring buffer put data
 *
 * @param rb ring buffer context
 * @param data data being put
 * @param length data length
 */
frost_errcode_t rb_put(rb_header_t* rb, void* data, size_t length);

/**
 * @brief ring buffer read data
 *
 * @param rb ring buffer context
 * @param data data being put
 * @param length data length
 * @param remain remain
 */
frost_errcode_t rb_read(rb_header_t* rb, void* data, size_t* length, size_t* remain);

/**
 * @brief clear ring buffer and destroy
 *
 * @param rb ring buffer context
 */
frost_errcode_t rb_destroy(rb_header_t* rb);

#endif /* _FROST_DATA_SLAB_RB_H */
