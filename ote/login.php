<?php
include "sec.inc";

include "roundcubeLogin.inc";
$baseDir = "/home/gregoire/mail/";
$url = "https://gentil.com/mail/";
$local = "localhost";

if (isset($_POST["email"]))
	$email = strtolower($_POST["email"]);
else if (isset($_GET["email"]))
	$email = strtolower($_GET["email"]);
else {
	echo "Error, no email provided";
	exit;
}

$ar = explode("@", $email);
$handle = fopen($baseDir . "/password-" . $ar[1], "r");
$password = "";
while (($line = fgets($handle)) !== false) {
	if (str_starts_with($line, $email)) {
		$line_ = explode(":", trim($line));
		$password = str_replace("{plain}", "", $line_[1]);
		break;
	}
}
fclose($handle);

login($url, $email, $password, $local);
?>
