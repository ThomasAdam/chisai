/* Chisai - Lightweight, floating WM */
/* Includes */
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <xcb/xcb.h>

/* Macros */
#define MAX(a, b) ((a > b) ? (a) : (b))

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

/* TODO: Connect to x function and socket and all that pizzaz and all the individual functoins themselves feelsbadman */
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
    while(true)
    {
        
    }
}

