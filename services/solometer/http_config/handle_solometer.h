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
