// Maikuro - Chisai's Client
// Imports
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>

// Helper Function - Exit with error
static void
die(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    exit(EXIT_FAILURE);
}

// Variables
int sock_fd;
const char *sock_path;
struct sockaddr_un sock_addr;

// Main Client
int 
main(int argc, char* argv[])
{
    // Make sure there are enough arguments
    if (argc < 2) {
        die("%s: not enough arguments", argv[0]);
    }
    
    // Open socket
    if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
	    die("%s: failed to open socket\n", argv[0]);
    }

    sock_addr.sun_family = AF_UNIX;

    // Get socket path
    sock_path = getenv("CHISAI_SOCKET");
    

    if (sock_path) {
	strncpy(sock_addr.sun_path, sock_path, sizeof(sock_addr.sun_path));
    } else {
	strncpy(sock_addr.sun_path, "/tmp/chisai.sock", sizeof(sock_addr.sun_path));
    }
}
