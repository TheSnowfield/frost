#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "common.h"
#include "log.h"

#include "list.h"

frost_errcode_t list_create(list_ctx_t** ctx) {

  // validate parameters
  if(ctx == NULL)
    return frost_err_invalid_parameter;

  // initialize context
  list_ctx_t* _ctx = malloc(sizeof(list_ctx_t)); {
    if(_ctx == NULL)
      return frost_err_out_of_memory;

    // clear buffer
    memset(_ctx, 0, sizeof(list_ctx_t));
    *ctx = _ctx;
  }

  frost_log(TAG, "chain list created %p", _ctx);

  return frost_err_ok;
}

frost_errcode_t list_destroy(list_ctx_t* ctx) {
  if(ctx == NULL)
    return frost_err_invalid_parameter;

  // free all nodes
  list_node_t* _node = ctx->head;
  while(_node != NULL) {
    list_node_t* _next = _node->next;
    free(_node);
    _node = _next;
  }

  // free context
  free(ctx);
  frost_log(TAG, "chain list destroyed %p", ctx);

  return frost_err_ok;
}

frost_errcode_t list_put(list_ctx_t* ctx, void* data, size_t length, list_node_t** node) {
  if(ctx == NULL || data == NULL)
    return frost_err_invalid_parameter;

  if(ctx->size == SIZE_MAX)
    return frost_err_out_of_memory;

  // allocate an node + userdata length buffer
  // to reduce memory fragmentation
  size_t _length = sizeof(list_node_t) + length;
  list_node_t* _node = malloc(_length); {
    if(_node == NULL)
      return frost_err_out_of_memory;
  }

  // initialize node and copy data into
  memset(_node, 0, _length); {

    // setup next node pointer
    _node->next = NULL;

    // setup prev node pointer
    if(ctx->head == NULL) _node->prev = NULL;
    else _node->prev = ctx->tail;

    // copy data
    _node->data = (&_node->data) + 1;
    memcpy(_node->data, data, length);
  }

  // append node to list
  if(ctx->head == NULL) {
    ctx->head = _node;
    ctx->tail = _node;
  }
  else {
    ctx->tail->next = _node;
    ctx->tail = _node;
  }

  ++ctx->size;

  // return node pointer
  if(node != NULL) *node = _node;

  return frost_err_ok;
}

frost_errcode_t list_delete(list_ctx_t* ctx, list_node_t* node) {

  if(ctx == NULL || node == NULL)
    return frost_err_invalid_parameter;

  frost_log(TAG, "delete node %p", node);
  frost_log(TAG, "node prev %p, node next %p", node->prev, node->next);

  if(node->prev == NULL)
    ctx->head = node->next;
  else
    node->prev->next = node->next;

  if(node->next == NULL)
    ctx->tail = node->prev;
  else
    node->next->prev = node->prev;

  free(node);

  --ctx->size;

  return frost_err_ok;
}
