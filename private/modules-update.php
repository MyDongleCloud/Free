<?php
$pathname = "password-githubkey.txt";
$h = fopen($pathname, "r");
$githubAPIKey = fread($h, filesize($pathname));
fclose($h);
$pathname = "modules.csv";
$h = fopen($pathname, "r");
$data = fread($h, filesize($pathname));
$modules = explode("\n", $data);
fclose($h);
$stars = 0;
$posgh = array_search("Github URL", explode(";", $modules[0]));
for ($i = 1; $i < count($modules); $i++) {
	if (strlen($modules[$i]) == 0)
		continue;
	$m = explode(";", $modules[$i]);
	echo $m[0] . "\n";
	if (strlen($m[$posgh]) != 0) {
		$headers = array("Accept: application/json", "Authorization: Bearer " . $githubAPIKey, "X-GitHub-Api-Version: 2022-11-28", "User-Agent: MyDongleCloud");
		$ch = curl_init("https://api.github.com/repos/" . $m[$posgh]);
		curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
		$response = curl_exec($ch);
		curl_close($ch);
		$resp = json_decode($response, true);
		$m[$posgh + 1] = intval($resp["stargazers_count"]);
		$m[$posgh + 2] = str_replace(";", " ", $resp["license"]["name"]);
		$m[$posgh + 3] = str_replace(";", " ", $resp["description"]);
	}
	$stars += intval($m[$posgh + 1]);
	$modules[$i] = implode(";", $m);
}
$h = fopen($pathname, "w");
foreach ($modules as $l)
	if (strlen($l) > 0)
		fwrite($h, $l . "\n");
fclose($h);
echo "Github stars: " . $stars . "\n";
?>
