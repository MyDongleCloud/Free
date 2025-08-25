<html>
<body>
<?php
$path = "/disk/admin/.modules/MyDongleCloud/users.json";
$h = fopen($path, "r");
$users = json_decode(fread($h, filesize($path)), true);
fclose($h);
if (isset($_GET["reset"])) {
	$path = "/";
	$domain = "." . $_SERVER['HTTP_HOST'];
	setcookie("user", "", 1, $path, $domain);
	unset($_COOKIE["user"]);
	setcookie("token", "", 1, $path, $domain);
	unset($_COOKIE["token"]);
}
if (isset($_COOKIE["user"]) && isset($_COOKIE["token"]) && isset($users[$_COOKIE["user"]]) && $users[$_COOKIE["user"]]["token"] === $_COOKIE["token"] && !isset($_GET["bypass"])) {
?>
You are already registered in this browser. <a href="login.php?bypass">Bypass</a>. <a href="login.php?reset">Logout</a>.
<?php
} else {
?><form method="post" action="login_submit.php">
Email: <input type="email" name="email"><br>
Password: <input type="password" name="password"><br>
<input type="submit" value="Login">
</form>
<?php
}
?>
</body>
</html>
