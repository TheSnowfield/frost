#ifndef _FROST_AWAIT_H
#define _FROST_AWAIT_H

frost_task_awaiter_t* awaiter_create();
frost_task_awaiter_t* awaiter_from_value(frost_handle_t value, frost_errcode_t status);
frost_errcode_t awaiter_destroy(frost_task_awaiter_t* awaiter);
frost_task_awaiter_t* awaiter_await(frost_task_awaiter_t* awaiter);
frost_errcode_t awaiter_finish(frost_task_awaiter_t* awaiter, frost_handle_t result);
frost_errcode_t awaiter_cancel(frost_task_awaiter_t* awaiter);

#endif /* _FROST_AWAIT_H */
