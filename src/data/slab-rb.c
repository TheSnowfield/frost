// SPDX-License-Identifier: MIT
/*******************************************************************************
 * This file is the part of the Frost library
 *
 * (C) Copyright 2025 TheSnowfield.
 *
 * Authors: TheSnowfield <17957399+TheSnowfield@users.noreply.github.com>
 ****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../common.h"
#include "../log.h"
#include "slab-rb.h"

/**
 * MARK: __rbnode_needs_free
 * @brief conditicates the node is need to free
 *
 * @param rb ringbuffer header pointer
 * @param node the ringbuffer node
 */
static bool __rbnode_needs_free(rb_header_t* rb, rb_node_t* node) {
  return (node < rb->init_rb_body) || (node > (rb->init_rb_body + rb->init_rb_body_size));
}

/**
 * MARK: __rbnode_data_area
 * @brief get rb node data area
 */
static void* __rbnode_data_area(rb_node_t* node) {
  return (void*)(node + 1);
}

/**
 * MARK: rb_create
 * @brief create ring buffer
 *
 * @param capacity initialize size
 * @param blocksz block size every node
 * @param rb ring buffer context
 */
frost_errcode_t rb_create(size_t capacity, size_t blocksz, rb_header_t** rb) {

  rb_header_t* _rb_header = (rb_header_t*)malloc(sizeof(rb_header_t)); {
    if(!_rb_header) return frost_err_out_of_memory;
  }

  size_t _rb_body_length = capacity * (sizeof(rb_node_t) + blocksz);
  rb_node_t* _rb_body = (rb_node_t*)malloc(_rb_body_length); {
    if(!_rb_body) {
      free(_rb_header);
      return frost_err_out_of_memory;
    }
  }

  memset(_rb_header, 0, sizeof(*_rb_header));
  memset(_rb_body, 0, _rb_body_length);

  #define __rb_slab(body, index, block) \
    ((rb_node_t*)(((uint8_t*)body) + (index) * (sizeof(rb_node_t)+block)))

  // build rb chain
  rb_node_t* _rb_node_start = __rb_slab(_rb_body, 0, blocksz);
  rb_node_t* _rb_node_end = __rb_slab(_rb_body, capacity - 1, blocksz); {
    _rb_node_end->next = _rb_node_start;
    for(size_t i = 0; i < capacity - 1; ++i) {
      rb_node_t* _rb_node = __rb_slab(_rb_body, i, blocksz);
      _rb_node->next = __rb_slab(_rb_body, i + 1, blocksz);
    }
  }

  // build rb header
  _rb_header->rb_start = _rb_node_start;
  _rb_header->rb_end = _rb_node_start;
  _rb_header->init_rb_body = _rb_body;
  _rb_header->init_rb_body_size = capacity;
  _rb_header->size = 0;
  _rb_header->capacity = capacity;
  _rb_header->block_size = blocksz;

  if(rb) {
    *rb = _rb_header;
  }

  return frost_err_ok;
}

frost_errcode_t rb_put(rb_header_t* rb, void* data, size_t length) {

  if(!rb || !data) {
    return frost_err_invalid_parameter;
  }

  // write out of the block size is not allowed
  if(length > rb->block_size) {
    return frost_err_invalid_parameter;
  }

  // rb not full case
  if(rb->size < rb->capacity - 1) {

    // allocate erase and write
    void* _area = __rbnode_data_area(rb->rb_end); {
      rb->rb_end->size = length;
      memcpy(_area, data, length);
    }

    // update header
    rb->rb_end = rb->rb_end->next;
    rb->size++;
  }

  // rb is full (not considering insert)
  // todo: insert
  else {
    return frost_err_full;
  }

  return frost_err_ok;
}

frost_errcode_t rb_read(rb_header_t* rb, void* data, size_t* length, size_t* remain) {

  if(!rb) {
    return frost_err_invalid_parameter;
  }

  // get information
  if(!data) {

    if(length) {
      *length = rb->rb_start->size;
    }

    if(remain) {
      *remain = rb->size;
    }

    return frost_err_ok;
  }

  else if(data && length) {

    // read out of the block size is not allowed
    if(*length > rb->block_size) {
      return frost_err_invalid_parameter;
    }

    // rb is empty
    if(rb->size <= 0) {

      if(remain) {
        *remain = 0;
      }

      return frost_err_eof;
    }

    // do read data
    void* _ptr = __rbnode_data_area(rb->rb_start); {
      memcpy(data, _ptr, *length);
      *length = rb->rb_start->size;
    }

    // update header
    rb->rb_start = rb->rb_start->next;
    rb->size--;

    // get remain
    if(remain) {
      *remain = rb->size;
    }

    return frost_err_ok;
  }

  return frost_err_fatal_error;
}

frost_errcode_t rb_destroy(rb_header_t* rb) {

  if(!rb) {
    return frost_err_invalid_parameter;
  }

  rb->size = 0;
  rb->capacity = 0;
  free(rb->init_rb_body);
  free(rb);

  return frost_err_ok;
}
