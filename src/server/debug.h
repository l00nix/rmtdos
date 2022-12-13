// Debugging tools.

#include <stdio.h>

#include "lib16/types.h"

void hex_dump(FILE *fp, const void *buffer, size_t bytes);

void log_checkpoint(const char *file, int line);

#define CP() log_checkpoint(__FILE__, __LINE__)
