/* Chisai - Lightweight, floating WM */
/* Includes */
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <xcb/xcb.h>

/* Macros */
#define MAX(a, b) ((a > b) ? (a) : (b))

/* Variables */
struct sockaddr_un sock_addr;
int sock_fd;
const char *sock_path;

/* Function Signatures */

/* Helper Functions */
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
 * Function: opensocket
 * --------------------
 * Opens a connection to the client's socket
 *
 * returns: nothing, will die if failed to connect
 */
static void
opensocket(void)
{
    if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        die("chisai: failed to open socket\n");
    }
}

/*
 * Function: getsocketpath
 * -----------------------
 * Initializes the socket path
 *
 * returns: nothing
 */
static void
getsocketpath(void)
{
    sock_path = getenv("CHISAI_SOCKET");

    if (sock_path) {
        strncpy(sock_addr.sun_path, sock_path, sizeof(sock_addr.sun_path));
    } else {
        strncpy(sock_addr.sun_path, "/tmp/chisai.sock", sizeof(sock_addr.sun_path));G
    }
}

/*
 * Function: connectsocket
 * -----------------------
 * Connects to the socket to start reading
 *
 * return: nothing
 */
static void
connectsocket(void)
{
    if (connect(sock_fd, (struct sock_addr*)&sock_addr, sizeof(sock_addr)) < 0) {
        die("chisai: failed to connect to socket\n");
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
    /* Setup */
    opensocket();
    getsocketpath();
    connectsocket();
    
    /* Event Loop */
    while(true)
    {
        
    }
}

