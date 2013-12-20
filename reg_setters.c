#include <stdbool.h>
#include <string.h>

#include "configuration.h"
#include "arg_parser.h"
#include "reg_setters.h"

static void get_rgb_parts(unsigned int color, unsigned char *r, unsigned char *g, unsigned char *b) {
	*r = ((color & 0xFF0000) >> 2*8);
	*g = ((color & 0x00FF00) >> 8);
	*b = (color & 0x0000FF);
}

bool set_color(volatile unsigned char *mem, const char *dev, unsigned int color) {
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

bool set_status(volatile unsigned char *mem, const char *dev, bool enable) {
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

bool set_intensity(volatile unsigned char *mem, unsigned char intensity) {
	(void) mem; (void) intensity;
#ifdef PARSER_DEBUG
	fprintf(stderr, "Setting intensity\tto %u\n", intensity);
#endif

	return true;
}
