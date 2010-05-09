<?php
//
//
// Copyright (c) 2009 by Peter Hartmann <solometerklammeraffehartmann-peter.de>
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

$uploadpath = "upload/";
$out_datei = "day.dat";
$hist_datei = "days_hist.dat";
$month_datei = "months.dat";

$log_datei = "upload_log.html";
$write_logdatei = 1;
$KEEP_RAW_FILES = RawFilesKept;
//$ACCEPTED_COOKIE = "MeinKeks";
$ACCEPTED_COOKIES = array(
  // "nutg76f8lo" => "SMA Sunny Boy2100TL",
  "MeinKeks" => "Effekta ES3300"
  );

function cleanup($dirname,$keep_secs)
{
  $current_time = time();
  $dircontent = scandir($dirname);
  foreach($dircontent as $filename) {
    if ($filename != '.' && $filename != '..') {
      $arr = explode(".",$filename);
      if($current_time - hexdec($arr[0]) > $keep_secs) {
	$pstr .= "Deleting ".$filename.".<BR>";
	unlink($dirname.$filename);
      } else {
	$pstr .= $filename." not deleted.<BR>";
      }
    }
  }
  return $pstr;
}

// Header and Footer of return page
$HEAD="<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\"  \"http://www.w3.org/TR/html4/strict.dtd\">\n<html>\n<head><title>Upload</title></head>\n<body>";
$FOOT="</body></html>";
// Return string
$pstr .= $HEAD;

$cookie = htmlspecialchars($_COOKIE["PVID"]);
//$pstr .= "Cookie: ".$cookie. "<br />";
$ac = array_keys($ACCEPTED_COOKIES);
$k = array_search($cookie,$ac);
if($k === FALSE) {
  $pstr .= "Wrong Cookie: ".$cookie."<BR>";
  $pstr .= $FOOT;
  if($write_logdatei > 0) {
    $datei = fopen ($log_datei, "w");
    fputs($datei,$pstr);
    fclose($datei);
  }
  return $pstr;
} else {
  $pstr .= "Cookie ".$cookie." found at position ".$k."<BR>";
}

// File upload
if (($_FILES["file"]["type"] == "application/octet-stream") && ($_FILES["file"]["size"] < 20000)) {

  if ($_FILES["file"]["error"] > 0) {
    $pstr .= "Error: " . $_FILES["file"]["error"] . "<br />";
  }

    move_uploaded_file($_FILES["file"]["tmp_name"], $uploadpath.$_FILES["file"]["name"]);
    $pstr .= "Ok: "  . $_FILES["file"]["name"]. "<br />";
} else {
  $pstr .= "Error: Invalid file";
}

// Letzte nicht leere Zeile des Tagesfiles einlesen um alte Werte zu ermitteln
if (file_exists($out_datei)) {
  $min_day = fopen ($out_datei, "r");
  $md_zeile = fgets ($min_day, 1024);
  fclose($min_day);
  // Bisherigen Tag, Monat, Jahr und bisherige Energie ermitteln
  if(($el_found = preg_match_all("/[0-9]+/",$md_zeile,&$a,PREG_PATTERN_ORDER)) > 8) {
    $pstr .= "<br/>Hole Werte."; 
    $md[0] = $a[0][0]; //day
    $md[1] = $a[0][1]; //month
    $md[2] = $a[0][2]; //year
    $md[3] = $a[0][8]; //sum
  } else {
    $pstr .= "<br/>Keine Werte gefunden in \"".$md_zeile."\""; 
    $md[0] = 0;$md[1] = 0;$md[2] = 0;$md[3] = 0;
  }
} else {
  $pstr .= "<br/>Altes File nicht zu oeffnen."; 
  $md[0] = 0;$md[1] = 0;$md[2] = 0;$md[3] = 0;
}
$pstr .= "<br/>Tag:".$md[0]." Monat:".$md[1]." Jahr:".$md[2]." Summe:".$md[3]; 

// Über Daten aus eben hochgeladener Datei mitteln
$summe = 0;
$num = 0;
$datum = "";
//$in_datei = $_FILES["file"]["name"];
$in_datei = $uploadpath.$_FILES["file"]["name"];
if ($datei = fopen ( $in_datei, "r")) {
  while (!feof ($datei)) {
    // Zeilenweise einlesen
    $zeile = fgets ($datei, 1024);
    // Numerische Daten per regexp in Array einlesen (Mind. 7: Tag, Monat, Jahr, Stunde, Minute, Sekunde, Messwert(e))
    if(($el_found = preg_match_all("/[0-9]+/",$zeile,&$a,PREG_PATTERN_ORDER)) > 6) {
      // Falls dies der erste Schleifenaufruf ist, ist $datum == ""
      if(strlen($datum) < 1) {
	$pstr .= "<br/>Erster Schleifendurchlauf";
	// Falls Tagesfile existiert und Tag des geposteten Files != Tag des Datenfiles
	$pstr .= sprintf("<br/>Tag ex. File:%d, Tag neue daten: %d <br>",$md[0],$a[0][0]);
	if(($md[0] != 0) && ($a[0][0] != $md[0])) {
	  if(file_exists($out_datei)) {
	    $log_datei = "upload_log_newday.html";
	    // Tagesdatei unter neuem Namen abspeichern
	    $newname = sprintf("day%02d%02d%02d.dat",$md[2],$md[1],$md[0]);
	    rename($out_datei,$newname);
	  }

	  // Akkumulierte Energie des bisherigen Tagesfiles abspeichern
	  // Append to days_hist.dat "15.3.09|s;0|0;0|0;0|0;0|0;0"
	  if((file_exists($hist_datei)) && (($fs = filesize($hist_datei)) > 0)) {
	    $datei1 = fopen ($hist_datei, "r+");
	    $str = fread ($datei1, $fs);
	    rewind($datei1);
	    fputs($datei1,sprintf("%02d.%02d.%02d|%d;0|0;0|0;0|0;0|0;0\n",$md[0],$md[1],$md[2],$md[3]));
	    fwrite($datei1,$str);
	    fclose($datei1);
	  } else {
	    $datei1 = fopen ($hist_datei, "a+");
	    fputs($datei1,sprintf("%02d.%02d.%02d|%d;0|0;0|0;0|0;0|0;0\n",$md[0],$md[1],$md[2],$md[3]));
	    fclose($datei1);
	  }

	  // Update der Monatsenergie
	  // Öffne Monatsfile months.dat
	  if (file_exists($month_datei)) {
	    $pstr .= sprintf("<br/>Öffne Monatsdatei %d.",filesize($month_datei));
	    $fs = filesize($month_datei);
	    $datei1 = fopen ( $month_datei, "r+");
	    // Hole Fileinhalt
	    $str = fread ($datei1, $fs);
	    $pstr .= sprintf("<br/>Datei-inhalt:--%s--",$str);
	    $mon = explode( "\n", $str);
	    $pstr .= sprintf("<br/>Mon[0] = %s",$mon[0]);
	    $pstr .= sprintf("<br/>Mon[1] = %s",$mon[1]);
	    // Falls die erste Zeile einen vern. Wert enthält
	    if(($el_found1 = preg_match_all("/[0-9]+/",$mon[0],&$b,PREG_PATTERN_ORDER)) > 4) {
	      $pstr .= sprintf("<br/>el_found1 = %d",$el_found1);
	      // Falls gleicher Monat
	      if($b[0][1] == $md[1]) {
		$pstr .= sprintf("<br/>Gleicher Monat. Addiere Werte %d %d.",$b[0][3],$md[3]);
		// Addiere Werte und
		$res = $b[0][3] + $md[3];
		// Ersetze erste Zeile
		$beginn = 1;
	      } else {
		$log_datei = "upload_log_newmonth.html";
		$pstr .= sprintf("<br/>Neuer Monat. Schreibe Wert %d.",$md[3]);
		// Starte neuen Monat und
		$res = $md[3];
		// Rücke erst Zeile nach hinten
		$beginn = 0;
	      }
	    }
	    rewind($datei1);
	    $pstr .= sprintf("<br/>Schreibe: %02d.%02d.%02d|%d|0|0|0|0",$md[0],$md[1],$md[2],$res);
	    // Schreibe neue erste Zeile
	    fputs($datei1,sprintf("%02d.%02d.%02d|%d|0|0|0|0\n",$md[0],$md[1],$md[2],$res));
	    // Schreibe Rest des Files, beginnend bei 2. Zeile
	    for($i=$beginn;$i<sizeof($mon);$i++) {
	      if(strlen($mon[$i]) > 8) { //Do not add newlines
		fputs($datei1,$mon[$i]."\n");
		$pstr .= sprintf("<br/>Schreibe: %s",$mon[$i]);
	      }
	    }
	    fclose($datei1);
	  } else {
	    $pstr .= "<br/>Erzeuge neue Datei.";
	    $datei1 = fopen ( $month_datei, "w+");
	    $pstr .= sprintf("<br/>Schreibe: %02d.%02d.%02d|%d|0|0|0|0",$md[0],$md[1],$md[2],$md[3]);
	    fputs($datei1,sprintf("%02d.%02d.%02d|%d|0|0|0|0\n",$md[0],$md[1],$md[2],$md[3]));
	    fclose($datei1);
	  }

	  // Falls sich auch der Monat geändert hat
	  if($a[0][1] != $md[1]) {
	    $md[1] = $a[0][1];
	  }
	  // Setze neuen Tag und setze Tagesleistung = 0
	  $md[0] = $a[0][0];$md[3] = 0;
	}
	// Erzeuge einen neuen Datumsstring
	$datum = sprintf("%02d.%02d.%02d %02d:%02d:%02d",$a[0][0],$a[0][1],$a[0][2],$a[0][3],$a[0][4],$a[0][5]);
      }
      // Summiere Messwert
      $num += 1;
      $summe += $a[0][6];
    }
  }
  fclose($datei);
}

// Öffne Tagesdatei und füge gemittelten Wert am Anfang ein
if((file_exists($out_datei)) && (($fs = filesize($out_datei)) > 0)) {
  $datei = fopen ($out_datei, "r+");
  $str = fread ($datei, $fs);
  rewind($datei);
  fputs($datei,$datum."|".intval($summe/$num).";0;".intval($md[3]+$summe/(12*$num)).";0");
  fputs($datei,"|0;0;0;0|0;0;0;0|0;0;0;0|0;0;0;0\n");
  fwrite($datei,$str);
  fclose($datei);
} else {
  $datei = fopen ($out_datei, "a+");
  fputs($datei,$datum."|".intval($summe/$num).";0;".intval($md[3]+$summe/(12*$num)).";0");
  fputs($datei,"|0;0;0;0|0;0;0;0|0;0;0;0|0;0;0;0\n");
  //fputs($datei,$datum." ".$summe/$num."\n");
  fclose($datei);
}

// Delete old raw files
$pstr .= cleanup($uploadpath,$KEEP_RAW_FILES*24*3600);

$pstr .= $FOOT;
if($write_logdatei > 0) {
  $datei = fopen ($log_datei, "w");
  fputs($datei,$pstr);
  fclose($datei);
}
echo $pstr;
?> 