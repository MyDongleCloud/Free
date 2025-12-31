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
	var argCmdLine = [];
	if (window.location.hostname.indexOf("mondongle.cloud") != -1)
		argCmdLine = [ "-l", "fr" ];
	else if (navigator.language.startsWith("fr"))
		argCmdLine = [ "-l", "fr" ];
	Module = {
		print: function(text) { if (log) console.log(text); },
		canvas: document.getElementById("canvas"),
		arguments: slave ? [ "-s" ] : argCmdLine
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
		socket.send(JSON.stringify({ a:"connection", c:0 }));
		socket.close();
		socket = null;
		if (thisble) thisble.connectedWS = 0;
	} else {
		if (thisble) thisble.connectedWS = 1;
		const protocol = "ws" + (window.location.protocol === "https:" ? "s" : "") + "://";
		let host = window.location.hostname == "" || window.location.hostname == "localhost" ? "localhost:8094" : window.location.hostname;
		const ipv4Regex = /^(?:(?:25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])\.){3}(?:25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])$/;
		if (ipv4Regex.test(host))
			host += ":9400";
		const ws = protocol + host + "/ws/";
		console.log("socketInit " + ws);
		socket = new WebSocket(ws);
		socket.binaryType = "arraybuffer";
		socket.onopen = () => {
			console.log("Socket onopen");
			if (thisble) thisble.connectedWS = 2;
		}
		socket.onerror = (e) => {
			console.log("Socket onerror " + JSON.stringify(e));
			socket = null;
			if (thisble) thisble.connectedWS = 0;
			appCommunicationStatus(0);
		}
		socket.onclose = (e) => {
			console.log("Socket onclose");
			socket = null;
			if (thisble) thisble.connectedWS = 0;
			appCommunicationStatus(0);
		}
		socket.onmessage = (msg) => {
			appServerReceiveHtml(msg.data, 1);
		}
	}
}

function appShutdown() {
	if (socket != null)
		socket.send(JSON.stringify({ a:"shutdown" }));
}

function appOtp(email) {
	if (socket != null)
		socket.send(JSON.stringify({ a:"otp", v:-1, e:email }));
}

function appLanguage(la) {
	if (socket != null)
		socket.send(JSON.stringify({ a:"language", l:la }));
}
