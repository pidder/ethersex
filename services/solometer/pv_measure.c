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
#include "services/solometer/pv.h"

MESSWERT messpuf[MESSPUFSIZE];
uint16_t mwindex = 0;

uint8_t FILE_COMPLETE = 0; // muss auf 0 initialisiert werden
uint8_t PV_MESSEN = 1;
uint8_t PV_POSTEN = 1;

void
messen(uint32_t messwert)
{
  struct clock_datetime_t zeit;
  //MESSWERT tmpwert;
  static struct clock_datetime_t zeit_vor = {0,{{0,0,0,0,0}},0};

  debug_printf("Messen aufgerufen %lu.\n",messwert);
  // Messung per Kommandozeile gestoppt
  if(PV_MESSEN == 0) {
    debug_printf("Messung gestoppt.\n");
    return;
  }

#ifndef PV_CALC_TINY
  messpuf[mwindex].zeitstempel = clock_get_time(); // UTC
  clock_localtime(&zeit, messpuf[mwindex].zeitstempel);
#else
  messpuf[0].zeitstempel = clock_get_time(); // UTC
  clock_localtime(&zeit, messpuf[0].zeitstempel);
#endif

  // Uhrzeit nicht aktuell (Läuft der NTP daemon nicht?)
  if(zeit.year < 110) {
    debug_printf("Uhr nicht gestellt.\n");
#ifndef SOLOMETER_WEB_DEBUG
    return;
#endif
  }
#ifndef SOLOMETER_WEB_DEBUG
  // Erster Durchlauf: Zeit_vor initialisieren.
  if(zeit_vor.year < 110) {
    zeit_vor = zeit;
  }
#endif

#ifndef PV_CALC_TINY
  MESSWERT tmpwert;
  // Das ist der Messwert
  messpuf[mwindex].wert1 = messwert;
  // Pufferzähler eins hoch
  mwindex++;

  // Am Ende der Messperiode Werte abspeichern
  debug_printf("Zeit.min: %d, Zeit_vor.min: %d\n",zeit.min,zeit_vor.min);
  if(!(zeit.min % POST_EVERY_MINS) && zeit.min != zeit_vor.min) {
    debug_printf("\nMessperiode beendet.\n");
    // Index eins zurück
    mwindex--;
    tmpwert = messpuf[mwindex];
    // Alle Werte vor diesem Wert speichern 
    FILE_COMPLETE++;
    post_file();
    // Diesen Wert an den Anfang kopieren
    messpuf[0] = tmpwert;
    // Index setzen
    mwindex = 1;
    zeit_vor = zeit;
  }

  // Falls der Rampuffer voll ist, letzten Wert überschreiben
  if((mwindex > MESSPUFSIZE-1)) {
    debug_printf("Messpuffer übergelaufen!\n");
    mwindex--;
  }
#else
  messpuf[0].wert1 = messpuf[0].wert1 + messwert;
  mwindex += 1;
  //debug_printf("Nach:Summe %lu Index %u.\n",messpuf[0].wert1,mwindex);

  // Am Ende der Messperiode Werte abspeichern
  debug_printf("Zeit.min: %d, Zeit_vor.min: %d\n",zeit.min,zeit_vor.min);
  if(!(zeit.min % POST_EVERY_MINS) && zeit.min != zeit_vor.min) {
    debug_printf("\nMessperiode beendet.\n");
    // Index eins zurück
    //mwindex--;
    //tmpwert = messpuf[mwindex];
    messpuf[0].wert1 /= mwindex;
    mwindex = 1;
    // Alle Werte vor diesem Wert speichern 
    FILE_COMPLETE++;
    debug_printf("Sende Mittelwert %u.\n",messpuf[0].wert1);
    status.request_reset = 1;
    post_file();
    // Diesen Wert an den Anfang kopieren
    messpuf[0].wert1 = messwert;
    // Index setzen
    //mwindex = 1;
    zeit_vor = zeit;
  }
#endif //PV_CALC_TINY
}
