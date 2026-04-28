#include <wayland-server-core.h>
#include <wayland-server.h>
#include <wlr/backend.h>
#include <cpp/jank/c_api.h>
#include <stdio.h>

jank_object_ref *callback = NULL;

struct wl_listener output_new_listener;

void output_new(struct wl_listener *listener, void *data) {
  if (callback == NULL) {
    callback = jank_eval(jank_read_string_c("#'wonk.output/output-new-callback"));
  }
//  jank_eval(callback);
  jank_call0(callback);
}
 
void wire_backend_listeners(struct wlr_backend *backend) {
  output_new_listener.notify = output_new;
  wl_signal_add(&backend->events.new_output, &output_new_listener);
}

void clean_up_listeners() {
  wl_list_remove(&output_new_listener.link);
}
