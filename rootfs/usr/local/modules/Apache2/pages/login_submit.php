<?php
/*
function passwordMake($password) {
	$iter = 36000;
	$salt = base64_encode(random_bytes(64));
	$salt = substr($salt, 0, 12);
	$calc = hash_pbkdf2("sha256", $password, $salt, $iter, 32, true);
	return "pbkdf2_sha256\${$iter}\${$salt}\$" . base64_encode($calc);
}
*/

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
foreach ($users as $key => $val)
	if ($val["email"] == $_POST["email"]) {
		$user = $key;
		break;
	}
$ret = passwordCheck($_POST["password"], $users[$user]["password"]);
if ($ret) {
	$token = bin2hex(random_bytes(16));
	$users[$user]["token"] = $token;
	$h = fopen($path, "w");
	fwrite($h, json_encode($users));
	fclose($h);
	$expiration_time = time() + (86400 * 30);
	$path = "/";
	$domain = "." . $_SERVER['HTTP_HOST'];
	setcookie("user", $user, $expiration_time, $path, $domain);
	setcookie("token", $token, $expiration_time, $path, $domain);
}
?><html>
<body>
<?=$ret ? "Success" : "Failure"?>
</body>
</html>
