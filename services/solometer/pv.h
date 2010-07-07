/*
*
* Copyright (c) 2010 by Peter Hartmann <solometerklammeraffehartmann-peter.de>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 3
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
* For more information on the GPL, please go to:
* http://www.gnu.org/copyleft/gpl.html
*/

#include "config.h"
#include "core/debug.h"

// Genereller Kram

#define REBOOT_AFTER_DAYS 5
#define REBOOT_HOUR_0_23 1
#define REBOOT_MIN_0_59 37

// Do not set to zero!
#define MEAS_PERIOD_IN_SEC 2
#define POST_EVERY_MINS 5

// Messwert vom Wechselrichter
typedef struct {
    uint32_t zeitstempel;
    uint32_t curpow;
    uint16_t invtemp;
    uint16_t hstemp;
    uint16_t dc1v;
    uint16_t dc2v;
    uint16_t dc1i;
    uint16_t dc2i;
    uint32_t dc1p;
    uint32_t dc2p;
    uint32_t totpow;
    uint16_t nummeas;
} MESSWERT;

/*#ifndef PV_CALC_TINY
#define MESSPUFSIZE 256/sizeof(MESSWERT)
#else*/
#define MESSPUFSIZE 1
// #endif //PV_CALC_TINY

// Function prototypes

// From pv_effekta
void wr_read(void);
void wr_eval(void);

// From pv_measure
void messen(MESSWERT);

// From pv_serial
void pv_init(void);
void pv_periodic(void);
uint8_t pv_rxstart(uint8_t*, uint8_t);

#define PV_SERIAL_BUFFER_LEN 48

// From pv_inet
void post_file(void);

struct pv_serial_buffer {
  uint8_t len;
  uint8_t sent;
  uint8_t data[PV_SERIAL_BUFFER_LEN];
};

// Network related stuff
#define PV_NOTCONN 0x00
#define PV_IDLE 0x01
#define PV_PS1 0x02
#define PV_PS2 0x03
#define PV_DATA 0x04
#define PV_DATA_END 0x05
#define PV_PS3 0x06
#define PV_WAIT 0x07

// Data to be sent on the network
struct pdata {
    uint8_t stage;
    uint8_t sent;
    uint16_t length;
} post_data;
