/*
 * Rainbow is tool for changing color and status of LEDs of the Turris router
 * Rainbow daemon provides indication of WiFi status by controlling its LED
 *
 * Copyright (C) 2013 CZ.NIC, z.s.p.o. (http://www.nic.cz/)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef REG_SETTERS_H
#define REG_SETTERS_H

#include <stdbool.h>

void set_color(volatile unsigned char *mem, int dev, unsigned int color);
void set_status(volatile unsigned char *mem, int dev, int status);

#endif //REG_SETTERS_H
