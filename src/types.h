#ifndef WM_TYPES_H
#define WM_TYPES_H

#include <stdbool.h>
#include <xcb/randr.h>

enum position {
    TOP,
    BOTTOM,
    RIGHT,
    LEFT,
    ALL,
};

struct conf {
    enum position border_side;
    int8_t border_width;
    uint32_t focus_color, unfocus_color;
    uint32_t workspaces;
    bool sloppy_focus;
};

#endif
