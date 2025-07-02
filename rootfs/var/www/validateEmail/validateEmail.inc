<?php
function validateEmail_($server, $email, $debug) {
	$ret = 0;

	if ($debug >= 1)
		echo "Connecting to " . $server . " on port 25 for " . $email . "\n";
	$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
	if ($socket === false) {
		if ($debug >= 1)
			echo "socket_create() failed: " . socket_strerror(socket_last_error()) . "\n";
		exit;
	}
	socket_set_option($socket, SOL_SOCKET, SO_RCVTIMEO, array("sec" => 10, "usec" => 0));
	socket_set_option($socket, SOL_SOCKET, SO_SNDTIMEO, array("sec" => 10, "usec" => 0));
	$result = socket_connect($socket, $server, 25);
	if ($result === false) {
		if ($debug >= 1)
			echo "socket_connect() failed: " . socket_strerror(socket_last_error($socket)) . "\n";
		exit;
	}

	$stages = array(
		0 => array(
			"name" => "CHECK_CONNECTION_ESTABLISHED",
			"command" => "",
			"expected_reply_code" => "220"
		),
		1 => array(
			"name" => "SEND_EHLO",
			"command" => "EHLO mail.gentil.com\r\n",
			"expected_reply_code" => "250"
		),
		2 => array(
			"name" => "SEND_MAIL_FROM",
			"command" => "MAIL FROM:<name@example.org>\r\n",
			"expected_reply_code" => "250"
		),
		3 => array(
			"name" => "SEND_RECIPIENT_TO",
			"command" => "RCPT TO:<inboxAddress>\r\n",
			"expected_reply_code" => "250"
		),
		4 => array(
			"name" => "SEND_QUIT",
			"command" => "QUIT\r\n",
			"expected_reply_code" => "221"
		)
	);
	$currentStageName = 0;

	while (true) {
		$data = socket_read($socket, 1024);

		if ($data === false) {
			$errorCode = socket_last_error($socket);
			$errorMessage = socket_strerror($errorCode);
			if ($debug >= 1)
				echo "Error: $errorMessage\n";
			break;
		}
		if ($data === "") {
			if ($debug >= 1)
				echo "Closing because data is empty\n";
			break;
		}

		if ($debug >= 2)
			echo "<-- " . $data;
		if (!str_starts_with($data, $stages[$currentStageName]["expected_reply_code"])) {
			if ($debug >= 1)
				echo "Error stage: " . $stages[$currentStageName]["name"] . "\n";
			socket_close($socket);
			break;
		}
		if ($currentStageName == 4) {
			$ret = 1;
			break;
		}
		$currentStageName++;
		$cmd = $stages[$currentStageName]["command"];
		$cmd = str_replace("inboxAddress", $email, $cmd);
		socket_write($socket, $cmd);
		if ($debug >= 2)
			echo "--> " . $cmd;
	}

	if ($debug >= 1) {
		echo "#########################\n";
		echo "" . $email . ": " . $ret . "\n";
		echo "#########################\n";
	}
	return $ret;
}

function list_cmp($a, $b) {
	global $weights;
	return $weights[$a] >= $weights[$b];
}

function validateEmail($email, $debug) {
	global $weights;
	$pattern = "/^[_a-z0-9-]+(\.[_a-z0-9-]+)*@[a-z0-9-]+(\.[a-z0-9-]+)*(\.[a-z]{2,})$/i";
	if (preg_match($pattern, $email) !== 1) {
		echo "Not a valid email";
		return array(
			"Valid" => 0,
			"Exists" => 0,
			"Risky"	=> 0);
	}
	$hostname = substr($email, strpos($email, "@") + 1);
	getmxrr($hostname, $hosts, $weights);
	uksort($hosts, "list_cmp");

	$ret1 = validateEmail_($hosts[0], $email, $debug);
	$ret2 = validateEmail_($hosts[0], bin2hex(random_bytes(10)) . "@" . $hostname, $debug);
	return array(
		"Valid" => 1,
		"Exists" => $ret1,
		"Risky"	=> $ret2);
}
?>
