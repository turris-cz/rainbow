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

#define WAN_COLOR_R 0x10
#define WAN_COLOR_G 0x11
#define WAN_COLOR_B 0x12

#define LAN_COLOR_R 0x13
#define LAN_COLOR_G 0x14
#define LAN_COLOR_B 0x15

#define WIFI_COLOR_R 0x16
#define WIFI_COLOR_G 0x17
#define WIFI_COLOR_B 0x18

#define PWR_COLOR_R 0x19
#define PWR_COLOR_G 0x1A
#define PWR_COLOR_B 0x1B

#define BASE_REGISTER 0xFFA00000
#define MAPPED_SIZE 4096

#define DEV_WAN		"wan"
#define DEV_LAN		"lan"
#define DEV_WIFI	"wifi"
#define DEV_PWR		"pwr"
#define DEV_ALL		"all"

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

int main(int argc, char **argv) {
	//For now is even count of arguments expected
	if (((argc - 1) % 2) != 0) {
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

	//argv[i] != NULL is because of SSH. SSHD puts some additional arguments
	for (int i = 1; argv[i] != NULL; i += 2) {
		//Debug...
		printf("%s - %s\n", argv[i], argv[i+1]);

		//Try to get RGB value from pre-defined color names
		unsigned int color;
		if (!get_color_from_name(argv[i+1], &color)) {
			//No predefiend color was found, convert user's input to integer
			color = (unsigned int) strtol(argv[i+1], NULL, 16);
		}

		//"all" is abbreviation in interface for all devices
		if (strcmp(argv[1], DEV_ALL) == 0) {
			set_color(mem, DEV_PWR, color);
			set_color(mem, DEV_WAN, color);
			set_color(mem, DEV_WIFI, color);
			set_color(mem, DEV_LAN, color);
			break;

		} else {
			set_color(mem, argv[i], color);
		}

	}

	//Clean-up phase
	if (munmap(mem, MAPPED_SIZE) < 0) {
		fprintf(stderr, "Unmap error\n");
	}

	close(mem_fd);

	return 0;
}
