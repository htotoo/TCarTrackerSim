<?php
$uri = "https://hooks.nabu.casa/aaaaaaabbbbbbccc";   //put here your Home Assistant webhook for gps tracker
$uri2 = "http://MYTRACCARURL:50550/?id=982356";  //put here yout TRACCAR url, for osmand protocol.
//if you want to disable traccar, simply write $uri2 = ""; same to the HA.


date_default_timezone_set("Europe/Budapest");
$imei = "1";//(isset($_REQUEST['i']))?$_REQUEST['i']:"";  //todo send imei from car too
//if (strlen($imei)!=15) die("ERROR\r\n");

$lat = (isset($_REQUEST['la']))?$_REQUEST['la']:""; //ns
if (strlen($lat)<=3) die("ERROR\r\n");

$lon = (isset($_REQUEST['lo']))?$_REQUEST['lo']:""; //ew
if (strlen($lon)<=3) die("ERROR\r\n");

$valid = (isset($_REQUEST['v']))?$_REQUEST['v']:""; //i = gps, M = mobile, N = invalid
if (strlen($valid)!=1) die("ERROR\r\n");

$speed = (isset($_REQUEST['s']))?(float)$_REQUEST['s']:"0";

$altitude = (isset($_REQUEST['a']))?(float)$_REQUEST['a']:"0"; 
$hdop = (isset($_REQUEST['ac']))?(float)$_REQUEST['ac']:0; //hdop
$satnum = (isset($_REQUEST['sn']))?$_REQUEST['sn']:0; //in use sat number
$battery = (isset($_REQUEST['ba']))?$_REQUEST['ba']:0; //battery
$acc = (isset($_REQUEST['ac']))?$_REQUEST['ac']:0; //accuracy
$vt = (isset($_REQUEST['vt']))?$_REQUEST['vt']:0; //wakeup by timer 1 = yes
$anyaccel = (isset($_REQUEST['anyaccel']))?$_REQUEST['anyaccel']:0; //was there any accelerator related event? 1 = yes
$rion = (isset($_REQUEST['rion']))?$_REQUEST['rion']:0; //is the radio module on?
$alh = (isset($_REQUEST['alh']))?$_REQUEST['alh']:0; //accel last hit in sec

if ($speed<1) $speed = 0; //it stays in place, but gps fakes movement.


$tt = time();//todo fix
$needalert = ($tt+15*60 < time());
$long_deg = (float)substr($lon, 1,3);
$long_min = (float)substr($lon, 4);
$longitude  = $long_deg + $long_min/60;

$lat_deg = (float)substr($lat, 1,2);
$lat_min = (float)substr($lat, 3);
$latitude = $lat_deg + $lat_min/60;


$longitude = $lon;
$latitude = $lat;

if ($needalert) echo "!";

$date = date ("Y-m-d H:i:s");

echo "OK\r\n";

if ($latitude == 0 && $longitude == 0) die("0");

if ( ( (int)$speed) >= 5) $act = "moving"; else $act = "still";
if ($vt == 1 && $anyaccel == 0) $act = "nothing";
if ($acc ==0) $acc = $hdop * $hdop * 4;
$batt = $battery;
$dev = "FordFocus";
$alt = $altitude;
$provider = "GPS";
if ($valid == "M") $provider = "MOBILE";
$provider .= "-". $satnum . "-" . date("Y-m-d%20H:i:s");

if (strlen($uri)>2)
{
$data = "latitude=$latitude&longitude=$longitude&device=$dev&accuracy=$acc&battery=$batt&speed=$speed&direction=0&altitude=$alt&provider=$provider&activity=$act";

$options = array(
    'http' => array(
        'header'  => "Content-type: application/x-www-form-urlencoded\r\n",
        'method'  => 'POST',
        'timeout' => 10,
        'content' => $data
    ));
$context  = stream_context_create($options);
$result = file_get_contents($uri, false, $context);
}

//osmand
ini_set('default_socket_timeout', 15); 
$acc *= 10;
if (strlen($uri2)>2)
{
    $uri2 .= "&timestamp=". time() ."&lat=".$latitude."&lon=".$longitude."&speed=".$speed."&bearing=0.0&altitude=".$alt."&hdop=".$hdop."&accuracy=".$acc."&batt=$batt";

    $result = file_get_contents($uri2);
    //echo $uri;
    //echo $result;
}
?>