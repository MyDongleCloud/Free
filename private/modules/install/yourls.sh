#!/bin/sh

cd /usr/local/modules/yourls
ln -sf /disk/admin/modules/yourls/config.php user/config.php
mkdir user/plugins/allow-privatebin
cat > user/plugins/allow-privatebin/plugin.php << EOF
<?php
/*
Plugin Name: Allow PrivateBin
Plugin URI: /
Description: Allow PrivateBin to use YOURLS API
Version: 1.0
Author: Admin
Author URI: /
*/

yourls_add_action("plugins_loaded", "custom_api_early_auth");
function custom_force_valid_user() { return true; }
function custom_api_early_auth() {
	if ( !defined("YOURLS_API") || !YOURLS_API )
		return;
	if (isset(\$_POST["signature"])) {
		\$config = parse_ini_file("/disk/admin/modules/privatebin/conf.php");
		\$signature = \$config["signature"] ?? null;
		if (\$signature === \$_POST["signature"])
			yourls_add_filter("shunt_is_valid_user", "custom_force_valid_user");
	}
}
?>
EOF
