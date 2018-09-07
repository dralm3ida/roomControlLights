<?php
$SERIALPORT = "COM4";

include("PhpSerial.php");
@header("Content-Type: application/json");

$serial = new PhpSerial;
$serial->deviceSet($SERIALPORT);
$serial->confBaudRate(9600);
$serial->confParity("none");
$serial->confCharacterLength(8);
$serial->confStopBits(1);
$serial->confFlowControl("none");

$serial->deviceOpen();
$serial->sendMessage("0");
$serial->serialflush();

echo $serial->readPort(1);

$serial->deviceClose();

//var_dump($_SERVER);



?>
