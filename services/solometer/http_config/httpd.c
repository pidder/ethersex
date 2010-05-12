/*
 * (c) by Alexander Neumann <alexander@bumpern.de>
 * Copyright (c) 2008,2009 by Stefan Siegl <stesie@brokenpipe.de>
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

#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "httpd.h"
#include "base64.h"
#include "core/eeprom.h"
#include "core/vfs/vfs.h"

#ifdef DEBUG_HTTPD
# include "core/debug.h"
# define printf(a...)  debug_printf(a)
#else
# define printf(...)   ((void)0)
#endif


void
httpd_init(void)
{
    uip_listen(HTONS(HTTPD_PORT), httpd_main);
    uip_listen(HTONS(HTTPD_ALTERNATE_PORT), httpd_main);
}



void
httpd_cleanup (void)
{
}


static void
httpd_handle_input (void)
{
    char *ptr = (char *) uip_appdata;

    if (uip_len < 10) {
	printf ("httpd: received request to short (%d bytes).\n", uip_len);
	STATE->handler = httpd_handle_400;
	return;
    }

    if (strncasecmp_P (uip_appdata, PSTR ("GET /"), 5)) {
	printf ("httpd: received request is not GET.\n");
	STATE->handler = httpd_handle_400;
	return;
    }

    char *filename = uip_appdata + 5; /* beyond slash */
    ptr = strchr (filename, ' ');

    if (ptr == NULL) {
	printf ("httpd: space after filename not found.\n");
	STATE->handler = httpd_handle_400;
	return;
    }

    *ptr = 0;			/* Terminate filename. */

    /*
     * Successfully parsed the GET request,
     * possibly check authentication.
     */

    //Handle solometer config request here

    /* Fallback, send 404. */
    STATE->handler = httpd_handle_404;
}

void
httpd_main(void)
{
    if (uip_aborted() || uip_timedout()) {
	httpd_cleanup ();
	printf ("httpd: connection aborted\n");
    }

    if (uip_closed()) {
	httpd_cleanup ();
	printf ("httpd: connection closed\n");
    }

    if (uip_connected()) {
	printf ("httpd: new connection\n");

	/* initialize struct */
	STATE->handler = NULL;
	STATE->header_acked = 0;
	STATE->eof = 0;
	STATE->header_reparse = 0;
    }

    if (uip_newdata() && (!STATE->handler || STATE->header_reparse)) {
	printf ("httpd: new data\n");
	httpd_handle_input ();
    }

    if(uip_rexmit() ||
       uip_newdata() ||
       uip_acked() ||
       uip_poll() ||
       uip_connected()) {

	/* Call associated handler, if set already. */
	if (STATE->handler && (!STATE->header_reparse))
	    STATE->handler ();
    }
}

/*
  -- Ethersex META --
  header(services/httpd/httpd.h)
  net_init(httpd_init)

  state_header(services/httpd/httpd_state.h)
  state_tcp(struct httpd_connection_state_t httpd)
*/
