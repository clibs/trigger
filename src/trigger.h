/**
 * The MIT License (MIT).
 *
 * https://github.com/clibs/trigger
 *
 * Copyright (c) 2014 clibs, Jonathan Barronville (jonathan@scrapum.photos), and contributors.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __TRIGGER_H__INCLUDED__
#define __TRIGGER_H__INCLUDED__

#ifdef __cplusplus
#include <cstdbool>
#include <cstdlib>
#include <cstring>

using namespace std;
#else
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <pthread.h>

#include "list/list.h"
#include "uthash/uthash.h"

#if                         \
  defined(__GNUC__) &&      \
  (                         \
    (__GNUC__ > 2) ||       \
    (                       \
      (__GNUC__ == 2) &&    \
      (__GNUC_MINOR__ >= 7) \
    )                       \
  )
#define _UNUSED_VAR __attribute__ ((unused))
#else
#define _UNUSED_VAR
#endif

#ifdef __cplusplus
#define _CALLOC(                         \
    type,                                \
    count                                \
  )                                      \
  ((type *) calloc(count, sizeof(type)))

#define _MALLOC(                          \
    type,                                 \
    count                                 \
  )                                       \
  ((type *) malloc(sizeof(type) * count))

#define _REALLOC(                                 \
    ptr,                                          \
    type,                                         \
    count                                         \
  )                                               \
  ((type *) realloc(ptr, (sizeof(type) * count)))
#else
#define _CALLOC(                \
    type,                       \
    count                       \
  )                             \
  (calloc(count, sizeof(type)))

#define _MALLOC(                 \
    type,                        \
    count                        \
  )                              \
  (malloc(sizeof(type) * count))

#define _REALLOC(                        \
    ptr,                                 \
    type,                                \
    count                                \
  )                                      \
  (realloc(ptr, (sizeof(type) * count)))
#endif

#define _FREE free

#if                         \
  defined(__GNUC__) &&      \
  (                         \
    (__GNUC__ > 3) ||       \
    (                       \
      (__GNUC__ == 3) &&    \
      (__GNUC_MINOR__ >= 3) \
    )                       \
  )
#define TRIGGER_ABI_EXPORT __attribute__ ((visibility ("default")))
#define TRIGGER_ABI_HIDDEN __attribute__ ((visibility ("hidden")))
#else
#define TRIGGER_ABI_EXPORT
#define TRIGGER_ABI_HIDDEN
#endif

#ifdef __cplusplus
extern "C" {
#endif

// +-------+------------------+
// | BEGIN | type definitions |
// +-------+------------------+

typedef enum {
  TRIGGER_FIRING_TYPE_CONCURRENT,
  TRIGGER_FIRING_TYPE_SEQUENTIAL
} trigger_firing_type_t;

typedef struct _trigger_event_handle trigger_event_handle_t;
typedef struct _trigger_event_handle_ht trigger_event_handle_ht_t;

typedef void (* trigger_event_handler_t)(
  void *,
  const char *,
  trigger_event_handle_t *,
  void (*)(trigger_event_handle_t *)
);

// +-----+------------------+
// | END | type definitions |
// +-----+------------------+

// +-------+----------+
// | BEGIN | core api |
// +-------+----------+

TRIGGER_ABI_EXPORT void
trigger_delete(
  trigger_event_handle_t *
);

TRIGGER_ABI_EXPORT void
trigger_event_on(
  trigger_event_handle_t *,
  const char *,
  trigger_event_handler_t
);

TRIGGER_ABI_EXPORT void
trigger_event_trigger(
  trigger_event_handle_t *,
  const char *,
  void *
);

TRIGGER_ABI_EXPORT trigger_event_handle_t *
trigger_new(
  trigger_firing_type_t
);

// +-----+----------+
// | END | core api |
// +-----+----------+

// +-------+--------------------+
// | BEGIN | struct definitions |
// +-------+--------------------+

struct _trigger_event_handle {
  bool _cease_fire;
  trigger_firing_type_t firing_type;
  trigger_event_handle_ht_t * ht;
};

struct _trigger_event_handle_ht {
  void * data;
  const char * event;
  list_t * event_handlers;
  UT_hash_handle hh;
};

// +-----+--------------------+
// | END | struct definitions |
// +-----+--------------------+

// +-------+-----------+
// | BEGIN | utilities |
// +-------+-----------+

#define TRIGGER_DEFINE_EVENT_HANDLER(               \
    base_name,                                      \
    event_,                                         \
    func_body                                       \
  )                                                 \
    static void                                     \
    TRIGGER_EVENT_HANDLER_FOR(base_name, event_)(   \
      void * data,                                  \
      const char * event,                           \
      trigger_event_handle_t * event_handle,        \
      void (* cease_fire)(trigger_event_handle_t *) \
    ) func_body

#define TRIGGER_EVENT_HANDLER_FOR( \
    base_name,                     \
    event                          \
  ) handle_##base_name##_##event

// +-----+-----------+
// | END | utilities |
// +-----+-----------+

#ifdef __cplusplus
}
#endif

#endif
