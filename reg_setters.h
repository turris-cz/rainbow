#ifndef REG_SETTERS_H
#define REG_SETTERS_H

#include <stdbool.h>

bool set_color(volatile unsigned char *mem, const char *dev, unsigned int color);
bool set_status(volatile unsigned char *mem, const char *dev, bool enable);
bool set_intensity(volatile unsigned char *mem, unsigned char intensity);

#endif //REG_SETTERS_H
