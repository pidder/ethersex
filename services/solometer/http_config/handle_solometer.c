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
#include "protocols/uip/parse.h"

extern uint8_t WRID[];
static char PROGMEM p1[] = "HTTP/1.1 200 OK\n"
"Host: solometer.local\n"
"Content-Length: ";
static char PROGMEM p2[] = "Content-Type: text/html; charset=utf-8\n\n";

// char PROGMEM website[] = "<html>\n<head>\n"
// "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\">\n"
// "<title>SOL-O-METER CONFIGURATION</title>\n"
// "</head>\n<body>\n"
// "<form action=\"http://%%0.%%1.%%2.%%3/solometer\" method=\"get\" accept-charset=\"utf-8\">\n"
// "  <p>Wechselrichter ID [%%4]:<br><input name=\"WRID\" type=\"text\" size=\"3\" maxlength=\"3\"></p>\n"
// "  <p>Solometer ID [%%5]:<br><input name=\"PVID\" type=\"text\" size=\"10\" maxlength=\"10\"></p>\n"
// "  <p>Webhost Name [%%6]:<br><input name=\"HST\" type=\"text\" size=\"32\" maxlength=\"63\"></p>\n"
// "  <p>Webhost IP (optional) [%%7.%%8.%%9.%%10]:<br><input name=\"HIP\" type=\"text\" size=\"16\" maxlength=\"16\"></p>\n"
// "  <p>Webhost Script [%%11]:<br><input name=\"SCR\" type=\"text\" size=\"32\" maxlength=\"63\"></p>\n"
// "  <p>DNS Server IP [%%12.%%13.%%14.%%15]:<br><input name=\"DNS\" type=\"text\" size=\"16\" maxlength=\"16\"></p>\n"
// "  <input type=\"submit\" value=\" Absenden \">\n"
// "</form>\n"
// "</BODY>\n</HTML>\n";

char PROGMEM website[] = "<html><head><style>\n"
".b {padding:10px;padding-bottom:0px;display:inline;}\n"
"#b1 {background:royalblue}\n"
".t { padding:30px;margin-left:0px;margin-right:20px;background:royalblue;display: none;}\n"
"#d1 {display: block;}\n"
"</style>\n"
"<script type=\"text/javascript\">\n"
"function s(a) {\n"
"var bs = document.getElementsByName('b');\n"
"var ts = document.getElementsByName('t');\n"
"for(i=0; i<bs.length; i++)\n"
"if(bs[i].id == a) {\n"
"bs[i].style.background = \"royalblue\";\n"
"ts[i].style.display = \"block\";\n"
"} else {\n"
"bs[i].style.background = \"lightblue\";\n"
"ts[i].style.display = 'none';\n"
"}}\n"
"</script>\n"
"<title>SOL-O-METER CONFIGURATION</title>\n"
"</head><body style=\"background:lightblue;\">\n"
"<H1 align=\"center\">Sol-O-Meter Konfiguration</H1>\n"
"<!-- Reiter -->\n"
"<div id=\"b1\" class=\"b\" name=\"b\" onClick=\"s('b1');\">Sol-O-Meter</div>\n"
"<div id=\"b2\" class=\"b\" name=\"b\" onClick=\"s('b2');\">Webhost</div>\n"
"<div id=\"b3\" class=\"b\" name=\"b\" onClick=\"s('b3');\">Wechselrichter</div>\n"
"<!-- Forms -->\n"
"<div id=\"d1\" class=\"t\" name=\"t\">\n"
"<form action=\"http://%%0.%%1.%%2.%%3/solometer\" method=\"get\" accept-charset=\"utf-8\">\n"
"<p>Solometer ID [%%5]:<br><input name=\"PVID\" type=\"text\" size=\"10\" maxlength=\"10\"></p>\n"
"<p>DNS Server IP [%%12.%%13.%%14.%%15]:<br><input name=\"DNS\" type=\"text\" size=\"16\" maxlength=\"16\"></p>\n"
"<input type=\"submit\" value=\" Absenden \">\n"
"</form></div>\n"
"<div id=\"d2\" class=\"t\" name=\"t\">\n"
"<form action=\"http://%%0.%%1.%%2.%%3/solometer\" method=\"get\" accept-charset=\"utf-8\">\n"
"<p>Webhost Name [%%6]:<br><input name=\"HST\" type=\"text\" size=\"32\" maxlength=\"63\"></p>\n"
"<p>Webhost IP (optional) [%%7.%%8.%%9.%%10]:<br><input name=\"HIP\" type=\"text\" size=\"16\" maxlength=\"16\"></p>\n"
"<p>Webhost Script [%%11]:<br><input name=\"SCR\" type=\"text\" size=\"32\" maxlength=\"63\"></p>\n"
"<input type=\"submit\" value=\" Absenden \">\n"
"</form></div>\n"
"<div id=\"d3\" class=\"t\" name=\"t\">\n"
"<form action=\"http://%%0.%%1.%%2.%%3/solometer\" method=\"get\" accept-charset=\"utf-8\">\n"
"<p>Wechselrichter ID [%%4]:  <input name=\"WRID\" type=\"text\" size=\"3\" maxlength=\"3\"></p>\n"
"<p>Fabrikat [SMA]:  <select name=\"FAB\" size=\"1\"><option value=\"1\">Effekta</option><option value=\"2\">SMA</option></select></p>\n"
"<p>Protokoll [SMANet]:  <select name=\"PRT\" size=\"1\"><option value=\"1\">Effekta</option><option value=\"1\">SMANet</option><option value=\"2\">SunnyNet</option></select></p>\n"
"<input type=\"submit\" value=\" Absenden \">\n"
"</form></div></body></html>\n";
 
typedef struct parameter {
  uint8_t typ;
  char *s2;
  char *s3;
} PARAM;

#define PR_STRING 0
#define PR_INT 1
#define PR_U8 2

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

int get_string(char *ptr, const char *pattern, char *target, int len)
{
  char *ptr1,*ptr2,c;

  //*target = 0;
  ptr1 = strstr_P(ptr,pattern);
  if(ptr1 != NULL) {
    ptr1 += strlen_P(pattern);
    ptr2 = strchrnul(ptr1,'&');
    if(ptr2 != ptr1) {
      c = *ptr2;
      *ptr2 = 0;
      ptr1 = strncpy(target,ptr1,len);
      *ptr2 = c;
      target[len-1] = 0;
      return(strlen(target));
    }
  }
  return 0;
}

int8_t
solometer_parse (char* ptr)
{
  char *str,*ptr1,c,*buf;
#ifdef DNS_SUPPORT
  uip_ipaddr_t dnsserver;
#endif
  int n;
  int16_t *int1_p,*int2_p,*int3_p,*int4_p;

  //buf=(char*)malloc(24);
  if(!(buf=(char*)malloc(24))) {
    //debug_printf("smt_parse: Not enough memory!\n");
    return -1;
  }
  
  int1_p = (int16_t*)(buf+16);
  int2_p = (int16_t*)(buf+18);
  int3_p = (int16_t*)(buf+20);
  int4_p = (int16_t*)(buf+22);

  ptr1 = strchr (ptr, ' ');
  if (ptr1 == NULL) {
    //debug_printf ("smt_parse: space after filename not found.\n");
    return -1;
  }

  /* Make a copy of the parameter string */
  c = *ptr1;
  *ptr1 = 0;
  if(!(str = malloc(strlen(ptr) + 1))) {
    //debug_printf("smt_parse: Not enough memory. Exiting.\n");
    *ptr1 = c;
    return -1;
  }
  ptr = strcpy(str,ptr);
  *ptr1 = c;

  urldecode(ptr);
  //debug_printf("String to parse: --%s--\n",ptr);

  if(get_string(ptr,PSTR("WRID="),buf,3)) {
    WRID[0] = (uint8_t)atoi(buf);
    eeprom_save(solometer_WRID, &WRID[0], 1);
    eeprom_update_chksum();
  }
  //debug_printf("Ausgewertet:WRID=--%u--\n",WRID[0]);

  if(get_string(ptr,PSTR("PVID="),post_cookie,11)) {
    eeprom_save(solometer_cookie, post_cookie, strlen(post_cookie) + 1);
    eeprom_update_chksum();
  }
  //debug_printf("Ausgewertet:PVID=--%s--\n",post_cookie);

  if(get_string(ptr,PSTR("HST="),post_hostname,64)) {
    eeprom_save(solometer_host, post_hostname, strlen(post_hostname) + 1);
    eeprom_update_chksum();
  }
  //debug_printf("Ausgewertet:HST=--%s--\n",post_hostname);

  if(get_string(ptr,PSTR("SCR="),post_scriptname,64)) {
    eeprom_save(solometer_script, post_scriptname, strlen(post_scriptname) + 1);
    eeprom_update_chksum();
  }
  //debug_printf("Ausgewertet:SCRPT=--%s--\n",post_scriptname);

  if(get_string(ptr,PSTR("HIP="),buf,16)) {
    n = sscanf(buf,"%d.%d.%d.%d",int1_p,int2_p,int3_p,int4_p);
    if(n == 4) {
      //debug_printf("Setting Host IP: %d %d %d %d\n",*int1_p,*int2_p,*int3_p,*int4_p);
      uip_ipaddr(&post_hostip,*int1_p,*int2_p,*int3_p,*int4_p);
      eeprom_save(solometer_hostip, post_hostip, sizeof(uip_ipaddr_t));
      eeprom_update_chksum();
      memset(buf,0,16);
      print_ipaddr(&post_hostip,buf,16);
      buf[15]=0;
    } else {
      //debug_printf("Not setting Host IP (n != 4).\n");
      buf[0] = 0;
    }
  } else {
    buf[0] = 0;
  }
  //debug_printf("Ausgewertet:HIP=--%s--\n",buf);

#ifdef DNS_SUPPORT
  if(get_string(ptr,PSTR("DNS="),buf,16)) {
    n = sscanf(buf,"%d.%d.%d.%d",int1_p,int2_p,int3_p,int4_p);
    if(n == 4) {
      //debug_printf("Setting DNS server IP\n");
      uip_ipaddr(&dnsserver,*int1_p,*int2_p,*int3_p,*int4_p);
      eeprom_save(dns_server, &dnsserver, sizeof(uip_ipaddr_t));
      eeprom_update_chksum();
      resolv_conf(&dnsserver);
      memset(buf,0,16);
      print_ipaddr(&dnsserver,buf,16);
      buf[15]=0;
    } else {
      //debug_printf("Not setting DNS server IP (n <> 4).\n");
      buf[0] = 0;
    }
  } else {
    buf[0] = 0;
  }
  //debug_printf("Ausgewertet:DNS=--%s--\n",buf);
#endif

  free(ptr);
  free(buf);
  return 0;
}

void
httpd_handle_solometer (void)
{
  static int8_t i = 0;
  static uint8_t cont_send = 0, parsing = 0;
  uint16_t mss,page_size;
  static uip_ipaddr_t hostaddr, dnsserver;
  //char *buf;
  static uint16_t ppos;
  uint16_t lpos,wslen;
  uint8_t p_par,pct,send_packet,buf[64];

  uip_gethostaddr(&hostaddr);
#ifdef DNS_SUPPORT
  eeprom_restore(dns_server, &dnsserver, IPADDR_LEN);
#endif

  #define NUM_PAR 16
  PARAM p[NUM_PAR] = {
	      {PR_U8,"%u",(uint8_t *)&hostaddr},
	      {PR_U8,"%u",((uint8_t *)&hostaddr)+1},
	      {PR_U8,"%u",((uint8_t *)&hostaddr)+2},
	      {PR_U8,"%u",((uint8_t *)&hostaddr)+3},
	      {PR_U8,"%u",&WRID[0]},
	      {PR_STRING,"%s",post_cookie},
	      {PR_STRING,"%s",post_hostname},
	      {PR_U8,"%u",(uint8_t *)&post_hostip},
	      {PR_U8,"%u",((uint8_t *)&post_hostip)+1},
	      {PR_U8,"%u",((uint8_t *)&post_hostip)+2},
	      {PR_U8,"%u",((uint8_t *)&post_hostip)+3},
	      {PR_STRING,"%s",post_scriptname},
	      {PR_U8,"%u",(uint8_t *)&dnsserver},
	      {PR_U8,"%u",((uint8_t *)&dnsserver)+1},
	      {PR_U8,"%u",((uint8_t *)&dnsserver)+2},
	      {PR_U8,"%u",((uint8_t *)&dnsserver)+3}
  };

  //debug_printf("Handle_solometer called.\n");
  mss = uip_mss();
  if(mss > 400)
    mss = 400;
  wslen = strlen_P(website);
  
  if (uip_newdata()) {
    /* We've received new data (maybe even the first time).  We'll
      receive something like this:
      GET /solometer[?...]
    */
    /* Make sure it's zero-terminated, so we can safely use strstr */
    char *ptr = (char *)uip_appdata;
    ptr[uip_len] = 0;

    //debug_printf("Newdata: ---------\n%s\n-----------\n",ptr);

    if(strncasecmp_P (uip_appdata, PSTR ("GET /solometer"),14) == 0) {
      //debug_printf("This is the GET request header...\n");
      ptr = strstr_P (uip_appdata, PSTR("?")) + 1;
      if(!ptr || *ptr == 0) {
	//debug_printf("This is a request only. Send page.\n");
	i = 0;
      } else {
	//debug_printf("This is a set operation. Parsing...\n");
	i = solometer_parse(ptr);
      }
      //debug_printf("Setze Parsing auf 1.\n");
      parsing = 1;
    }
    
    if (parsing == 1) {
      // Do not start answering until all packets have arrived
      //debug_printf("Parsing = 1\n");
      ptr = strstr_P (uip_appdata, PSTR("\r\n\r\n"));
      if (ptr) {
	//debug_printf("Setze Parsing auf 2.\n");
	parsing = 2;
      } else {
	//debug_printf("Double NL not found. Waiting...\n");
	return;
      }
    }
    
    if (parsing == 2) {
      //debug_printf("Parsing = 2. Sende Antwort.\n");
      PASTE_RESET ();
      if(i || mss < 200) {
	PASTE_P (httpd_header_500_smt);
	cont_send = 0;
      } else {
	page_size = wslen);
	for(i=0;i<NUM_PAR;i++)
	  if(p[i].typ == PR_STRING)
	    page_size += sprintf(buf,p[i].s2,p[i].s3);
	  else
	    page_size += sprintf(buf,p[i].s2,*(uint8_t *)p[i].s3);
	PASTE_P (p1);
	sprintf(uip_appdata+uip_len,"%u\n",page_size);
	PASTE_P (p2);
	cont_send = 1;
	ppos = 0;
      }
      //debug_printf("%d: %s\n",cont_send,uip_appdata);
      PASTE_SEND ();
      parsing = 0;
      return;
    }
  }

  if(uip_acked()) {
    // Send webpage and fill in parameters %%n, where n
    // is n'th record of array p of type PARAM
    debug_printf("uip_acked\n");
    debug_printf("Aussen c: %c, lpos: %d, ppos: %d\n",c,lpos,ppos);
    if(cont_send && ppos < wslen) {
      PASTE_RESET();
      lpos = 0;
      pct = 0;
      //ppos = 0;
      p_par = 0;
      //printf("%d\n",strlen(website));
      //while(ppos < strlen(website)) {
      //lpos = 0;
      send_packet = 0;
      memcpy_P(buf,website+64*(ppos/64),64);
      while(lpos < mss && ppos < wslen && send_packet == 0) {
	if(!(ppos%64)) {
	  debug_printf("Innen c: %c, lpos: %d, ppos: %d\n",c,lpos,ppos);
	  memcpy_P(buf,website+64*(ppos/64),64);
	}
	c = buf[ppos%64];
	//c = website[ppos++];
	//memcpy_P(&c,website+ppos,1);
	ppos++;
	//debug_printf("c: %c, lpos: %d, ppos: %d\n",c,lpos,ppos);
	switch(pct) {
	  case 1:
	    if(c == '%') {
	      pct = 2;
	    } else {
	      *(char *)(uip_appdata+(lpos++)) = '%';
	      *(char *)(uip_appdata+(lpos++)) = c;
	      *(char *)(uip_appdata+lpos) = 0;
	      pct = 0;
	    }
	    break;
	  case 2:
	    if(c == '%') {
	      *(char *)(uip_appdata+(lpos++)) = '%';
	      *(char *)(uip_appdata+lpos) = 0;
	    } else if(c >= 0x30 && c <= 0x39) {
	      p_par = p_par*10 + (c-0x30);
	      pct = 3;
	    } else {
	      *(char *)(uip_appdata+(lpos++)) = '%';
	      *(char *)(uip_appdata+(lpos++)) = '%';
	      *(char *)(uip_appdata+(lpos++)) = c;
	      *(char *)(uip_appdata+lpos) = 0;
	      pct = 0;
	    }
	    break;
	  case 3:
	    if(c >= 0x30 && c <= 0x39) {
	      p_par = p_par*10 + (c-0x30);
	    } else {
	      switch(p[p_par].typ) {
		case PR_INT:
		  lpos += sprintf(uip_appdata+lpos,p[p_par].s2,*(int*)p[p_par].s3);
		  break;
		case PR_U8:
		  lpos += sprintf(uip_appdata+lpos,p[p_par].s2,*(uint8_t*)p[p_par].s3);
		  break;
		default:
		  lpos += sprintf(uip_appdata+lpos,p[p_par].s2,p[p_par].s3);
		  break;
	      }
	      p_par = 0;
	      if(c == '%') {
		pct = 1;
	      } else {
		pct = 0;
		*(char *)(uip_appdata+(lpos++)) = c;
		*(char *)(uip_appdata+lpos) = 0;
	      }
	    }
	    break;
	  default:
	    if(c == '%') {
	      if(lpos > mss-64) {
		ppos--;
		send_packet = 1;
	      } else {
		pct = 1;
	      }
	    } else {
	      *(char *)(uip_appdata+(lpos++)) = c;
	      *(char *)(uip_appdata+lpos) = 0;
	    }
	    break;
	}
      }
      debug_printf("Sending c: %c, lpos: %d, ppos: %d\n",c,lpos,ppos);
      PASTE_SEND();
    }
    return;
  }
/*  debug_printf("Unbekanntes Paket. Sende Antwort.\n");
  PASTE_RESET ();
  if(i || mss < 200) {
    PASTE_P (httpd_header_500_smt);
    cont_send = 0;
  } else {
    PASTE_P (p1);
    cont_send = 1;
  }
  //debug_printf("%d: %s\n",cont_send,uip_appdata);
  PASTE_SEND ();
  parsing = 0;
  //uip_close();*/
  return;
}
