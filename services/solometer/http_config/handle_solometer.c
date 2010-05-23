/*
 * Copyright (c) 2009 by Stefan Siegl <stesie@brokenpipe.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (either version 2 or
 * version 3) as published by the Free Software Foundation.
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
#include "httpd.h"

const char page_header[] = "HTTP/1.1 200 OK\n"
"Host: solometer.local\n"
"Content-Length: 1000\n"
"Content-Type: text/html; charset=utf-8\n\n"
"<html>\n<head>\n"
"<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
"<title>SOLOMETER CONFIGURATION</title>\n"
"</head>\n<body>\n"
"Hello World!"
"\n</BODY>\n</HTML>\n";

char PROGMEM httpd_header_500_smt[] =
"HTTP/1.1 500 Server Error\n"
"Connection: close\n";

int
solometer_parse (char* ptr)
{
  debug_printf("String to parse: --%s--\n",ptr);
  return 0;
}

void
httpd_handle_solometer (void)
{
  int i = 0;

  if (uip_newdata()) {
    /* We've received new data (maybe even the first time).  We'll
      receive something like this:
      GET /solometer[?...]

    /* Make sure it's zero-terminated, so we can safely use strstr */
    char *ptr = (char *)uip_appdata;
    ptr[uip_len] = 0;

    debug_printf("Newdata: ---------\n%s\n-----------",ptr);

    ptr = strstr_P (ptr, PSTR("?")) + 1;
    if(!ptr || *ptr == 0) {
      debug_printf("This is a request only. Send page.\n");
      i = 0;
    } else {
	debug_printf("This is a set operation. Parsing...\n");
	i = solometer_parse(ptr);
    }
  }

  if (uip_acked ()) {
    uip_close ();
    return;
  }

  PASTE_RESET ();
  if(i) {
    PASTE_P (httpd_header_500_smt);
  } else {
    PASTE_P (page_header);
  }
  PASTE_SEND ();

}
