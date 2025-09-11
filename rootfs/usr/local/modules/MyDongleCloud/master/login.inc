<?php
$path = "../../../../../disk/admin/.modules/MyDongleCloud/users.json";
$h = fopen($path, "r");
$users = json_decode(fread($h, filesize($path)), true);
fclose($h);
$user = "";
$token = "";
if (isset($_POST["user"]))
	$user = $_POST["user"];
else if (isset($_COOKIE["user"]))
	$user = $_COOKIE["user"];
if (isset($_POST["token"]))
	$token = $_POST["token"];
else if (isset($_COOKIE["token"]))
	$token = $_COOKIE["token"];
if ($user == "" || $token == "" || !isset($users[$user]) || !isset($users[$user]["token"]) || $users[$user]["token"] != $token) {
	echo json_encode(["error" => 1, "reason" => "Unauthorized"]);
	exit;
}
?>
