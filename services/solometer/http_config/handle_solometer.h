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

extern char post_hostname[], post_hostip[], post_scriptname[], post_cookie[];
extern char solometer_host[], solometer_cookie[], solometer_hostip[], solometer_script[];
/*
char PROGMEM page_header1[] = "HTTP/1.1 200 OK\n"
"Host: solometer.local\n"
"Content-Length: 1000\n"
"Content-Type: text/html; charset=utf-8\n\n"
"<html>\n<head>\n"
"<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
"<title>SOLOMETER CONFIGURATION</title>\n"
"</head>\n<body>\n";

char PROGMEM page_footer1[] = "  <input type=\"submit\" value=\" Absenden \">"
"</form>"
"</BODY>\n</HTML>\n";

typedef struct param {
  char *desc;
  char *name;
  char *typ;
  char *ramname;
  int maxlen;
  char *eepromname;
};

struct param parameter[] = {
// Beschreibung 	Name 	Typ	RAM		maxlen	EEPROM
{ "Sol-O-Meter ID:",	"ID",	"text",	post_cookie,	10,	solometer_cookie},
{ "Webhost Name:",	"HST",	"text",	post_hostname,	63,	solometer_host},
{ "Webhost IP:",	"IP",	"text",	post_hostip,	15,	solometer_hostip},
{ "Webhost script:",	"SCR",	"text",	post_scriptname,63,	solometer_script}
};
*/