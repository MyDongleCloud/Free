<html>
<body>
Module "<?=$_GET["m"];?>" is disabled. Contact the administrator.
<?php
if (isset($_GET["notSetup"]))
        echo "<br><br>This module has not been setup. Visit the app with an administrator account and click on the module card. It will be automatically setup.";
?>
</body>
</html>
