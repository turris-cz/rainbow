#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#define G_STATUS_REG 0x23
#define G_OVERRIDE_REG 0x22

#define WAN_COLOR_R 0x13
#define WAN_COLOR_G 0x14
#define WAN_COLOR_B 0x15
#define WAN_MASK 0x01

#define LAN_COLOR_R 0x16
#define LAN_COLOR_G 0x17
#define LAN_COLOR_B 0x18
#define LAN_MASK 0x3E
#define LAN1_MASK 0x20
#define LAN2_MASK 0x10
#define LAN3_MASK 0x08
#define LAN4_MASK 0x04
#define LAN5_MASK 0x02

#define WIFI_COLOR_R 0x19
#define WIFI_COLOR_G 0x1A
#define WIFI_COLOR_B 0x1B
#define WIFI_MASK 0x40

#define PWR_COLOR_R 0x1C
#define PWR_COLOR_G 0x1D
#define PWR_COLOR_B 0x1E
#define PWR_MASK 0x80

#define BASE_REGISTER 0xFFA00000
#define MAPPED_SIZE 4096

#endif //CONFIGURATION_H
