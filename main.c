/*
 * Rainbow is tool for changing color and status of LEDs of the Turris router
 * Rainbow daemon provides indication of WiFi status by controlling its LED
 *
 * Copyright (C) 2013 CZ.NIC, z.s.p.o. (http://www.nic.cz/)
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
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <getopt.h>

#include "configuration.h"
#include "arg_parser.h"
#include "reg_setters.h"
#include "daemon.h"
#include "turris.h"

static struct option long_options[] = {
	{"help", no_argument, 0, 'h'},
	{"daemonize", no_argument, 0, 'D'},
	{0, 0, 0, 0}
};

void help() {
	fprintf(stdout,
		"Usage:\n"
		"  Show this help: rainbow --help or -h\n"
		"  Start as daemon: rainbow -D DEV_CONFIGURATION [DEV_CONFIGURATION ...]]\n"
		"  Set devices: rainbow DEV_CONFIGURATION [DEV_CONFIGURATION ...]\n"
		"\n"
		"DEV_CONFIGURATION is one of the next options:\n"
		"DEV COLOR STATUS or DEV STATUS COLOR or DEV STATUS or DEV COLOR, where:\n"
		"  DEV: 'wan' (LED of WAN port), 'lan' (LEDs of all LAN ports),\n"
		"       'wifi' (LED of WiFI), 'pwr' (LED of Power signalization)\n"
		"       or alias 'all' for all previous devices\n"
		"  COLOR: name of predefined color (red, blue, green, white, black)\n"
		"         or 3 bytes for RGB, so red is 'FF0000', green '00FF00'\n"
		"         blue '0000FF' etc.\n"
		"  STATUS: 'enable' (device is shining), 'disable' (device is off)\n"
		"          'auto' (device is operated by HW - typically flashing)\n"
		"\n"
		"DEV STATUS, where:\n"
		"  DEV: 'lan1', 'lan2', ..., 'lan5' (one of the LAN LEDs)\n"
		"  STATUS: the same meaning like above\n"
		"\n"
		"'intensity' NUMBER, where:\n"
		"  NUMBER is number from 0 to 7 that represents one of the predefined values of\n"
		"  light intensity\n"
		"\n"
		"'binmask' NUMBER, where:\n"
		"  NUMBER is number its binary representation is used as mark for status of\n"
		"  all of the LEDs. MSB is WAN LED and LSB is Power LED.\n"
		"\n"
		"'get' VALUE, where:\n"
		"  VALUE is 'intensity' (no more getters aren't available for now)\n"
		"\n"
		"Examples:\n"
		"rainbow all blue auto - reset status of all LEDs and set their color to blue\n"
		"rainbow all blue pwr red - set color of all LEDs to blue except the Power one\n"
		"rainbow all enable wan auto - all LEDs will shining except the LED of WAN port\n"
		"                              that will flashing according to traffic\n"



	);
}

int main(int argc, char **argv) {
	if (argc <= 1) {
		help();
		return 1;
	}

	//Parse options
	int c; //returned char
	bool daemonize = false;

	while ((c = getopt_long(argc, argv, "hD", long_options, NULL)) != -1) {
		switch (c) {
			case 'h':
				help();
				return 0;
				break;
			case 'D':
				daemonize = true;
				break;
		}
	}

	bool was_input_error = false;

#ifndef DEBUG
	//Open memory raw device
	int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (mem_fd < 0) {
		fprintf(stderr, "Cannot open memory RAW device\n");
		return 2;
	}

	//Map physical memory to virtual address space
	volatile unsigned char *mem = mmap(NULL, MAPPED_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, BASE_REGISTER);
#else
	volatile unsigned char *mem = malloc(MAPPED_SIZE);
#endif
	if (mem == NULL) {
		fprintf(stderr, "Memory map error.\n");
		return 2;
	}

	int i = optind;
	int last_device = DEV_UNDEF;
	int act_device = DEV_UNDEF;
	unsigned int color = 0;
	unsigned char number = 0;
	int status = 1;

	while(argv[i] != NULL) {
		if (parse_device(argv[i], &act_device)) {
			//Space for some special rules
			if (last_device == DEV_GET && act_device == DEV_INTEN) {
				printf("%d\n", mem[INTENSLVL_REG]);

			} else {
				//If there aren't any special rules, just set value
				last_device = act_device;
			}

		} else if (parse_status(argv[i], &status) && last_device != DEV_UNDEF) {
			//This part has to be before color parser because "enable" is valid color in color parser
			if (last_device == DEV_ALL) {
				set_status(mem, DEV_PWR, status);
				set_status(mem, DEV_WAN, status);
				set_status(mem, DEV_WIFI, status);
				set_status(mem, DEV_LAN, status);
			} else {
				set_status(mem, last_device, status);
			}

		} else if (parse_color(argv[i], &color) && last_device != DEV_UNDEF && last_device != DEV_LAN1 && last_device != DEV_LAN2 && last_device != DEV_LAN3 && last_device != DEV_LAN4 && last_device != DEV_LAN5) {
			if (last_device == DEV_ALL) {
				set_color(mem, DEV_PWR, color);
				set_color(mem, DEV_WAN, color);
				set_color(mem, DEV_WIFI, color);
				set_color(mem, DEV_LAN, color);
			} else {
				set_color(mem, last_device, color);
			}

		} else if (parse_turrisdefault(argv[i]) && last_device != DEV_UNDEF && last_device != DEV_LAN1 && last_device != DEV_LAN2 && last_device != DEV_LAN3 && last_device != DEV_LAN4 && last_device != DEV_LAN5) {
			unsigned int color = turris_get_default_color(turris_detect_version());
			if (last_device == DEV_ALL) {
				set_color(mem, DEV_PWR, color);
				set_color(mem, DEV_WAN, color);
				set_color(mem, DEV_WIFI, color);
				set_color(mem, DEV_LAN, color);
			} else {
				set_color(mem, last_device, color);
			}

		} else if (parse_number(argv[i], &number) && last_device != DEV_UNDEF) {
			if (last_device == DEV_INTEN) {
				//Condition (number >= 0 && number <= 7) causes warning
				if (number <= 7) {
					mem[INTENSLVL_REG] = number;
				} else {
					fprintf(stderr, "Parse error - number is not in range\nUse rainbow -h for help.\n");
				}
			} else if (last_device == DEV_BINMASK) {
				if (number & 0x80) set_status(mem, DEV_WAN, ST_ENABLE); else set_status(mem, DEV_WAN, ST_DISABLE);
				if (number & 0x40) set_status(mem, DEV_LAN1, ST_ENABLE); else set_status(mem, DEV_LAN1, ST_DISABLE);
				if (number & 0x20) set_status(mem, DEV_LAN2, ST_ENABLE); else set_status(mem, DEV_LAN2, ST_DISABLE);
				if (number & 0x10) set_status(mem, DEV_LAN3, ST_ENABLE); else set_status(mem, DEV_LAN3, ST_DISABLE);
				if (number & 0x08) set_status(mem, DEV_LAN4, ST_ENABLE); else set_status(mem, DEV_LAN4, ST_DISABLE);
				if (number & 0x04) set_status(mem, DEV_LAN5, ST_ENABLE); else set_status(mem, DEV_LAN5, ST_DISABLE);
				if (number & 0x02) set_status(mem, DEV_WIFI, ST_ENABLE); else set_status(mem, DEV_WIFI, ST_DISABLE);
				if (number & 0x01) set_status(mem, DEV_PWR, ST_ENABLE); else set_status(mem, DEV_PWR, ST_DISABLE);
			} else {
				fprintf(stderr, "Parse error - unspecified device for number.\nUse rainbow -h for help.\n");
				was_input_error = true;
				break;
			}
		} else {
			fprintf(stderr, "Parse error.\nUse rainbow -h for help.\n");
			was_input_error = true;
			break;
		}
		i++;
	}

	if (daemonize) {
		pid_t pid = fork();
		if (pid < 0) {
			fprintf(stderr, "Fork failed!\n");
			return 3;

		} else if (pid != 0) {
			FILE *pid_file = fopen(PID_FILE_PATH, "w");
			if (pid_file == NULL)  {
				fprintf(stderr, "Cannot create pid file.\n");
				return 3;
			}

			fprintf(pid_file, "%d\n", (int) pid);
			fclose(pid_file);

			return 0; //I'm parent

		} else {
			//TODO: Add some return code checking
			chdir("/");
			int dev_null_fd = open("/dev/null", O_RDWR);
			dup2(dev_null_fd, 0); //redirect stdin
			dup2(dev_null_fd, 1); //redirect stdout
			dup2(dev_null_fd, 2); //redirect stderr

			//Do some daemon stuff
			do_some_daemon_stuff(mem);
		}
	}

#ifndef DEBUG
	//Clean-up phase
	if (munmap((void *)mem, MAPPED_SIZE) < 0) {
		fprintf(stderr, "Unmap error\n");
	}

	close(mem_fd);
#endif

	if (was_input_error) {
		return 1;
	}

	return 0;
}
