var Module;
var thisble;
var initDone = false;
var socket = null;

function loadScript(src) {
	return new Promise((resolve, reject) => {
		const script = document.createElement("script");
		script.type = "text/javascript";
		script.src = src;
		script.onload = () => resolve(script);
		script.onerror = (error) => reject(error);
		document.body.appendChild(script);
	});
}

async function appInit(tb, path, log, slave) {
	if (!initDone) {
		thisble = tb;
		await loadScript(path);
		initDone = true;
	}
	Module = {
		print: function(text) { if (log) console.log(text); },
		canvas: document.getElementById("canvas"),
		arguments: slave ? ["-s"] : []
	};
	await appCreate(Module);
}

function appButton(b, l) {
	if (!initDone)
		return;
	Module._button(b, l);
}

function appCommunicationStatus(s) {
	if (!initDone)
		return;
	Module._communicationStatus(s);
}

function appServerWriteDataHtml(data, isB64) {
	if (!initDone)
		return;
	var st;
	if (isB64)
		st = atob(data);
	else
		st = data;
	//Module.print("(JS) appServerWriteDataHtml: data#" + st + "#B64:" + isB64);
	if (socket != null)
		socket.send(st);
	else
		thisble.writeData(st);
}

function appServerReceiveHtml(data, doB64) {
	if (!initDone)
		return;
	//Module.print("(JS) appServerReceiveHtml: data#" + data + "#B64:" + doB64);
	var st;
	if (doB64)
		st = btoa(data);
	else
		st = data;
	var size = Module.lengthBytesUTF8(st) + 1;
	var ptr = Module._malloc(size);
	Module.stringToUTF8(st, ptr, size);
	Module._serverReceiveHtml(ptr, doB64);
	Module._free(ptr);
}

function appConnectToggle() {
	if (socket != null) {
		socket.close();
		socket = null;
	} else {
		const protocol = "ws" + (window.location.protocol === "https:" ? "s" : "") + "://";
		const host = (window.location.hostname == "" ? "localhost" : window.location.hostname);
		const ipv4Regex = /^(?:25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)(?:\.(?:25[0-5]|2[0-4]\d|1\d\d|[1-9]\d|\d)){3}$/gm;
		const port = ipv4Regex.host || window.location.hostname == "" ? ":8094" : "";
		const ws = protocol + host + port + "/ws/";
		console.log("socketInit " + ws);
		socket = new WebSocket(ws);
		socket.binaryType = "arraybuffer";
		socket.onopen = () => {
			console.log("socket onopen");
		}
		socket.onerror = (e) => {
			console.log("socket onerror " + JSON.stringify(e));
			socket = null;
		}
		socket.onclose = (e) => {
			console.log("socket onclose " + JSON.stringify(e));
			socket = null;
		}
		socket.onmessage = (msg) => {
			appServerReceiveHtml(msg.data, 1);
		}
	}
}
