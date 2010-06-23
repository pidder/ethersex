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

#include "handle_solometer.h"

// Parameters to be set by the Web-Frontend
/*#define NUM_PAR 4
struct param parameter[] = {
// Beschreibung 	Name 	Typ	RAM		maxlen	EEPROM
{ "Sol-O-Meter ID:",	"ID",	"text",	post_cookie,	10,	solometer_cookie},
{ "Webhost Name:",	"HST",	"text",	post_hostname,	63,	solometer_host},
{ "Webhost IP:",	"IP",	"text",	post_hostip,	15,	solometer_hostip},
{ "Webhost script:",	"SCR",	"text",	post_scriptname,63,	solometer_script}
};
*/
static char PROGMEM p1[] = "HTTP/1.1 200 OK\n"
"Host: solometer.local\n"
"Content-Length: 1000\n";
static char PROGMEM p1a[] = "Content-Type: text/html; charset=utf-8\n\n"
"<html>\n<head>\n"
"<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n";
static char PROGMEM p1b[] = "<title>SOL-O-METER CONFIGURATION</title>\n"
"</head>\n<body>\n"
"<form action=\"http://";
// My Ipaddr goes here
static char PROGMEM p2[] = "/solometer\" method=\"get\" accept-charset=\"utf-8\">\n";
// Inputs go here
static char PROGMEM p3[] = "  <input type=\"submit\" value=\" Absenden \">\n"
"</form>\n"
"</BODY>\n</HTML>\n";

char PROGMEM httpd_header_500_smt[] =
"HTTP/1.1 500 Server Error\n"
"Connection: close\n";

// START Decode GET Method-sent Form data
static char *in, c, c1, c2;
void next()
{
  if(c == 0)
     return;

  c = c1;
  c1 = c2;
  c2 = *(in++);
}

int urldecode(char* ptr)
{
  char *out;
  
  in = ptr;
  out = ptr;

  c = *(in++);
  c1 = 0;
  c2 = 0;

  if(c != 0) {
    c1 = *(in++);
    if(c1 != 0)
      c2 = *(in++);
  }

  while( c != 0) {
    if(c == '%' ) {
      if( isxdigit((int)c1) && isxdigit((int)c2) ){
	c1 = tolower((int)c1);
	c2 = tolower((int)c2);

	if( c1 <= '9' )
	  c1 = c1 - '0';
	else
	  c1 = c1 - 'a' + 10;
	if( c2 <= '9' )
	  c2 = c2 - '0';
	else
	  c2 = c2 - 'a' + 10;

	*(out++) = 16 * c1 + c2;

	next();
	next();
	next();
      } else {
	*(out++) = '%';
	next();
      }
    } else if( c == '+' ) {
      *(out++) = ' ';
      next();
    } else {
      *(out++) = c;
      next();
    }
  }
  *out = 0;
  return 0;
}
// END Decode GET Method-sent Form data

int
solometer_parse (char* ptr)
{
  char *ptr1,*ptr2,c;
  
  ptr1 = strchr (ptr, ' ');
  if (ptr1 == NULL) {
    debug_printf ("smt_parse: space after filename not found.\n");
    return -1;
  }
  *ptr1 = 0;

  debug_printf("String to parse: --%s--\n",ptr);
  urldecode(ptr);
  
  ptr1 = strstr_P(ptr,PSTR("ID="));
  if(ptr1 != NULL) {
    ptr1 += 3;
    ptr2 = strchrnul(ptr1,'&');
    if(ptr2 != ptr1) {
      c = *ptr2;
      *ptr2 = 0;
      ptr1 = strncpy(post_cookie,ptr1,10);
      *ptr2 = c;
      post_cookie[10] = 0;
      eeprom_save(solometer_cookie, post_cookie, strlen(post_cookie) + 1);
      eeprom_update_chksum();
    }
  }

  debug_printf("Ausgewertet:PVID=--%s--\n",post_cookie);
  
  ptr1 = strstr_P(ptr,PSTR("HST="));
  if(ptr1 != NULL) {
    ptr1 += 4;
    ptr2 = strchrnul(ptr1,'&');
    if(ptr2 != ptr1) {
      c = *ptr2;
      *ptr2 = 0;
      ptr1 = strncpy(post_hostname,ptr1,63);
      *ptr2 = c;
      post_hostname[63] = 0;
      eeprom_save(solometer_host, post_hostname, strlen(post_hostname) + 1);
      eeprom_update_chksum();
    }
  }

  debug_printf("Ausgewertet:HST=--%s--\n",post_hostname);
  
  ptr1 = strstr_P(ptr,PSTR("SCR="));
  if(ptr1 != NULL) {
    ptr1 += 4;
    ptr2 = strchrnul(ptr1,'&');
    if(ptr2 != ptr1) {
      c = *ptr2;
      *ptr2 = 0;
      ptr1 = strncpy(post_scriptname,ptr1,63);
      *ptr2 = c;
      post_scriptname[63] = 0;
      eeprom_save(solometer_script, post_scriptname, strlen(post_scriptname) + 1);
      eeprom_update_chksum();
    }
  }

  debug_printf("Ausgewertet:SCRPT=--%s--\n",post_scriptname);
  return 0;
}

void
httpd_handle_solometer (void)
{
  static uint8_t cont_send, parsing = 0;
  int i = 0, mss;
  uip_ipaddr_t hostaddr;
  //char *p;

  debug_printf("Handle_solometer called.\n");
  mss = uip_mss();

  if (uip_newdata()) {
    /* We've received new data (maybe even the first time).  We'll
      receive something like this:
      GET /solometer[?...]
    */
    /* Make sure it's zero-terminated, so we can safely use strstr */
    char *ptr = (char *)uip_appdata;
    ptr[uip_len] = 0;

    debug_printf("Newdata: ---------\n%s\n-----------\n",ptr);

    if(strncasecmp_P (uip_appdata, PSTR ("GET /solometer"),14) == 0) {
      debug_printf("This is the GET request header...\n");
      ptr = strstr_P (uip_appdata, PSTR("?")) + 1;
      if(!ptr || *ptr == 0) {
	debug_printf("This is a request only. Send page.\n");
	i = 0;
      } else {
	debug_printf("This is a set operation. Parsing...\n");
	i = solometer_parse(ptr);
      }
      parsing = 1;
    }

    if (parsing == 1) {
      // Do not start answering until all packets have arrived
      ptr = strstr_P (ptr, PSTR("\r\n\r\n"));
      if (ptr) {
	parsing = 2;
      }
    }

    if (parsing == 2) {
      PASTE_RESET ();
      if(i || mss < 200) {
	PASTE_P (httpd_header_500_smt);
	cont_send = 0;
      } else {
	PASTE_P (p1);
	cont_send = 1;
      }
      debug_printf("%d: %s\n",cont_send,uip_appdata);
      PASTE_SEND ();
      parsing = 0;
      return;
    }
  }

  if(uip_acked()) {
    PASTE_RESET();
    switch(cont_send) {
      case 0:
	uip_close();
	break;
      case 1:
	if(strlen(uip_appdata) + strlen_P(p1a) > mss) {
	  debug_printf("%d: %s\n",cont_send,uip_appdata);
	  PASTE_SEND();
	  break;
	} else {
	  PASTE_P(p1a);
	  cont_send++;
	}
      case 2:
	if(strlen(uip_appdata) + strlen_P(p1b) > mss) {
	  debug_printf("%d: %s\n",cont_send,uip_appdata);
	  PASTE_SEND();
	  break;
	} else {
	  PASTE_P(p1b);
	  cont_send++;
	}
      case 3:
	if(strlen(uip_appdata) + 16 > mss) {
	  debug_printf("%d: %s\n",cont_send,uip_appdata);
	  PASTE_SEND();
	  break;
	} else {
	  uip_gethostaddr(&hostaddr);
	  uint8_t *ip = (uint8_t *) &hostaddr;
	  snprintf_P(uip_appdata + strlen(uip_appdata),16, PSTR("%u.%u.%u.%u"),ip[0], ip[1], ip[2], ip[3]);
	  cont_send++;
	}
      case 4:
	if(strlen(uip_appdata) + strlen_P(p2) > mss) {
	  debug_printf("%d: %s\n",cont_send,uip_appdata);
	  PASTE_SEND();
	  break;
	} else {
	  PASTE_P(p2);
	  cont_send++;
	}
      case 5:
	if(strlen(uip_appdata) + 40 > mss) {
	  debug_printf("%d: %s\n",cont_send,uip_appdata);
	  PASTE_SEND();
	  break;
	} else {
	  snprintf_P(uip_appdata + strlen(uip_appdata),40,PSTR("  <p>Solometer ID [%s"),post_cookie);
	  cont_send++;
	}
      case 6:
	if(strlen(uip_appdata) + 75 > mss) {
	  debug_printf("%d: %s\n",cont_send,uip_appdata);
	  PASTE_SEND();
	  break;
	} else {
	  PASTE_P (PSTR("]:<br><input name=\"ID\" type=\"text\" size=\"10\" maxlength=\"10\"></p>\n"));
	  cont_send++;
	}
      case 7:
	if(strlen(uip_appdata) + 90 > mss) {
	  debug_printf("%d: %s\n",cont_send,uip_appdata);
	  PASTE_SEND();
	  break;
	} else {
	  snprintf_P(uip_appdata + strlen(uip_appdata),90, PSTR("  <p>Webhost Name [%s"),post_hostname);
	  cont_send++;
	}
      case 8:
	if(strlen(uip_appdata) + 75 > mss) {
	  debug_printf("%d: %s\n",cont_send,uip_appdata);
	  PASTE_SEND();
	  break;
	} else {
	  PASTE_P(PSTR("]:<br><input name=\"HST\" type=\"text\" size=\"32\" maxlength=\"63\"></p>\n"));
	  cont_send++;
	}
      case 9:
	if(strlen(uip_appdata) + 90 > mss) {
	  debug_printf("%d: %s\n",cont_send,uip_appdata);
	  PASTE_SEND();
	  break;
	} else {
	  snprintf_P(uip_appdata + strlen(uip_appdata),90, PSTR("  <p>Webhost Script [%s"),post_scriptname);
	  cont_send++;
	}
      case 10:
	if(strlen(uip_appdata) + 75 > mss) {
	  debug_printf("%d: %s\n",cont_send,uip_appdata);
	  PASTE_SEND();
	  break;
	} else {
	  PASTE_P(PSTR("]:<br><input name=\"SCR\" type=\"text\" size=\"32\" maxlength=\"63\"></p>\n"));
	  cont_send++;
	}
      case 11:
	if(strlen(uip_appdata) + strlen_P(p3) > mss) {
	  debug_printf("%d: %s\n",cont_send,uip_appdata);
	  PASTE_SEND();
	  break;
	} else {
	  PASTE_P(p3);
	  cont_send++;
	}
      default:
	debug_printf("%d: %s\n",cont_send,uip_appdata);
	cont_send = 0;
	PASTE_SEND();
	break;
    }
    return;
  }
/*
  all_sent = 0;
  mss = uip_mss();
  PASTE_RESET ();
  if(i) {
    PASTE_P (httpd_header_500_smt);
    PASTE_SEND ();
  } else {
    
    PASTE_P (p1);
    PASTE_P (p1a);
    PASTE_P (p1b);
    uip_gethostaddr(&hostaddr);
    uint8_t *ip = (uint8_t *) &hostaddr;
    snprintf_P(uip_appdata + strlen(uip_appdata),16, PSTR("%u.%u.%u.%u"),ip[0], ip[1], ip[2], ip[3]);
    PASTE_P (p2);
    snprintf_P(uip_appdata + strlen(uip_appdata),40,PSTR("  <p>Solometer ID [%s"),post_cookie);
    PASTE_P (PSTR("]:<br><input name=\"ID\" type=\"text\" size=\"10\" maxlength=\"10\"></p>\n"));
    snprintf_P(uip_appdata + strlen(uip_appdata),90, PSTR("  <p>Webhost Name [%s"),post_hostname);
    PASTE_P(PSTR("]:<br><input name=\"HST\" type=\"text\" size=\"32\" maxlength=\"63\"></p>\n"));
    snprintf_P(uip_appdata + strlen(uip_appdata),90, PSTR("  <p>Webhost Script [%s"),post_scriptname);
    PASTE_P(PSTR("]:<br><input name=\"SCR\" type=\"text\" size=\"32\" maxlength=\"63\"></p>\n"));
    PASTE_P (p3);
    
    PASTE_SEND ();
  }
    all_sent = 1;
*/
}
