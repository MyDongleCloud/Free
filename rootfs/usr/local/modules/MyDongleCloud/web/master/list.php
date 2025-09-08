<?php
$path = "/usr/local/modules/MyDongleCloud/modules.json";
$path2 = "/disk/admin/.modules/MyDongleCloud/modules.json";
$h = fopen($path, "r");
$modules = json_decode(fread($h, filesize($path)), true);
fclose($h);
$h2 = fopen($path2, "r");
$modules2 = json_decode(fread($h2, filesize($path2)), true);
fclose($h2);
$ret = array();
foreach ($modules as $key => $val) {
	if (isset($val["web"]) && $val["web"] === true) {
		$ret[$key] = array();
		$ret[$key]["enabled"] = !isset($modules2[$key]) || $modules2[$key]["enabled"] !== false;
		$ret[$key]["authorized"] = $modules2[$key]["authorized"] ?? $modules[$key]["authorized"];
	}
}
echo json_encode($ret);
?>
