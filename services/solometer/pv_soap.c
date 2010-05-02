/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
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

#include "pv.h"
#include "protocols/soap/soap.h"

float POWER_CAL = 0.987;
int16_t cal;

uint8_t soap_rpc_calfact(uint8_t len, soap_data_t *args, soap_data_t *result)
{
  if (len != 0) return 1;	/* we don't want args. */

  result->type = SOAP_TYPE_INT;
  result->u.d_int = -17;
  return 0;
}

uint8_t soap_rpc_GetCal(uint8_t len, soap_data_t *args, soap_data_t *result)
{
  if (len != 0) return 1;	/* we don't want args. */

  result->type = SOAP_TYPE_INT;
  result->u.d_int = cal;
  return 0;
}

uint8_t soap_rpc_SetCal(uint8_t len, soap_data_t *args, soap_data_t *result)
{
  uint8_t i;

  debug_printf("pv_soap Len: %u\n",len);

  for(i=0;i<len;i++) {
    if(args[i].type == SOAP_TYPE_INT) {
      debug_printf("pv_soap INT received: %d\n",args[i].u.d_int);
      cal = args[i].u.d_int;
    } else if(args[i].type == SOAP_TYPE_STRING) {
      debug_printf("pv_soap STRING received: --%s--\n",args[i].u.d_string);
      result->type = SOAP_TYPE_STRING;
      result->u.d_string = "ok";
      return 0;
    } else if(args[i].type == SOAP_TYPE_UINT32) {
      debug_printf("pv_soap UINT32 received: %lu\n",args[i].u.d_uint32);
    } else {
      debug_printf("pv_soap SOAP_TYPE %u unknown.\n",args[i].type);
    }
  }
  result->type = SOAP_TYPE_INT;
  result->u.d_int = 0;
  return 0;
}


/*
  -- Ethersex META --
  soap_rpc(soap_rpc_calfact, "calfact")
  soap_rpc(soap_rpc_GetCal, "GetCal")
  soap_rpc(soap_rpc_SetCal, "SetCal")
*/
