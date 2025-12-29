<html>
<head>
<body>
<table style="">
<?php
$files = scandir(__DIR__ . "/modules");
$count = 0;
$columns = 8;
foreach ($files as $index => $file) {
	if (!str_ends_with($file, ".json"))
		continue;
	$obj = json_decode(file_get_contents(__DIR__ . "/modules/" . $file), true);
	if (($count % $columns) == 0)
		echo "<tr>";
	$name = $obj["name"];
	$module = $obj["module"];
	$description = $obj["description"];
	//if (filesize(__DIR__ . "/modules/icons/" . $module . ".png") != 42435) continue;
	echo "<td align='center' valign='top' style='border:1px solid black;'><img src='icons_/" . $module . ".png' style='width:64px; height:64px'><br><br><span style='font-weight: bold; color:red;'>" . $name . "</span><br><span style='font-size:small;'>" . $module . "</span><br><span style='font-size:small;'>" . $description . "</span></td>\n";
	$count++;
	if (($count % $columns) == 0)
		echo "</tr>";
}
?>
</table>
</body>
<br><br><?=$count; ?> modules
</html>
