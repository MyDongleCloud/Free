<?php
function store($p, $o) {
	file_put_contents(__DIR__ . $p, str_replace("\\/", "/", str_replace("    ", "\t", json_encode($o, JSON_PRETTY_PRINT))) . "\n");
}

if (PHP_SAPI !== "cli")
	exit;
$modulesPath = __DIR__ . "/modules";
$files = scandir($modulesPath);
$starsTotal = 0;
$csv = array();
$modulesDefault = array();
foreach ($files as $file) {
    $fullPath = $modulesPath . "/" . $file;
	if ($file == "." || $file == ".." || is_dir($fullPath))
		continue;
	$h = fopen($fullPath, "r");
	$data = fread($h, filesize($fullPath));
	$module = json_decode($data, true);
	$module["keywords"] = implode("|", $module["keywords"]);
	$modulesDefault[$module["module"]] = $module["default"];
	unset($module["default"]);
	if (count($csv) == 0) {
		$names = array_keys((array)$module);
		array_push($csv, implode(";", $names));
	}
	array_push($csv, implode(";", array_values((array)$module)));
}
file_put_contents("/tmp/modules.csv", implode("\n", $csv));

system("libreoffice /tmp/modules.csv");

$csvPath = "/tmp/modules.csv";
$h = fopen($csvPath, "r");
$data = fread($h, filesize($csvPath));
$modules = explode("\n", $data);
fclose($h);
$name = explode(";", $modules[0]);
for ($i = 1; $i < count($modules); $i++) {
	if (strlen($modules[$i]) == 0)
		continue;
	$m = explode(";", $modules[$i]);
	$out = array();
	for ($j = 0; $j < count($m); $j++)
		$out[$names[$j]] = $m[$j];
	$out["keywords"] = explode("|", $out["keywords"]);
	$out["ai"] = $out["ai"] === "1";
	$out["web"] = $out["web"] === "1";
	$out["finished"] = $out["finished"] === "1";
	$out["default"] = $modulesDefault[$m[0]];
	store("/modules/" . $m[0] . ".json", $out);
}
?>
