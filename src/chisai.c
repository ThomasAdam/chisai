/* Chisai - Lightweight, floating WM */
/* Includes */
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
#include <xcb/xcb_aux.h>

/* Macros */
#define MAX(a, b) ((a > b) ? (a) : (b))

/* Socket Variables */
struct sockaddr_un sock_addr;
int sock_fd;
int client_fd;
int message_length;
const char *sock_path;
char message[BUFSIZ];

/* XCB Variables */
static xcb_connection_t *conn;
static xcb_screen_t *scr;

/* Function Signatures */
static void die(char *fmt, ...);
static void initialize(void);
static void init_xcb(xcb_connection_t **con);
static void kill_xcb(xcb_connection_t **con);


/*
 * Function: die
 * -------------
 * Returns the message and exists with an error
 *
 * fmt: Error message to return
 * ...: Discard the rest of the arguments
 *
 * returns: nothing, just exists the file with the error
 */
static void
die(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}

/*
 * Function: initialize
 * --------------------
 * Creates and 
 *
 * return: nothing
 */
static void
initialize(void)
{
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
        die("chisai: failed to open socket\n");
    }
    
    sock_addr.sun_family = AF_UNIX;
    
    unlink(sock_addr.sun_path);

    /* Bind socket */
    if (bind(sock_fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) < 0) {
        perror("Sjo");
        die("chisai: failed to bind socket\n");       
    } 
    
    /* Listen to the socket */
    if (listen(sock_fd, 1) < 0) {
        die("chisai: socket listen failed");
    }
}

/*
 * Function: init_xcb
 * ------------------
 * Initialize X connection
 *
 * returns: nothing
 */
void
init_xcb(xcb_connection_t **con)
{
    *con = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(*con)) {
        die("chisai: failed to connect to xcb");
    }
}

/*
 * Function: get_screen
 * --------------------
 * Gets the current screen the user is on
 *
 * returns: nothing
 */
void
get_screen(xcb_connection_t *con, xcb_screen_t **scr)
{
	*scr = xcb_setup_roots_iterator(xcb_get_setup(con)).data;
	if (*scr == NULL)
		errx(1, "unable to retrieve screen informations");
}

void
kill_xcb(xcb_connection_t **con)
{
    if (*con)
        xcb_disconnect(*con);
}
/*
 *
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
    /* Setup socket and X */
    initialize();
    init_xcb(&conn);
    get_screen(conn, &scr);

    /* Event Loop */
    while(true)
    {   
        /* Accept client socket */
        if ((client_fd = accept(sock_fd, NULL, 0)) < 0) {
            die("chisai: failed to accept client socket");
        }
        
        /* Read message and execute */
        if ((message_length = read(client_fd, message, sizeof(message))) > 0) {
            xcb_aux_sync(conn);
            message[message_length] = '\0';

            // Quit the window manager
            if (strcmp(message, "quit") == 0) {
                break;
            }

            printf("%s\n", message);
            fflush(stdout);
        }
    }
    
    /* Kill X connection */
    kill_xcb(&conn);
}

