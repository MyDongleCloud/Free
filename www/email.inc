<?php
date_default_timezone_set('America/Los_Angeles');
$DATE = date("Ymd_His");
$EMAIL = $_POST["email"];
$handle = fopen("sec/email.txt", "a");
fputs($handle, $DATE . ": " . $EMAIL . "\n");
fclose($handle);
?>
