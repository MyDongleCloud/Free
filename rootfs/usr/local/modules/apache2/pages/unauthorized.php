<html>
<body>
<b>Unauthorized access for module "<?=$_GET["m"];?>"</b>. Please login <a href="/MyDongleCloud/login/">here</a>. You will be redirected to this login page in 2 seconds...
<script>
setTimeout(function() { window.location.href = "/MyDongleCloud/login/"; }, 2000);
</script>
</body>
</html>
