//
// Copyright (c) 2010 by Peter Hartmann <solometerklammeraffehartmann-peter.de>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 3
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// For more information on the GPL, please go to:
// http://www.gnu.org/copyleft/gpl.html
//
//
//***********************
//       Parameter      *
//***********************
ANZEIGEDAUER = 10;  // Standbild
HINTERGRUNDFARBE = "SkyBlue";
WBOXFARBE = "RoyalBlue";
//***********************
//    Ende Parameter    *
//***********************

function ShowHistory(evt,ide,n)
{
  if(ide == "Jahr") {
    var mywindow = window.open("showmonth.svg?MONAT="+binmonat[n],binmonat[n]);
  } else {
    // ide == Monat
    var mywindow = window.open("showday.svg?TAG="+bindatum[n],bindatum[n]);
  }
}

function ShowTooltip(evt,ide,n)
{
  var i, ttrelem, ttrelem, posx, posy;

  if(dieses.id != ide)
    return;

  animation_stop = true;
  //alert("STT: "+n);

  if(isNaN(messwerte[n][0]) && isNaN(messwerte[n][0]) && isNaN(messwerte[n][0])) {
    ttboxelem=document.getElementById("ndbox");
    ttrelem=document.getElementById("ndr");
    tttelem=document.getElementById("ndt");

  } else {
    ttboxelem=document.getElementById("ttbox");
    ttrelem=document.getElementById("ttr");
    tttelem=document.getElementById("ttt");
    var hr = 6 + Math.floor(n/12);
    var mn = (n%12)*5;
    if(mn < 10)
      tttelem.firstChild.replaceData(0,100,"Messwerte "+hr+":0"+mn);
    else
      tttelem.firstChild.replaceData(0,100,"Messwerte "+hr+":"+mn);

    //tttelem.firstChild.replaceData(0,100,"Id: "+ide);

    i = 0;
    var child = tttelem.firstChild;
    while (child != null) {
      if (child.nodeName == "tspan" && child.hasChildNodes()) {
	if (child.firstChild.nodeType == 3 && child.id == "wert") {
	  child.firstChild.replaceData(0,100,messwerte[n][i++]);
	}
      }
      child = child.nextSibling;
    }
  }
  var posx=evt.pageX;
  var posy=evt.pageY;
  var ttboxwidth=ttrelem.getAttribute("width");
  var ttboxheight=ttrelem.getAttribute("height");

  if(posx < Fensterweite()-ttboxwidth)
    ttboxelem.setAttribute("x",posx);
  else
    ttboxelem.setAttribute("x",posx-ttboxwidth);

  if(posy < ttboxheight)
    ttboxelem.setAttribute("y",posy);
  else
    ttboxelem.setAttribute("y",posy-ttboxheight);

  ttboxelem.setAttribute("style","visibility: visible");
}

function HideTooltip()
{
  // Hide all tooltips
  var ttboxelem=document.getElementById("ttbox");
  var ndboxelem=document.getElementById("ndbox");
  ttboxelem.setAttribute("style","visibility: hidden");
  ndboxelem.setAttribute("style","visibility: hidden");
  animation_stop = false;
}

// Window-Resize detektieren und Fenstergroesse anpassen
function Fensterweite () {
  if (window.innerWidth) {
    return window.innerWidth;
  } else if (document.body && document.body.offsetWidth) {
    return document.body.offsetWidth;
  } else {
    return 0;
  }
}

function Fensterhoehe () {
  if (window.innerHeight) {
    return window.innerHeight;
  } else if (document.body && document.body.offsetHeight) {
    return document.body.offsetHeight;
  } else {
    return 0;
  }
}

function neuAufbau () {
  if (Weite != Fensterweite() || Hoehe != Fensterhoehe())
    location.href = location.href;
}

/* Überwachung von Netscape initialisieren */
if (!window.Weite && window.innerWidth) {
  window.onresize = neuAufbau;
  Weite = Fensterweite();
  Hoehe = Fensterhoehe();
}

// Berechnet aus einem Array mit Werten die Anzahl und
// die Beschriftung der y-Achsenabschnitte
// Rückgabewert ist ein Array, das die Beschriftungen
// als Zahlenwerte in aufsteigender Folge enthält. 
// Die Anzahl der Elemente ist die Zahl der Ticks
function calc_ticks(werte,ident)
{
  var maxval, i,j,ypow=1;
  var ynamen = new Array(), yticks, yeinh, ymaxscale;
  var mant, base, b1;

  // Finde Maximum der Werte
  maxval = werte[0];
  for(i=1;i<werte.length;i++) {
    if(werte[i] > maxval) maxval = werte[i];
  }
  if(maxval < 1) maxval = 1;

  // Finde Zehnerpotenz (vorsicht: log() hat basis e)
  mant = Math.floor(Math.log(maxval)/Math.log(10));
  base = Math.pow(10,mant);
  // Normiere -> 1..10
  b1 = maxval/base;

  // Finde geeignete Skalierung in array yscales
  i=-1;
  while(i < yscales.length && yscales[++i][0] < b1);
  // yscales ist 'größter Wert zuerst'
  for(j=0;j<yscales[i].length;j++) ynamen[j]=yscales[i][j];
  // Sonst by reference (s.u.) (workaround)
  // ynamen = yscales[i];
  ynamen.reverse();
  // Setze Anzahl der Ticks
  yticks = ynamen.length;
  // Größter Wert der Skala < 1000
  if(mant >= 9) {
    yeinh = "GW";
    ypow = mant - 9;
  } else if(mant >= 6) {
    yeinh = "MW";
    ypow = mant - 6;
  } else if(mant >= 3) {
    yeinh = "kW";
    ypow = mant - 3;
  } else {
    ypow = mant;
    yeinh = "W";
  }
  // Nur Minutenwerte sind Watt, andere sind Wh
  if(ident != "Tag") yeinh += "h";
  // Echter Maximalwert
  ymaxscale = yscales[i][0] * base;
  // Skaliere ynamen
  for(j = 0; j < yticks; j++) ynamen[j] = num2str(ynamen[j] * Math.pow(10,ypow))[0];

  return([ynamen,ymaxscale,yeinh]);
}

// Wandelt eine Fliesskommazahl 10^12 > x > 0 in einen String gerundet auf 2 sign. Stellen
// und einen Einheiten-Multiplikator (k,M,G) um.
function num2str(zahl)
{
  var str, einh, expo, mant, tmpstr;

  if(zahl <= 0)
    return(["0",""]);

  expo = Math.floor(Math.log(zahl)/Math.log(10));
  mant = zahl / Math.pow(10,expo);

  if(expo >= 9) {
    expo -= 9;
    einh = "G";
  } else if(expo >= 6) {
    expo -= 6;
    einh = "M";
  } else if(expo >= 3) {
    expo -= 3;
    einh = "k";
  } else if(expo <= -3) {
    expo += 3;
    einh = "m";
  } else
    einh = "";

  // Runde Zahl > 1 auf 2 signif. Stellen
  if(expo >= 1) {
    // Runde auf 0 Nachkommastellen
    str = Math.floor(mant * Math.pow(10,expo) + 0.5).toString();
  } else if(expo < 0) {
    // Runde auf 1 sign. Stelle
    str = Math.floor(mant + 0.5).toString();
    // Füge Nullen und Dezimalpunkt ein
    tmpstr = "0,";
    for(i = expo+1;i < 0;i++) tmpstr += "0";
    str = tmpstr + str;
  } else {
    // Wandele gerundete Zahl*10 in String
    str = Math.floor(mant * 10 + 0.5).toString();
    l = str.length;
    // Füge Dezimalpunkt an vorletzter Stelle ein
    str = str.substr(0,l-1)+','+str.substr(l-1,1);
  }
  return([str,einh])
}

function num2kstr(zahl)
{
  var l, str, einh, expo, mant, tmpstr;

  if(zahl < 1000)
    return(num2str(zahl));

  expo = Math.floor(Math.log(zahl)/Math.log(10));
  mant = zahl / Math.pow(10,expo);

  expo -= 3;
  einh = "k";

  if(expo >= 1) {
    // Runde auf 0 Nachkommastellen
    str = Math.floor(mant * Math.pow(10,expo) + 0.5).toString();
    // Füge Tausender-Trenner ein
    l = str.length;
    while(l > 3) {
      l -= 3;
      str = str.substr(0,l)+'.'+str.substr(l,str.length - l);
    }
  } else {
    // Wandele gerundete Zahl*10 in String
    str = Math.floor(mant * 10 + 0.5).toString();
    l = str.length;
    // Füge Dezimalpunkt an vorletzter Stelle ein
    str = str.substr(0,l-1)+','+str.substr(l-1,1);
  }
  return([str,einh])
}

// Generiert einen Basisgraph, der alle Komponenten enthält
// Die tatsächlichen Skalierungen bzw. Balkengrößen werden
// in der Funktion update_graph vorgenommen.
function create_graph(titel,ident,namen,werte,opacity)
{
  var shape, gruppe, data;
  var i,j;
  var rotate = 0;
  var yeinh,yticks,ynamen,ymaxscale,dummy=new Array();
  var xticklabel_fontsize, yticklabel_fontsize, yaxislabel_fontsize;

  xticks = namen.length;

  // Schriftgröße pauschal
  yticklabel_fontsize = hoehe/20;
  yaxislabel_fontsize = hoehe/20;

  // Berechne x-Achsenbeschriftung
  // Setze Texthöhe auf standardwert hoehe/20
  xticklabel_fontsize = hoehe/20;
  // Finde längsten String
  var ind = 0; j = 0;
  for(i=0;i<xticks;i++) {
    if(namen[i].length > j) {
      ind = i;
      j = namen[i].length;
    }
  }
  // Berechne Verhältnis von Textbreite zu binbreite
  var breitenverh = j * xticklabel_fontsize * 0.7 / (breite/xticks);
  //alert('Breitenverhältnis:'+breitenverh);
  // Ist breite > binbreite
  if(breitenverh > 2.0) {
    // Setze hoehe auf binbreite*0.8 und drehe den Text
    xticklabel_fontsize = (0.8 * breite / xticks > hoehe/20)?hoehe/20:0.8 * breite / xticks;
    rotate = 30;
  } else if(breitenverh > 0.9) {
    //Halbiere Fontgröße
    xticklabel_fontsize /= 2;
  }
  yeinh = "Wh";
  // Das kann man noch verbessern!!
  if(ident == "Tag") {
    xticklabel_fontsize = hoehe/20;
    rotate = 0;
    yeinh = "W";
  } else if(ident == "Monat") {
    xticklabel_fontsize = 0.8 * breite / xticks;
    rotate = 90;
  }

  yticks = ymaxticks;
  ynamen = yscales[ymaxticksindex];
  ynamen.reverse();
  yticks = ynamen.length;
  ymaxscale = yscales[ymaxticksindex][1];

  // Erzeuge die Gruppe, die den Graph enthält
  gruppe = document.createElementNS(svgns, "g");
  gruppe.setAttributeNS(null, "id", ident);
  gruppe.setAttributeNS(null, "fill-opacity", opacity);
  gruppe.setAttributeNS(null, "stroke-opacity", opacity);

  // Hintergrund
  shape = document.createElementNS(svgns, "rect");
  shape.setAttributeNS(null, "id", ident+".bg");
  shape.setAttributeNS(null, "x", 0);
  shape.setAttributeNS(null, "y", 0);
  shape.setAttributeNS(null, "width", fensterbreite);
  shape.setAttributeNS(null, "height", fensterhoehe);
  shape.setAttributeNS(null, "fill", HINTERGRUNDFARBE);
  gruppe.appendChild(shape);

  // Y-Achse
  for (i=0;i<yticks;i++) {
    // Ticks (ytick)
    shape = document.createElementNS(svgns, "line");
    shape.setAttributeNS(null, "id", ident+".ytick."+i);
    shape.setAttributeNS(null, "x1", xoff-ticklength);
    shape.setAttributeNS(null, "y1", yoff+hoehe-(i+1)*hoehe/yticks);
    shape.setAttributeNS(null, "x2", xoff+breite);
    shape.setAttributeNS(null, "y2", yoff+hoehe-(i+1)*hoehe/yticks);
    shape.setAttributeNS(null, "visibility", "visible");
    shape.setAttributeNS(null, "stroke", "black");
    shape.setAttributeNS(null, "stroke-width", "1");
    gruppe.appendChild(shape);

    // Beschriftung (yticklabel)
    txt = document.createTextNode(ynamen[i]);
    shape = document.createElementNS(svgns, "text");
    shape.setAttributeNS(null, "id", ident+".yticklabel."+i);
    shape.setAttributeNS(null, "x", xoff-2*ticklength);
    shape.setAttributeNS(null, "y", yoff+hoehe-(i+1)*hoehe/yticks+yticklabel_fontsize/2);
    shape.setAttributeNS(null, "visibility", "visible");
    shape.setAttributeNS(null, "text-anchor", "end");
    shape.setAttributeNS(null, "font-size", yticklabel_fontsize);
    shape.setAttributeNS(null, "fill", "black");
    shape.appendChild(txt);
    gruppe.appendChild(shape);
  }

  // Achsenskalierung, -beschriftung und Balken
  // X-Achse
  for (i=0;i<xticks;i++) {

    // Ticks (xtick)
    shape = document.createElementNS(svgns, "line");
    shape.setAttributeNS(null, "id", ident+".xtick."+i);
    shape.setAttributeNS(null, "x1", xoff+(i+1)*breite/xticks);
    shape.setAttributeNS(null, "y1", yoff+hoehe+ticklength);
    shape.setAttributeNS(null, "x2", xoff+(i+1)*breite/xticks);
    shape.setAttributeNS(null, "y2", yoff+hoehe);
    shape.setAttributeNS(null, "stroke", "#000000");
    shape.setAttributeNS(null, "stroke-width", "1");
    gruppe.appendChild(shape);

    // Beschriftung (xticklabel)
    data = document.createTextNode(namen[i]);
    shape = document.createElementNS(svgns, "text");
    shape.setAttributeNS(null, "id", ident+".xticklabel."+i);
    shape.setAttributeNS(null, "font-size", xticklabel_fontsize);
    shape.setAttributeNS(null, "fill", "black");
    if(rotate) {
      xpos = xoff+(i+0.8)*breite/xticks;
      ypos = yoff+hoehe+2*ticklength;
      shape.setAttributeNS(null, "x", xpos);
      shape.setAttributeNS(null, "y", ypos);
      shape.setAttributeNS(null, "text-anchor", "end");
      shape.setAttributeNS(null, "transform", "rotate(-"+rotate+","+xpos+","+ypos+")");
    } else {
      xpos = xoff+(i+0.5)*breite/xticks;
      ypos = yoff+hoehe+ticklength+xticklabel_fontsize;
      shape.setAttributeNS(null, "text-anchor", "middle");
      shape.setAttributeNS(null, "x", xpos);
      shape.setAttributeNS(null, "y", ypos);
    }
    shape.appendChild(data);
    gruppe.appendChild(shape);

    // Balken
    shape = document.createElementNS(svgns, "rect");
    shape.setAttributeNS(null, "id", ident+".balken."+i);
    shape.setAttributeNS(null, "x", xoff+(i+0.05)*breite/xticks);
    shape.setAttributeNS(null, "y", yoff);
    shape.setAttributeNS(null, "width", 0.9*breite/xticks);
    shape.setAttributeNS(null, "height", 0);
    shape.setAttributeNS(null, "stroke", "#000000");
    shape.setAttributeNS(null, "stroke-width", "1");
    shape.setAttributeNS(null, "fill", "yellow");
    if(ident == "Tag") {
      shape.setAttributeNS(null, "onmouseover", "ShowTooltip(evt,\""+ident+"\","+i+")");
      shape.setAttributeNS(null, "onmouseout", "HideTooltip()");
    } else {
      if(ident != "Monat" || (i != (xticks -1)))
	shape.setAttributeNS(null, "onclick", "ShowHistory(evt,\""+ident+"\","+i+")");
    }
    gruppe.appendChild(shape);
  }

  // Erzeuge Überschrift und Koordinatenachsen
  // Überschrift
  data = document.createTextNode(titel);
  shape = document.createElementNS(svgns, "text");
  shape.setAttributeNS(null, "id", ident+".titel");
  shape.setAttributeNS(null, "x", xoff+breite/2);
  shape.setAttributeNS(null, "y", yoff*0.7);
  shape.setAttributeNS(null, "text-anchor", "middle");
  shape.setAttributeNS(null, "font-size", yoff*0.5);
  shape.setAttributeNS(null, "fill", "black");
  shape.appendChild(data);
  gruppe.appendChild(shape);

  // Updatestring
  data = document.createTextNode("Letzte Daten vom");
  shape = document.createElementNS(svgns, "text");
  shape.setAttributeNS(null, "id", ident+".last");
  shape.setAttributeNS(null, "x", xoff+breite/2);
  shape.setAttributeNS(null, "y", yoff*0.9);
  shape.setAttributeNS(null, "text-anchor", "middle");
  shape.setAttributeNS(null, "font-size", yoff*0.1);
  shape.setAttributeNS(null, "fill", "black");
  shape.appendChild(data);
  gruppe.appendChild(shape);

  // Yaxis-Label
  data = document.createTextNode(yeinh);
  shape = document.createElementNS(svgns, "text");
  shape.setAttributeNS(null, "id", ident+".yaxislabel");
  shape.setAttributeNS(null, "x", xoff);
  shape.setAttributeNS(null, "y", yoff-yaxislabel_fontsize);
  shape.setAttributeNS(null, "text-anchor", "end");
  shape.setAttributeNS(null, "font-size", yaxislabel_fontsize);
  shape.setAttributeNS(null, "fill", "black");
  shape.appendChild(data);
  gruppe.appendChild(shape);

  // Vertikale Linie (Y-Achse)
  shape = document.createElementNS(svgns, "line");
  shape.setAttributeNS(null, "id", ident+".yaxis");
  shape.setAttributeNS(null, "x1", xoff);
  shape.setAttributeNS(null, "y1", yoff);
  shape.setAttributeNS(null, "x2", xoff);
  shape.setAttributeNS(null, "y2", yoff+hoehe);
  shape.setAttributeNS(null, "stroke", "#000000");
  shape.setAttributeNS(null, "stroke-width", "3");
  gruppe.appendChild(shape);

  // Horizontale Linie (X-Achse)
  shape = document.createElementNS(svgns, "line");
  shape.setAttributeNS(null, "id", ident+".xaxis");
  shape.setAttributeNS(null, "x1", xoff);
  shape.setAttributeNS(null, "y1", yoff+hoehe);
  shape.setAttributeNS(null, "x2", xoff+breite);
  shape.setAttributeNS(null, "y2", yoff+hoehe);
  shape.setAttributeNS(null, "stroke", "#000000");
  shape.setAttributeNS(null, "stroke-width", "3");
  gruppe.appendChild(shape);

  // W(erte)box1
  xpos = xoff+breite-2*xoff;
  xpos1 = xoff+breite-xoff;
  ypos = yoff*1.05;
  shape = document.createElementNS(svgns, "rect");
  shape.setAttributeNS(null, "id", ident+".wbox");
  shape.setAttributeNS(null, "x", xpos);
  shape.setAttributeNS(null, "y", ypos);
  shape.setAttributeNS(null, "width", 2*xoff);
  shape.setAttributeNS(null, "height", yoff*1.2);
  shape.setAttributeNS(null, "rx", xoff/20);
  shape.setAttributeNS(null, "ry", xoff/20);
  shape.setAttributeNS(null, "stroke", "black");
  shape.setAttributeNS(null, "stroke-width", "1");
  shape.setAttributeNS(null, "fill", WBOXFARBE);
  shape.setAttributeNS(null, "opacity", "0.8");
  shape.setAttributeNS(null, "filter", "url(#Schatten)");
  gruppe.appendChild(shape);

  // Wbox text0
  if(ident == "Tag")
    data = document.createTextNode("Aktuelle Daten:");
  else
    data = document.createTextNode("Erzeugte Energie:");
  shape = document.createElementNS(svgns, "text");
  shape.setAttributeNS(null, "id", ident+".wboxtxt0");
  shape.setAttributeNS(null, "x", xpos1);
  shape.setAttributeNS(null, "y", yoff*1.35);
  shape.setAttributeNS(null, "text-anchor", "middle");
  shape.setAttributeNS(null, "font-size", yoff*0.3);
  shape.setAttributeNS(null, "fill", "white");
  shape.appendChild(data);
  gruppe.appendChild(shape);

  // Wbox text1
  if(ident == "Tag")
    data = document.createTextNode("Leistung: 0 W");
  else if(ident == "Monat")
    data = document.createTextNode("Dieser Monat: 0 Wh");
  else
    data = document.createTextNode("Dieses Jahr: 0 Wh");
  shape = document.createElementNS(svgns, "text");
  shape.setAttributeNS(null, "id", ident+".wboxtxt1");
  shape.setAttributeNS(null, "x", xpos1);
  shape.setAttributeNS(null, "y", yoff*1.73);
  shape.setAttributeNS(null, "text-anchor", "middle");
  shape.setAttributeNS(null, "lengthAdjust", "spacing");
  shape.setAttributeNS(null, "font-size", yoff*0.3);
  shape.setAttributeNS(null, "fill", "white");
  shape.appendChild(data);
  gruppe.appendChild(shape);

  // Wbox text1L
  data = document.createTextNode("");
  shape = document.createElementNS(svgns, "text");
  shape.setAttributeNS(null, "id", ident+".wboxtxt1L");
  shape.setAttributeNS(null, "x", xpos1);
  shape.setAttributeNS(null, "y", yoff*1.73);
  shape.setAttributeNS(null, "text-anchor", "middle");
  shape.setAttributeNS(null, "lengthAdjust", "spacing");
  shape.setAttributeNS(null, "font-size", yoff*0.3);
  shape.setAttributeNS(null, "fill", "white");
  shape.appendChild(data);
  gruppe.appendChild(shape);

  // Wbox text1R
  data = document.createTextNode("");
  shape = document.createElementNS(svgns, "text");
  shape.setAttributeNS(null, "id", ident+".wboxtxt1R");
  shape.setAttributeNS(null, "x", xpos1);
  shape.setAttributeNS(null, "y", yoff*1.73);
  shape.setAttributeNS(null, "text-anchor", "start");
  shape.setAttributeNS(null, "lengthAdjust", "spacing");
  shape.setAttributeNS(null, "font-size", yoff*0.3);
  shape.setAttributeNS(null, "fill", "white");
  shape.appendChild(data);
  gruppe.appendChild(shape);

  // Wbox text2
  if(ident == "Tag")
    data = document.createTextNode("Energie: 0 Wh");
  else if(ident == "Monat")
    data = document.createTextNode("Letzter Monat: 0 Wh");
  else
    data = document.createTextNode("Letztes Jahr: 0 Wh");
  shape = document.createElementNS(svgns, "text");
  shape.setAttributeNS(null, "id", ident+".wboxtxt2");
  shape.setAttributeNS(null, "x", xpos1);
  shape.setAttributeNS(null, "y", yoff*2.1);
  shape.setAttributeNS(null, "text-anchor", "middle");
  shape.setAttributeNS(null, "lengthAdjust", "spacing");
  shape.setAttributeNS(null, "font-size", yoff*0.3);
  shape.setAttributeNS(null, "fill", "white");
  shape.appendChild(data);
  gruppe.appendChild(shape);

  // Wbox text2L
  data = document.createTextNode(t_wboxtxt2+"");
  shape = document.createElementNS(svgns, "text");
  shape.setAttributeNS(null, "id", ident+".wboxtxt2L");
  shape.setAttributeNS(null, "x", xpos1);
  shape.setAttributeNS(null, "y", yoff*2.1);
  shape.setAttributeNS(null, "text-anchor", "start");
  shape.setAttributeNS(null, "lengthAdjust", "spacing");
  shape.setAttributeNS(null, "font-size", yoff*0.3);
  shape.setAttributeNS(null, "fill", "white");
  shape.appendChild(data);
  gruppe.appendChild(shape);

  // Wbox text2R
  data = document.createTextNode(t_wboxtxt2+"");
  shape = document.createElementNS(svgns, "text");
  shape.setAttributeNS(null, "id", ident+".wboxtxt2R");
  shape.setAttributeNS(null, "x", xpos1);
  shape.setAttributeNS(null, "y", yoff*2.1);
  shape.setAttributeNS(null, "text-anchor", "end");
  shape.setAttributeNS(null, "lengthAdjust", "spacing");
  shape.setAttributeNS(null, "font-size", yoff*0.3);
  shape.setAttributeNS(null, "fill", "white");
  shape.appendChild(data);
  gruppe.appendChild(shape);

  // Gruppe ins Dokument einhängen
  document.documentElement.appendChild(gruppe);

  // Copyright
  xpos = fensterbreite-4, ypos = yoff + hoehe/2;
  data = document.createTextNode("(C) P. Hartmann, 59348 Lüdinghausen");
  shape = document.createElementNS(svgns, "text");
  shape.setAttributeNS(null, "id", "Copyright");
  shape.setAttributeNS(null, "x", xpos);
  shape.setAttributeNS(null, "y", ypos);
  shape.setAttributeNS(null, "text-anchor", "middle");
  shape.setAttributeNS(null, "font-size", yaxislabel_fontsize/2);
  shape.setAttributeNS(null, "fill", "black");
  shape.setAttributeNS(null, "transform", "rotate(-90,"+xpos+","+ypos+")");
  shape.appendChild(data);
  document.documentElement.appendChild(shape);
}

// Setzt globale Parameter
function global_parameters()
{
    var i,j;
    svgns = "http://www.w3.org/2000/svg";
    fensterbreite = window.innerWidth;
    fensterhoehe = window.innerHeight;
    xoff = 0.13 * fensterbreite;
    breite = 0.82 * fensterbreite;
    yoff = 0.15 * fensterhoehe;
    hoehe = 0.7 * fensterhoehe;
    mnamenlookup = new Array("Jan", "Feb", "Mrz", "Apr", "Mai", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dez");
    lmnamenlookup = new Array("Januar", "Februar", "März", "April", "Mai", "Juni", "Juli", "August", "September", "Oktober", "November", "Dezember");
    monatsnamen = new Array("Jan", "Feb", "Mrz", "Apr", "Mai", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dez");
    jahrestitel="Jahresübersicht";
    monatstitel = "Monatsübersicht"
    tagestitel = "Tagesübersicht"
    vortag = 99; // 99 ist beim Programmstart immer verschieden von jedem nat. Tag
    vormonat = 99;
    vorjahr = 10000;
    wiederhole_tag_update = 1;
    tag_last = "Letzte Daten vom";
    monat_last = "Letzte Daten vom";
    jahr_last = "Letzte Daten vom";
    t_wboxtxt1 = "";
    t_wboxtxt2 = "";
    m_wboxtxt1 = "";
    m_wboxtxt2 = "";
    j_wboxtxt1 = "";
    j_wboxtxt2 = "";
    // Werte erstmal willkürlich initialisieren
    monatswerte = new Array(10,20,30,40,50,60,70,80,90,100,110,120);
    akt_jahr = -1;
    akt_monat = -1;
    akt_monatswert_bis_gestern = 0;
    //monatstage = new Array(31,28,31,30,31,30,31,31,30,31,30,31);
    tagesnamen = new Array();
    tageswerte = new Array();
    bindatum = new Array();
    binmonat = new Array();
    for(i=0;i<31;i++) {
      tagesnamen[i] = (i+1).toString()+".";
      //tageswerte[i] = Math.random() * 10000;
    }
    minutennamen = new Array();
    minutenwerte = new Array();
    messwerte = new Array();
    for (i = 0; i < 180; i++)
      messwerte[i] = new Array(10);

    // 6:00 bis 21:00
    // Bin 0 = 6:00 - 6:05 usw.
    for(i=0;i<180;i++) {
      if(!(i%24)) {
	minutennamen[i] = (6+i/12).toString()+":00";
      } else {
	minutennamen[i] = "";
      }
      //minutenwerte[i] = Math.random() * 100;
    }
    jahresdaten = new Array([0,0],[0,0]);
    // Diese Skalierungen für die y-Achse gibt es
    yscales= new Array(
	      [1.2,0.9,0.6,0.3],
	      [1.5,1.2,0.9,0.6,0.3],
	      [1.6,1.2,0.8,0.4],
	      [1.8,1.2,0.6],
	      [2,1.5,1,0.5],
	      [2.1,1.4,0.7],
	      [2.4,1.8,1.2,0.6],
	      [2.5,2,1.5,1,0.5],
	      [2.7,1.8,0.9],
	      [2.8,2.1,1.4,0.7],
	      [3,2,1],
	      [4,3,2,1],
	      [4.5,3,1.5],
	      [5,4,3,2,1],
	      [6,4,2],
	      [7.5,5,2.5],
	      [8,6,4,2],
	      [9,6,3],
	      [10,8,6,4,2]
    );
  // Finde längste Anzahl an ticks f. y-Achse
    ymaxticks = 0;
    ymaxticksindex = 0;
    for(i=0;i<yscales.length;i++) {
      if(yscales[i].length > ymaxticks) {
	ymaxticks = yscales[i].length;
	ymaxticksindex = i;
      }
    }
    // Länge der Tick-Striche
    ticklength = 5;
  animation_stop = false;
}

// Trage nachgeladene Daten in Graph ein
function update_graph(ident,namen,werte,titel,last,wboxtxt1,wboxtxt2)
{
  var i,j,yticklabel_fontsize,dummy,yticks,ymaxscale,yeinh,txtbr,boxbr;
  var shape,txt0,txt1,txt2,wtxt1,wtxt2;
  var summe = new Array();

  // Y-Achsenskalierung anpassen
  // Berechne y-Achsenbeschriftung
  dummy = calc_ticks(werte,ident);
  ynamen = dummy[0];
  yticks = ynamen.length;
  ymaxscale = dummy[1];
  yeinh = dummy[2];

  for(i=0;i<ynamen.length;i++) {
    shape = document.getElementById(ident+".ytick."+i);
    shape.setAttributeNS(null, "y1", yoff+hoehe-(i+1)*hoehe/yticks);
    shape.setAttributeNS(null, "y2", yoff+hoehe-(i+1)*hoehe/yticks);
    shape.setAttributeNS(null, "visibility", "visible");

    shape = document.getElementById(ident+".yticklabel."+i);
    yticklabel_fontsize = shape.getAttributeNS(null, "font-size");
    shape.setAttributeNS(null, "y", yoff+hoehe-(i+1)*hoehe/yticks+yticklabel_fontsize/2);
    shape.setAttributeNS(null, "visibility", "visible");
    // Riskante referenzierung...
    shape.firstChild.replaceData(0,10,ynamen[i])
  }
  // Die übrigen Ticks müssen noch verschwinden
  while(i < ymaxticks) {
    shape = document.getElementById(ident+".ytick."+i);
    shape.setAttributeNS(null, "visibility", "hidden");
    shape = document.getElementById(ident+".yticklabel."+i);
    shape.setAttributeNS(null, "visibility", "hidden");
    i++;
  }
  // y-Achsenbeschriftung anpassen
  shape = document.getElementById(ident+".yaxislabel");
  // Riskante referenzierung...
  shape.firstChild.replaceData(0,10,yeinh)

  // x-ticklabelbeschriftung anpassen
  for(i=0;i<namen.length;i++) {
    shape = document.getElementById(ident+".xticklabel."+i);
    // Riskante referenzierung...
    shape.firstChild.replaceData(0,10,namen[i])
  }
  // Balken anpassen
  for(i=0;i<werte.length;i++) {
    shape = document.getElementById(ident+".balken."+i);   
    shape.setAttributeNS(null, "y", yoff+hoehe-werte[i]/ymaxscale*hoehe);
    shape.setAttributeNS(null, "height", werte[i]/ymaxscale*hoehe);
  }

  // Titel
  shape = document.getElementById(ident+".titel");   
  shape.firstChild.replaceData(0,100,titel)

  // Wbox
  // Text0: hole Textbreite
  txt0 = document.getElementById(ident+".wboxtxt0");   
  boxbr = txt0.getComputedTextLength();
  // Ersetze Text1 und hole Textbreite
  txt1 = document.getElementById(ident+".wboxtxt1");   
  txt1.firstChild.replaceData(0,100,wboxtxt1)
  txtbr = txt1.getComputedTextLength();
  // boxbr = größerer Wert
  if(txtbr > boxbr)
    boxbr = txtbr;

  // Left + Right aligned
  wboxtxt1LR = wboxtxt1.split(":");
  txt1L = document.getElementById(ident+".wboxtxt1L");
  txt1L.firstChild.replaceData(0,100,wboxtxt1LR[0]+":")
  txt1R = document.getElementById(ident+".wboxtxt1R");
  txt1R.firstChild.replaceData(0,100,wboxtxt1LR[1])

  // Ersetze Text2 und hole Textbreite
  txt2 = document.getElementById(ident+".wboxtxt2");   
  txt2.firstChild.replaceData(0,100,wboxtxt2)
  txtbr = txt2.getComputedTextLength();
  // Boxbreite = maximum + 10%
  if(txtbr > boxbr)
    boxbr = txtbr;

  // Left + Right aligned
  wboxtxt2LR = wboxtxt2.split(":");
  txt2L = document.getElementById(ident+".wboxtxt2L");
  txt2L.firstChild.replaceData(0,100,wboxtxt2LR[0]+":")
  txt2R = document.getElementById(ident+".wboxtxt2R");
  txt2R.firstChild.replaceData(0,100,wboxtxt2LR[1])

  txtbr = boxbr;
  boxbr *= 1.1;

  // Setze Box an die Stelle, an der sie am wenigsten stört
  // Breite der Box in Bins
  numbins = Math.ceil((boxbr * werte.length)/breite);
  // Bewerte Positionen, balken > 70%  schlecht
  for(i=0;i<=werte.length-numbins;i++) {
    summe[i] = 0;
    for(j=i;j<=i+numbins;j++) {
      summe[i] += (werte[j]/ymaxscale) > 0.7 ? 1 : 0;
    }
  }
  // Suche das rechteste Bin mit Minimum der Summe
  var boxbin = werte.length - numbins;
  var minsum = 100;
  for(i=werte.length-numbins;i>=0;i--) {
    if(summe[i] < minsum) {
      minsum = summe[i];
      boxbin = i;
    }
  }
  // Berechne Position aus Binnummer
  xpos = xoff + ((boxbin+numbins/2)/werte.length)*breite-boxbr/2;
  xpos1 = xpos + boxbr/2;

  // Box und Texte verschieben
  txt0.setAttributeNS(null, "x", xpos + boxbr*0.05);
  txt1.setAttributeNS(null, "x", xpos1);
  txt1.setAttributeNS(null, "textLength", "80");

  txt1.setAttributeNS(null, "fill-opacity", 0);
  txt1L.setAttributeNS(null, "x", xpos + boxbr*0.05);
  txt1R.setAttributeNS(null, "x", xpos + boxbr*0.95);

  txt2.setAttributeNS(null, "x", xpos1);
  txt2.setAttributeNS(null, "textLength", "80");

  txt2.setAttributeNS(null, "fill-opacity", 0);
  txt2L.setAttributeNS(null, "x", xpos + boxbr*0.05);
  txt2R.setAttributeNS(null, "x", xpos + boxbr*0.95);

  shape = document.getElementById(ident+".wbox");
  shape.setAttributeNS(null, "x", xpos);
  shape.setAttributeNS(null, "width", boxbr);
  
  // Updatestring
  shape = document.getElementById(ident+".last");   
  shape.firstChild.replaceData(0,100,last)
}

// Die folgenden Funktionen lesen
// Daten aus Files und stellt die Daten in
// globalen Arrays zur Verfügung
// months.dat : Enthält die integrierten Monatsdaten
// day_hist.dat : Enthält die integrierten Tageswerte
// day.dat : Enthält die Minutenwerte des heutigen Tags

function update_months_data(str)
{
  var i,j,monatsnr,jahr,wert,jahr_vor=999,jahresindex=-1,first_line=true;
  // var     jahresdaten=[[0,0],[0,0]];
  // Zeilen trennen
  tmparr = str.split('\n');
  // i zählt Zeilen, j zählt erfolgr. Daten
  // Am ersten des Monats
  j = 0;
  for(i=0;i<tmparr.length && i < 24;i++) {
    // Suche 8 Int 0-2 = Datum
    Ergebnis = tmparr[i].match(/[0-9]+/g);
    if(Ergebnis && Ergebnis.length == 8) {
      monatsnr = parseInt(Ergebnis[1],10) - 1;
      jahr = parseInt(Ergebnis[2],10);
      // Addiere alle 5 WR-Werte (*1 wg str->int) kann man besser machen
      wert = parseInt(Ergebnis[3],10)+parseInt(Ergebnis[4],10)+parseInt(Ergebnis[5],10)+parseInt(Ergebnis[6],10)+parseInt(Ergebnis[7],10);
      // Nur die 12 letzten Monate anzeigen, aber alle Monate des Vorjahrs in den Jahresertrag mitnehmen
      if(j < 12) {
	if(first_line) {
	  if(akt_monat != monatsnr+1) {
	    akt_monatswert_bis_gestern = 0;
	    monatsnamen[11] = mnamenlookup[akt_monat-1];
	    monatswerte[11] = tageswerte[30];
	    j = 1;
	  }
	  first_line=false;
	}
	// Hole Monatsnamen zu Monatsstring
	binmonat[11-j] = Ergebnis[1] + "_" + Ergebnis[2];
	monatsnamen[11-j] = mnamenlookup[monatsnr];
	if(j == 0) {
	  monatswerte[11-j] = wert + tageswerte[30];
	  akt_monatswert_bis_gestern = wert;
	} else {
	  monatswerte[11-j] = wert;
	}
      }
      // Akkumuliere Jahresertrag
      if(jahr != jahr_vor) {
	jahresindex++;
	jahresdaten[jahresindex][0] = jahr;
	jahresdaten[jahresindex][1] = 0;
	jahr_vor = jahr;
      }
      jahresdaten[jahresindex][1] += wert;
      j++;
    }
  }
  // Titel der Jahresdaten
  if(jahresindex == 0)
    jahrestitel = "Jahresübersicht "+Number(jahresdaten[jahresindex][0]+2000);
  else if(jahresindex == 1)
    jahrestitel = "Jahresübersicht "+Number(jahresdaten[jahresindex][0]+2000)+"/"+Number(jahresdaten[jahresindex-1][0]+2000);

  // Gesamtenergie Jahre
  while(jahresindex >= 0) {
    if(jahresindex == 1) {
      tmpstr = num2kstr(jahresdaten[jahresindex][1]);
      j_wboxtxt2 = Number(jahresdaten[jahresindex][0]+2000)+": "+tmpstr[0]+" "+tmpstr[1]+"Wh";
    }
    jahresindex--;
  }

  // Gesamtenergie Monate
  tmpstr = num2kstr(monatswerte[11]);
  m_wboxtxt1 = monatsnamen[11]+".: "+tmpstr[0]+" "+tmpstr[1]+"Wh";
  tmpstr = num2kstr(monatswerte[10]);
  m_wboxtxt2 = monatsnamen[10]+".: "+tmpstr[0]+" "+tmpstr[1]+"Wh";
  tmpstr = num2kstr(jahresdaten[0][1]+tageswerte[30]);
  j_wboxtxt1 = Number(jahresdaten[0][0]+2000)+": "+tmpstr[0]+" "+tmpstr[1]+"Wh";

  // Falls die Anlage noch nicht 12 Monate läuft, monatswerte auffüllen
  --monatsnr
  while(j < 12) {
    monatsnamen[11-j] = mnamenlookup[monatsnr];
    monatswerte[11-j] = 0;
    monatsnr  = (monatsnr - 1 >= 0)?monatsnr-1:11;
    j++;
  }
  update_graph("Jahr",monatsnamen,monatswerte,jahrestitel,jahr_last,j_wboxtxt1,j_wboxtxt2);
}

function read_months_data()
{
  var req = new XMLHttpRequest(),dateiname;
  // Caching umgehen...Ok, so ist's suboptimal.
  dateiname='months.dat?'+Math.random();
  req.open('GET', dateiname, true);
  //req.channel.loadFlags |= Components.interfaces.nsIRequest.LOAD_BYPASS_CACHE;
  req.onreadystatechange = function (aEvt) {
    if (req.readyState == 4) {
      if(req.status == 200) {
	var a = new Date();
	jahr_last = "Letzte Daten vom "+a.getDate()+"."+(Number(a.getMonth())+1)+". "+a.getHours()+":"+(Number(a.getMinutes())>9?"":"0")+a.getMinutes();
	update_months_data(req.responseText);
      }
    }
  }
  req.send(null);
}

// Update der Tagesdaten aus dem geladenen File
function update_days_data(str)
{
  var i,j,jahr,monat,tag,monat_akt,monat_akt_setzen=true;

  // Array initialisieren, letzter Wert wird von 
  // update_mins_data verwaltet
  for(i=0;i<30;i++)
    tageswerte[i] = 0;
  // Zeilen trennen
  tmparr3 = str.split('\n');
  for(i=0;i<tmparr3.length && i < 30;i++) {
    // Suche 13 Int 0-2 = Datum
    Erg3 = tmparr3[i].match(/[0-9]+/g);
    if(Erg3 && Erg3.length == 13) {
      tag = parseInt(Erg3[0],10);
      monat = parseInt(Erg3[1],10) - 1;
      jahr = parseInt(Erg3[2],10);
      bindatum[29-i] = Erg3[2]+Erg3[1]+Erg3[0];
      tageswerte[29-i] = (Erg3[3]*1)+(Erg3[5]*1)+(Erg3[7]*1)+(Erg3[9]*1)+(Erg3[11]*1);
      tagesnamen[29-i] = tag+'.'+mnamenlookup[monat];
      j = i;
    }
  }
  var cdate = new Date(jahr,monat,tag,12,0,0);
  var ndate = new Date();
  var start = j;
  var step = 24 * 3600000;
  while(++j < 30) {
    ndate.setTime(cdate.getTime() -((j-start)*step));
    tagesnamen[29-j] = ndate.getDate() + "." + mnamenlookup[ndate.getMonth()];
  }
  update_graph("Monat",tagesnamen,tageswerte,monatstitel,monat_last,m_wboxtxt1,m_wboxtxt2);
}

function read_days_data()
{
  var req = new XMLHttpRequest(), dateiname;
  // Caching umgehen...Ok, so ist's suboptimal.
  dateiname='days_hist.dat?'+Math.random();
  req.open('GET', dateiname, true);
  //req.channel.loadFlags |= Components.interfaces.nsIRequest.LOAD_BYPASS_CACHE;
  req.onreadystatechange = function (aEvt) {
    if (req.readyState == 4) {
      if(req.status == 200) {
	var a = new Date();
	monat_last = "Letzte Daten vom "+a.getDate()+"."+(Number(a.getMonth())+1)+". "+a.getHours()+":"+(Number(a.getMinutes())>9?"":"0")+a.getMinutes();
	update_days_data(req.responseText);
      }
    }
  }
  req.send(null);
}

// Liest und verarbeitet die Minutendaten
function update_mins_data(str)
{
  var i,j,monat,tag,datum_setzen=true,tagessumme=0,tmpstr;

  // Array initialisieren
  for(i=0;i<180;i++)
    minutenwerte[i] = 0;
  // Verwalten heute-Wert in tageswerte
  tageswerte[30] = 0;
  // Zeilen trennen
  tmparr2 = str.split('\n');
  for(i=0;i<tmparr2.length;i++) {
    // Suche 26 Int 0-2 = Datum, 3 - 5 = Zeit
    Erg = tmparr2[i].match(/[0-9]+/g);
    if(Erg && Erg.length == 26) {
      if(datum_setzen) {
	tag = parseInt(Erg[0],10);
	monat = parseInt(Erg[1],10);
	akt_monat = monat;
	akt_jahr = parseInt(Erg[2],10);
	tagestitel = tag+". "+lmnamenlookup[monat-1]+" 20"+Erg[2];
	tageswerte[30] = (Erg[8]*1)+(Erg[12]*1)+(Erg[16]*1)+(Erg[20]*1)+(Erg[24]*1);
	tagesnamen[30] = tag+'.'+mnamenlookup[monat-1];
	tmpstr = num2kstr(tageswerte[30]); //Tagesleistung
	t_wboxtxt2 = "Energie: "+tmpstr[0]+" "+tmpstr[1]+"Wh";
	tmpstr = num2kstr(((Erg[6]*1)+(Erg[10]*1)+(Erg[14]*1)+(Erg[18]*1)+(Erg[22]*1))); //Momentane Leistung
	t_wboxtxt1 = "Leistung: "+tmpstr[0]+" "+tmpstr[1]+"W";
	datum_setzen = false;
      }
      ind = ((Erg[3]*1)-6)*12 + Erg[4]/5;
      if(ind >= 0 && ind < 180)
	minutenwerte[ind] = (Erg[6]*1)+(Erg[10]*1)+(Erg[14]*1)+(Erg[18]*1)+(Erg[22]*1);
    }
  }
  monatstitel = lmnamenlookup[((monat-2)>=0?monat-2:11)]+' / '+lmnamenlookup[monat-1];
  update_graph("Tag",minutennamen,minutenwerte,tagestitel,tag_last,t_wboxtxt1,t_wboxtxt2);
  read_meas_data();
  if(tag != vortag) {
    vortag = tag;
    // Der Datenupdate muss am Morgen evtl. mehrmals wiederholt werden bis alle Files aktualisiert sind
    wiederhole_tag_update = 2;
    // Reihenfolge!
    read_months_data();
    read_days_data();
  } else if(wiederhole_tag_update > 0) {
    wiederhole_tag_update--;
    // Reihenfolge!
    read_months_data();
    read_days_data();
  } else {
    // Gesamtenergie dieser Monat
    tmpstr = num2kstr(akt_monatswert_bis_gestern + tageswerte[30]);
    monatswerte[11] = akt_monatswert_bis_gestern + tageswerte[30];
    m_wboxtxt1 = monatsnamen[11]+".: "+tmpstr[0]+" "+tmpstr[1]+"Wh";
    // Gesamtenergie dieses Jahr
    tmpstr = num2kstr(jahresdaten[0][1]+tageswerte[30]);
    j_wboxtxt1 = Number(jahresdaten[0][0]+2000)+": "+tmpstr[0]+" "+tmpstr[1]+"Wh";
    // Grafiken aktualisieren
    update_graph("Monat",tagesnamen,tageswerte,monatstitel,monat_last,m_wboxtxt1,m_wboxtxt2);
    update_graph("Jahr",monatsnamen,monatswerte,jahrestitel,jahr_last,j_wboxtxt1,j_wboxtxt2);
  }
}

function read_mins_data()
{
  var req = new XMLHttpRequest(),dateiname;
  // Caching umgehen...Ok, so ist's suboptimal.
  dateiname='day.dat?'+Math.random();
  req.open('GET', dateiname, true);
  // Eigentlich sollte man den cache so umgehen. Das funktioniert aber nicht zuverlässig.
  //req.channel.loadFlags |= Components.interfaces.nsIRequest.LOAD_BYPASS_CACHE;
  req.onreadystatechange = function (aEvt) {
    if (req.readyState == 4) {
      if(req.status == 200) {
	var a = new Date();
	tag_last = "Letzte Daten vom "+a.getDate()+"."+(Number(a.getMonth())+1)+". "+a.getHours()+":"+(Number(a.getMinutes())>9?"":"0")+a.getMinutes();
	update_mins_data(req.responseText);
      }
    }
  }
  req.send(null);
  // Alle 5 Minuten Daten neu laden
  setTimeout("read_mins_data()",300000);
}

// Liest und verarbeitet die zus. Messdaten
function update_meas_data(str)
{
  var i;

  // Array initialisieren
  tmparr2 = str.split('\n');
  for(i=0;i<tmparr2.length;i++) {
    // Suche 12 Int 0-2 = Datum, 3 - 5 = Zeit
    Erg = tmparr2[i].match(/[0-9]+/g);
    if(Erg && Erg.length > 13) {
      ind = (parseInt(Erg[3],10)-6)*12 + parseInt(Erg[4],10)/5;
      if(ind >= 0 && ind < 180) {
	messwerte[ind][0] = parseInt(Erg[6],10);
	messwerte[ind][1] = parseInt(Erg[7],10);
	messwerte[ind][2] = parseInt(Erg[8],10);
	messwerte[ind][3] = parseInt(Erg[9],10);
	messwerte[ind][4] = parseFloat(Erg[10],10)/10.0;
	messwerte[ind][5] = parseFloat(Erg[11],10)/10.0;
	messwerte[ind][6] = parseFloat(Erg[12],10);
	messwerte[ind][7] = parseFloat(Erg[13],10);
	messwerte[ind][8] = minutenwerte[ind];
	messwerte[ind][9] = Math.floor(100.0*minutenwerte[ind]/(messwerte[ind][6]+messwerte[ind][7])+0.5);
      }
    }
  }
}

function read_meas_data()
{
  var req = new XMLHttpRequest(),dateiname;
  // Caching umgehen...Ok, so ist's suboptimal.
  dateiname='meas.dat?'+Math.random();
  req.open('GET', dateiname, true);
  // Eigentlich sollte man den cache so umgehen. Das funktioniert aber nicht zuverlässig.
  //req.channel.loadFlags |= Components.interfaces.nsIRequest.LOAD_BYPASS_CACHE;
  req.onreadystatechange = function (aEvt) {
    if (req.readyState == 4) {
      if(req.status == 200) {
	update_meas_data(req.responseText);
      }
    }
  }
  req.send(null);
}

// Diese Funktion sorgt für den Bildwechsel und das Überblenden der Graphen
function animiere()
{
  if(animation_stop) {
    setTimeout("animiere()", 100);
    return;
  }
  // Setze neue Werte für die Opazität
  dieses.setAttributeNS(null, "fill-opacity", 1-z/30);
  dieses.setAttributeNS(null, "stroke-opacity", 1-z/30);
  naechstes.setAttributeNS(null, "fill-opacity", z/30);
  naechstes.setAttributeNS(null, "stroke-opacity", z/30);
  // Falls Überblenden fertig ist warte länger
  if ( ++z > 30 ) {
    //Fixme: This is a really dirty trick to get naechstes to the front
    // SVG designers, please introduce the z-index in the next release!
    naechstes.parentNode.appendChild(naechstes);
    var ttboxelem=document.getElementById("ttbox");
    ttboxelem.parentNode.appendChild(ttboxelem);
    z = 0;
    k = (k+1)%3;
    dieses = naechstes;
    naechstes = groups[k];
    setTimeout("animiere()", ANZEIGEDAUER*1000);
  } else {
    setTimeout("animiere()", 10);
  }
}
