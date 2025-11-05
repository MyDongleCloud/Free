<?php
function gc($a) {
	global $modules;
	return array_search($a, explode(";", $modules[0]));
}

if (PHP_SAPI !== "cli")
	exit;
$pathkey = __DIR__ . "/password-githubkey.txt";
$h = fopen($pathkey, "r");
$githubAPIKey = fread($h, filesize($pathkey));
fclose($h);
$pathname = __DIR__ . "/modules.csv";
$h = fopen($pathname, "r");
$data = fread($h, filesize($pathname));
$modules = explode("\n", $data);
fclose($h);
$starsTotal = 0;
$modulesMeta = array();
$modulesTranslationTitle = array();
$modulesTranslationDescription = array();
for ($i = 1; $i < count($modules); $i++) {
	$stars = 0;
	if (strlen($modules[$i]) == 0)
		continue;
	$m = explode(";", $modules[$i]);
	echo $m[0] . "\n";
	$github = array();
	if ($m[gc("github")] != "") {
		$headers = array("Accept: application/json", "Authorization: Bearer " . $githubAPIKey, "X-GitHub-Api-Version: 2022-11-28", "User-Agent: MyDongleCloud");
		$ch = curl_init("https://api.github.com/repos/" . $m[gc("github")]);
		curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
		$resp = curl_exec($ch);
		curl_close($ch);
		$github = json_decode($resp, true);
		//$posgh = gc("github");
		//$m[$posgh + 1] = intval($github["stargazers_count"]);
		//$m[$posgh + 2] = str_replace(";", " ", $github["license"]["name"]);
		//$m[$posgh + 3] = str_replace(";", " ", $github["description"]);
		//$stars = intval($github["stargazers_count"]);
		$starsTotal += intval($github["stargazers_count"]);
	}
	$modules[$i] = implode(";", $m);
	$modulesMeta[$m[0]] = array(
		"module" => $m[gc("module")],
		"title" => $m[gc("title")],
		"name" => $m[gc("name")],
		"status" => $m[gc("status")],
		"githubUrl" => $m[gc("github")],
		"githubLicense" => $github["license"]["name"] ?? "",
		"githubDescription" => $github["description"] ?? "",
		"githubLanguage" => $github["language"] ?? "",
		"githubStars" => intval($github["stargazers_count"] ?? 0),
		"version" => $m[gc("version")],
		"category" => $m[gc("category")],
		"description" => $m[gc("description")],
		"keywords" => empty($m[gc("keywords")]) ? array() : explode("|", $m[gc("keywords")]),
		"proprietary" => empty($m[gc("proprietary")]) ? array() : explode("|", $m[gc("proprietary")])
	);
	$modulesTranslationTitle[$m[gc("title")]] = "";
	$modulesTranslationDescription[$m[gc("description")]] = "";
}
echo "Github stars: " . $starsTotal . "\n";
file_put_contents(__DIR__ . "/../rootfs/usr/local/modules/mydonglecloud/modulesmeta.json", str_replace("    ", "\t", json_encode($modulesMeta, JSON_PRETTY_PRINT)));
$modulesTranslation = array( "modules" => array( "title" => $modulesTranslationTitle, "description" => $modulesTranslationDescription));
file_put_contents(__DIR__ . "/../client/src/assets/i18n/modules-en.json", str_replace("    ", "\t", json_encode($modulesTranslation, JSON_PRETTY_PRINT)));
?>
