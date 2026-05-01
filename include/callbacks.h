#ifndef WONK_LISTENERS
#define WONK_LISTENERS
#include <wayland-server-core.h>

extern "C" {
struct listeners {
  int length;
  struct wl_listener *listeners;
};

void cleanup_listeners(struct listeners *);
struct listeners * wire_backend_listeners(struct wlr_backend *);
struct listeners * wire_output_listeners(struct wlr_output *);
struct listeners * wire_keyboard_listeners(struct wlr_input_device *, 
		struct wlr_keyboard *);
}

#endif
