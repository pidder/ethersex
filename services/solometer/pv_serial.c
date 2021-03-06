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
#include <stdio.h>
#include "services/clock/clock.h"
#include "protocols/uip/uip.h"
#include "services/solometer/pv.h"
#include "core/eeprom.h"

#define USE_USART 0
#define BAUD 9600
#include "core/usart.h"

/* We generate our own usart init module, for our usart port */
generate_usart_init()

extern char post_hostname[];
extern char post_cookie[];
extern uip_ipaddr_t post_hostip;
extern char post_scriptname[];
extern uint8_t WRID[];

struct pv_serial_buffer pv_recv_buffer;
struct pv_serial_buffer pv_send_buffer;
uint16_t expected_bytes;
uint32_t BOOT_TIME;
static uint16_t messperiode;

void
pv_init()
{
  usart_init();

  messperiode = 5 * MEAS_PERIOD_IN_SEC;
  BOOT_TIME = 0;
//#ifdef PV_WEBHOST_NAME
  memset(post_hostname,0,64);
  eeprom_restore(solometer_host, post_hostname, 64);
  //n = snprintf(post_hostname,63,PV_WEBHOST_NAME);
  debug_printf("Set webhost_name: --%s--\n",post_hostname);
//#endif //PV_WEBHOST_NAME
//#ifdef PV_WEBHOST_IP
/*  memset(post_hostip,0,16);
  eeprom_restore(solometer_hostip, post_hostip, 16);
  //debug_printf(PV_WEBHOST_IP "\n");
  int n;
  static int ip1,ip2,ip3,ip4;
  n = sscanf(post_hostip,"%d.%d.%d.%d",&ip1,&ip2,&ip3,&ip4);
  if(n == 4) {
    uip_ipaddr(&post_hostip,ip1,ip2,ip3,ip4);
    debug_printf("Set webhost_ip to %d.%d.%d.%d\n",ip1,ip2,ip3,ip4);
  } else {
    debug_printf("Webhost IP scan failed: n=%d %d %d %d %d\n",n,ip1,ip2,ip3,ip4);
  }*/
//#endif //PV_WEBHOST_IP
  memset(post_hostip,0,sizeof(uip_ipaddr_t));
  eeprom_restore(solometer_hostip, post_hostip, sizeof(uip_ipaddr_t));

  WRID[0] = 1;
  eeprom_restore(solometer_WRID, WRID, 1);

  //#ifdef PV_SOLOMETER_ID
  memset(post_cookie,0,11);
  eeprom_restore(solometer_cookie, post_cookie, 10);
  //n = snprintf(post_cookie,12,PV_SOLOMETER_ID);
  debug_printf("Set webhost_cookie --%s--\n",post_cookie);
//#endif
  memset(post_scriptname,0,64);
  eeprom_restore(solometer_script, post_scriptname, 63);
  //n = snprintf(post_scriptname,63,PV_WEBHOST_SCRIPT);
  debug_printf("Set webhost_script --%s--\n",post_scriptname);
}

void
pv_periodic()
{
  static uint16_t counter = 0;
  void *p;

  if(expected_bytes > 0 && pv_recv_buffer.len >= expected_bytes) {
    // Daten sind vollständig. Abspeichern.
    debug_printf("%u bytes received.\n",pv_recv_buffer.len);
    wr_eval();
    p = memset(pv_recv_buffer.data,0,PV_SERIAL_BUFFER_LEN);
    //bzero(pv_recv_buffer.data,PV_SERIAL_BUFFER_LEN);
    pv_recv_buffer.len = 0;
    expected_bytes = 0;
  }

  if(counter++ > messperiode) {
    if(expected_bytes > 0 && pv_recv_buffer.len < expected_bytes) {
      //Timeout. Buffer löschen. len auf Null.
      debug_printf("%u bytes received. --Timeout--.\n",pv_recv_buffer.len);
      p = memset(pv_recv_buffer.data,0,PV_SERIAL_BUFFER_LEN);
      //bzero(pv_recv_buffer.data,PV_SERIAL_BUFFER_LEN);
      pv_recv_buffer.len = 0;
      expected_bytes = 0;
      wr_eval();
    }
    counter = 0;
    wr_read();
  }
}

uint8_t
pv_rxstart(uint8_t *data, uint8_t len)
{
  uint8_t diff = pv_send_buffer.len - pv_send_buffer.sent;
  if (diff == 0) {
    /* Copy the data to the send buffer */
    memcpy(pv_send_buffer.data, data, len);
    pv_send_buffer.len = len;
    goto start_sending;
  /* The actual packet can be pushed into the buffer */
  } else if (((uint16_t) (diff + len)) < PV_SERIAL_BUFFER_LEN) {
    memmove(pv_send_buffer.data, pv_send_buffer.data + pv_send_buffer.sent, diff);
    memcpy(pv_send_buffer.data + diff, data, len);
    pv_send_buffer.len = diff + len;
    goto start_sending;
  }
  return 0;
start_sending:
    pv_send_buffer.sent = 1;
    /* Enable the tx interrupt and send the first character */
    usart(UCSR,B) |= _BV(usart(TXCIE));
    usart(UDR) = pv_send_buffer.data[0];
    return 1;
}

SIGNAL(usart(USART,_TX_vect))
{
  if (pv_send_buffer.sent < pv_send_buffer.len) {
    usart(UDR) = pv_send_buffer.data[pv_send_buffer.sent++];
  } else {
    /* Disable this interrupt */
    usart(UCSR,B) &= ~(_BV(usart(TXCIE)));
  }
}

SIGNAL(usart(USART,_RX_vect))
{
  /* Ignore errors */
  if ((usart(UCSR,A) & _BV(usart(DOR))) || (usart(UCSR,A) & _BV(usart(FE)))) {
    uint8_t v = usart(UDR);
    (void) v;
    return;
  }
  uint8_t data = usart(UDR);
  if (pv_recv_buffer.len < PV_SERIAL_BUFFER_LEN) {
    pv_recv_buffer.data[pv_recv_buffer.len++] = data;
    debug_printf("0x%02x received\n",pv_recv_buffer.data[pv_recv_buffer.len - 1]);
  }
}

/*
  -- Ethersex META --
  net_init(pv_init)
  header(services/solometer/pv.h)
  timer(10,pv_periodic())
*/
