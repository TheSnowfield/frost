// SPDX-License-Identifier: MIT
/*******************************************************************************
 * This file is the part of the Frost library
 *
 * (C) Copyright 2025 TheSnowfield.
 *
 * Authors: TheSnowfield <17957399+TheSnowfield@users.noreply.github.com>
 ****************************************************************************/

/**
* @brief allocate channel
*
* @param task context
*/
frost_errcode_t frost_chan_alloc_ex(frost_task_ctx_t* task);
frost_errcode_t frost_chan_alloc();

/**
 * @brief bind channel from A to B (A -> B)
 * task A can access to channel B, however the reverse accessing is impossible
 *
 * @param task_a task A context
 * @param task_b task B context
 */
frost_errcode_t frost_chan_bind_ex(frost_task_ctx_t* task_a, frost_task_ctx_t* task_b);
frost_errcode_t frost_chan_bind(frost_task_ctx_t* task_b);

/**
 * @brief bind channel between A and B (A <-> B)
 * task A can access to channel B, allows reverse accessing (echo)
 *
 * @param task_a task A context
 * @param task_b task B context
 */
frost_errcode_t frost_chan_crossbind_ex(frost_task_ctx_t* task_a, frost_task_ctx_t* task_b);
frost_errcode_t frost_chan_crossbind(frost_task_ctx_t* task_b);

bool frost_chan_is_allocated_ex(frost_task_ctx_t* task);

typedef struct _chan_pack_t {
  uint32_t __ref_count;
  void* data;
  uint32_t data_len;
} chan_pack_t;

/**
 * @brief write channel pack
 *
 * @param task_b task B context, pass NULL to write the bound channels
 * @param pack the chan_pack_t pointer on the stack
 */
frost_errcode_t frost_chan_write_ex(frost_task_ctx_t* task_b, chan_pack_t* pack);

/**
 * @brief write channel <T>
 * @param T type
 */
#define frost_chan_write(T) \
  frost_chan_write_ex(NULL, &(chan_pack_t) { \
    .data = (uint8_t *)(T), \
    .data_len = sizeof(*(T)) \
  })

/**
 * @brief read channel pack
 *
 * @param pack 
 * @return frost_errcode_t 
 */
frost_errcode_t frost_chan_read(chan_pack_t** pack, frost_chanctl_t* ctrl);

frost_errcode_t frost_chan_destroy_ex(frost_task_ctx_t* task_a);


/**
 * @brief free a chan pack after read
 *
 * @param pack channel pack
 */
frost_errcode_t frost_chan_free_pack(chan_pack_t* pack);
