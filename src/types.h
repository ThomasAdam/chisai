#ifndef WM_TYPES_H
#define WM_TYPES_H

#include <stdbool.h>
#include <xcb/randr.h>

/* 
 * Enum : position
 * ---------------
 * The sides of the window the border can be all
 */
enum position {
    TOP,
    BOTTOM,
    RIGHT,
    LEFT,
    ALL,
};


/*
 * Struct: sizepos
 * ---------------
 * Size and postiion of a window
 *
 * x       - X coordinate
 * y       - Y coordinate
 * width   - Width of the window
 * height  - Height of the window
 */
struct sizepos {
	int16_t x, y;
	uint16_t width,height;
};


/* 
 * Struct: conf
 * ------------
 * Holds the different values config can hold
 *
 * border_side    - The side the border will be on 
 * border_width   - The width of the border
 * focus_color    - Border color of the focused window
 * unfocus_color  - Border color of the unfocued windows
 * workspaces     - Number of workspaces 
 * sloppy_fous    - Whether or not sloppy focus is enabled
 */
struct conf {
    enum position border_side;
    int8_t border_width;
    uint32_t focus_color, unfocus_color;
    uint32_t workspaces;
    bool sloppy_focus;
};


/*
 * Struct: Arg
 * -----------
 * Argument structure to be passed in
 *
 * command  - Command to run
 * i        - Integer to represent the different states
 */
struct Arg {
    const char** command;
    const int i;
};


/*
 * Struct: client
 * --------------
 * Client wrapper for windows 
 *
 * window         - The window
 * maxed          - Whether or not the window is maxed
 * original_size  - Save the size for maxed windows
 */ 
struct client {
    xcb_window_t window;
    bool maxed;
    struct sizepos original_size;
};

#endif
