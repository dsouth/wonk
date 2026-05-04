#include <wayland-server-core.h>
#include <wayland-server.h>
#include <wayland-util.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_keyboard.h>
#include <cpp/jank/c_api.h>
#include <stdlib.h>

#include "../include/callbacks.h"

jank_object_ref *output_new_callback = NULL;
jank_object_ref *output_destroy_callback = NULL;
jank_object_ref *output_frame_callback = NULL;
jank_object_ref *output_request_state_callback = NULL;
jank_object_ref *input_new_callback = NULL;
jank_object_ref *keyboard_key_callback = NULL;
jank_object_ref *keyboard_modifier_callback = NULL;
jank_object_ref *keyboard_destroy_callback = NULL;

void eval_callback(jank_object_ref **ref, const char* name) {
  if (*ref == NULL) {
    *ref = jank_eval(jank_read_string_c(name));
  }
  jank_call0(*ref);
}

void eval_callback1(jank_object_ref **ref, const char* name, jank_object_ref arg) {
  if (*ref == NULL) {
    *ref = jank_eval(jank_read_string_c(name));
  }
  jank_call1(*ref, arg);
}

void eval_callback2(jank_object_ref **ref, const char* name, 
                    jank_object_ref arg1, jank_object_ref arg2) {
  if (*ref == NULL) {
    *ref = jank_eval(jank_read_string_c(name));
  }
  jank_call2(*ref, arg1, arg2);
}

void output_new(struct wl_listener *listener, void *data) {
  struct wlr_output *output = data;
  jank_object_ref box_output = jank_box("wlr_output*", output);
  eval_callback1(&output_new_callback, "#'wonk.output/new-callback", box_output);
}
 
void input_new(struct wl_listener *listener, void *data) {
  struct wlr_input_device *device = data;
  jank_object_ref box_data = jank_box("wlr_input_device*", device);
  eval_callback1(&input_new_callback, "#'wonk.input/new-callback", box_data);
}
 
struct listeners* initialize_listeners(uint count) {
  struct listeners *listeners = calloc(1, sizeof(struct listeners));
  listeners->length = count;
  listeners->listeners = calloc(listeners->length, sizeof(struct wl_listener));
  return listeners;
 }

struct listeners* wire_backend_listeners(struct wlr_backend *backend) {
  struct listeners *listeners = initialize_listeners(2);
  listeners->listeners[0].notify = output_new;
  wl_signal_add(&backend->events.new_output, &listeners->listeners[0]);
  listeners->listeners[1].notify = input_new;
  wl_signal_add(&backend->events.new_input, &listeners->listeners[1]);
  return listeners;
}

void output_destroy(struct wl_listener *listener, void *data) {
  struct wlr_output *output = data;;
  jank_object_ref box_output = jank_box("wlr_output*", output);
  eval_callback1(&output_destroy_callback, "#'wonk.output/destroy-callback", box_output);
}

void output_frame(struct wl_listener *listener, void *data) {
  eval_callback(&output_frame_callback, "#'wonk.output/frame-callback");
}

void output_request_state(struct wl_listener *listener, void *data) {
  eval_callback(&output_request_state_callback, "#'wonk.output/request-state-callback");
}

struct listeners* wire_output_listeners(struct wlr_output *output) {
  struct listeners *listeners = initialize_listeners(3);
  listeners->listeners[0].notify = output_destroy;
  wl_signal_add(&output->events.destroy, &listeners->listeners[0]);
  listeners->listeners[1].notify = output_frame;
  wl_signal_add(&output->events.frame, &listeners->listeners[1]);
  listeners->listeners[2].notify = output_request_state;
  wl_signal_add(&output->events.request_state, &listeners->listeners[2]);
  return listeners;
}

void keyboard_key(struct wl_listener *listener, void *data) {
  struct wlr_keyboard_key_event *event = data;
  jank_object_ref box_data = jank_box("wlr_keyboard_key_event*", event);
  jank_object_ref listener_box = jank_box("wl_listener*", listener);
  eval_callback2(&keyboard_key_callback, "#'wonk.input/key-press", box_data, listener_box);
}

void keyboard_modifier(struct wl_listener *listener, void *data) {
  struct wlr_keyboard_key_event *event = data;
  jank_object_ref box_data = jank_box("wlr_keyboard_key_event*", event);
  eval_callback1(&keyboard_key_callback, "#'wonk.input/modifier-press", box_data);
}

void keyboard_destroy(struct wl_listener *listener, void *data) {
  struct wlr_keyboard *keyboard = data;
  jank_object_ref box_data = jank_box("wlr_keyboard*", keyboard);
  eval_callback1(&keyboard_key_callback, "#'wonk.input/keyboard-destroy", box_data);
}

struct listeners * wire_keyboard_listeners(struct wlr_input_device *device,
                                           struct wlr_keyboard *keyboard) {
  struct listeners *listeners = initialize_listeners(3);
  listeners->listeners[0].notify = keyboard_key;
  wl_signal_add(&keyboard->events.key, &listeners->listeners[0]);
  listeners->listeners[1].notify = keyboard_modifier;
  wl_signal_add(&keyboard->events.modifiers, &listeners->listeners[1]);
  listeners->listeners[2].notify = keyboard_destroy;
  wl_signal_add(&device->events.destroy, &listeners->listeners[2]);
  return listeners;
}

void cleanup_listeners(struct listeners *listeners) {
  for (int i = 0; i < listeners->length; i++) {
    wl_list_remove(&listeners->listeners[i].link);
  }
  free(listeners->listeners);
  free(listeners);
}
