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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "daemon.h"
#include "configuration.h"

#define BUFFSIZE 512
#define SLEEP_TIME_USEC 150000
#define FLASH_DELAY 100000
#define SNIFF_FILE_NET "/proc/net/dev"
#define MAX_LEN_INTERFACE_NAME 20
#define WIFI_DEV_NAME "wlan0:"
#define EPS 10

#define WIFI_HW_ENABLE 0
#define WIFI_HW_DISABLE 1

struct network_state {
	unsigned long long int last_r;
	unsigned long long int last_t;
	bool wifi_active;
};

static bool iteration_network(volatile unsigned char *mem, struct network_state *state) {
	char buffer[BUFFSIZE];
	FILE *f = fopen(SNIFF_FILE_NET, "r");
	if (f == NULL) return false;

	//Skip first 2 lines with header
	for (size_t i = 0; i < 2; i++) {
		if (fgets(buffer, BUFFSIZE, f) == NULL) {
			fclose(f);
			return false;
		}
	}

	char name[MAX_LEN_INTERFACE_NAME];
	unsigned long long int r_packets, t_packets, dummy;

	//Read all devices and find WIFI network
	while (fgets(buffer, BUFFSIZE, f) != NULL) {
		sscanf(buffer, "%20s%llu%llu%llu%llu%llu%llu%llu%llu%llu%llu%llu%llu%llu%llu%llu%llu",
			name, &dummy, &r_packets,
			&dummy, &dummy, &dummy, &dummy, &dummy, &dummy,
			&dummy, &t_packets,
			&dummy, &dummy, &dummy, &dummy, &dummy, &dummy
		);

		if (strcmp(WIFI_DEV_NAME, name) == 0) {
			//printf("R: [%llu, %llu, %llu] T:[%llu, %llu, %llu]\n", state->last_r, r_packets, (r_packets - state->last_r), state->last_t, t_packets, (t_packets - state->last_t));
			state->wifi_active = true;
			//kontrola zmeny hodnot
			if (((r_packets - state->last_r) > EPS) || ((t_packets - state->last_t) > EPS)) {
				//make flash
				mem[WIFI_HW_STATUS_REG] = WIFI_HW_DISABLE;
				usleep(FLASH_DELAY);
				mem[WIFI_HW_STATUS_REG] = WIFI_HW_ENABLE;
				//printf("Flashing!\n");

				state->last_r = r_packets;
				state->last_t = t_packets;
			}
			//Go to the end of loop
			break;
		}

		//If WIFI is found, this value set to true and the jump behind this is taken
		//So, the 'true' couldn't be overwritten.
		state->wifi_active = false;
	}

	if (state->wifi_active) {
		mem[WIFI_HW_STATUS_REG] = WIFI_HW_ENABLE;
	} else {
		mem[WIFI_HW_STATUS_REG] = WIFI_HW_DISABLE;
	}

	fclose(f);

	return true;
}

void do_some_daemon_stuff(volatile unsigned char *mem) {
	struct network_state network = { 0, 0, false };

	while (true) {
		iteration_network(mem, &network);
		usleep(SLEEP_TIME_USEC);
	}
}
