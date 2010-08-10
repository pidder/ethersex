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

#include <string.h>
#include "services/clock/clock.h"
#include "protocols/uip/uip.h"
#include "services/solometer/pv.h"
#include "services/ntp/ntp.h"

MESSWERT messpuf[MESSPUFSIZE];
uint16_t mwindex = 0;

uint8_t FILE_COMPLETE = 0; // muss auf 0 initialisiert werden
uint8_t PV_MESSEN = 1;
uint8_t PV_POSTEN = 1;
extern uint32_t BOOT_TIME;
extern void ntp_send_packet(void);

void
messen(MESSWERT messwert)
{
  struct clock_datetime_t zeit;
  static struct clock_datetime_t zeit_vor = {0,{{0,0,0,0,0}},0};
  static uip_ipaddr_t ntp_ipaddr;

  debug_printf("Messen aufgerufen.\n");

  // Messung per Kommandozeile gestoppt
  if(PV_MESSEN == 0) {
    debug_printf("Messung gestoppt.\n");
    return;
  }

  messpuf[0].zeitstempel = clock_get_time(); // UTC
  clock_localtime(&zeit, messpuf[0].zeitstempel);

  // Uhrzeit nicht aktuell (DNS??)
  if(zeit.year < 110) {
    debug_printf("Uhr nicht gestellt.\n");
#   ifndef SOLOMETER_WEB_DEBUG
    if(zeit.min > 1) {
      debug_printf("Trying PTB server 1.\n");
      // Try PTB servers ptbtime1.ptb.de ptbtime2.ptb.de
      // 192.53.103.108 und 192.53.103.104
      uip_ipaddr(&ntp_ipaddr, 192,53,103,108);
      ntp_conf(&ntp_ipaddr);
      ntp_send_packet();
    }
    if(zeit.min > 2) {
      debug_printf("Trying PTB server 2.\n");
      // Try PTB servers ptbtime1.ptb.de ptbtime2.ptb.de
      // 192.53.103.108 und 192.53.103.104
      uip_ipaddr(&ntp_ipaddr, 192,53,103,104);
      ntp_conf(&ntp_ipaddr);
      ntp_send_packet();
    }
    if(zeit.min > 3) {
      debug_printf("Trying 192.168.0.1.\n");
      uip_ipaddr(&ntp_ipaddr, 192,168,0,1);
      ntp_conf(&ntp_ipaddr);
      ntp_send_packet();
    }
    //Reboot, if no NTP conn. after 10 mins
    if(zeit.min > 10)
      status.request_reset = 1;
    return;
#   endif
  }
# ifndef SOLOMETER_WEB_DEBUG
  // Erster Durchlauf: Zeit_vor initialisieren.
  if(zeit_vor.year < 110) {
    zeit_vor = zeit;
  }
# endif

  if(BOOT_TIME == 0) {
    BOOT_TIME = messpuf[0].zeitstempel;
    debug_printf("Boot time set to %lx\n",BOOT_TIME);
  }

  messpuf[0].curpow += messwert.curpow;
  messpuf[0].invtemp += messwert.invtemp;
  messpuf[0].hstemp += messwert.hstemp;
  messpuf[0].dc1v += messwert.dc1v;
  messpuf[0].dc2v += messwert.dc2v;
  messpuf[0].dc1i += messwert.dc1i;
  messpuf[0].dc2i += messwert.dc2i;
  messpuf[0].dc1p += messwert.dc1p;
  messpuf[0].dc2p += messwert.dc2p;
  messpuf[0].totpow = messwert.totpow;
  mwindex += 1;

  // Am Ende der Messperiode Werte abspeichern
  debug_printf("Zeit.min: %d, Zeit_vor.min: %d\n",zeit.min,zeit_vor.min);
  if(!(zeit.min % POST_EVERY_MINS) && zeit.min != zeit_vor.min) {
    debug_printf("\nMessperiode beendet.\n");
    // Mittelwerte berechnen
    if(mwindex != 0) {
      messpuf[0].curpow /= mwindex;
      messpuf[0].invtemp /= mwindex;
      messpuf[0].hstemp /= mwindex;
      messpuf[0].dc1v /= mwindex;
      messpuf[0].dc2v /= mwindex;
      messpuf[0].dc1i /= mwindex;
      messpuf[0].dc2i /= mwindex;
      messpuf[0].dc1p /= mwindex;
      messpuf[0].dc2p /= mwindex;
      messpuf[0].nummeas = mwindex;
    }
    // Alle Werte vor diesem Wert speichern 
    FILE_COMPLETE++;
    debug_printf("Sende Mittelwert %u.\n",messpuf[0].curpow);
    post_file();
    // Neu initialisieren
    mwindex = 0;
    messpuf[0].curpow = 0;
    messpuf[0].invtemp = 0;
    messpuf[0].hstemp = 0;
    messpuf[0].dc1v = 0;
    messpuf[0].dc2v = 0;
    messpuf[0].dc1i = 0;
    messpuf[0].dc2i = 0;
    messpuf[0].dc1p = 0;
    messpuf[0].dc2p = 0;
    messpuf[0].totpow = 0;
    zeit_vor = zeit;
  }

  // Reboot or resync local clock
  if(zeit.hour == REBOOT_HOUR_0_23 && zeit.min == REBOOT_MIN_0_59) {
    if(messpuf[0].zeitstempel - BOOT_TIME > (uint32_t)REBOOT_AFTER_DAYS * 3600 * 24)
      status.request_reset = 1;
    else
      ntp_send_packet();
  }
}
