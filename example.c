/**
 * Note:
 * To compile, simply run `make example`.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "trigger/trigger.h"

#define _SLEEP_WITHIN_2_SECS (sleep(rand() % 2));

TRIGGER_DEFINE_EVENT_HANDLER(main_concurrent_1, BLAH, {
  _SLEEP_WITHIN_2_SECS
  printf("-------------------------\nI am event handler #1!\n-------------------------\n\n");
})

TRIGGER_DEFINE_EVENT_HANDLER(main_concurrent_2, BLAH, {
  _SLEEP_WITHIN_2_SECS
  printf("-------------------------\nI am event handler #2!\n-------------------------\n\n");
})

TRIGGER_DEFINE_EVENT_HANDLER(main_concurrent_3, BLAH, {
  _SLEEP_WITHIN_2_SECS
  printf("-------------------------\nI am event handler #3!\n-------------------------\n\n");
})

TRIGGER_DEFINE_EVENT_HANDLER(main_concurrent_4, BLAH, {
  _SLEEP_WITHIN_2_SECS
  printf("-------------------------\nI am event handler #4!\n-------------------------\n\n");
})

TRIGGER_DEFINE_EVENT_HANDLER(main_concurrent_5, BLAH, {
  _SLEEP_WITHIN_2_SECS
  printf("-------------------------\nI am event handler #5!\n-------------------------\n\n");
})

#define _MAIN_SEQUENTIAL_BODY                                                      \
  {                                                                                \
    _SLEEP_WITHIN_2_SECS                                                           \
                                                                                   \
    const char * _data = data;                                                     \
                                                                                   \
    printf("[sequential] Event \"%s\" fired with data: %s.\n", event, _data);      \
                                                                                   \
    if (strcmp(_data, "Hello!") == 0) {                                            \
      printf("Data \"Hello!\" found ... no more event handlers will be fired.\n"); \
      cease_fire(event_handle);                                                    \
    }                                                                              \
  }

TRIGGER_DEFINE_EVENT_HANDLER(main_sequential_1, BLAH, _MAIN_SEQUENTIAL_BODY)
TRIGGER_DEFINE_EVENT_HANDLER(main_sequential_2, BLAH, _MAIN_SEQUENTIAL_BODY)

int
main(void) {
  trigger_event_handle_t * test_handle_concurrent = trigger_new(TRIGGER_FIRING_TYPE_CONCURRENT);
  trigger_event_handle_t * test_handle_sequential = trigger_new(TRIGGER_FIRING_TYPE_SEQUENTIAL);

  trigger_event_on(
    test_handle_concurrent,
    "BLAH",
    TRIGGER_EVENT_HANDLER_FOR(main_concurrent_1, BLAH)
  );

  trigger_event_on(
    test_handle_concurrent,
    "BLAH",
    TRIGGER_EVENT_HANDLER_FOR(main_concurrent_2, BLAH)
  );

  trigger_event_on(
    test_handle_concurrent,
    "BLAH",
    TRIGGER_EVENT_HANDLER_FOR(main_concurrent_3, BLAH)
  );

  trigger_event_on(
    test_handle_concurrent,
    "BLAH",
    TRIGGER_EVENT_HANDLER_FOR(main_concurrent_4, BLAH)
  );

  trigger_event_on(
    test_handle_concurrent,
    "BLAH",
    TRIGGER_EVENT_HANDLER_FOR(main_concurrent_5, BLAH)
  );

  trigger_event_on(
    test_handle_sequential,
    "BLAH",
    TRIGGER_EVENT_HANDLER_FOR(main_sequential_1, BLAH)
  );

  trigger_event_on(
    test_handle_sequential,
    "BLAH",
    TRIGGER_EVENT_HANDLER_FOR(main_sequential_2, BLAH)
  );

  trigger_event_trigger(test_handle_concurrent, "BLAH", NULL);
  trigger_event_trigger(test_handle_sequential, "BLAH", "Hello!");

  trigger_delete(test_handle_concurrent);
  trigger_delete(test_handle_sequential);

  return 0;
}
