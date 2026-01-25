<html>
<head>
<meta http-equiv="refresh" content="0; url=/_app_/login">
</head>
<body>
<div style="display:none;" id="msg"><b>Unauthorized access for module "<?=$_GET["m"];?>"</b>. Please login <a href="/_app_/login">here</a>. You will be redirected now...</div>
<script>
setTimeout(function() { document.getElementById("msg").style.display = "block"; }, 1000);
</script>
</body>
</html>
