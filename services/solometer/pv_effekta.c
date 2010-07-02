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

uint16_t CALCUL_CRC(uint8_t *Msg, uint16_t lenght)
{
  uint16_t Crc;
  int16_t i,n;
  Crc = 0xFFFF;
  for ( i = 0 ; i < lenght ; i++ ) {
    Crc ^= Msg[i];
    for ( n = 1 ; n <= 8 ; n++) {
      /* if CRC is even */
      if ((Crc % 2) == 0)
	/* to right decrement */
	Crc >>= 1;
      else {
	Crc >>= 1;
	Crc ^= 0xA001;
      }
    }
  }
  return( Crc );
}

void
wr_read()
{

  /* ToDo: Get the right codes for 40 byte return. This only reads 16 bytes */
  //const uint8_t effekta_msg[] = { 0x01, 0x03, 0xC0, 0x20, 0x00, 0x10, 0x79, 0xCC };
  uint8_t effekta_msg[] = { 0x01, 0x03, 0xC0, 0x20, 0x00, 0x13, 0x00, 0x00 };
  uint8_t ret;
  void *p;
  uint16_t CRCsum;
  
  //bzero(pv_recv_buffer.data,PV_SERIAL_BUFFER_LEN);
  p = memset(pv_recv_buffer.data,0,PV_SERIAL_BUFFER_LEN);
  pv_recv_buffer.len = 0;
  CRCsum = CALCUL_CRC(effekta_msg,6);
  effekta_msg[6] = (unsigned char)(CRCsum & 0xff);
  effekta_msg[7] = (unsigned char)((CRCsum >> 8) & 0xff);
  expected_bytes = (((uint16_t)effekta_msg[4]<<8) + effekta_msg[5]) * 2 + 3;
  debug_printf("Sending %02x %02x %02x %02x %02x %02x %02x %02x.\n",
    effekta_msg[0],effekta_msg[1],effekta_msg[2],effekta_msg[3],
    effekta_msg[4],effekta_msg[5],effekta_msg[6],effekta_msg[7]);
  debug_printf("%d bytes expected.\n",expected_bytes);
  ret = pv_rxstart(effekta_msg,8);
}

void
wr_eval()
{
  MESSWERT result;

  // Byte 4=H,5=L 0xC020 Current Power output
  // Byte 22=H,23=L 0xC029 Inverter int. temp
  // Byte 24=H,25=L 0xC02A Inverter heatsink temp
  // Byte 26=H,27=L 0xC02B DC1 input voltage
  // Byte 28=H,29=L 0xC02C DC2 input voltage
  // Byte 30=H,31=L 0xC02D DC1 input current
  // Byte 32=H,33=L 0xC02E DC2 input current
  // Byte 34=H,35=L 0xC02F Input power A
  // Byte 36=H,37=L 0xC030 Input power B
  // Byte 38=H,39=L 0xC031 Total output power HIGH word
  // Byte 40=H,41=L 0xC032 Total output power LOW word

  result.curpow = 10*((((uint32_t)pv_recv_buffer.data[3])<<8) + (uint32_t)pv_recv_buffer.data[4]);
  result.invtemp = (((uint16_t)pv_recv_buffer.data[21])<<8) + (uint16_t)pv_recv_buffer.data[22];
  //result.invtemp = pv_recv_buffer.len;
  result.hstemp = (((uint16_t)pv_recv_buffer.data[23])<<8) + (uint16_t)pv_recv_buffer.data[24];
  result.dc1v = (((uint16_t)pv_recv_buffer.data[25])<<8) + (uint16_t)pv_recv_buffer.data[26];
  result.dc2v = (((uint16_t)pv_recv_buffer.data[27])<<8) + (uint16_t)pv_recv_buffer.data[28];
  result.dc1i = (((uint16_t)pv_recv_buffer.data[29])<<8) + (uint16_t)pv_recv_buffer.data[30];
  result.dc2i = (((uint16_t)pv_recv_buffer.data[31])<<8) + (uint16_t)pv_recv_buffer.data[32];
  result.totpow = (((uint32_t)pv_recv_buffer.data[37])<<24) + ((uint32_t)pv_recv_buffer.data[38]<<16);
  result.totpow += ((((uint32_t)pv_recv_buffer.data[39])<<8) + (uint32_t)pv_recv_buffer.data[40]);

  //debug_printf("curpow: %lu, totpow: %lu.\n",result.curpow, result.totpow);
  messen(result);
}
