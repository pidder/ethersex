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

#include <stdlib.h>
#include "pv.h"
#include "protocols/soap/soap.h"

extern char post_hostname[];
extern char post_scriptname[];
extern char post_cookie[];
double POWER_CAL = 1.0;

uint8_t soap_rpc_SetUplScr(uint8_t len, soap_data_t *args, soap_data_t *result)
{
  int n;
  if(len != 1 || args[0].type != SOAP_TYPE_STRING) {
    debug_printf("SetUplScr: len =! 1 or wrong SOAP_TYPE\n");
    return 1;
  }
  n = snprintf(post_scriptname,63,args[0].u.d_string);
  if(n>0) {
    result->type = SOAP_TYPE_INT;
    result->u.d_int = 0;
  }
  return 0;
}

uint8_t soap_rpc_GetUplScr(uint8_t len, soap_data_t *args, soap_data_t *result)
{
  if (len != 0) return 1;	/* we don't want args. */

  result->type = SOAP_TYPE_STRING;
  result->u.d_string = post_scriptname;
  return 0;
}

uint8_t soap_rpc_SetWebHstNm(uint8_t len, soap_data_t *args, soap_data_t *result)
{
  int n;
  if(len != 1 || args[0].type != SOAP_TYPE_STRING) {
    debug_printf("SetUplScr: len =! 1 or wrong SOAP_TYPE\n");
    return 1;
  }
  debug_printf("pv_soap: args[0].u.d_string: --%s--\n",args[0].u.d_string);
  n = snprintf(post_hostname,63,args[0].u.d_string);
  if(n>0) {
    result->type = SOAP_TYPE_INT;
    result->u.d_int = 0;
  }
  debug_printf("pv_soap: post_hostname set: --%s--\n",post_hostname);
  return 0;
}

uint8_t soap_rpc_GetWebHstNm(uint8_t len, soap_data_t *args, soap_data_t *result)
{
  if (len != 0) return 1;	/* we don't want args. */

  result->type = SOAP_TYPE_STRING;
  result->u.d_string = post_hostname;
  return 0;
}

uint8_t soap_rpc_SetCal(uint8_t len, soap_data_t *args, soap_data_t *result)
{
  int n;
  if(len != 1 || args[0].type != SOAP_TYPE_STRING) {
    debug_printf("SetUplScr: len =! 1 or wrong SOAP_TYPE\n");
    return 1;
  }
  POWER_CAL = atof(args[0].u.d_string);
  if(POWER_CAL > 0) {
    result->type = SOAP_TYPE_INT;
    result->u.d_int = 0;
  }
  return 0;
}

uint8_t soap_rpc_GetCal(uint8_t len, soap_data_t *args, soap_data_t *result)
{
  static char s[12];
  int n;

  if (len != 0) return 1;	/* we don't want args. */

  result->type = SOAP_TYPE_STRING;
  n = snprintf(s,10,"%.3f",POWER_CAL);
  result->u.d_string = s;
  return 0;
}

uint8_t soap_rpc_SetCookie(uint8_t len, soap_data_t *args, soap_data_t *result)
{
  int n;
  if(len != 1 || args[0].type != SOAP_TYPE_STRING) {
    debug_printf("SetUplScr: len =! 1 or wrong SOAP_TYPE\n");
    return 1;
  }
  debug_printf("pv_soap: args[0].u.d_string: --%s--\n",args[0].u.d_string);
  n = snprintf(post_cookie,12,args[0].u.d_string);
  if(n>0) {
    result->type = SOAP_TYPE_INT;
    result->u.d_int = 0;
  }
  debug_printf("pv_soap: post_cookie set: --%s--\n",post_cookie);
  return 0;
}

uint8_t soap_rpc_GetCookie(uint8_t len, soap_data_t *args, soap_data_t *result)
{
  if (len != 0) return 1;	/* we don't want args. */

  result->type = SOAP_TYPE_STRING;
  result->u.d_string = post_cookie;
  return 0;
}

/*
  -- Ethersex META --
  soap_rpc(soap_rpc_GetCal, "GetCal")
  soap_rpc(soap_rpc_SetCal, "SetCal")
  soap_rpc(soap_rpc_SetUplScr, "SetUplScr")
  soap_rpc(soap_rpc_GetUplScr, "GetUplScr")
  soap_rpc(soap_rpc_SetWebHstNm, "SetWebHstNm")
  soap_rpc(soap_rpc_GetWebHstNm, "GetWebHstNm")
  soap_rpc(soap_rpc_SetCookie, "SetCookie")
  soap_rpc(soap_rpc_GetCookie, "GetCookie")
*/
