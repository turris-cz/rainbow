/*
 * Rainbow is tool for changing color and status of LEDs of the Turris router
 * Rainbow daemon provides indication of WiFi status by controlling its LED
 *
 * Copyright (C) 2014 CZ.NIC, z.s.p.o. (http://www.nic.cz/)
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "turris.h"
#include "configuration.h"

#define TURRIS_GET_VERSION_CMD "atsha204cmd serial-number"
// Read only revision part of SN
#define TURRIS_SN_BUFFSIZE 9

int turris_detect_version() {
	char snc[TURRIS_SN_BUFFSIZE];
	FILE *atshacmd = popen(TURRIS_GET_VERSION_CMD, "r");
	char *ret = fgets(snc, TURRIS_SN_BUFFSIZE, atshacmd);
	pclose(atshacmd);
	if (ret == NULL)
		return TURRIS_VERSION_ERR;
	long int sn = strtol(snc, NULL, 16);
	if (sn == 0)
		return TURRIS_VERSION_ERR;
	if (sn > 0x00000005)
		return TURRIS_VERSION_1_1;

	return TURRIS_VERSION_1_0;
}

unsigned int turris_get_default_color(int turris_version) {
	switch (turris_version) {
		case TURRIS_VERSION_1_0:
			return 0x33FF33;
		case TURRIS_VERSION_1_1:
			return 0xFFFFFF;
	}

	// In case of error return "real" white
	return 0xFFFFFF;
}
