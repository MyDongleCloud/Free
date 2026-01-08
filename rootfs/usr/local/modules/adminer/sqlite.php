<?php
define('ADMINER_DIR', '/usr/share/adminer');
function adminer_object() {
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
			$replace = "<select name='auth[driver]'><option value='sqlite'>SQLite</option></select>";
			$ret = preg_replace($search, $replace, $ret);
			$search = "/<tr><th>Database.*<\/table>/s";
			$replace = "<tr><th>Database<td><select name=\"auth[db]\">";
			$vv = array(
				"/disk/admin/modules/2fauth/database.sqlite",
				"/disk/admin/modules/homeassistant/home-assistant_v2.db",
				"/disk/admin/modules/lidarr/lidarr.db",
				"/disk/admin/modules/openwebui/data/webui.db",
				"/disk/admin/modules/radarr/radarr.db",
				"/disk/admin/modules/sonarr/sonarr.db",
				"/disk/admin/modules/prowlarr/prowlarr.db",
				"/disk/admin/modules/triliumnotes/document.db",
				"/disk/admin/modules/wgdashboard/db/wgdashboard.db",
				"/disk/admin/modules/whisparr/whisparr2.db",
				"/disk/admin/modules/zigbee2mqtt/database.db"
			);
			foreach($vv as $v)
				$replace .= "<option value=\"" . $v . "\">" . $v . "</option>";
			$replace .= "</select></table>";
			$ret = preg_replace($search, $replace, $ret);
			$ret = str_replace("<tr><th>System<td>", "<tr style='display:none'><th>System<td>", $ret);
			$ret = str_replace("<tr><th>Server<td>", "<tr style='display:none'><th>Server<td>", $ret);
			$ret = str_replace("<tr><th>Username<td>", "<tr style='display:none'><th>Username<td>", $ret);
			$ret = str_replace("<tr><th>Password<td>", "<tr style='display:none'><th>Password<td>", $ret);
			echo $ret;
        }

		function login($login, $password) {
			if ($password === "")
				return true;
		}
	}
    return new Adminer2($plugins);
}
include ADMINER_DIR . "/adminer.php";
?>
