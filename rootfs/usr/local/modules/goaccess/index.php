<?php
function process($name) {
	system("zcat -f /var/log/apache2/" . ($name != "_all_" ? "module-" : "") . $name . ".log* | goaccess --log-format=VCOMBINED --html-report-title=\"Log Analysis of " . $name . "\" -o /usr/local/modules/goaccess/modules/" . $name . ".html - 2>&1 > /dev/null", $retval);
	return $retval;
}

if (isset($_POST["update"])) {
	if ($_POST["update"] == "_everything_") {
		$modules = [];
		$files = scandir("/var/log/apache2/");
		foreach ($files as $file)
			if (strpos($file, "module-") === 0) {
				$parts = explode('.', substr($file, 7));
				$moduleName = $parts[0];
				if (!in_array($moduleName, $modules) && $moduleName !== "")
					$modules[] = $moduleName;
			}
		foreach ($modules as $name)
			process($name);
		$retval = process("_all_");
	} else
		$retval = process($_POST["update"]);
	echo ($retval === 0) ? "Success" : "Error executing command";
	exit;
}

$modules = [];
$files = scandir("/var/log/apache2/");
foreach ($files as $file)
	if (strpos($file, "module-") === 0) {
		$parts = explode('.', substr($file, 7));
		$moduleName = $parts[0];
		if (!in_array($moduleName, $modules) && $moduleName !== "")
			$modules[] = $moduleName;
	}

$logs = array();
$files = scandir(__DIR__ . "/modules/");
foreach ($files as $file)
	if (str_ends_with($file, ".html"))
		$logs[str_replace(".html", "", $file)] = filemtime(__DIR__ . "/modules/" . $file);
?>
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<script src="tailwindcss.js"></script>
<script>
var modules = <?php echo json_encode($modules); ?>;
var logs = <?php echo json_encode($logs); ?>;

function onStart() {
	const showSelect = document.getElementById("showID");
	modules.forEach(mod => {
		showSelect.add(new Option(mod, mod));
	});
	show("_all_");
}

function show() {
	const name = document.getElementById("showID").value;
	const url = "/modules/" + name + ".html";
	const frame = document.getElementById("iframeID");
	if (logs[name] && logs[name] > Math.floor(Date.now() / 1000) - 3600) {
		frame.src = url;
		frame.onload = null;
	} else {
		frame.src = "about:blank";
		frame.onload = () => {
			const doc = frame.contentDocument || frame.contentWindow.document;
			doc.body.innerHTML = "<div style='padding:20px; color:#721c24; background:#f8d7da; border:1px solid #f5c6cb; border-radius:4px;'> The log analysis is being prepared for module " + name + ". Please wait...</div>";
		};
		update(false);
	}
}

function update(e) {
	const name = e ? "_everything_" : document.getElementById("showID").value;
	const btn1 = document.getElementById("button1ID");
	const btn2 = document.getElementById("button2ID");
	btn1.disabled = true;
	btn2.disabled = true;
	if (e)
		btn2.innerText = "Updating. Please wait...";
	else
		btn1.innerText = "Updating...";
	var xhr = new XMLHttpRequest();
	xhr.onreadystatechange = function() {
		if (this.readyState == 4) {
			btn1.disabled = false;
			btn1.innerText = "Update";
			btn2.disabled = false;
			btn2.innerText = "Update Everything";
			if (this.responseText.indexOf("Success") != -1) {
				document.getElementById("showID").value = name == "_everything_" ? "_all_" : name;
				if (name == "_everything_")
					document.location.href = "/";
				else {
					logs[name] = Math.floor(Date.now() / 1000);
					show();
				}
			}
		}
	};
	xhr.open("POST", window.location.href);
	var formData = new FormData();
	formData.append("update", name);
	xhr.send(formData);
}
</script>
</head>
<body onload="onStart();" class="flex flex-col h-dvh m-0 overflow-hidden">
<div class="p-2 border-b flex items-center justify-between gap-4 bg-[#fdfcf3]">
	<div>
		<label>Show:</label>
		<select id="showID" onchange="show()" class="border p-1 rounded">
			<option value="_all_">All combined</option>
		</select>
		<button type="button" id="button1ID" onclick="update(false)" value="" class="bg-blue-500 text-white px-4 py-1 rounded hover:bg-blue-600">Update</button>
	</div>
	<div>
		<button type="button" id="button2ID" onclick="update(true)" value="" class="bg-blue-500 text-white px-4 py-1 rounded hover:bg-blue-600">Update Everything</button>
	</div>
</div>
<iframe id="iframeID" class="flex-1 w-full border-none"></iframe>
</body>
</html>
