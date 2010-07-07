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

#include "services/clock/clock.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/parse.h"
#include "protocols/dns/resolv.h"

#include "services/solometer/pv.h"

extern uint8_t PV_MESSEN;
extern uint8_t PV_POSTEN;
extern uint8_t FILE_COMPLETE;
extern MESSWERT messpuf[];
extern uint16_t mwindex;
MESSWERT sendpuf[MESSPUFSIZE];
uint16_t spindex = 0;

char post_hostname[64];
uip_ipaddr_t post_ipaddr, *post_ipaddr_p;
char post_hostip[16];
char post_scriptname[64];
char post_filename[16] = {'1','2','3','4','5','6','7','8','.','M','C','P',0};
char post_cookie[16];

static uip_conn_t *post_conn;
static char PROGMEM ps1[] = "POST ";
// post_scriptname goes here
static char PROGMEM ps2[] = " HTTP/1.0\r\nHost: ";
// post_hostname goes here
static char PROGMEM ps3[] = "\r\nCookie: PVID=";
// post_cookie goes here
static char PROGMEM ps4[] = "\r\nContent-Type: multipart/form-data; boundary=bOunDaRy"
	"\r\nContent-Length: 160000\r\n\r\n";
static char PROGMEM poststr2[] = "--bOunDaRy\r\nContent-Disposition: form-data; name=\"file\"; filename=\"";
// post_filename goes here
static char PROGMEM poststr3[] = "\"\r\nContent-Type: application/octet-stream\r\n\r\n";
static char PROGMEM poststr4[] = "\r\n--bOunDaRy\r\n"
	"Content-Disposition: form-data; name=\"submit\"\r\n\r\nSubmit\r\n--bOunDaRy--";

void mcp_net_main()
{
  char *p;
  //static char puffer[80];
  char *puffer;
  static uint16_t rbp = 0,j;
  static uint32_t akt_std;
  MESSWERT tmpwert;
  struct clock_datetime_t zeit;

  puffer = (char *)malloc(96);
  if(!puffer) {
    debug_printf("mcp_net_main: not enough memory!\n");
    return;
  }

  if (uip_aborted() || uip_timedout()) {
    debug_printf("Connection aborted.\n");
    post_data.stage = PV_NOTCONN;
    post_conn = NULL;
  }
  if (uip_closed()) {
    debug_printf("Connection closed.\n");
    post_data.stage = PV_NOTCONN;
    post_conn = NULL;
  }
  if (uip_connected()) {
    debug_printf("Connected + PS1. MSS: %u\n",uip_mss());
    post_data.stage = PV_PS1;
    rbp = 0;
    tmpwert = sendpuf[rbp];
    clock_localtime(&zeit,tmpwert.zeitstempel);
    akt_std = tmpwert.zeitstempel;
    p = uip_sappdata;
    //p += sprintf(p,poststr1);
    p += strlcpy_P(p,ps1,strlen_P(ps1)+1);
    p += strlcpy(p,post_scriptname,strlen(post_scriptname)+1);
    //p += sprintf(p,"%s%s",ps1,post_scriptname);
    p += strlcpy_P(p,ps2,strlen_P(ps2)+1);
    p += strlcpy(p,post_hostname,strlen(post_hostname)+1);
    //p += sprintf(p,"%s%s",ps2,post_hostname);
    p += strlcpy_P(p,ps3,strlen_P(ps3)+1);
    p += strlcpy(p,post_cookie,strlen(post_cookie)+1);
    //p += sprintf(p,"%s%s",ps3,post_cookie);
    p += strlcpy_P(p,ps4,strlen_P(ps4)+1);
    //p += sprintf(p,"%s",ps4);
    post_data.length = p-(char *)uip_sappdata;
    uip_send(uip_sappdata,post_data.length);
    debug_printf("%s",uip_sappdata);
  }

  if (uip_rexmit ()) {
    debug_printf("Resending...\n");
    uip_send(uip_sappdata,post_data.length);

  } else if(post_data.stage == PV_PS1 && uip_acked()) {
    post_data.stage = PV_DATA;
    debug_printf("PS2...\n");
    p = uip_sappdata;
    p += strlcpy_P(p,poststr2,strlen_P(poststr2)+1);
    p += strlcpy(p,post_filename,strlen(post_filename)+1);
    p += strlcpy_P(p,poststr3,strlen_P(poststr3)+1);
    //p += sprintf(uip_sappdata,"%s%s%s",poststr2,post_filename,poststr3);
    post_data.length = p-(char *)uip_sappdata;
    uip_send(uip_sappdata,post_data.length);
    debug_printf("%s",uip_sappdata);

  } else if(post_data.stage == PV_DATA && uip_acked()) {
    debug_printf("DATA...\n");
    post_data.stage = PV_DATA;
    p = uip_sappdata;
    tmpwert = sendpuf[rbp];
    clock_localtime(&zeit,tmpwert.zeitstempel);
    j = sprintf_P(puffer,PSTR("%u.%u.%u %u:%u:%u %lu "),zeit.day,zeit.month,
	  zeit.year-100,zeit.hour,zeit.min,zeit.sec,tmpwert.curpow);
    j += sprintf_P(puffer+j,PSTR("%u %u %u %u %u %u %lu %lu %lu %u\n"),tmpwert.invtemp,tmpwert.hstemp,
	  tmpwert.dc1v,tmpwert.dc2v,tmpwert.dc1i,tmpwert.dc2i,tmpwert.dc1p,tmpwert.dc2p,tmpwert.totpow,tmpwert.nummeas);
    while((p-(char *)uip_sappdata + j < uip_mss()) && (post_data.stage == PV_DATA)) {
      //debug_printf("p:%u rbp:%u %u.%u.%u %u:%u:%u UTC\n",p-(char *)uip_sappdata,rbp,zeit.day,
	//zeit.month,zeit.year+1900,zeit.hour,zeit.min,zeit.sec);
      memcpy(p,puffer,strlen(puffer));
      p += j;
      rbp++;
      if(rbp >= spindex) {
	//Beenden
	debug_printf("Beende...\n");
	post_data.stage = PV_DATA_END;
      } else {
	tmpwert = sendpuf[rbp];
	clock_localtime(&zeit,tmpwert.zeitstempel);
	j = sprintf_P(puffer,PSTR("%u.%u.%u %u:%u:%u %lu "),zeit.day,zeit.month,
	    zeit.year-100,zeit.hour,zeit.min,zeit.sec,tmpwert.curpow);
	j += sprintf_P(puffer+j,PSTR("%u %u %u %u %u %u %lu %lu %lu %u\n"),tmpwert.invtemp,tmpwert.hstemp,
	    tmpwert.dc1v,tmpwert.dc2v,tmpwert.dc1i,tmpwert.dc2i,tmpwert.dc1p,tmpwert.dc2p,tmpwert.totpow,tmpwert.nummeas);
      }
    }
    post_data.length = p-(char *)uip_sappdata;
    uip_send(uip_sappdata,post_data.length);
    //debug_printf("%s",uip_sappdata);

  } else if(post_data.stage == PV_DATA_END && uip_acked()) {
    // Letztes Paket ist weg. Trailer senden
    post_data.stage = PV_PS3;
    debug_printf("PS3...\n");
    p = uip_sappdata;
    p += strlcpy_P(p,poststr4,strlen_P(poststr4)+1);
    //p += sprintf(uip_sappdata,"%s",poststr4);
    post_data.length = p-(char *)uip_sappdata;
    uip_send(uip_sappdata,post_data.length);
    debug_printf("%s",uip_sappdata);

  } else if(post_data.stage == PV_PS3 && uip_acked()) {
    post_data.stage = PV_IDLE;
    uip_close();
    FILE_COMPLETE--;
    debug_printf("Beende Verbindung...\n");
  }

  if(uip_newdata() && uip_len && post_data.stage == PV_IDLE) {
    debug_printf("NEWDATA...\n%s",uip_appdata);
  }

  free(puffer);
  return;
}

void
mcp_net_connect(char *name, uip_ipaddr_t *ipaddr)
{
  //char ip[20];
  //print_ipaddr(ipaddr,ip,16);
  //debug_printf("Host: %s IP: %s\n",name,ip);
  post_conn = uip_connect(ipaddr, HTONS(80), mcp_net_main);

  if (! post_conn)
    debug_printf("No uip_conn available.\n");
  return;
}

void
mcp_net_init()
{
  //uip_ipaddr_t *ip;

  // Falls noch keine Verbindung besteht ist post_conn=NULL
  if (! post_conn) {
#ifdef DNS_SUPPORT
    if (! (post_ipaddr_p = resolv_lookup(post_hostname))) {
      debug_printf("Hostname unknown. Starting Query.\n");
      resolv_query(post_hostname, mcp_net_connect);
    } else {
      debug_printf("Hostname known. Connecting...\n");
      mcp_net_connect(post_hostname, post_ipaddr_p);
    }
#else
    mcp_net_connect("Webhost", &post_ipaddr);
#endif
  }
  return;
}

void
post_file()
{
  MESSWERT tmpwert;
  struct clock_datetime_t zeit;

/*#ifndef PV_CALC_TINY
  for(spindex=0 ; spindex < mwindex ; spindex++) {
    sendpuf[spindex] = messpuf[spindex];
  }
#else*/
  sendpuf[0] = messpuf[0];
  spindex = 1;
// #endif

  PV_MESSEN = 0;
  tmpwert = sendpuf[0];
  clock_localtime(&zeit,tmpwert.zeitstempel);
  sprintf_P(post_filename,PSTR("%04x%04x.mcp"),(uint16_t)(tmpwert.zeitstempel>>16),(uint16_t)tmpwert.zeitstempel);
  debug_printf("Timestamp in Buffer: %u.%u.%u %u:%u:%u UTC\n",zeit.year+1900,
      zeit.month,zeit.day,zeit.hour,zeit.min,zeit.sec);

  debug_printf("\nSending File %s\n",post_filename);

  if(! post_conn)
    mcp_net_init();

  PV_MESSEN = 1;
}
