var _app_Form;
function _app_Credentials() {
	var input = document.createElement('input');
	input.type = 'hidden';
	input.name = 'mdcAL';
	input.value = 1;
	_app_Form.appendChild(input);
	HTMLFormElement.prototype.submit.call(_app_Form);
};
window.__app_Credentials = _app_Credentials;
var _app_Tries = 0;
function _app_Insert() {
	if (_app_Tries++ > 5)
		return;
	_app_Form = document.querySelector('form%s');
	if (_app_Form !== null)
		document.body.insertAdjacentHTML('beforeend', '<div style="position:absolute; z-index:99; top:100px; right:50px; padding:10px; background-color:#000f4e; color:white; font-weight:bold; font-size:; text-align:center; border:2px solid white; border-radius:15px;">MyDongle.Cloud<br><button style="text-align:center; background-color:#0092ce; color:white; margin-top:10px; border-radius:10px; padding:5px; cursor:pointer;" onclick="window.__app_Credentials();">Automatic<br>Login</button></div>');
	else
		setTimeout(_app_Insert, 1000);
}

document.addEventListener('DOMContentLoaded', (event) => { _app_Insert(); });
