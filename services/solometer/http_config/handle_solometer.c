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

extern char solometer_cfg[];
char text[12];

void
httpd_handle_solometer (void)
{
  int i;

  i = strlen(solometer_cfg);
  debug_printf("Solometer config: strlen = %d\n",i);

  if(i>0)
    debug_printf("Solometer config: %s\n",solometer_cfg);

  i = sscanf(solometer_cfg,"PVID=%10s&",text);
  text[11] = 0;
  if(i > 0) {
    debug_printf("Parsed PVID: --%s--\n",text);
  }
}
