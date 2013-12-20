#ifndef ARG_PARSER_H
#define ARG_PARSER_H

#include <stdbool.h>

#define DEV_WAN		"wan"
#define DEV_LAN		"lan"
#define DEV_WIFI	"wifi"
#define DEV_PWR		"pwr"

#define DEV_PSEUDO_ALL			"all"
#define DEV_PSEUDO_INTENSITY	"intensity"

#define PARAM_CMD_ENABLE "enable"
#define PARAM_CMD_DISABLE "disable"

struct color {
	const char *name;
	int rgb;
};

bool parse_device(const char *param);
bool parse_color(const char *param, unsigned int *color);
bool parse_status(const char *param, bool *status);
bool parse_intensity(const char *param, unsigned char *intensity);

#endif //ARG_PARSER_H
