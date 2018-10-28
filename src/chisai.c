/* Chisai - Lightweight, floating WM */
/* Includes */
#include <err.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <xcb/xcb.h>

#include "config.h"
#include "types.h"

/* Macros */
#define MAX(a, b) ((a > b) ? (a) : (b))
#define CLEANMASK(mask) ((mask & ~0x80))

/* Modifiers */
#define SUPER	XCB_MOD_MASK_4
#define ALT		XCB_MOD_MASK_1
#define CTRL	XCB_MOD_MASK_CONTROL
#define SHIFT	XCB_MOD_MASK_SHIFT

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


static void
load_defaults(void)
{
    config.mod = MOD
    config.border_side  = BORDER_SIDE;
    config.border_width = BORDER_WIDTH;
    config.color_focus = COLOR_FOCUS;
    config.color_unfocus = COLOR_UNFOCUS;
    config.workspaces = WORKSPACES;
    config.sloppy_focus = SLOPPY_FOCUS;
}


static void
events_loop(void)
{
    xcb_generic_event_t *event;
    uint32_t values[3];
    xcb_get_geometry_reply_t *geometry;
    xcb_window_t window = 0;

    while (true)
    {
        event = xcb_wait_for_event(connection);

        if (!event) {
            errx(1, "xcb connection broken");
        }

        switch (CLEANMASK(event->response_type))
        {
            case XCB_CREATE_NOTIFY:
            {
                
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
    /* Event Loop */
    while(true)
    {   
        events_loop();
        /* Accept client socket */
        if ((client_fd = accept(sock_fd, NULL, 0)) < 0) {
            errx(EXIT_FAILURE, "chisai: failed to accept client socket");
        }
        
        /* Read message and execute */
        if ((message_length = read(client_fd, message, sizeof(message))) > 0) {
            message[message_length] = '\0';

            // Quit the window manager
            if (strcmp(message, "quit") == 0) {
                break;
            }

            printf("%s\n", message);
            fflush(stdout);
        }
    }
    
    return EXIT_FAILURE;

}
