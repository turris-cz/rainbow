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

#include "configuration.h"
#include "arg_parser.h"
#include "reg_setters.h"

#define PARSER_DEBUG

int main(int argc, char **argv) {
	int retval = 0;
	if (argc < 3) {
		fprintf(stderr, "Bad agrument count\n");
		return 1;
	}

	//Open memory raw device
	int mem_fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (mem_fd < 0) {
		fprintf(stderr, "Cannot open memory RAW device\n");
		return 2;
	}

	//Map physical memory to virtual address space
	volatile unsigned char *mem = mmap(NULL, MAPPED_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, BASE_REGISTER);
	if (mem == NULL) {
		fprintf(stderr, "Memory map error.\n");
		return 2;
	}

	int i = 1;
	const char *last_device = NULL;
	bool all_devices = false;
	bool is_pseudo_device = false;
	unsigned int color = 0;
	int status = 1;
	unsigned char intensity = 0;

	while(argv[i] != NULL) {
		if (parse_device(argv[i])) {
			last_device = argv[i];

			if (strcmp(last_device, DEV_PSEUDO_INTENSITY) == 0 || strcmp(last_device, DEV_PSEUDO_ALL) == 0) {
				is_pseudo_device = true;
			} else {
				is_pseudo_device = false;
			}

			if (is_pseudo_device && strcmp(last_device, DEV_PSEUDO_ALL) == 0) {
				all_devices = true;
			} else {
				all_devices = false;
			}


		} else if (parse_status(argv[i], &status) && last_device != NULL) {
			//This part has to be before color parser because "enable" is valid color in color parser
			if (is_pseudo_device && all_devices) {
				set_status(mem, DEV_PWR, status);
				set_status(mem, DEV_WAN, status);
				set_status(mem, DEV_WIFI, status);
				set_status(mem, DEV_LAN, status);
			} else if (!is_pseudo_device) {
				set_status(mem, last_device, status);
			}

		} else if (parse_color(argv[i], &color) && last_device != NULL) {
			if (is_pseudo_device && all_devices) {
				set_color(mem, DEV_PWR, color);
				set_color(mem, DEV_WAN, color);
				set_color(mem, DEV_WIFI, color);
				set_color(mem, DEV_LAN, color);
			} else if (!is_pseudo_device) {
				set_color(mem, last_device, color);
			}

		} else if (parse_intensity(argv[i], &intensity) && last_device != NULL) {
			if (strcmp(last_device, DEV_PSEUDO_INTENSITY) == 0) {
				set_intensity(mem, intensity);
			}
		} else {
			fprintf(stderr, "BAD ARGUMENT\n");
			retval = 1;
			break;
		}
		i++;
	}

	//Clean-up phase
	if (munmap(mem, MAPPED_SIZE) < 0) {
		fprintf(stderr, "Unmap error\n");
	}

	close(mem_fd);

	return retval;
}
