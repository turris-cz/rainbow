#include <stdbool.h>
#include <string.h>

#include "configuration.h"
#include "arg_parser.h"
#include "reg_setters.h"

struct dev_config {
	unsigned char color_r;
	unsigned char color_g;
	unsigned char color_b;
	unsigned char override_reg;
	unsigned char override_mask;
	unsigned char status_reg;
	unsigned char status_mask;
};

//OK, this is a overkill, but i hope it is universal solution for future
static const struct dev_config devices[] = {
	{ WAN_COLOR_R, WAN_COLOR_G, WAN_COLOR_B, G_OVERRIDE_REG, WAN_MASK, G_STATUS_REG, WAN_MASK }, //DEV_WAN
	{ WIFI_COLOR_R, WIFI_COLOR_G, WIFI_COLOR_B, G_OVERRIDE_REG, WIFI_MASK, G_STATUS_REG, WIFI_MASK }, //DEV_WIFI
	{ PWR_COLOR_R, PWR_COLOR_G, PWR_COLOR_B, G_OVERRIDE_REG, PWR_MASK, G_STATUS_REG, PWR_MASK }, //DEV_PWR
	{ LAN_COLOR_R, LAN_COLOR_G, LAN_COLOR_B, G_OVERRIDE_REG, LAN_MASK, G_STATUS_REG, LAN_MASK }, //DEV_LAN
	{ 0, 0, 0, G_OVERRIDE_REG, LAN1_MASK, G_STATUS_REG, LAN1_MASK }, //DEV_LAN1
	{ 0, 0, 0, G_OVERRIDE_REG, LAN2_MASK, G_STATUS_REG, LAN2_MASK }, //DEV_LAN2
	{ 0, 0, 0, G_OVERRIDE_REG, LAN3_MASK, G_STATUS_REG, LAN3_MASK }, //DEV_LAN3
	{ 0, 0, 0, G_OVERRIDE_REG, LAN4_MASK, G_STATUS_REG, LAN4_MASK }, //DEV_LAN4
	{ 0, 0, 0, G_OVERRIDE_REG, LAN5_MASK, G_STATUS_REG, LAN5_MASK } //DEV_LAN5
	//All other devices are pseudodevices and they hasn't any configuration
};

static void get_rgb_parts(unsigned int color, unsigned char *r, unsigned char *g, unsigned char *b) {
	*r = ((color & 0xFF0000) >> 2*8);
	*g = ((color & 0x00FF00) >> 8);
	*b = (color & 0x0000FF);
}

void set_color(volatile unsigned char *mem, int dev, unsigned int color) {
	unsigned char r, g, b;
	get_rgb_parts(color, &r, &g, &b);

	mem[devices[dev].color_r] = r;
	mem[devices[dev].color_g] = g;
	mem[devices[dev].color_b] = b;
}

void set_status(volatile unsigned char *mem, int dev, int status) {
	//0 means enabled... It's kinda confusing

	if (status == ST_DISABLE) {
		mem[devices[dev].override_reg] |= devices[dev].override_mask;
		mem[devices[dev].status_reg] |= devices[dev].status_mask;
	} else if (status == ST_ENABLE) {
		mem[devices[dev].override_reg] |= devices[dev].override_mask;
		mem[devices[dev].status_reg] &= ~(devices[dev].status_mask);
	} else if (status == ST_AUTO) {
		mem[devices[dev].override_reg] &= ~(devices[dev].override_mask);
	}
}
