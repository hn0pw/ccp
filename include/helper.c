
#include <stdio.h>


/**
 * Print debug output to stderr
 * @param text
 */
void debug(char* text) {
    fprintf(stderr, "Debug: %s\n", text);
}
