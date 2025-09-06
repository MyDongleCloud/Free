<?php
function domainFromFqdn($fqdn) {
	$parts = explode('.', $fqdn);
	if (count($parts) <= 2)
		return $fqdn;
	$last_two_parts = array_slice($parts, -2);
	return implode('.', $last_two_parts);
}

function passwordCheck($password, $dHash): bool {
	$pieces = explode('$', $dHash);
	if (count($pieces) !== 4)
		return -1;
	list($header, $iter, $salt, $hash) = $pieces;
	if (preg_match('#^pbkdf2_([a-z0-9A-Z]+)$#', $header, $m))
		$algo = $m[1];
	else
		return -1;
	$calc = hash_pbkdf2($algo, $password, $salt, (int)$iter, 32, true);
	return hash_equals($calc, base64_decode($hash));
}

$path = "/disk/admin/.modules/MyDongleCloud/users.json";
$h = fopen($path, "r");
$users = json_decode(fread($h, filesize($path)), true);
fclose($h);
$ret = false;
$user = "";
foreach ($users as $key => $val)
	if ($val["email"] == $_POST["email"]) {
		$user = $key;
		break;
	}
if ($user != "") {
	if (isset($users[$user]["password"]))
		$ret = passwordCheck($_POST["password"], $users[$user]["password"]);
	else {
		$url = (str_starts_with($_SERVER["HTTP_REFERER"], "https") ? "https://" : "http://") . $_SERVER["HTTP_HOST"];
		$protectedUrl = $url . "/MyDongleCloud/protected/empty.html";
		$ch = curl_init();
		curl_setopt($ch, CURLOPT_URL, $protectedUrl);
		curl_setopt($ch, CURLOPT_USERPWD, $user . ":" . $_POST["password"]);
		curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
		$response = curl_exec($ch);
		$http_status = curl_getinfo($ch, CURLINFO_HTTP_CODE);
		$ret = $http_status === 200;
		$curl_error = curl_error($ch);
		curl_close($ch);
	}
	if ($ret) {
		$token = bin2hex(random_bytes(16));
		$users[$user]["token"] = $token;
		$users[$user]["tokenExpiration"] = 0;
		$h = fopen($path, "w");
		fwrite($h, json_encode($users));
		fclose($h);
		$expiration_time = time() + (86400 * 30);
		$path = "/";
		$domain = "." . domainFromFqdn($_SERVER["HTTP_HOST"]);
		setcookie("user", $user, $expiration_time, $path, $domain);
		setcookie("token", $token, $expiration_time, $path, $domain);
	}
}
?><html>
<body>
<?php
if ($ret) {
	echo "Success. Go to <a href='/'>module homepage</a>.";
} else
	echo "Failure. Go back to <a href='/MyDongleCloud/login.php'>login</a>.";
?>
</body>
</html>
