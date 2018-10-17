/* Maikuro - Chisai's Client */
/* Imports */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

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

/* Variables */
char message[HUGE];
struct sockaddr_un sock_addr;
int sock_fd;
const char *sock_path;

/*
 * Function: main
 * --------------
 * Main loop of the client, passes messages to Chisai
 *
 * argc: number of arguments
 * argv: the arguments
 * 
 * returns: error if something goes wrong, else messages
 * just get passed into Chisai
 */
int 
main(int argc, char* argv[])
{
    /* Make sure there are enough arguments */
    if (argc < 2) {
        die("%s: not enough arguments", argv[0]);
    }
    
    /* Open socket */
    if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
	    die("%s: failed to open socket\n", argv[0]);
    }

    sock_addr.sun_family = AF_UNIX;

    /* Get socket path */
    sock_path = getenv("CHISAI_SOCKET");
    
    if (sock_path) {
        strncpy(sock_addr.sun_path, sock_path, sizeof(sock_addr.sun_path));
    } else {
        strncpy(sock_addr.sun_path, "/tmp/chisai.sock", sizeof(sock_addr.sun_path));
    }

    /* Connect to Chisai socket */
    if (connect(sock_fd, (struct sock_addr*)&sock_addr, sizeof(sock_addr)) < 0)  {
        die("%s: failed to connect to socket\n", argv[0]);
    }    
 
    /* Concatenate arguments into one string */
    strcpy(message, argv[1]);
    for (int i = 2; i < argc; i ++)
    {
        strcat(message, " ");
        strcat(message, argv[i]);
    }
    
    /* Send message to Chisai */
    if (write(sock_fd, message, sizeof(message)) < 0) {
        die("%s: failed to send message to chisai\n", argv[0]);
    }
}
