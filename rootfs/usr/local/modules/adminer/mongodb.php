<?php
define('ADMINER_DIR', '/usr/share/adminer');
function adminer_object() {
	include_once ADMINER_DIR . "/plugins/drivers/mongo.php";
	foreach (glob(ADMINER_DIR . "/plugins/*.php") as $filename)
		include_once $filename;
	$plugins = array(
		new AdminerVersionNoverify(),
	);
	class Adminer2 extends Adminer\Adminer {
		function loginForm() {
			ob_start();
			parent::loginForm();
			$ret = ob_get_clean();
			$search = "/<select name='auth\\[driver\\]'>.*?<\/select>/s";
			$replace = "<select name='auth[driver]'><option value='mongo'>MongoDB</option></select>";
			$ret = preg_replace($search, $replace, $ret);
			$search = "/<input name=\"auth\\[db\\]\" value=\"\"/";
			$replace = "<input name=\"auth[db]\" value=\"librechatDB\"";
			$ret = preg_replace($search, $replace, $ret);
			$ret = str_replace("<tr><th>System<td>", "<tr style='display:none'><th>System<td>", $ret);
			$ret = str_replace("<tr><th>Server<td>", "<tr style='display:none'><th>Server<td>", $ret);
			echo $ret;
        }
	}
    return new Adminer2($plugins);
}
include ADMINER_DIR . "/adminer.php";
?>
