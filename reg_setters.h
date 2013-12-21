#ifndef REG_SETTERS_H
#define REG_SETTERS_H

#include <stdbool.h>

void set_color(volatile unsigned char *mem, int dev, unsigned int color);
void set_status(volatile unsigned char *mem, int dev, int status);

#endif //REG_SETTERS_H
