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
#include "services/httpd/httpd.h"
#include <ctype.h>
#include "core/eeprom.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/parse.h"

extern char post_hostname[], post_hostip[], post_scriptname[], post_cookie[];
extern char solometer_host[], solometer_cookie[], solometer_hostip[], solometer_script[];

struct param {
  char *desc;
  char *name;
  char *typ;
  char *ramname;
  int maxlen;
  char *eepromname;
};
