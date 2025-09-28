<?php

function doProcess() {
	$baseDir = "/home/gregoire/mail/";
	$domain = "mail2.cloud";

	$dbPath = $baseDir . "ote-db.txt";
	$handle = fopen($dbPath, "r");
	$content = trim(fread($handle, filesize($dbPath)));
	if (strlen($content) == 0)
		return;
	$content_ = explode("\n", $content);
	fclose($handle);
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
	$oteAliasH = fopen($baseDir . "otealias", "w");
	fwrite($oteAliasH, $oteAlias);
	fclose($oteAliasH);
	$oteMapsH = fopen($baseDir . "otemaps", "w");
	fwrite($oteMapsH, $oteMaps);
	fclose($oteMapsH);
	$otePasswordH = fopen($baseDir . "password-" . $domain, "w");
	fwrite($otePasswordH, $otePassword);
	fclose($otePasswordH);
	shell_exec("postmap " . $baseDir . "otemaps; postmap " . $baseDir . "otealias");
}

if (php_sapi_name() === 'cli') {
	doProcess();
}
?>
