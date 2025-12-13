let _app_Form;
let _app_Arg1;
let _app_Arg2;
let _app_Arg3;

function _app_React(_app_El) {
	const _app_Key = Object.keys(_app_El).find(key => key.startsWith('__reactProps'));
	if (_app_Key) {
		const _app_Prop = _app_El[_app_Key];
		_app_Prop.value = _app_El.value;
		if (_app_Prop.onChange)
			_app_Prop.onChange({ target: _app_El, currentTarget: _app_El, nativeEvent: new Event('input', { bubbles: true }) });
	}
}

function _app_Color(_app_Ele) {
	const _app_Style = window.getComputedStyle(_app_Ele);
	const _app_PrevColor = _app_Style.color;
    _app_Ele.style.color = _app_Style.backgroundColor;
	setTimeout(function() { _app_Ele.style.color = _app_PrevColor; }, 2000);
}

function _app_Credentials() {
	_app_Color(_app_Arg1);
	_app_Arg1.value = '%s';
	_app_Arg1.dispatchEvent(new Event('input', { bubbles: true }));
	_app_React(_app_Arg1);
	_app_Color(_app_Arg2);
	_app_Arg2.value = '%s';
	_app_Arg2.dispatchEvent(new Event('input', { bubbles: true }));
	_app_React(_app_Arg2);
	_app_Arg3.click();
}
window._app_Credentials = _app_Credentials;

let _app_Tries = 0;
function _app_Insert() {
	if (_app_Tries++ > 5)
		return;
	_app_Form = document.querySelector('%s');
	if (_app_Form !== null) {
		_app_Arg1 = _app_Form.querySelector('%s');
		_app_Arg2 = _app_Form.querySelector('%s');
		_app_Arg3 = _app_Form.querySelector('%s');
		if (_app_Arg1 !== null && _app_Arg2 !== null && _app_Arg3 !== null)
			document.body.insertAdjacentHTML('beforeend', '<div style="position:absolute; z-index:99; top:100px; right:50px; padding:10px; background-color:#000f4e; color:white; font-weight:bold; text-align:center; border:2px solid white; border-radius:15px;">MyDongle.Cloud<br><button style="text-align:center; background-color:#0092ce; color:white; margin-top:10px; border-radius:10px; padding:5px; cursor:pointer;" onclick="window._app_Credentials();">Automatic<br>Login</button></div>');
		else
			setTimeout(_app_Insert, 1000);
	} else
		setTimeout(_app_Insert, 1000);
}

document.addEventListener('DOMContentLoaded', (event) => { _app_Insert(); });
