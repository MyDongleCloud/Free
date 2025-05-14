<?php
$IPADDRESS = $_SERVER['REMOTE_ADDR'];
global $IPAUTHORIZED;
include "/etc/apache2/sites-enabled/ippaloalto.inc";
if ($IPADDRESS != $IPAUTHORIZED) {
	date_default_timezone_set('America/Los_Angeles');
	$DATE = date("Ymd_His");
	$handle = fopen("sec/log.txt", "a");
	fputs($handle, $DATE . ": " . $IPADDRESS . "\n");
	fclose($handle);
}
?>
