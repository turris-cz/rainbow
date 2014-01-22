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

#ifndef ARG_PARSER_H
#define ARG_PARSER_H

#include <stdbool.h>

#define KW_WAN		"wan"
#define KW_WIFI		"wifi"
#define KW_PWR		"pwr"
#define KW_LAN		"lan"
#define KW_LAN1		"lan1"
#define KW_LAN2		"lan2"
#define KW_LAN3		"lan3"
#define KW_LAN4		"lan4"
#define KW_LAN5		"lan5"
#define KW_ALL		"all"
#define KW_INTEN	"intensity"
#define KW_BINMASK	"binmask"
#define KW_ENABLE	"enable"
#define KW_DISABLE	"disable"
#define KW_AUTO		"auto"

#define DEV_UNDEF	-1
#define DEV_WAN		0
#define DEV_WIFI	1
#define DEV_PWR		2
#define DEV_LAN		3
#define DEV_LAN1	4
#define DEV_LAN2	5
#define DEV_LAN3	6
#define DEV_LAN4	7
#define DEV_LAN5	8
#define DEV_ALL		9
#define DEV_INTEN	10
#define DEV_BINMASK	11

#define ST_DISABLE	0
#define ST_ENABLE	1
#define ST_AUTO		2

bool parse_device(const char *param, int *device);
bool parse_color(const char *param, unsigned int *color);
bool parse_status(const char *param, int *status);
bool parse_number(const char *param, unsigned char *number);

#endif //ARG_PARSER_H
