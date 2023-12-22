#ifndef _FROST_TRICKS_H
#define _FROST_TRICKS_H

/**
 * @brief push a new task to engine and return.
 * the new task will be executed in next schedule cycle.
 */
#define run_async frost_task_run_ex

/**
 * @brief await
 */
#define await awaiter_await

/**
 * @brief yield
 */
#define yield engine_schedule_tasks()

#endif /* _SYS_TASK_TRICKS_H */
