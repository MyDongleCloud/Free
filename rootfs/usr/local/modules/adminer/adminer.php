<?php
define('ADMINER_DIR', '/usr/share/adminer');
function adminer_object() {
	include_once ADMINER_DIR . "/plugins/drivers/mongo.php";
	foreach (glob(ADMINER_DIR . "/plugins/*.php") as $filename)
		include_once $filename;
	$plugins = array(
		new AdminerVersionNoverify(),
	);
	class Adminer2 extends Adminer\Adminer {
		function login($login, $password) {
			if ($password === "" && $_SERVER["auth[driver]"] == "sqlite")
				return true;
		}
	}
    return new Adminer2($plugins);
}
include ADMINER_DIR . "/adminer.php";
?>
