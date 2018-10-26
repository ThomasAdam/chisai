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

/* Macros */
#define MAX(a, b) ((a > b) ? (a) : (b))

/* Variables */
struct sockaddr_un sock_addr;
int sock_fd;
int client_fd;
const char *sock_path;
char message[BUFSIZ];

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

    /* Bind socket */
    if (bind(sock_fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) < 0) {
        perror("Shit went worng");
        die("chisai: failed to bind socket\n");       
    } 
    
    /* Listen to the socket */
    if (listen(sock_fd, 1) < 0) {
        die("chisai: socket listen failed");
    }
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
    /* Setup */
    initialize();
    
    /* Event Loop */
    while(true)
    {   
        /* Accept client socket */
        if ((client_fd = accept(sock_fd, NULL, 0)) < 0) {
            die("chisai: failed to accept client socket");
        }

        if (read(client_fd, message, sizeof(message)) > 0) {
            printf("%s", message);
        }
    }
}

