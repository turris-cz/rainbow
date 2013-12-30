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

static struct option long_options[] = {
	{"help", no_argument, 0, 'h'},
	{"daemonize", no_argument, 0, 'D'},
	{0, 0, 0, 0}
};

void help() {
	fprintf(stderr,
		"Bad argument count\n"
		"TODO: Write help content\n"
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

	int i = optind;
	int last_device = DEV_UNDEF;
	int act_device = DEV_UNDEF;
	unsigned int color = 0;
	unsigned char number = 0;
	int status = 1;

	while(argv[i] != NULL) {
		if (parse_device(argv[i], &act_device)) {
			//Space for some special rules
			if (last_device == DEV_INTEN && act_device == DEV_INLVL) {
				last_device = DEV_INLVL;
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

		} else if (parse_number(argv[i], &number) && last_device != DEV_UNDEF) {
			if (last_device == DEV_INTEN) {
				printf("INTENSTITY not supported yet. (set intenstity to  %d)\n", number);
			} else if (last_device == DEV_INLVL) {
				printf("INTENSTITY LEVEL not supported yet. set intensity level to %d\n", number);
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
				fprintf(stderr, "nespecifikovane device v number\n");
			}
		} else {
			fprintf(stderr, "PARSE ERROR\n");
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

	//Clean-up phase
	if (munmap(mem, MAPPED_SIZE) < 0) {
		fprintf(stderr, "Unmap error\n");
	}

	close(mem_fd);

	if (was_input_error) {
		return 1;
	}

	return 0;
}
