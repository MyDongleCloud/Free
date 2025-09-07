var Module;
var thisble;
var initDone = false;

function appInit(tb, log) {
	if (initDone) return;
	thisble = tb;
	Module = {
		print: function(text) { if (log) console.log(text); },
		canvas: (function() { return document.getElementById("canvas"); })(),
		arguments: ["-s"]
	};

	const script = document.createElement("script");
	script.type = "text/javascript";
	script.src = "assets/app.js";
	document.body.appendChild(script);
	initDone = true;
}

function appButton(b, l) {
	Module._button(b, l);
}

function appRequestPasscode(p) {
	Module._requestPasscode(p);
}

function appCommunicationStatus(s) {
	Module._communicationStatus(s);
}

function appServerWriteData(data, isB64) {
	var st;
	if (isB64)
		st = atob(data);
	else
		st = data;
	//Module.print("(JS) serverWriteData: data#" + st + "#B64:" + isB64);
	thisble.writeData(st);
}

function appServerReceive(data, doB64) {
	//Module.print("(JS) appServerReceive: data#" + data + "#B64:" + doB64);
	var st;
	if (doB64)
		st = btoa(data);
	else
		st = data;
	var size = lengthBytesUTF8(st) + 1;
	var ptr = Module._malloc(size);
	stringToUTF8(st, ptr, size);
	Module._serverReceive(ptr, doB64);
	Module._free(ptr);
}
