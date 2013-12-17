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

#define GLOBAL_STATUS_REG 0x08

#define WAN_COLOR_R 0x10
#define WAN_COLOR_G 0x11
#define WAN_COLOR_B 0x12
#define WAN_STATUS_REG GLOBAL_STATUS_REG
#define WAN_STATUS_MASK 0x01

#define LAN_COLOR_R 0x13
#define LAN_COLOR_G 0x14
#define LAN_COLOR_B 0x15
#define LAN_STATUS_REG GLOBAL_STATUS_REG
#define LAN_STATUS_MASK 0x01

#define WIFI_COLOR_R 0x16
#define WIFI_COLOR_G 0x17
#define WIFI_COLOR_B 0x18
#define WIFI_STATUS_REG GLOBAL_STATUS_REG
#define WIFI_STATUS_MASK 0x01

#define PWR_COLOR_R 0x19
#define PWR_COLOR_G 0x1A
#define PWR_COLOR_B 0x1B
#define PWR_STATUS_REG GLOBAL_STATUS_REG
#define PWR_STATUS_MASK 0x01

#define BASE_REGISTER 0xFFA00000
#define MAPPED_SIZE 4096

#define DEV_WAN		"wan"
#define DEV_LAN		"lan"
#define DEV_WIFI	"wifi"
#define DEV_PWR		"pwr"

#define DEV_PSEUDO_ALL			"all"
#define DEV_PSEUDO_INTENSITY	"intensity"

#define PARAM_CMD_ENABLE "enable"
#define PARAM_CMD_DISABLE "disable"

#define PARSER_DEBUG

struct color {
	const char *name;
	int rgb;
};

static struct color colors[] = {
	{ "red",	 0xFF0000},
	{ "green",	 0x00FF00},
	{ "blue",	 0x0000FF},
	{ NULL,	0}
};

static bool get_color_from_name(const char *color_name, unsigned int *color) {
	for (size_t i = 0; colors[i].name != NULL; i++) {
		if (strcmp(color_name, colors[i].name) == 0) {
			*color = colors[i].rgb;
			return true;
		}
	}

	return false;
}

static void get_rgb_parts(unsigned int color, unsigned char *r, unsigned char *g, unsigned char *b) {
	*r = ((color & 0xFF0000) >> 2*8);
	*g = ((color & 0x00FF00) >> 8);
	*b = (color & 0x0000FF);
}

static bool set_color(unsigned char *mem, const char *dev, unsigned int color) {
#ifdef PARSER_DEBUG
	fprintf(stderr, "Setting color\tof device %s\tto 0x%06X\n", dev, color);
#endif
	unsigned char r, g, b;
	get_rgb_parts(color, &r, &g, &b);

	if (strcmp(dev, DEV_PWR) == 0) {
		mem[PWR_COLOR_R] = r;
		mem[PWR_COLOR_G] = g;
		mem[PWR_COLOR_B] = b;

	} else if (strcmp(dev, DEV_WAN) == 0) {
		mem[WAN_COLOR_R] = r;
		mem[WAN_COLOR_G] = g;
		mem[WAN_COLOR_B] = b;

	} else if (strcmp(dev, DEV_WIFI) == 0) {
		mem[WIFI_COLOR_R] = r;
		mem[WIFI_COLOR_G] = g;
		mem[WIFI_COLOR_B] = b;

	} else if (strcmp(dev, DEV_LAN) == 0) {
		mem[LAN_COLOR_R] = r;
		mem[LAN_COLOR_G] = g;
		mem[LAN_COLOR_B] = b;

	} else {
		return false;
	}

	return true;
}

static bool set_status(unsigned char *mem, const char *dev, bool enable) {
#ifdef PARSER_DEBUG
	fprintf(stderr, "Setting status\tof device %s\tto %d\n", dev, enable);
#endif
	//0 means enabled... It's kinda confusing
	unsigned char maskable_value;
	if (enable) {
		maskable_value = 0x00;
	} else {
		maskable_value = 0xFF;
	}

	if (strcmp(dev, DEV_PWR) == 0) {
		mem[PWR_STATUS_REG] = PWR_STATUS_MASK & maskable_value;

	} else if (strcmp(dev, DEV_WAN) == 0) {
		mem[WAN_STATUS_REG] = WAN_STATUS_MASK & maskable_value;

	} else if (strcmp(dev, DEV_WIFI) == 0) {
		mem[WIFI_STATUS_REG] = WIFI_STATUS_MASK & maskable_value;

	} else if (strcmp(dev, DEV_LAN) == 0) {
		mem[LAN_STATUS_REG] = LAN_STATUS_MASK & maskable_value;

	} else {
		return false;
	}

	return true;
}

static bool set_intensity(unsigned char *mem, unsigned char intensity) {
#ifdef PARSER_DEBUG
	fprintf(stderr, "Setting intensity\tto %u\n", intensity);
#endif

	return true;
}

static bool parse_device(const char *param) {
	if (strcmp(param, DEV_PWR) == 0 || strcmp(param, DEV_WIFI) == 0 || strcmp(param, DEV_WAN) == 0 ||
		strcmp(param, DEV_LAN) == 0 || strcmp(param, DEV_PSEUDO_ALL) == 0 || strcmp(param, DEV_PSEUDO_INTENSITY) == 0) {
		return true;
	}

	return false;
}

static bool parse_color(const char *param, unsigned int *color) {
	//Is param pre-defined color?
	if (get_color_from_name(param, color)) {
		return true;
	}

	//Is param color in format AABBCC?
	if (strlen(param) == 6) {
		*color = (unsigned int) strtol(param, NULL, 16);
		return true;
	}

	//OK, it is not color
	return false;
}

static bool parse_status(const char *param, bool *status) {
	if (strcmp(param, PARAM_CMD_ENABLE) == 0) {
		*status = true;
		return true;

	} else if (strcmp(param, PARAM_CMD_DISABLE) == 0) {
		*status = false;
		return true;
	}

	return false;
}

static bool parse_intensity(const char *param, unsigned char *intensity) {
	if (strchr(param, '%') != NULL) {
		*intensity = strtol(param, NULL, 10);
		return true;
	}

	return false;
}

int main(int argc, char **argv) {
	int retval = 0;
	//For now is even count of arguments expected
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
	unsigned char *mem = mmap(NULL, MAPPED_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, BASE_REGISTER);
	if (mem == NULL) {
		fprintf(stderr, "Memory map error.\n");
		return 2;
	}

	int i = 1;
	const char *last_device = NULL;
	bool all_devices = false;
	bool is_pseudo_device = false;
	unsigned int color = 0;
	bool enable = true;
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


		} else if (parse_status(argv[i], &enable) && last_device != NULL) {
			//This part has to be before color parser because "enable" is valid color in color parser
			if (is_pseudo_device && all_devices) {
				set_status(mem, DEV_PWR, enable);
				set_status(mem, DEV_WAN, enable);
				set_status(mem, DEV_WIFI, enable);
				set_status(mem, DEV_LAN, enable);
			} else if (!is_pseudo_device) {
				set_status(mem, last_device, enable);
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
