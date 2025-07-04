<?php
include "../email/sec.inc";

$baseDir = "/home/gregoire/mail/";
$domain = "mail2.cloud";
$accountCur = "";

include "process.inc";
$dbPath = $baseDir . "ote-db.txt";
if (isset($_POST["formId"])) {
	$handle = fopen($dbPath, "r");
	$content = trim(fread($handle, filesize($dbPath)));
	$content_ = explode("\n", $content);
	fclose($handle);
	if (isset($_POST["formDelay"]))
		$expiration = time() + intval($_POST["formDelay"]) * 60;
	else
		$expiration = strtotime($_POST["formDate"] . " " . $_POST["formTime"]);
	if (isset($_POST["formRedirect"]))
		$to = $_POST["formRedirect"];
	else
		$to = "";
	if ($_POST["formId"] == "new") {
		$content .= "\n" . (count($content_) + 1) . ";" . $_POST["formAccount"] . ";" . $_POST["formFrom"] . "@" . $domain . ";" . $to . ";" . $expiration . "\n";
	} else {
		for ($i = 0; $i < count($content_); $i++) {
			$ar = explode(";", $content_[$i]);
			if ($ar[0] == $_POST["formId"]) {
				$content_[$i] = $ar[0] . ";" . $_POST["formAccount"] . ";" . $_POST["formFrom"] . "@" . $domain . ";" . $to . ";" . $expiration . "\n";
				break;
			}
		}
		$content = implode("\n", $content_) . "\n";
	}
	$handle = fopen($dbPath, "w");
	fwrite($handle, $content);
	fclose($handle);
	clearstatcache();
	doProcess();
	echo "success";
	exit;
}
?>
<!DOCTYPE HTML>
<html language="en">
<head>
<meta name="google" content="notranslate">
<link rel="stylesheet" href="https://fonts.googleapis.com/icon?family=Material+Icons">
<style type="text/css">
td { border:1px black solid; }
tr:nth-child(even) td { background-color: #FFFFFF; }
tr:nth-child(odd) td { background-color: #EEEEEE; }
tr:first-child td { background-color: #DDDDDD; }
</style>
<script>
function fillDateTime(inD) {
	let date;
	if (inD == 0)
		date = Date.now() / 1000;
	else if (inD == -1)
		date = Date.now() / 1000 + document.getElementById("formDelay").value * 60;
	else
		date = inD;
	date = new Date(date * 1000);
	const year = date.getFullYear();
	const month = (date.getMonth() + 1).toString().padStart(2, '0');
	const day = date.getDate().toString().padStart(2, '0');
	const formattedDate = `${year}-${month}-${day}`;
	const hours = date.getHours().toString().padStart(2, '0');
	const minutes = date.getMinutes().toString().padStart(2, '0');
	const formattedTime = `${hours}:${minutes}`;
	document.getElementById("formDate").value = formattedDate;
	document.getElementById("formTime").value = formattedTime;
}

function openMail(email) {
	const form = document.createElement("form");
	form.setAttribute("method", "post");
	form.setAttribute("action", "login.php");
	form.setAttribute("target", "_blank");
	const hiddenInput = document.createElement("input");
	hiddenInput.setAttribute("type", "hidden");
	hiddenInput.setAttribute("name", "email");
	hiddenInput.setAttribute("value", email);
	form.appendChild(hiddenInput);
	document.body.appendChild(form);
	form.submit();
	setTimeout(() => { form.remove(); }, 100);
}

function fillForm(id, account, from, to, expiration) {
	document.getElementById("formId").value = id;
	document.getElementById("formAccount").value = account;
	document.getElementById("formFrom").value = from.replace(/@.*/, "");
	document.getElementById("formTo0").checked = from != to;
	document.getElementById("formTo1").checked = from == to;
	if (from != to)
		document.getElementById("formRedirect").value = to;
	else
		document.getElementById("formRedirect").value = "";
	document.getElementById("formExpiration0").checked = expiration <= 0;
	if (expiration <= 0)
		document.getElementById("formDelay").value = expiration == 0 ? 0 : 15;
	fillDateTime(expiration);
	document.getElementById("formExpiration1").checked = expiration > 0;
	document.getElementById("formSubmit").innerHTML = "Save";
//	document.getElementById("formExpiration").value = expiration;
	formToSelect(from == to);
	formExpirationSelect(expiration > 0);
}

function fillFormNew() {
	randomEmail();
	formToSelect(1);
	formExpirationSelect(0);
	fillDateTime(-1);
}

function sendForm() {
	var xhr = new XMLHttpRequest();
	xhr.onreadystatechange = function() {
		if (this.readyState == 4) {
			if (this.responseText == "success") {
				console.log("Success!");
				location.reload(true);
			} else
				alert("Error: " + this.responseText);
		}
	};
	xhr.open("POST", "#");
	var formData = new FormData();
	for (let i = 0; i < formForm.length; i++) {
		const element = formForm[i];
		formData.append(element.name, element.value);
	}
	xhr.send(formData);
}

function copyEmail(e) {
	navigator.clipboard.writeText(e);
	showToast("Copied!");
}

var cc = 0;
function randomEmail() {
	const charSet = "23456789abcdefghkmnpqrstuvwxyz";
	const names = [["james", "michael", "john", "robert", "david", "william", "richard", "joseph", "thomas", "christopher"], ["mary", "patricia", "jennifer", "linda", "elizabeth", "barbara", "susan", "jessica", "karen", "sarah"]];
	let result = names[cc++ % 2][Math.floor(Math.random() * names[0].length)] + "_";
	const length = 5;
	for (let i = 0; i < length; i++) {
        const randomIndex = Math.floor(Math.random() * charSet.length);
        result += charSet.charAt(randomIndex);
    }
	document.getElementById("formFrom").value = result;
}

function formToSelect(v) {
	document.getElementById("formRedirect").disabled = v != 0;
}

function formExpirationSelect(v) {
	document.getElementById("formDelay").disabled = v != 0;
	document.getElementById("formDate").disabled = v == 0;
	document.getElementById("formTime").disabled = v == 0;
}

function formDelaySelect() {
	console.log(Date.now());
	console.log(document.getElementById("formDelay").value);
	fillDateTime(-1);
}

function onStart() {
	fillFormNew();
}
</script>
</head>
<body onload="onStart();">
<form action="#" method="post" id="formForm">
Id: <input type="text" id="formId" name="formId" readonly value="new" style="color:gray;"><br><br>
Account: <input type="text" id="formAccount" name="formAccount" value="gregoiregentil"><br><br>
From: <input type="text" id="formFrom" name="formFrom">@<?=$domain;?> (<a href="javascript:void();" onclick="randomEmail();">random</a>)<br><br>
To: <input type="radio" name="formTo" id="formTo0" onchange="formToSelect(0);"> Redirect to <input type="email" id="formRedirect" name="formRedirect"> or <input type="radio" name="formTo" id="formTo1" onchange="formToSelect(1);" checked> Local<br><br>
Expiration: <input type="radio" name="formExpiration" id="formExpiration0" onchange="formExpirationSelect(0);" checked> In <select id="formDelay" name="formDelay" onchange="formDelaySelect();">
<option value="15">15 minutes</option>
<option value="60">1 hour</option>
<option value="1440">1 day</option>
<option value="0">Never</option>
</select> or <input type="radio" name="formExpiration" id="formExpiration1" onchange="formExpirationSelect(1);"> On <input type="date" id="formDate" name="formDate"> at <input type="time" id="formTime" name="formTime" disabled> (<a href="javascript:void();" onclick="fillDateTime(0);">now</a>) <br><br>
<button id="formSubmit" name="formSubmit" onclick="sendForm(); return false;">Create</button>
</form>
<br><br><br>
<table border="0">
<tr><td nowrap><span class="material-icons">list_alt</span></td><td>Id</td><td>Account</td><td>From</td><td>To</td><td>Status</td><td>Expires in</td></tr>
<?php
function isLocal($email) {
	global $baseDir;
	$ar = explode("@", $email);
	$dirPath = $baseDir . $ar[1] . "/" . $ar[0];
	return is_dir($dirPath);
}

function epochToString($e) {
	$date = new DateTime("@$e");
	$newTimezone = new DateTimeZone("America/Los_Angeles");
	$date->setTimezone($newTimezone);
	return $date->format("Y-m-d H:i:s T");
}

function getTimeDifference($epochTime) {
	$difference = $epochTime - time();
	$hours = floor($difference / 3600);
	$minutes = floor(($difference % 3600) / 60);
	$seconds = $difference % 60;
	return $difference < 0 ? "Expired" : sprintf("%dh %dm %ds", $hours, $minutes, $seconds);
}

$handle = fopen($dbPath, "r");
$content = trim(fread($handle, filesize($dbPath)));
$content_ = array();
if (strlen($content) == 0)
	exit;
$content_ = array_reverse(explode("\n", $content));
for ($i = 0; $i < count($content_); $i++) {
	$ar = explode(";", $content_[$i]);
	$id = $ar[0];
	$account = $ar[1];
	$from = $ar[2];
	$to = $ar[3];
	if ($to == "")
		$to = $from;
	$expiration = $ar[4];
	$active = $expiration != -1;
	if ($expiration > 1 && $expiration < time())
		$active = false;
	if ($accountCur != "" && $accountCur != $account)
		continue;
	echo "<tr id='tr" . $id . "'>\n";
	echo "<td " . ($active == false ? "style='background-color:#888888;' " : "") . "nowrap><span class='material-icons' style='cursor:pointer;' onclick='fillForm(" . $id . ", \"" . $account . "\", \"" . $from . "\", \"" . $to . "\", " . $expiration . ");'>list_alt</span></td>\n";
	echo "<td " . ($active == false ? "style='background-color:#888888;' " : "") . "nowrap>" . $id . "</td>\n";
	echo "<td " . ($active == false ? "style='background-color:#888888;' " : "") . "nowrap>" . $account . "</td>\n";
	echo "<td " . ($active == false ? "style='background-color:#888888;' " : "") . "nowrap>" . $from . " (<a href='javascript:void();' onclick='copyEmail(\"" . $from . "\");'>copy</a>)</td>\n";
	echo "<td " . ($active == false ? "style='background-color:#888888;' " : "") . "nowrap>" . (isLocal($to) ? "<button onclick='openMail(\"". $to . "\");'>". $to . "</button>" : $to) . "</td>\n";
	echo "<td " . ($active == false ? "style='background-color:#888888;' " : "") . "nowrap>" . ($active ? "Active" : "Inactive") . "</td>\n";
	echo "<td " . ($active == false ? "style='background-color:#888888;' " : "") . "nowrap>" . ($expiration > 1 ? (getTimeDifference($expiration) . " (" . epochToString($expiration) . ")") : "") . "</td>\n";
	echo "</tr>";
}
fclose($handle);
?>
</table>
</body></html>
