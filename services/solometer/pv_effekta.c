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

extern uint16_t expected_bytes;
extern struct pv_serial_buffer pv_recv_buffer;
const uint8_t effekta_msg[] = { 0x01, 0x03, 0xC0, 0x20, 0x00, 0x10, 0x79, 0xCC };

void
wr_read()
{

  uint8_t ret;
  void *p;

  //bzero(pv_recv_buffer.data,PV_SERIAL_BUFFER_LEN);
  p = memset(pv_recv_buffer.data,0,PV_SERIAL_BUFFER_LEN);
  pv_recv_buffer.len = 0;
  expected_bytes = (effekta_msg[4]*0x100+effekta_msg[5])*2;
  debug_printf("%d bytes expected.\n",expected_bytes);
  ret = pv_rxstart(effekta_msg,8);
}

void
wr_eval()
{
  uint32_t result;

  result = 10*(((uint32_t)pv_recv_buffer.data[3])*0x100 + (uint32_t)pv_recv_buffer.data[4]);
  debug_printf("%lu power.\n",result);
  messen(result);
}
