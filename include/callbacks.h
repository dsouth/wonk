#ifndef WONK_LISTENERS
#define WONK_LISTENERS
#include <wayland-server-core.h>
#include <xkbcommon/xkbcommon.h>

#ifdef __cplusplus
extern "C" {
#endif

struct array_of_listeners {
  int length;
  struct wl_listener **the_listeners;
};

void cleanup_listeners(struct array_of_listeners *);
struct array_of_listeners *wire_backend_listeners(struct wlr_backend *);
struct array_of_listeners *wire_output_listeners(struct wlr_output *);
struct array_of_listeners *wire_keyboard_listeners(struct wlr_input_device *,
                                                   struct wlr_keyboard *);
struct array_of_listeners *wire_cursor_listeners(struct wlr_cursor *);
struct array_of_listeners *wire_cursor_listeners(struct wlr_cursor *);

// TODO These two should go when aget is fully implemented!
xkb_keysym_t get_sym_at_index(const xkb_keysym_t *, int);
struct wl_listener *get_listener_pointer_at_index(struct wl_listener **, int);
bool is_listener_in_listeners(struct wl_listener *target,
                              struct wl_listener **listeners, int count);
struct wlr_output_layout_output *
output_get_primary_output_layout(struct wlr_output_layout *);
#ifdef __cplusplus
}
#endif

#endif
