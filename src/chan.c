// SPDX-License-Identifier: MIT
/*******************************************************************************
 * This file is the part of the Frost library
 *
 * (C) Copyright 2025 TheSnowfield.
 *
 * Authors: TheSnowfield <17957399+TheSnowfield@users.noreply.github.com>
 ****************************************************************************/

#include "engine.h"
#include "chan.h"
#include "utils.h"

/**
 * MARK: __chan_pack_retain
 * @brief retain a chanel message pack
 * this function can make a message pack copy to to heap memory
 *
 * @param stack the message pack located on the stack (or anywhere have a chance being overwritten)
 * @param retained the retained copy
 */
static frost_errcode_t __chan_pack_retain(chan_pack_t* stack, chan_pack_t** retained) {

  size_t _length = sizeof(chan_pack_t) + stack->data_len;
  chan_pack_t* _retained = malloc(_length); {
    if(!_retained) return frost_err_out_of_memory;
    memset(_retained, 0, sizeof(_length));
  }

  // make a copy
  _retained->__ref_count = 0;
  _retained->data = (uint8_t*)_retained++;
  _retained->data_len = stack->data_len;
  memcpy(_retained->data, stack->data, stack->data_len);

  return frost_err_ok;
}

frost_errcode_t frost_chan_alloc_ex(frost_task_ctx_t* task) {

  if(task->chan.ref != NULL) {
    return frost_err_invalid_parameter;
  }

  // prepare chan instance
  frost_chan_t* _chan = (frost_chan_t *)malloc(sizeof(frost_chan_t)); {
    if(!_chan) return frost_err_out_of_memory;
    memset(_chan, 0, sizeof(*_chan));
  }

  // create ring buffer
  list_node_t* _buffer = NULL;
  if(!frost_ok(list_create_ring(&_buffer))) {
    free(_chan);
    return frost_err_out_of_memory;
  }

  // initialize chan parameters
  task->chan.ref = _chan; {
    _chan->notify_cnt = 0;
    _chan->circular = _buffer;
  }

  return frost_err_ok;
}

frost_errcode_t frost_chan_bind_ex(frost_task_ctx_t* task_a, frost_task_ctx_t* task_b) {

  

  
}

frost_errcode_t frost_chan_crossbind_ex(frost_task_ctx_t* task_a, frost_task_ctx_t* task_b) {
  
  // bind A -> B
  frost_errcode_t _ret_a = frost_chan_bind_ex(task_a, task_b);
  if(!frost_ok(_ret_a)) return _ret_a;

  // bind B -> A
  frost_errcode_t _ret_b = frost_chan_bind_ex(task_b, task_a);
  if(!frost_ok(_ret_b)) return _ret_b;

  return frost_err_ok;
}

bool frost_chan_is_allocated_ex(frost_task_ctx_t* task) {
  return task->chan.ref != NULL;
}

frost_errcode_t frost_chan_write_ex(frost_task_ctx_t* task_b, chan_pack_t* pack) {
  
  frost_task_ctx_t* _task_a = __get_task_ctx(NULL);
  frost_task_ctx_t* _task_b = task_b; {
    
    // if task B is NULL, try get the channel already bound
    if(!_task_b) {

      // task A and task B both invalid, return error
      // the case of invalid task A is the call from outside of the frost context
      if(!_task_a && !_task_a->chan.bind) {
        return frost_err_invalid_chan;
      }

      // retain the message pack on the heap
      chan_pack_t* _retained_pack = NULL;
      if(!frost_ok(__chan_pack_retain(pack, &_retained_pack))) {
        return frost_err_out_of_memory;
      }

      //    /--> B
      // A -+--> C
      //    \--> D

      // to write messages
      list_node_t* _node = _task_a->chan.bind->head;
      while(_node) {
        frost_task_ctx_t* _to_post = (frost_task_ctx_t *)_node->data; {
          list_put(_to_post->chan.ref->circular, _retained_pack, sizeof(chan_pack_t), NULL);
          ++_to_post->chan.ref->notify_cnt;
          ++_retained_pack->__ref_count;
        }
        _node = _node->next;
      }
    }

    // if task B is not contains a chan
    else if(!_task_b->chan.ref) {
      return frost_err_invalid_chan;
    }

    // A --> B
    // write to B channel
    else {

      // retain the message pack on the heap
      chan_pack_t* _retained_pack = NULL;
      if(!frost_ok(__chan_pack_retain(pack, &_retained_pack))) {
        return frost_err_out_of_memory;
      }

      list_put(_task_b->chan.ref->circular, _retained_pack, sizeof(chan_pack_t), NULL);
      ++_task_b->chan.ref->notify_cnt;
      ++_retained_pack->__ref_count;
    }
  }

  return frost_err_ok;
}

frost_errcode_t frost_chan_read(chan_pack_t** pack) {

  frost_task_ctx_t* _task_a = __get_task_ctx(NULL);
  if(!_task_a || !_task_a->chan.ref) {
    return frost_err_invalid_chan;
  }

  // no message came in
  if(_task_a->chan.ref->notify_cnt == 0) {
    if(pack != NULL) *pack = NULL;
    return frost_err_ok;
  }

  list_ctx_t* _ctx = _task_a->chan.ref->circular;


}

frost_errcode_t frost_chan_destroy_ex(frost_task_ctx_t* task_a) {

}

frost_errcode_t frost_chan_alloc() {
  return frost_chan_alloc_ex(NULL);
}

frost_errcode_t frost_chan_bind(frost_task_ctx_t* task_b) {
  return frost_chan_bind_ex(NULL, task_b);
}

frost_errcode_t frost_chan_crossbind(frost_task_ctx_t* task_b) {
  return frost_chan_crossbind_ex(NULL, task_b);
}

bool frost_chan_is_allocated(frost_task_ctx_t* task) {
  return (__get_task_ctx(NULL))->chan.ref != NULL;
}
