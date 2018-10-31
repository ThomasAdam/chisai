/* Chisai - Lightweight, floating WM */
/* Includes */
#include <err.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <xcb/xcb.h>

#include "config.h"
#include "types.h"

/* Macros */
#define MAX(a, b) ((a > b) ? (a) : (b))
#define CLEANMASK(mask) ((mask & ~0x80))

enum { INACTIVE, ACTIVE };

/* Modifiers - You can change to set different MOD */
#define SUPER XCB_MOD_MASK_4
#define ALT	  XCB_MOD_MASK_1
#define CTRL  XCB_MOD_MASK_CONTROL
#define SHIFT XCB_MOD_MASK_SHIFT
#define MOD	  SUPER

/* Socket Variables */
/* TODO: Make these all local */
static int sock_fd;
static int client_fd;
static const char *sock_path;

/* XCB Variables */
static xcb_connection_t *connection;
static xcb_screen_t *screen;
static xcb_window_t focused_window;
static struct conf config;

/* Function Signatures */
static void cleanup(void);
static int socket_deploy(void);
static int x_deploy(void);
static void load_defaults(void);
static void subscribe(xcb_window_t);
static void focus(xcb_window_t, int);
static void events_loop(void);

/*
 * Function: cleanup
 * -----------------
 * Cleans up past X connection
 *
 * returns: nothing
 */
static void
cleanup(void)
{
    if (connection != NULL) {
        xcb_disconnect(connection);
    }
}


/*
 * Function: socket_deploy
 * -----------------------
 * Connects to the socket and starts listening
 *
 * return: nothing
 */
static int
socket_deploy(void)
{
    struct sockaddr_un sock_addr;

    /* Get socket path */
    sock_path = getenv("CHISAI_SOCKET");
    memset(&sock_addr, 0, sizeof(sock_addr));

    if (sock_path) {
        strncpy(sock_addr.sun_path, sock_path, sizeof(sock_addr.sun_path));
    } else {
        strncpy(sock_addr.sun_path, "/tmp/chisai.sock", sizeof(sock_addr.sun_path));
    }

    /* Create socket */
    if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        return -1;
    }
    
    sock_addr.sun_family = AF_UNIX;
    
    unlink(sock_addr.sun_path);

    /* Bind socket */
    if (bind(sock_fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) < 0) {
        return -1;
    } 
    
    /* Listen to the socket */
    if (listen(sock_fd, 1) < 0) {
        return -1;
    }

    return 0;
}


/*
 * Function: deploy_x
 * ------------------
 * Connects to X
 *
 * returns: nothing
 */
static int
x_deploy(void)
{
    /* Init XCB and grab events */
    uint32_t values[2];
    int mask;
    
    /* Make sure XCB and the screen are working properly */
    if (xcb_connection_has_error(connection = xcb_connect(NULL, NULL))) {
        return -1;
    }
    
    if ((screen = xcb_setup_roots_iterator(xcb_get_setup(connection)).data) == NULL) {
        return -1;
    }

    focused_window = screen->root;

    /* Grab mouse buttons */
    xcb_grab_button(connection, 0, screen->root, XCB_EVENT_MASK_BUTTON_PRESS |
			XCB_EVENT_MASK_BUTTON_RELEASE, XCB_GRAB_MODE_ASYNC,
			XCB_GRAB_MODE_ASYNC, screen->root, XCB_NONE, 1, MOD);

	xcb_grab_button(connection, 0, screen->root, XCB_EVENT_MASK_BUTTON_PRESS |
			XCB_EVENT_MASK_BUTTON_RELEASE, XCB_GRAB_MODE_ASYNC,
            XCB_GRAB_MODE_ASYNC, screen->root, XCB_NONE, 3, MOD);
    
    /* Update mask and root */
    mask = XCB_CW_EVENT_MASK;
    values[0] = XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
	xcb_change_window_attributes_checked(connection, screen->root, mask, values);

    xcb_flush(connection);

    return 0;
}


/*
 * Function: load_defaults
 * -----------------------
 * Load the default settings from config.h
 *
 * returns: nothing
 */
static void
load_defaults(void)
{
    config.border_side   = BORDER_SIDE;
    config.border_width  = BORDER_WIDTH;
    config.focus_color   = COLOR_FOCUS;
    config.unfocus_color = COLOR_UNFOCUS;
    config.workspaces    = WORKSPACES;
    config.sloppy_focus  = SLOPPY_FOCUS;
}


/*
 * Function: subscribe
 * -------------------
 * Start listening for events that happen
 * to the window passed in
 *
 * returns: nothing
 */
static void
subscribe(xcb_window_t window)
{
    uint32_t values[2];

    /* Subscribe to events */
    values[0] = XCB_EVENT_MASK_ENTER_WINDOW;
    values[1] = XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY;
    xcb_change_window_attributes(connection, window, XCB_CW_EVENT_MASK, values);

    /* TODO: "SiCcCk BoRdErs", rn --> Border Application */
    values[0] = config.border_width;
    xcb_configure_window(connection, window, XCB_CONFIG_WINDOW_BORDER_WIDTH, values);
}


/*
 * Function: focus
 * ---------------
 * Focuses on the window passed if the mode
 * argument says to make it ACTIVE
 *
 * returns: nothing
 */
static void
focus(xcb_window_t window, int mode)
{
    uint32_t values[1];
    
    /* Pick which color to use for the border */
    values[0] = mode ? config.focus_color : config.unfocus_color;
    xcb_change_window_attributes(connection, window, XCB_CW_BORDER_PIXEL, values);

    /* Focus the window */
    if (mode == ACTIVE) {
        xcb_set_input_focus(connection, XCB_INPUT_FOCUS_POINTER_ROOT,
            window, XCB_CURRENT_TIME);
        if (window != focused_window) {
            focus(focused_window, INACTIVE);
            focused_window = window;
        }
    }
}


/* 
 * Function: events_loop
 * ---------------------
 * Listens for Maikuro and X events, also
 * handles them appropriately.
 * 
 * returns: nothing
 */
static void
events_loop(void)
{

    while (true)
    {
        /* Select file descriptors and select which one you receive */
        int x_fd = xcb_get_file_descriptor(connection);

        fd_set file_descriptors;
        FD_ZERO(&file_descriptors);
        FD_SET(x_fd, &file_descriptors);
        FD_SET(sock_fd, &file_descriptors);

        int max_fd = MAX(sock_fd, x_fd) + 1;

        select(max_fd, &file_descriptors, NULL, NULL, NULL);

        /* Pathway for if a message from the client is received */
        if (FD_ISSET(sock_fd, &file_descriptors)) {
            char message[BUFSIZ];
            int message_length;

            if ((client_fd = accept(sock_fd, NULL, 0)) < 0) {
                errx(EXIT_FAILURE, "chisai: failed to accept client socket");
            }
            
            if ((message_length = read(client_fd, message, sizeof(message))) > 0) {
                message[message_length] = '\0';
            }
        }   

        /* Pathway for if X event is received */
        if (FD_ISSET(x_fd, &file_descriptors)) {
            xcb_generic_event_t *event;
            uint32_t values[3];
            xcb_get_geometry_reply_t *geometry;
            xcb_window_t window = 0;

            event = xcb_poll_for_event(connection);
            
            /* Make sure there is an event */
            if (!event) {
                continue;
            }

            /* Handle all the X events we are accepting */
            switch(CLEANMASK(event->response_type)) 
            {
                /* Pathway for newly created window */
                case XCB_CREATE_NOTIFY: 
                {
                    xcb_create_notify_event_t *e;
                    e = (xcb_create_notify_event_t *)event;
                    
                    /* Make sure the window isn't a bar or panel */
                    if (!e->override_redirect) {
                        subscribe(e->window);
                        focus(e->window, ACTIVE);
                    }
                } break; 
                
                /* Pathway for killing a window */
                case XCB_DESTROY_NOTIFY:
                {
                    xcb_destroy_notify_event_t *e;
                    e = (xcb_destroy_notify_event_t *)event;
                    
                    xcb_kill_client(connection, e->window);
                } break;

                /* Pathway for if mouse enters a window */
                case XCB_ENTER_NOTIFY:
                {
                    /* Make sure sloppy focus is enabled */
                    if (config.sloppy_focus == true) {
                        xcb_enter_notify_event_t *e;
                        e = (xcb_enter_notify_event_t *)event;
                        focus(e->event, ACTIVE);
                    } else {
                        break;
                    }
                } break;

                case XCB_MAP_NOTIFY:
                {
                    xcb_map_notify_event_t *e;
                    e = (xcb_map_notify_event_t *)event;

                    if (!e->override_redirect) {
                        xcb_map_windows(connection, e->window);
                        focus(e->window, ACTIVE);
                    }
                } break;

                case XCB_BUTTON_PRESS:
                {
                    xcb_button_press_event_t *e;
                    e = (xcb_button_press_event_t *)event;
                    window = e->child;

                    if (!window || win == scr->root) {
                        break;
                    }

                    values[0] = XCB_STACK_MODE_ABOVE;
                    xcb_configure_window(connection, window,
                            XCB_CONFIG_WINDOW_STACK_MODE, values);
                    geometry = xcb_get_geometry_reply(connection,
                            xcb_get_geometry(connection, window), NULL);
                }
            }
        }
    }
}


/*
 * Function: main
 * --------------
 * Reads messages from socket and executes
 *
 * returns: error if something goes wrong, else it just 
 * executes the actions passed in from Maikuro
 */
int
main(void)
{
    /* Local variables */
    char message[BUFSIZ];
    int message_length;

    /* Cleanup X Connections */
    atexit(cleanup);

    /* Setup socket and X */
    if (socket_deploy() < 0) {
        errx(EXIT_FAILURE, "chisai: error connecting to socket");
    }
    if (x_deploy() < 0) {
        errx(EXIT_FAILURE, "chisai: error connecting to x");
    }

    load_defaults();

    events_loop();
    
    return EXIT_FAILURE;
    /* TODO: COnfig shoud be maikuro config border_width 5 or something 
     * of the likes where config shows it's editing the config
     */
    
}
