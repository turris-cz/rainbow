#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "arg_parser.h"

static struct color colors[] = {
	{ "red",	 0xFF0000},
	{ "green",	 0x00FF00},
	{ "blue",	 0x0000FF},
	{ "white",       0xFFFFFF},
	{ "black",       0x000000},
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

bool parse_device(const char *param) {
	if (strcmp(param, DEV_PWR) == 0 || strcmp(param, DEV_WIFI) == 0 || strcmp(param, DEV_WAN) == 0 ||
		strcmp(param, DEV_LAN) == 0 || strcmp(param, DEV_PSEUDO_ALL) == 0 || strcmp(param, DEV_PSEUDO_INTENSITY) == 0) {
		return true;
	}

	return false;
}

bool parse_color(const char *param, unsigned int *color) {
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

bool parse_status(const char *param, bool *status) {
	if (strcmp(param, PARAM_CMD_ENABLE) == 0) {
		*status = true;
		return true;

	} else if (strcmp(param, PARAM_CMD_DISABLE) == 0) {
		*status = false;
		return true;
	}

	return false;
}

bool parse_intensity(const char *param, unsigned char *intensity) {
	if (strchr(param, '%') != NULL) {
		*intensity = strtol(param, NULL, 10);
		return true;
	}

	return false;
}
