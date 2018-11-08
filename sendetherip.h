#ifndef __SEND__
#define __SEND__

#include "defines.h"

ip *fill_ip_packet(ip *ip_packet, uint16_t checksum);
void ip_transmit(ip *ip_packet, uint16_t checksum, char *name, unsigned char *nextmac, char *bufdata, int32_t datalength);

#endif