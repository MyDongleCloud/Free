<?php
function store($p, $o) {
	file_put_contents(__DIR__ . $p, str_replace("\\/", "/", str_replace("    ", "\t", json_encode($o, JSON_PRETTY_PRINT))));
}

if (PHP_SAPI !== "cli")
	exit;
$pathkey = __DIR__ . "/password-githubkey.txt";
$h = fopen($pathkey, "r");
$githubAPIKey = fread($h, filesize($pathkey));
fclose($h);
$modulesPath = __DIR__ . "/modules";
$files = scandir($modulesPath);
$starsTotal = 0;
$modulesDefault = array();
$modulesMeta = array();
$modulesTranslationTitle = array();
$modulesTranslationDescription = array();
$modulesMarkdown = array();
$modulesMarkdownHeader="|Module|Title|Description|⭐|Category|Version|\n|-|-|-|:-:|-|:-:|\n";
$modulesMarkdownFooter="\n||||" . number_format(intval($starsTotal / 1000 / 1000), 2) . "M ⭐|||";
$modulesKeywords = array();
$count = 0;
foreach ($files as $file) {
    $fullPath = $modulesPath . "/" . $file;
	if ($file == "." || $file == ".." || is_dir($fullPath))
		continue;
	$h = fopen($fullPath, "r");
	$data = fread($h, filesize($fullPath));
	$module = json_decode($data, true);
	$name = $module["module"];
	echo $name . ", ";
	fclose($h);
	if ($module["github"] != "") {
		$headers = array("Accept: application/json", "Authorization: Bearer " . $githubAPIKey, "X-GitHub-Api-Version: 2022-11-28", "User-Agent: MyDongleCloud");
		$ch = curl_init("https://api.github.com/repos/" . $module["github"]);
		curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
		$resp = curl_exec($ch);
		curl_close($ch);
		$github = json_decode($resp, true);
		$starsTotal += intval($github["stargazers_count"]);
		$module["githubStars"] = intval($github["stargazers_count"] ?? 0);
		$module["githubLicense"] = $github["license"]["name"] ?? "";
		$module["githubDescription"] = $github["description"] ?? "";
		$module["githubLanguage"] = $github["language"] ?? "";
	}
	$modulesDefault[$name] = $module["default"];
	foreach($module["keywords"] as $k)
		$modulesKeywords[$k] =  "";
	unset($module["default"]);
	$modulesMeta[$name] = $module;
	$modulesTranslationTitle[$module["title"]] = "";
	$modulesTranslationDescription[$module["description"]] = "";
	$modulesMarkdown[$count] = "|" . implode("|", array($module["github"] != "" ? ("[" . $module["name"] . "](https://github.com/" . $module["github"] . ")") : $module["name"], $module["title"], $module["description"], "" . number_format(intval($github["stargazers_count"] ?? 0) / 1000, 1) . "k", $module["web"] ? "web" : "terminal", $module["category"], $module["version"])) . "|";
	$modulesMarkdown[$count] = str_replace("0.0k", "", $modulesMarkdown[$count]);
	$count++;
}
echo "\nGithub stars: " . $starsTotal . "\n";
foreach ($files as $file) {
    $fullPath = $modulesPath . "/" . $file;
	if ($file == "." || $file == ".." || is_dir($fullPath))
		continue;
	if (!file_exists($modulesPath . "/icons/" . str_replace(".json", ".png", $file)))
		echo "File " . $fullPath . " doesn't exist\n";
}
if (!is_dir(__DIR__ . "/../build"))
	mkdir(__DIR__ . "/../build");
store("/../build/modulesmeta.json", $modulesMeta);
store("/../rootfs/usr/local/modules/mydonglecloud/modulesdefault.json", $modulesDefault);
store("/../client/src/assets/i18n/modules-en.json", array( "modules" => array( "title" => $modulesTranslationTitle, "description" => $modulesTranslationDescription)));
store("/../client/src/assets/i18n/keywords-en.json", array("keywords" => $modulesKeywords));
file_put_contents(__DIR__ . "/../build/README-modules.md", $modulesMarkdownHeader . implode("\n", $modulesMarkdown) . $modulesMarkdownFooter);
system("rm -rf " . __DIR__ . "/../rootfs/usr/local/modules/mydonglecloud/reset/; cp -a " . __DIR__ . "/modules/reset/ " . __DIR__ . "/../rootfs/usr/local/modules/mydonglecloud/");
?>
