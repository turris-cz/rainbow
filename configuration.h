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

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#define G_STATUS_REG 0x23
#define G_OVERRIDE_REG 0x22

#define WAN_COLOR_R 0x13
#define WAN_COLOR_G 0x14
#define WAN_COLOR_B 0x15
#define WAN_MASK 0x01

#define LAN_COLOR_R 0x16
#define LAN_COLOR_G 0x17
#define LAN_COLOR_B 0x18
#define LAN_MASK 0x3E
#define LAN1_MASK 0x20
#define LAN2_MASK 0x10
#define LAN3_MASK 0x08
#define LAN4_MASK 0x04
#define LAN5_MASK 0x02

#define WIFI_COLOR_R 0x19
#define WIFI_COLOR_G 0x1A
#define WIFI_COLOR_B 0x1B
#define WIFI_MASK 0x40
#define WIFI_HW_STATUS_REG 0x08

#define PWR_COLOR_R 0x1C
#define PWR_COLOR_G 0x1D
#define PWR_COLOR_B 0x1E
#define PWR_MASK 0x80

#define BASE_REGISTER 0xFFA00000
#define MAPPED_SIZE 4096

#define PID_FILE_PATH "/var/run/rainbow.pid"

#endif //CONFIGURATION_H
