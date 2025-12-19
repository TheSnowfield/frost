// SPDX-License-Identifier: MIT
/*******************************************************************************
 * This file is the part of the Frost library
 *
 * (C) Copyright 2025 TheSnowfield.
 *
 * Authors: TheSnowfield <17957399+TheSnowfield@users.noreply.github.com>
 ****************************************************************************/

#include "engine.h"
#include "utils.h"
#include "chan.h"

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
    memset(_retained, 0, _length);
  }

  // make a copy
  _retained->__ref_count = 0;
  _retained->data = (uint8_t*)(_retained + 1);
  _retained->data_len = stack->data_len;
  _retained->ctrl = stack->ctrl;
  _retained->from = stack->from;
  if(stack->data_len != 0) {
    memcpy(_retained->data, stack->data, stack->data_len);
  }

  *retained = _retained;

  return frost_err_ok;
}

/**
 * MARK: __chan_pack_free
 * @brief free pack
 * @param retained the retained copy
 */
static void __chan_pack_free(chan_pack_t* retained) {
  if(retained) free(retained);
}

/**
* MARK: frost_chan_alloc_ex
* @brief allocate channel
*
* @param task context
*/
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
  rb_header_t* _rb_header = NULL;
  if(!frost_ok(rb_create(FROST_CHAN_RINGBUFF_SIZE, sizeof(chan_pack_t*), &_rb_header))) {
    free(_chan);
    return frost_err_out_of_memory;
  }

  // initialize chan parameters
  task->chan.ref = _chan; {
    _chan->notify_cnt = 0;
    _chan->header = _rb_header;
  }

  frost_log(TAG, "chan rb[%p] has allocated for task '%s'[%p]", _rb_header, task->name, task);

  return frost_err_ok;
}

/**
 * MARK: frost_chan_bind_ex
 * @brief bind channel from A to B (A -> B)
 * task A can access to channel B, however the reverse accessing is impossible
 *
 * @param task_a task A context
 * @param task_b task B context
 */
frost_errcode_t frost_chan_bind_ex(frost_task_ctx_t* task_a, frost_task_ctx_t* task_b) {

  // if not bound yet
  if(!task_a->chan.bind) {
    if(!frost_ok(list_create(&task_a->chan.bind))) {
      return frost_err_out_of_memory;
    }
  }

  // add to task bind list
  if(!frost_ok(list_put(task_a->chan.bind, (void*)&task_b, sizeof(task_b), NULL))) {
    return frost_err_out_of_memory;
  }

  return frost_err_ok;
}

/**
 * MARK: frost_chan_crossbind_ex
 * @brief bind channel between A and B (A <-> B)
 * task A can access to channel B, allows reverse accessing (echo)
 *
 * @param task_a task A context
 * @param task_b task B context
 */
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

/**
 * MARK: frost_chan_write_ex
 * @brief write channel pack
 *
 * @param task_b task B context, pass NULL to write the bound channels
 * @param pack the chan_pack_t pointer on the stack
 */
frost_errcode_t frost_chan_write_ex(frost_task_ctx_t* task_b, chan_pack_t* pack) {

  if(pack == NULL) {
    return frost_err_invalid_parameter;
  }
  
  frost_task_ctx_t* _task_a = __get_task_ctx(NULL);
  frost_task_ctx_t* _task_b = task_b; {
    
    // if task B is NULL, try get the channel already bound
    if(!_task_b) {

      // task A and task B both invalid, return error
      // the case of invalid task A is the call from outside of the frost context
      if(!_task_a || !_task_a->chan.bind) {
        frost_log(TAG, "task[%p] intented to write a invalid chan", _task_a);
        return frost_err_invalid_chan;
      }

      // retain the message pack on the heap
      chan_pack_t* _retained_pack = NULL;
      if(!frost_ok(__chan_pack_retain(pack, &_retained_pack))) {
        frost_log(TAG, "task[%p] out of memory when retain a chanpack", _task_a);
        return frost_err_out_of_memory;
      }

      if(!_retained_pack->from) {
        _retained_pack->from = _task_a;
      }

      //    /--> B
      // A -+--> C
      //    \--> D

      // to write messages
      int32_t _ref_count = 0;
      list_node_t* _node = _task_a->chan.bind->head;
      while(_node) {
        frost_task_ctx_t* _to_post = *(frost_task_ctx_t **)_node->data; {

          if(frost_ok(rb_put(_to_post->chan.ref->header, (void*)&_retained_pack, sizeof(chan_pack_t*)))) {
            ++_to_post->chan.ref->notify_cnt;
            ++_ref_count;
            frost_log(TAG, "chanpak[%p]: write flow '%s' -> '%s'", _retained_pack, _task_a->name, _to_post->name);
          }

          else {
            frost_log(TAG, "task[%p] rb_put failed... consider out of memory? or full", _to_post);
          }
        }
        _node = _node->next;
      }

      // if no task handle this chanpack
      if (_ref_count == 0) {
        __chan_pack_free(_retained_pack);
        return frost_err_full;
      }

      _retained_pack->__ref_count = _ref_count;
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

      if(!_retained_pack->from) {
        _retained_pack->from = _task_a;
      }

      if(frost_ok(rb_put(_task_b->chan.ref->header, (void*)&_retained_pack, sizeof(chan_pack_t*)))) {
        ++_task_b->chan.ref->notify_cnt;
        ++_retained_pack->__ref_count;
      }
      else {
        __chan_pack_free(_retained_pack);
        frost_log(TAG, "rb_put failed... consider out of memory? consider chan is full");
        return frost_err_full;
      }
    }
  }

  return frost_err_ok;
}

/**
 * MARK: frost_chan_read
 * @brief read channel pack
 *
 * @param pack the chan_pack_t pointer on the stack
 */
frost_errcode_t frost_chan_read(chan_pack_t** pack, frost_chanctl_t* ctrl) {

  frost_task_ctx_t* _task_a = __get_task_ctx(NULL);
  if(!_task_a || !_task_a->chan.ref) {
    return frost_err_invalid_chan;
  }

  // no message came in
  if(_task_a->chan.ref->notify_cnt == 0) {
    if(pack != NULL) *pack = NULL;
    if(ctrl != NULL) *ctrl = frost_chanctl_ok;
    return frost_err_eof;
  }

  size_t _length = 0;
  size_t _remain = 0;
  chan_pack_t* _pack = NULL;

  // get ring buffer info
  if(!frost_ok(rb_read(_task_a->chan.ref->header, NULL, &_length, &_remain))) {
    // todo print something
    if(pack != NULL) *pack = NULL;
    return frost_err_ok;
  }

  // get ring buffer data
  if(!frost_ok(rb_read(_task_a->chan.ref->header, (void*)&_pack, &_length, &_remain))) {
    if(pack != NULL) *pack = NULL;
    return frost_err_fatal_error;
  }

  --_task_a->chan.ref->notify_cnt;

  // if the ref count is alrady 0, wtf?
  if(_pack->__ref_count <= 0) {
    frost_log(TAG, "task[%p] chanpak[%p]: warning __ref_count = %d", _task_a, _pack, _pack->__ref_count);
    return frost_err_fatal_error;
  }

  frost_log(TAG, "chanpak[%p]: read flow '%s' read from channel, __ref_count = %d", _pack, _task_a->name, _pack->__ref_count);

  // when channel close
  // remove bind from bind list
  if(_pack->ctrl == frost_chanctl_close) {

    if(_task_a->chan.bind == NULL) {
      goto out;
    }

    // TODO: move to chan_unbind function
    list_node_t* _node = _task_a->chan.bind->head;
    while(_node) {

      frost_task_ctx_t* _task_b; {
        _task_b = *(frost_task_ctx_t **)_node->data;
      }

      // remove dead channel
      if(_task_b == _pack->from) {
        list_delete(_task_a->chan.bind, _node);
        frost_log(TAG, "task[%p] unbinding with channel task[%p]", _task_a, _task_b);
        break;
      }

      _node = _node->next;
    }
  }

out:
  if(pack) *pack = _pack;
  if(ctrl) *ctrl = _pack->ctrl;

  return frost_err_ok;
}

frost_errcode_t frost_chan_free_pack(chan_pack_t* pack) {
  if(!pack) {
    return frost_err_invalid_parameter;
  }

  // destroy pack when end of pack lifetime
  if(--pack->__ref_count <= 0) {
    __chan_pack_free(pack);
    frost_log(TAG, "chanpak[%p]: destroyed", pack);
  }

  return frost_err_ok;
}

frost_errcode_t frost_chan_destroy_ex(frost_task_ctx_t* task_a) {

  if(!task_a->chan.ref) {
    return frost_err_invalid_parameter;
  }

  // notify unbind channels
  frost_chan_write_ex(NULL, &(chan_pack_t) {
    .ctrl = frost_chanctl_close,
    .from = task_a,
    .data_len = 0,
    .data = NULL
  });

  // unref all channel packs of the ringbuffer
  if(task_a->chan.ref->notify_cnt != 0) {
    chan_pack_t* _pack = NULL;
    while(frost_chan_read(&_pack, NULL) != frost_err_eof) {
      frost_chan_free_pack(_pack);
    }
  }

  // do destroy & cleanup
  list_destroy(task_a->chan.bind);
  rb_destroy(task_a->chan.ref->header);
  free(task_a->chan.ref); {
    task_a->chan.ref = NULL;
    task_a->chan.bind = NULL;
  }

  return frost_err_ok;
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
