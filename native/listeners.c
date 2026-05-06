#include <cpp/jank/c_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wayland-server.h>
#include <wayland-util.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_keyboard.h>

#include "../include/callbacks.h"

jank_object_ref *output_new_callback = NULL;
jank_object_ref *output_destroy_callback = NULL;
jank_object_ref *output_frame_callback = NULL;
jank_object_ref *output_request_state_callback = NULL;
jank_object_ref *input_new_callback = NULL;
jank_object_ref *keyboard_key_callback = NULL;
jank_object_ref *keyboard_modifier_callback = NULL;
jank_object_ref *keyboard_destroy_callback = NULL;
jank_object_ref *cursor_frame_callback = NULL;
jank_object_ref *cursor_motion_callback = NULL;
jank_object_ref *cursor_motion_absolute_callback = NULL;
jank_object_ref *cursor_button_callback = NULL;
jank_object_ref *cursor_axis_callback = NULL;

void eval_callback(jank_object_ref **ref, const char *name, bool debug) {
  if (debug)
    printf("evaluating no argument %s\n", name);
  if (*ref == NULL) {
    *ref = jank_eval(jank_read_string_c(name));
  }
  jank_call0(*ref);
}

void eval_callback1(jank_object_ref **ref, const char *name,
                    jank_object_ref arg, bool debug) {
  if (debug)
    printf("evaluating 1 argument %s\n", name);
  if (*ref == NULL) {
    *ref = jank_eval(jank_read_string_c(name));
  }
  jank_call1(*ref, arg);
}

void eval_callback2(jank_object_ref **ref, const char *name,
                    jank_object_ref arg1, jank_object_ref arg2, bool debug) {
  if (debug)
    printf("evaluating 2 argument %s\n", name);
  if (*ref == NULL) {
    *ref = jank_eval(jank_read_string_c(name));
  }
  jank_call2(*ref, arg1, arg2);
}

void output_new(struct wl_listener *listener, void *data) {
  struct wlr_output *output = data;
  jank_object_ref box_output = jank_box("wlr_output*", output);
  eval_callback1(&output_new_callback, "#'wonk.output/new-callback", box_output,
                 true);
}

void input_new(struct wl_listener *listener, void *data) {
  struct wlr_input_device *device = data;
  jank_object_ref box_data = jank_box("wlr_input_device*", device);
  eval_callback1(&input_new_callback, "#'wonk.input/new-callback", box_data,
                 true);
}

struct array_of_listeners *initialize_listeners(uint count) {
  struct array_of_listeners *listeners =
      calloc(1, sizeof(struct array_of_listeners));
  listeners->length = count;
  listeners->the_listeners =
      calloc(listeners->length, sizeof(struct wl_listener *));
  for (int i = 0; i < count; i++) {
    listeners->the_listeners[i] = calloc(1, sizeof(struct wl_listener));
  }
  return listeners;
}

struct array_of_listeners *wire_backend_listeners(struct wlr_backend *backend) {
  struct array_of_listeners *listeners = initialize_listeners(2);
  listeners->the_listeners[0]->notify = output_new;
  wl_signal_add(&backend->events.new_output, listeners->the_listeners[0]);
  listeners->the_listeners[1]->notify = input_new;
  wl_signal_add(&backend->events.new_input, listeners->the_listeners[1]);
  return listeners;
}

void output_destroy(struct wl_listener *listener, void *data) {
  struct wlr_output *output = data;
  jank_object_ref box_output = jank_box("wlr_output*", output);
  eval_callback1(&output_destroy_callback, "#'wonk.output/destroy-callback",
                 box_output, true);
}

void output_frame(struct wl_listener *listener, void *data) {
  struct wlr_output *output = data;
  jank_object_ref box_output = jank_box("wlr_output*", output);
  eval_callback1(&output_frame_callback, "#'wonk.output/frame-callback",
                 box_output, false);
}

void output_request_state(struct wl_listener *listener, void *data) {
  eval_callback(&output_request_state_callback,
                "#'wonk.output/request-state-callback", true);
}

struct array_of_listeners *wire_output_listeners(struct wlr_output *output) {
  struct array_of_listeners *listeners = initialize_listeners(3);
  listeners->the_listeners[0]->notify = output_destroy;
  wl_signal_add(&output->events.destroy, listeners->the_listeners[0]);
  listeners->the_listeners[1]->notify = output_frame;
  wl_signal_add(&output->events.frame, listeners->the_listeners[1]);
  listeners->the_listeners[2]->notify = output_request_state;
  wl_signal_add(&output->events.request_state, listeners->the_listeners[2]);
  return listeners;
}

void keyboard_key(struct wl_listener *listener, void *data) {
  struct wlr_keyboard_key_event *event = data;
  jank_object_ref box_data = jank_box("wlr_keyboard_key_event*", event);
  jank_object_ref listener_box = jank_box("wl_listener*", listener);
  eval_callback2(&keyboard_key_callback, "#'wonk.input/key-press", box_data,
                 listener_box, true);
}

void keyboard_modifier(struct wl_listener *listener, void *data) {
  struct wlr_keyboard_key_event *event = data;
  jank_object_ref box_data = jank_box("wlr_keyboard_key_event*", event);
  eval_callback1(&keyboard_modifier_callback, "#'wonk.input/modifier-press",
                 box_data, true);
}

void keyboard_destroy(struct wl_listener *listener, void *data) {
  struct wlr_keyboard *keyboard = data;
  jank_object_ref box_data = jank_box("wlr_keyboard*", keyboard);
  eval_callback1(&keyboard_destroy_callback, "#'wonk.input/keyboard-destroy",
                 box_data, true);
}

struct array_of_listeners *
wire_keyboard_listeners(struct wlr_input_device *device,
                        struct wlr_keyboard *keyboard) {
  struct array_of_listeners *listeners = initialize_listeners(3);
  listeners->the_listeners[0]->notify = keyboard_key;
  wl_signal_add(&keyboard->events.key, listeners->the_listeners[0]);
  listeners->the_listeners[1]->notify = keyboard_modifier;
  wl_signal_add(&keyboard->events.modifiers, listeners->the_listeners[1]);
  listeners->the_listeners[2]->notify = keyboard_destroy;
  wl_signal_add(&device->events.destroy, listeners->the_listeners[2]);
  return listeners;
}

// TODO
// Frame and axis events aren't currently used so maybe get rid 
// of them. Also, frame event may not even be the right type? :/
void cursor_frame(struct wl_listener *listener, void *data) {
  struct wlr_button_pointer_event *event = data;
  jank_object_ref box_data = jank_box("wlr_button_pointer_event*", event);
  eval_callback1(&cursor_frame_callback, "#'wonk.cursor/cursor-frame", box_data,
                 false);
}

void cursor_motion(struct wl_listener *listener, void *data) {
  struct wlr_pointer_motion_event *event = data;
  jank_object_ref box_data = jank_box("wlr_pointer_motion_event*", event);
  eval_callback1(&cursor_motion_callback, "#'wonk.cursor/cursor-motion",
                 box_data, false);
}

void cursor_motion_absolute(struct wl_listener *listener, void *data) {
  struct wlr_pointer_motion_absolute_event *event = data;
  jank_object_ref box_data = jank_box("wlr_pointer_motion_absolute_event*", event);
  eval_callback1(&cursor_motion_absolute_callback,
                 "#'wonk.cursor/cursor-motion-absolute", box_data, false);
}

void cursor_button(struct wl_listener *listener, void *data) {
  struct wlr_button_pointer_event *event = data;
  jank_object_ref box_data = jank_box("wlr_button_pointer_event*", event);
  eval_callback1(&cursor_button_callback, "#'wonk.cursor/cursor-button",
                 box_data, false);
}

void cursor_axis(struct wl_listener *listener, void *data) {
  struct wlr_pointer_axis_event *event = data;
  jank_object_ref box_data = jank_box("wlr_pointer_axis_event*", event);
  eval_callback1(&cursor_axis_callback, "#'wonk.cursor/cursor-axis", box_data,
                 false);
}

struct array_of_listeners *wire_cursor_listeners(struct wlr_cursor *cursor) {
  struct array_of_listeners *listeners = initialize_listeners(5);
  listeners->the_listeners[0]->notify = cursor_frame;
  wl_signal_add(&cursor->events.frame, listeners->the_listeners[0]);
  listeners->the_listeners[1]->notify = cursor_motion;
  wl_signal_add(&cursor->events.motion, listeners->the_listeners[1]);
  listeners->the_listeners[2]->notify = cursor_motion_absolute;
  wl_signal_add(&cursor->events.motion_absolute, listeners->the_listeners[2]);
  listeners->the_listeners[3]->notify = cursor_button;
  wl_signal_add(&cursor->events.button, listeners->the_listeners[3]);
  listeners->the_listeners[4]->notify = cursor_axis;
  wl_signal_add(&cursor->events.axis, listeners->the_listeners[4]);
  return listeners;
}

void cleanup_listeners(struct array_of_listeners *listeners) {
  for (int i = 0; i < listeners->length; i++) {
    wl_list_remove(&listeners->the_listeners[i]->link);
  }
  for (int i = 0; i < listeners->length; i++) {
    free(listeners->the_listeners[i]);
  }
  free(listeners->the_listeners);
  free(listeners);
}

// no aget so we are doing this
xkb_keysym_t get_sym_at_index(const xkb_keysym_t *syms, int index) {
  return syms[index];
}

bool is_listener_in_listeners(struct wl_listener *target,
                              struct wl_listener **listeners, int count) {
  for (int i = 0; i < count; i++) {
    if (target == listeners[i]) {
      return true;
    }
  }
  return false;
}

// this is also because we used a function that used aget to figure this out
struct wl_listener *get_listener_pointer_at_index(struct wl_listener **wl,
                                                  int index) {
  return wl[index];
}
