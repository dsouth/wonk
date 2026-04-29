#include <wayland-server-core.h>
#include <wayland-server.h>
#include <wlr/backend.h>
#include <cpp/jank/c_api.h>
#include <stdio.h>

jank_object_ref *output_new_callback = NULL;
struct wl_listener output_new_listener;

jank_object_ref *output_destroy_callback = NULL;
struct wl_listener output_destroy_listener;

jank_object_ref *output_frame_callback = NULL;
struct wl_listener output_frame_listener;

jank_object_ref *output_request_state_callback = NULL;
struct wl_listener output_request_state_listener;

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

void output_new(struct wl_listener *listener, void *data) {
  struct wlr_output *output = data;
  jank_object_ref box_output = jank_box("wlr_output*", output);
  // todo pass in output
  eval_callback1(&output_new_callback, "#'wonk.output/new-callback", box_output);
}
 
void wire_backend_listeners(struct wlr_backend *backend) {
  output_new_listener.notify = output_new;
  wl_signal_add(&backend->events.new_output, &output_new_listener);
}

void output_destroy(struct wl_listener *listener, void *data) {
  eval_callback(&output_destroy_callback, "#'wonk.output/destroy-callback");
}

void output_frame(struct wl_listener *listener, void *data) {
  eval_callback(&output_frame_callback, "#'wonk.output/frame-callback");
}

void output_request_state(struct wl_listener *listener, void *data) {
  eval_callback(&output_request_state_callback, "#'wonk.output/request-state-callback");
}

void wire_output_listeners(struct wlr_output *output) {
  printf("  -- wire output listners called --\n");
  output_destroy_listener.notify = output_destroy;
  wl_signal_add(&output->events.destroy, &output_destroy_listener);
  output_frame_listener.notify = output_frame;
  wl_signal_add(&output->events.frame, &output_frame_listener);
  output_request_state_listener.notify = output_request_state;
  wl_signal_add(&output->events.request_state, &output_request_state_listener);
}

void clean_up_listeners() {
  wl_list_remove(&output_new_listener.link);
}

void output_clean_up_listeners() {
  wl_list_remove(&output_frame_listener.link);
  wl_list_remove(&output_request_state_listener.link);
  wl_list_remove(&output_destroy_listener.link);
}
