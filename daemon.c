/*
 * Rainbow is tool for changing color and status of LEDs of the Turris router
 * Rainbow daemon provides indication of WiFi status by controlling its LED
 *
 * Copyright (C) 2013, 2015 CZ.NIC, z.s.p.o. (http://www.nic.cz/)
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
#include <time.h>

#include "daemon.h"
#include "configuration.h"
#include "arg_parser.h"
#include "reg_setters.h"

#define BUFFSIZE 512
#define SLEEP_TIME_USEC 150000
#define FLASH_DELAY 100000
#define SNIFF_FILE_NET "/proc/net/dev"
#define SNIFF_FILE_SMRT "/tmp/smrtd/eth2"
#define MAX_LEN_INTERFACE_NAME 20
#define MAX_LEN_STATUS 20
#define WIFI_DEV_NAME "wlan0:"
#define EPS 10

#define WIFI_HW_ENABLE 0
#define WIFI_HW_DISABLE 1

struct network_state {
	unsigned long long int last_r;
	unsigned long long int last_t;
	bool wifi_active;
};

struct smrt_state {
	int prev_state_cat;
	bool silent_second;
};

#define SMRT_STATE_OK 0
#define SMRT_STATE_ERR 1
#define SMRT_STATE_WAIT 2

struct smrt_status_map {
	const char *status;
	int status_type;
};

static struct smrt_status_map status_map[] = {
	{ "presence query", SMRT_STATE_WAIT },
	{ "upload firmware", SMRT_STATE_WAIT },
	{ "version query", SMRT_STATE_WAIT },
	{ "config", SMRT_STATE_WAIT },
	{ "activate", SMRT_STATE_WAIT },
	{ "no signal", SMRT_STATE_ERR },
	{ "reset", SMRT_STATE_WAIT },
	{ "not present", SMRT_STATE_OK },
	{ "idle", SMRT_STATE_ERR },
	{ "handshake", SMRT_STATE_WAIT },
	{ "training", SMRT_STATE_WAIT },
	{ "online", SMRT_STATE_OK },
	{ "CRC error", SMRT_STATE_ERR },
	{ "disabled", SMRT_STATE_ERR },
	{ NULL, 0 }
};

static int get_state_category(const char *state) {
	for (size_t i = 0; status_map[i].status != NULL; i++) {
		if (strcmp(state, status_map[i].status) == 0) {
			return status_map[i].status_type;
		}
	}

	return SMRT_STATE_ERR;
}

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
			state->wifi_active = true;
			if (((r_packets - state->last_r) > EPS) || ((t_packets - state->last_t) > EPS)) {
				//make flash
				mem[WIFI_HW_STATUS_REG] = WIFI_HW_DISABLE;
				usleep(FLASH_DELAY);
				mem[WIFI_HW_STATUS_REG] = WIFI_HW_ENABLE;

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

static bool iteration_smrt(volatile unsigned char *mem, struct smrt_state *ctx) {
	(void)mem; (void) ctx;
	char buffer[BUFFSIZE];
	char status[MAX_LEN_STATUS];
	int state_cat;

#ifdef DEBUG
	FILE *dbgf = fopen("/tmp/rainbow_debug", "a");
#endif

	FILE *f = fopen(SNIFF_FILE_SMRT, "r");
	if (f == NULL) {
		// This is not obvious: File is not exists if smrtd is not installed/enabled
		// AUTO mode is expected in this state
		state_cat = SMRT_STATE_OK;
	} else {

		while (fgets(buffer, BUFFSIZE, f) != NULL) {
			status[0] = '\0';
			if (sscanf(buffer, "<status>%[a-zA-Z ]20s</status>", status) > 0)
				break;
		}
		fclose(f);
#ifdef DEBUG
	fprintf(dbgf, "Obtained state: %s\n", status);
#endif
		state_cat = get_state_category(status);
	}

	long long int now = (long long int) time(NULL);
	ctx->silent_second = (now % 2) ? true : false;
#ifdef DEBUG
	fprintf(dbgf, "Now: %llu\n", now);
#endif

	if (state_cat == SMRT_STATE_OK) {
		// Switch WAN to AUTO mode only once
		// I don't want to rewrite user's rule 6 times per second :)
		if (ctx->prev_state_cat != SMRT_STATE_OK) {
			set_status(mem, DEV_WAN, ST_AUTO);
#ifdef DEBUG
			fprintf(dbgf, "LED: AAAAAA\n");
#endif
		}

	} else if (state_cat == SMRT_STATE_ERR) {
		if (ctx->silent_second) {
			set_status(mem, DEV_WAN, ST_DISABLE);
#ifdef DEBUG
			fprintf(dbgf, "LED: ______\n");
#endif
		} else {
			set_status(mem, DEV_WAN, ST_ENABLE);
#ifdef DEBUG
			fprintf(dbgf, "LED: FFFFFF\n");
#endif
			usleep(FLASH_DELAY);
			set_status(mem, DEV_WAN, ST_DISABLE);
#ifdef DEBUG
			fprintf(dbgf, "LED: ______\n");
#endif
		}
	} else if (state_cat == SMRT_STATE_WAIT) {
		if (ctx->silent_second) {
			set_status(mem, DEV_WAN, ST_DISABLE);
#ifdef DEBUG
			fprintf(dbgf, "LED: ______\n");
#endif
		} else {
			set_status(mem, DEV_WAN, ST_ENABLE);
#ifdef DEBUG
			fprintf(dbgf, "LED: FFFFFF\n");
#endif
		}
	}

	ctx->prev_state_cat = state_cat;

#ifdef DEBUG
	fclose(dbgf);
#endif

	return true;
}

void do_some_daemon_stuff(volatile unsigned char *mem) {
	struct network_state network = { 0, 0, false };
	struct smrt_state smrt = { SMRT_STATE_OK, false };

	while (true) {
		iteration_network(mem, &network);
		iteration_smrt(mem, &smrt);
		usleep(SLEEP_TIME_USEC);
	}
}
