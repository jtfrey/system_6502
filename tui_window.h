
#ifndef __TUI_WINDOW_H__
#define __TUI_WINDOW_H__

#include "system_6502_config.h"

enum {
    tui_window_opts_title_align_left = 0,
    tui_window_opts_title_align_center = 1,
    tui_window_opts_title_align_right = 2,
    tui_window_opts_title_align_horiz = 0x3,
    //
    tui_window_opts_title_align_top = 0,
    tui_window_opts_title_align_bottom = 1 << 2,
    tui_window_opts_title_align_vert = 0x4,
    //
    tui_window_opts_enable_scroll = 1 << 3
};
typedef unsigned int tui_window_opts_t;

typedef struct tui_window * tui_window_ref;

typedef void (*tui_window_refresh_callback_t)(tui_window_ref the_window);

tui_window_ref tui_window_alloc(int x, int y, int w, int h,
                    tui_window_opts_t opts,
                    const char *title, int title_len,
                    tui_window_refresh_callback_t refresh_fn, const void *refresh_context
                );
void tui_window_free(tui_window_ref the_window);
void tui_window_refresh(tui_window_ref the_window, int should_defer_update);

#endif /* __TUI_WINDOW_H__ */
