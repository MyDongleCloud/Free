<?php
$baseDir = "/disk/admin/modules/mail/";
$domain = "mail2.cloud";
$accountCur = "";

include "process.inc";
$dbPath = $baseDir . "ote-db.txt";
if (isset($_POST["formId"])) {
	$handle = fopen($dbPath, "r");
	$content = trim(fread($handle, filesize($dbPath)));
	$content_ = explode("\n", $content);
	fclose($handle);
	$expiration = $_POST["formExpiration_"];
	if (isset($_POST["formRedirect"]))
		$to = $_POST["formRedirect"];
	else
		$to = "";
	if ($_POST["formId"] == "new") {
		$content .= "\n" . (count($content_) + 1) . "|" . $_POST["formAccount"] . "|" . $_POST["formFrom"] . "@" . $domain . "|" . $to . "|" . $expiration . "|" . str_replace("|", "", str_replace("\n", "", $_POST["formUsedFor"])) . "|" . str_replace("|", "", str_replace("\n", "", $_POST["formComment"])) . "\n";
	} else {
		for ($i = 0; $i < count($content_); $i++) {
			$ar = explode("|", $content_[$i]);
			if ($ar[0] == $_POST["formId"]) {
				$content_[$i] = $ar[0] . "|" . $_POST["formAccount"] . "|" . $_POST["formFrom"] . "@" . $domain . "|" . $to . "|" . $expiration . "|" . str_replace("|", "", str_replace("\n", "", $_POST["formUsedFor"])) . "|" . str_replace("|", "", str_replace("\n", "", $_POST["formComment"])) . "\n";
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
tr:nth-child(odd) td { background-color: #FFFFFF; }
tr:first-child td { background-color: #DDDDDD; }
</style>
<script>
function showToast(message) {
	document.getElementById("toastID").style.display = "flex";
    document.getElementById("toastContentID").innerText = message;
	document.getElementById("toastContentID").style.opacity = "1";
    setTimeout(() => {
		document.getElementById("toastContentID").style.opacity = "0";
    }, 2000);
    setTimeout(() => {
		document.getElementById("toastID").style.display = "none";
    }, 3000);
}

function epochToLocalString(epoch, id) {
	const dateObject = new Date(epoch * 1000);
	document.getElementById("sp" + id + "Time").innerHTML = dateObject.toLocaleString();
}

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

function fillForm(id, account, from, to, expiration, usedFor, comment) {
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
	formToSelect(from == to);
	formExpirationSelect(expiration > 0);
	document.getElementById("formUsedFor").value = usedFor;
	document.getElementById("formComment").value = comment;
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
		if(element.disabled !== true)
			formData.append(element.name, element.value);
	}
	let expiration;
	if (document.getElementById("formDelay").disabled == true) {
		const localDate = new Date("" + document.getElementById("formDate").value + " " + document.getElementById("formTime").value);
		expiration = localDate.getTime() / 1000;
	} else
		expiration = Math.floor(Date.now() / 1000) + 60 * document.getElementById("formDelay").value;
	formData.append("formExpiration_", expiration);
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
Used for: <input type="text" id="formUsedFor" name="formUsedFor"><br><br>
Comment: <input type="text" id="formComment" name="formComment"><br><br>
<button id="formSubmit" name="formSubmit" onclick="sendForm(); return false;">Create</button>
</form>
<br><br><br>
<table border="0">
<tr><td nowrap><span class="material-icons">list_alt</span></td><td>Id</td><td>Account</td><td>From</td><td>To</td><td>Status</td><td>Expires in</td><td>Used for</td><td>Comment</td></tr>
<?php
function isLocal($email) {
	global $baseDir;
	$ar = explode("@", $email);
	$dirPath = $baseDir . $ar[1] . "/" . $ar[0];
	return is_dir($dirPath);
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
	$ar = explode("|", $content_[$i]);
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
	$usedFor = $ar[5];
	$comment = $ar[6];
	echo "<tr id='tr" . $id . "'>\n";
	echo "<td " . ($active == false ? "style='background-color:#888888;' " : "") . "nowrap><span class='material-icons' style='cursor:pointer;' onclick='fillForm(" . $id . ", \"" . $account . "\", \"" . $from . "\", \"" . $to . "\", " . $expiration . ", \"" . $usedFor . "\", \"" . $comment . "\");'>list_alt</span></td>\n";
	echo "<td " . ($active == false ? "style='background-color:#888888;' " : "") . "nowrap>" . $id . "</td>\n";
	echo "<td " . ($active == false ? "style='background-color:#888888;' " : "") . "nowrap>" . $account . "</td>\n";
	echo "<td " . ($active == false ? "style='background-color:#888888;' " : "") . "nowrap>" . $from . " (<a href='javascript:void();' onclick='copyEmail(\"" . $from . "\");'>copy</a>)</td>\n";
	echo "<td " . ($active == false ? "style='background-color:#888888;' " : "") . "nowrap>" . (isLocal($to) ? "<button onclick='openMail(\"". $to . "\");'>". $to . "</button>" : $to) . "</td>\n";
	echo "<td " . ($active == false ? "style='background-color:#888888;' " : "") . "nowrap>" . ($active ? "Active" : "Inactive") . "</td>\n";
	echo "<td " . ($active == false ? "style='background-color:#888888;' " : "") . "nowrap>" . ($expiration > 1 ? (getTimeDifference($expiration) . " (<span id='sp" . $id . "Time'></span><script>epochToLocalString(" . $expiration .", " . $id . ");</script>)") : "") . "</td>\n";
	echo "<td " . ($active == false ? "style='background-color:#888888;' " : "") . "nowrap>" . $usedFor . "</td>\n";
	echo "<td " . ($active == false ? "style='background-color:#888888;' " : "") . "nowrap>" . $comment . "</td>\n";
	echo "</tr>";
}
fclose($handle);
?>
</table>
<div id="toastID" style="position:fixed; top:20px; width:100%; background:transparent; display:none; justify-content:center;"><div id="toastContentID" style="color:white; border-radius: 15px; background:gray; padding:10px; opacity:1; transition:opacity 0.5s ease-in-out;"></div></div>
</body></html>
