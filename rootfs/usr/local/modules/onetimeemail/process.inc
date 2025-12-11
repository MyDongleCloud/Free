<?php

function doProcess() {
	$baseDir = "/home/gregoire/mail/";
	$domain = "mail2.cloud";

	$dbPath = $baseDir . "ote-db.txt";
	$content = trim(file_get_contents($dbPath));
	if (strlen($content) == 0)
		return;
	$content_ = explode("\n", $content);
	$oteAlias = "";
	$oteMaps = "";
	$otePassword = "";
	for ($i = 0; $i < count($content_); $i++) {
		$ar = explode("|", $content_[$i]);
		$id = intval($ar[0]);
		$account = $ar[1];
		$from = $ar[2];
		$to = $ar[3];
		$expiration = intval($ar[4]);
		if ($expiration == 0 || $expiration > time()) {
			if ($to == "") {
				$from_ = explode("@", $from);
				$oteMaps .= "" . $from . "\t" . $from_[1] . "/" . $from_[0] . "/\n";
				if (!file_exists($baseDir . $from_[1] . "/" . $from_[0]))
					mkdir($baseDir . $from_[1] . "/" . $from_[0], 0777);
			} else
				$oteAlias .= "" . $from . "\t" . $to . "\n";
		}
		if ($to == "")
			$otePassword .= "" . $from . ":{plain}" .  bin2hex(random_bytes(5)) . "\n";
	}
	file_put_contents($baseDir . "otealias", $oteAlias);
	file_put_contents($baseDir . "otemaps", $oteMaps);
	file_put_contents($baseDir . "password-" . $domain, $otePassword);
	shell_exec("postmap " . $baseDir . "otemaps; postmap " . $baseDir . "otealias");
}

if (php_sapi_name() === 'cli') {
	doProcess();
}
?>
