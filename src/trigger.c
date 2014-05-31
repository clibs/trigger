#include "trigger.h"

struct _trigger_event_trigger_th_proxy_object {
  void (* cease_fire)(
    trigger_event_handle_t *
  );

  void * data;
  const char * event;
  trigger_event_handle_t * event_handle;
  trigger_event_handler_t handler;
};

// +-------+---------------------+
// | BEGIN | static declarations |
// +-------+---------------------+

static void
trigger_event_trigger_cease_fire(
  trigger_event_handle_t *
);

static void
trigger_event_trigger_seq(
  trigger_event_handle_t *,
  trigger_event_handle_ht_t *,
  const char *,
  void *
);

static void
trigger_event_trigger_th(
  trigger_event_handle_t *,
  trigger_event_handle_ht_t *,
  const char *,
  void *
);

static void
trigger_event_trigger_th_proxy(
  struct _trigger_event_trigger_th_proxy_object *
);

// +-----+---------------------+
// | END | static declarations |
// +-----+---------------------+

// +-------+------------------+
// | BEGIN | core definitions |
// +-------+------------------+

void
trigger_delete(
  trigger_event_handle_t * event_handle
) {
  trigger_event_handle_ht_t * ht = event_handle->ht;
  trigger_event_handle_ht_t * other_ht;
  trigger_event_handle_ht_t * other_ht_tmp;

  HASH_ITER(
    hh,
    ht,
    other_ht,
    other_ht_tmp
  ) {
    list_destroy(other_ht->event_handlers);
    HASH_DEL(ht, other_ht);
    _FREE(other_ht);
  }

  _FREE(event_handle);
}

void
trigger_event_on(
  trigger_event_handle_t * event_handle,
  const char * event,
  trigger_event_handler_t event_handler
) {
  trigger_event_handle_ht_t * ht = event_handle->ht;
  trigger_event_handle_ht_t * other_ht;

  HASH_FIND_STR(
    ht,
    event,
    other_ht
  );

  if (! other_ht) {
    other_ht = _MALLOC(trigger_event_handle_ht_t, 1);

    if (! other_ht) { exit(-1); }

    other_ht->event          = event;
    other_ht->event_handlers = list_new();

    HASH_ADD_KEYPTR(
      hh,
      ht,
      other_ht->event,
      strlen(other_ht->event),
      other_ht
    );

    event_handle->ht = ht;
  }

  list_rpush(
    other_ht->event_handlers,
    list_node_new(event_handler)
  );
}

void
trigger_event_trigger(
  trigger_event_handle_t * event_handle,
  const char * event,
  void * data
) {
  trigger_event_handle_ht_t * ht = event_handle->ht;
  trigger_event_handle_ht_t * other_ht;

  HASH_FIND_STR(
    ht,
    event,
    other_ht
  );

  if (other_ht) {
    switch (event_handle->firing_type) {
      case TRIGGER_FIRING_TYPE_CONCURRENT:
        trigger_event_trigger_th(event_handle, other_ht, event, data);

        break;

      case TRIGGER_FIRING_TYPE_SEQUENTIAL:
        trigger_event_trigger_seq(event_handle, other_ht, event, data);

        break;
    }
  }
}

trigger_event_handle_t *
trigger_new(
  trigger_firing_type_t firing_type
) {
  trigger_event_handle_t * event_handle = _MALLOC(trigger_event_handle_t, 1);

  if (! event_handle) { exit(-1); }

  event_handle->_cease_fire = false;
  event_handle->firing_type = firing_type;
  event_handle->ht          = NULL;

  return event_handle;
}

// +-----+------------------+
// | END | core definitions |
// +-----+------------------+

// +-------+--------------------+
// | BEGIN | static definitions |
// +-------+--------------------+

static void
trigger_event_trigger_cease_fire(
  trigger_event_handle_t * event_handle
) {
  event_handle->_cease_fire = true;
}

static void
trigger_event_trigger_seq(
  trigger_event_handle_t * event_handle,
  trigger_event_handle_ht_t * ht,
  const char * event,
  void * data
) {
  list_iterator_t * ht_event_handlers_it = list_iterator_new(ht->event_handlers, LIST_HEAD);
  list_node_t * ht_event_handlers_it_node;

  while (
    (ht_event_handlers_it_node = list_iterator_next(ht_event_handlers_it)) &&
    (! event_handle->_cease_fire)
  ) {
    ((trigger_event_handler_t) ht_event_handlers_it_node->val)(
      data,
      event,
      event_handle,
      trigger_event_trigger_cease_fire
    );
  }

  list_iterator_destroy(ht_event_handlers_it);
}

static void
trigger_event_trigger_th(
  trigger_event_handle_t * event_handle,
  trigger_event_handle_ht_t * ht,
  const char * event,
  void * data
) {
  pthread_t * trigger_th = NULL;
  pthread_t * trigger_th_tmp;
  struct _trigger_event_trigger_th_proxy_object * trigger_th_proxy = NULL;
  struct _trigger_event_trigger_th_proxy_object * trigger_th_proxy_tmp;
  unsigned int trigger_th_count = 0;
  unsigned int trigger_th_count_tmp;

  int trigger_th_err;

  list_iterator_t * ht_event_handlers_it = list_iterator_new(ht->event_handlers, LIST_HEAD);
  list_node_t * ht_event_handlers_it_node;

  while (
    ht_event_handlers_it_node = list_iterator_next(ht_event_handlers_it)
  ) {
    trigger_th_count_tmp = trigger_th_count;

    trigger_th_count++;

    trigger_th_tmp = _REALLOC(trigger_th, pthread_t, trigger_th_count);

    if (! trigger_th_tmp) { exit(-1); }

    trigger_th = trigger_th_tmp;

    trigger_th_proxy_tmp = _REALLOC(trigger_th_proxy, struct _trigger_event_trigger_th_proxy_object, trigger_th_count);

    if (! trigger_th_proxy_tmp) { exit(-1); }

    trigger_th_proxy = trigger_th_proxy_tmp;

    trigger_th_proxy[trigger_th_count_tmp].event        = event;
    trigger_th_proxy[trigger_th_count_tmp].data         = data;
    trigger_th_proxy[trigger_th_count_tmp].cease_fire   = NULL;
    trigger_th_proxy[trigger_th_count_tmp].event_handle = event_handle;
    trigger_th_proxy[trigger_th_count_tmp].handler      = ht_event_handlers_it_node->val;

    trigger_th_err = pthread_create(
      &trigger_th[trigger_th_count_tmp],
      NULL,
      trigger_event_trigger_th_proxy,
      &trigger_th_proxy[trigger_th_count_tmp]
    );

    if (trigger_th_err) { exit(-1); }
  }

  list_iterator_destroy(ht_event_handlers_it);

  // TODO: Use pthread barriers instead.
  for (unsigned int a = 0; a < trigger_th_count; a++) {
    trigger_th_err = pthread_join(trigger_th[a], NULL);

    if (trigger_th_err) { exit(-1); }
  }

  _FREE(trigger_th_proxy);
  _FREE(trigger_th);
}

static void
trigger_event_trigger_th_proxy(
  struct _trigger_event_trigger_th_proxy_object * proxy
) {
  proxy->handler(
    proxy->data,
    proxy->event,
    proxy->event_handle,
    proxy->cease_fire
  );
}

// +-----+--------------------+
// | END | static definitions |
// +-----+--------------------+
