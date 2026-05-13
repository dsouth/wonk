#include <cpp/jank/c_api.h>
#include <stdio.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wayland-server.h>
#include <wayland-util.h>
#include <wlr/backend.h>
#include <wlr/util/log.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>

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
jank_object_ref *xdg_shell_new_toplevel_callback = NULL;
jank_object_ref *xdg_shell_new_popup_callback = NULL;
jank_object_ref *toplevel_surface_map_callback = NULL;
jank_object_ref *toplevel_surface_unmap_callback = NULL;
jank_object_ref *toplevel_surface_commit_callback = NULL;
jank_object_ref *toplevel_request_maximize_callback = NULL;
jank_object_ref *toplevel_request_fullscreen_callback = NULL;
jank_object_ref *toplevel_request_move_callback = NULL;
jank_object_ref *toplevel_request_resize_callback = NULL;
jank_object_ref *toplevel_destroy_callback = NULL;

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

void callback1_with_listener(struct wl_listener *listener,
                             jank_object_ref **callback,
                             const char *jank_function_name) {
  jank_object_ref data_box = jank_box("wl_listner*", listener);
  eval_callback1(callback, jank_function_name, data_box, true);
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
  jank_object_ref box_data =
      jank_box("wlr_pointer_motion_absolute_event*", event);
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

void xdg_shell_new_toplevel(struct wl_listener *listener, void *data) {
  struct wlr_pointer_axis_event *event = data;
  jank_object_ref box_data = jank_box("wlr_xdg_toplevel*", event);
  eval_callback1(&xdg_shell_new_toplevel_callback,
                 "#'wonk.xdg-shell/new-toplevel", box_data, true);
}

void xdg_shell_new_popup(struct wl_listener *listener, void *data) {
  eval_callback(&xdg_shell_new_popup_callback, "#'wonk.xdg-shell/new-toplevel",
                true);
}

struct array_of_listeners *
wire_xdg_shell_listeners(struct wlr_xdg_shell *shell) {
  wlr_log(WLR_DEBUG, "wiring listeners for xdg shell...");
  struct array_of_listeners *listeners = initialize_listeners(2);
  listeners->the_listeners[0]->notify = xdg_shell_new_toplevel;
  wl_signal_add(&shell->events.new_toplevel, listeners->the_listeners[0]);
  listeners->the_listeners[1]->notify = xdg_shell_new_popup;
  wl_signal_add(&shell->events.new_popup, listeners->the_listeners[1]);
  return listeners;
}

void toplevel_surface_map(struct wl_listener *listener, void *data) {
  callback1_with_listener(listener, &toplevel_surface_map_callback,
                          "#'wonk.xdg-shell/toplevel-surface-map");
}

void toplevel_surface_unmap(struct wl_listener *listener, void *data) {
  // maybe return the listener to map to the toplevel???
  eval_callback(&toplevel_surface_unmap_callback,
                "#'wonk.xdg-shell/toplevel-surface-unmap", true);
}

void toplevel_surface_commit(struct wl_listener *listener, void *data) {
  callback1_with_listener(listener, &toplevel_surface_commit_callback,
                          "#'wonk.xdg-shell/toplevel-surface-commit");
}

void toplevel_request_maximize(struct wl_listener *listener, void *data) {
  // maybe return the listener to map to the toplevel???
  eval_callback(&toplevel_request_maximize_callback,
                "#'wonk.xdg-shell/toplevel-request-maximize", true);
}

void toplevel_request_fullscreen(struct wl_listener *listener, void *data) {
  // maybe return the listener to map to the toplevel???
  eval_callback(&toplevel_request_fullscreen_callback,
                "#'wonk.xdg-shell/toplevel-request-fullscreen", true);
}

void toplevel_request_move(struct wl_listener *listener, void *data) {
  // maybe return the listener to map to the toplevel???
  eval_callback(&toplevel_request_move_callback,
                "#'wonk.xdg-shell/toplevel-request-move", true);
}

void toplevel_request_resize(struct wl_listener *listener, void *data) {
  // maybe return the listener to map to the toplevel???
  eval_callback(&toplevel_request_resize_callback,
                "#'wonk.xdg-shell/toplevel-request-resize", true);
}

void toplevel_destroy(struct wl_listener *listener, void *data) {
  callback1_with_listener(listener, &toplevel_destroy_callback,
                          "#'wonk.xdg-shell/toplevel-destroy");
}

// TODO constants for array indexes that are accessible from jank!
struct array_of_listeners *
wire_xdg_toplevel_listeners(struct wlr_xdg_toplevel *toplevel) {
  struct array_of_listeners *listeners = initialize_listeners(TOPLEVEL_COUNT);
  listeners->the_listeners[TOPLEVEL_MAP]->notify = toplevel_surface_map;
  wl_signal_add(&toplevel->base->surface->events.map,
                listeners->the_listeners[TOPLEVEL_MAP]);
  listeners->the_listeners[TOPLEVEL_UNMAP]->notify = toplevel_surface_unmap;
  wl_signal_add(&toplevel->base->surface->events.unmap,
                listeners->the_listeners[TOPLEVEL_UNMAP]);
  listeners->the_listeners[TOPLEVEL_COMMIT]->notify = toplevel_surface_commit;
  wl_signal_add(&toplevel->base->surface->events.commit,
                listeners->the_listeners[TOPLEVEL_COMMIT]);
  listeners->the_listeners[TOPLEVEL_MAXIMIZE]->notify =
      toplevel_request_maximize;
  wl_signal_add(&toplevel->events.request_maximize,
                listeners->the_listeners[TOPLEVEL_MAXIMIZE]);
  listeners->the_listeners[TOPLEVEL_FULLSCREEN]->notify =
      toplevel_request_fullscreen;
  wl_signal_add(&toplevel->events.request_fullscreen,
                listeners->the_listeners[TOPLEVEL_FULLSCREEN]);
  listeners->the_listeners[TOPLEVEL_MOVE]->notify = toplevel_request_move;
  wl_signal_add(&toplevel->events.request_move,
                listeners->the_listeners[TOPLEVEL_MOVE]);
  listeners->the_listeners[TOPLEVEL_RESIZE]->notify = toplevel_request_resize;
  wl_signal_add(&toplevel->events.request_resize,
                listeners->the_listeners[TOPLEVEL_RESIZE]);
  listeners->the_listeners[TOPLEVEL_DESTROY]->notify = toplevel_destroy;
  wl_signal_add(&toplevel->events.destroy,
                listeners->the_listeners[TOPLEVEL_DESTROY]);
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

// Don't really want to do this in jank land... :/
struct wlr_output_layout_output *
output_get_primary_output_layout(struct wlr_output_layout *layout) {
  struct wlr_output_layout_output *layout_output = NULL;
  wl_list_for_each(layout_output, &layout->outputs, link) {
    if (layout_output->x == 0 && layout_output->y == 0) {
      break;
    }
  }
  // if we don't find 0, 0 then we're just returning the last configured
  // output... :/
  // TODO
  return layout_output;
}

// this is also because we used a function that used aget to figure this out
struct wl_listener *get_listener_pointer_at_index(struct wl_listener **wl,
                                                  int index) {
  printf("get_listener_pointer at index called with index %d\n", index);
  return wl[index];
}

struct wlr_scene_rect* get_background_rect(struct wlr_scene_tree *background) {
  struct wlr_scene_node *node;
  node = wl_container_of(background->children.next, node, link);
  struct wlr_scene_rect *rect = wlr_scene_rect_from_node(node);
  return rect;
}

// colors are memcpy by wlroots, so it's OK to reuse the same array
// for every color (assuming single treaded...)
float * get_color_array(float r, float g, float b, float a) {
  color[0] = r;
  color[1] = g;
  color[2] = b;
  color[3] = a;
  return color;
}
