/*                     -*- mode: C; c-file-style: "stroustrup"; -*-
 *
 * Copyright (c) 2007 by Stefan Siegl <stesie@brokenpipe.de>
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

/* we want to live on the outer stack, if there are two ... */
#include "protocols/uip/uip_openvpn.h"
#include "protocols/uip/uip.h"

#include <stdlib.h>

#include "bootp_net.h"
#include "bootp_state.h"
#include "bootp.h"
#include "config.h"

void
bootp_net_init(void)
{
    uip_ipaddr_t ip;
    uip_ipaddr(&ip, 255,255,255,255);

    uip_udp_conn_t *bootp_conn = uip_udp_new(&ip, HTONS(BOOTPS_PORT), bootp_net_main);

    if(! bootp_conn)
	return; /* dammit. */

    uip_udp_bind(bootp_conn, HTONS(BOOTPC_PORT));

    bootp_conn->appstate.bootp.retry_timer = 0;
}


void
bootp_net_main(void)
{
    if(uip_newdata()) {
	bootp_handle_reply();
        return;
    }

    if(! uip_udp_conn->appstate.bootp.retry_timer) {
	bootp_send_request();
	debug_printf("Sending BOOTP request number %u.\n",uip_udp_conn->appstate.bootp.retry_counter + 1);
	if(uip_udp_conn->appstate.bootp.retry_counter < 5)
	    uip_udp_conn->appstate.bootp.retry_timer =
		2 << (++ uip_udp_conn->appstate.bootp.retry_counter);
	else if(++uip_udp_conn->appstate.bootp.retry_counter < 10)
	    uip_udp_conn->appstate.bootp.retry_timer = 64 + (rand() & 63);
	else {
	  // Set an IP from the Zeroconf area in 169.254.0.0/16,
	  // but not 169.254.0.0/8 and not 169.254.255.0/8.
	  // This is NOT the correct Zeroconf way and has to be improved
	  // because there is no check for collisions(, yet)!
	  uip_ipaddr_t zcip;
	  uip_ipaddr(&zcip,169,154,44,44);
	  uip_sethostaddr(&zcip);
	  uip_ipaddr(&zcip,255,255,0,0);
	  uip_setnetmask(&zcip);
	  debug_printf("Aborting BOOTP. Setting Zeroconf IP 169.154.44.44.\n");
	  uip_udp_remove(uip_udp_conn);
	}
    }
    else
	uip_udp_conn->appstate.bootp.retry_timer --;
}

/*
  -- Ethersex META --
  header(protocols/bootp/bootp_net.h)
  net_init(bootp_net_init)

  state_header(protocols/bootp/bootp_state.h)
  state_udp(struct bootp_connection_state_t bootp)
*/
