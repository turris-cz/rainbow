#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "arg_parser.h"

struct color {
	const char *name;
	int rgb;
};

static struct color colors[] = {
	{ "red",	0xFF0000},
	{ "green",	0x00FF00},
	{ "blue",	0x0000FF},
	{ "white",	0xFFFFFF},
	{ "black",	0x000000},
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

bool parse_device(const char *param, int *device) {
	if (strcmp(param, KW_ALL) == 0) {
		*device = DEV_ALL;
	} else if (strcmp(param, KW_INTEN) == 0) {
		*device = DEV_INTEN;
	} else if (strcmp(param, KW_INLVL) == 0) {
		*device = DEV_INLVL;
	} else if (strcmp(param, KW_WAN) == 0) {
		*device = DEV_WAN;
	} else if (strcmp(param, KW_WIFI) == 0) {
		*device = DEV_WIFI;
	} else if (strcmp(param, KW_PWR) == 0) {
		*device = DEV_PWR;
	} else if (strcmp(param, KW_LAN) == 0) {
		*device = DEV_LAN;
	} else if (strcmp(param, KW_LAN1) == 0) {
		*device = DEV_LAN1;
	} else if (strcmp(param, KW_LAN2) == 0) {
		*device = DEV_LAN2;
	} else if (strcmp(param, KW_LAN3) == 0) {
		*device = DEV_LAN3;
	} else if (strcmp(param, KW_LAN4) == 0) {
		*device = DEV_LAN4;
	} else if (strcmp(param, KW_LAN5) == 0) {
		*device = DEV_LAN5;
	} else {
		return false;
	}

	return true;
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

bool parse_status(const char *param, int *status) {
	if (strcmp(param, KW_ENABLE) == 0) {
		*status = ST_ENABLE;
		return true;

	} else if (strcmp(param, KW_DISABLE) == 0) {
		*status = ST_DISABLE;
		return true;

	} else if (strcmp(param, KW_AUTO) == 0) {
		*status = ST_AUTO;
		return true;
	}

	return false;
}

bool parse_number(const char *param, unsigned char *number) {
	char *endptr = param;
	int tmp_number = strtol(param, &endptr, 10);

	if (param == endptr) {
		//There wheren't any number
		return false;
	}

	if (tmp_number < 0 || tmp_number > 255) {
		//Target value must be unsigned byte
		return false;
	}

	*number = tmp_number;

	return true;
}
