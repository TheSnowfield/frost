#include <stdio.h>

#define FROST_PROTED_TICK_TIME
#define FROST_PROTED_LOG_PRINT
#include <frost/api.h>

void __callback_test() {
  printf("=w=");
}

int main() {

  frost_init();
  frost_task_interval(&__callback_test, 1000, NULL);

  // initialization okay, run tasks
  frost_errcode_t _result = frost_err_ok;
  while (frost_ok(_result = frost_schedule_tasks())) {
    // schedule the tasks
  }

}