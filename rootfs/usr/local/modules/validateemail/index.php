<?php
include "../email/sec.inc";
include "validateEmail.inc";

$debug = 0;
$email = "";
if (php_sapi_name() === 'cli') {
	$debug = 2;
	$email = $argv[1];
} else {
?>
<html><body><form action="#" method="post">Email: <input type="email" name="email" value="<?=$_POST['email'];?>"> <input type="submit" value="check"></form>
<?php
if (isset($_POST["email"]))
	$email = $_POST["email"];
}
if ($email != "") {
	$ret = validateEmail($email, $debug);
	if ($ret["Valid"] != 1)
		echo "Result for " . $email . ": Not a valid email\n";
	else if ($ret["Risky"] == 1)
		echo "Result for " . $email . ": Risky (server has a catch-all email schema)\n";
	else if ($ret["Exists"] == 1)
		echo "Result for " . $email . ": Exists!\n";
	else
		echo "Result for " . $email . ": Does NOT exist\n";
}
?>
