import { Injectable } from '@angular/core';
import { BleClient, textToDataView, numberToUUID, ScanResult, ConnectionPriority } from '@capacitor-community/bluetooth-le';
import { HttpClient } from '@angular/common/http';
import { Filesystem, Encoding } from '@capacitor/filesystem';
import { Global } from './env';
import { Observable, Subject } from 'rxjs';

const BLE_NAME = "MyDongle-";
const UUID_GATT = "0000fff0-0000-1000-8000-00805f9b34fb";
const UUID_VERSION = "0000fff1-0000-1000-8000-00805f9b34fb";
const UUID_DATA = "0000fff2-0000-1000-8000-00805f9b34fb";

const CHUNK_BLE = 181;

declare var appServerReceive: any;
declare var appCommunicationStatus: any;

@Injectable({
  providedIn: 'root'
})

export class BleService {
connectedBLE: number;
deviceID: string;
deviceName: string;
pairReceived: boolean;
firmwareDeviceVersion: string;
deviceAddr: string;
lastTouch = null;
bleClientInitialized: boolean = false;

constructor(private global: Global, private httpClient: HttpClient) {
	this.bleClientInitialize();
}

async bleClientInitialize() {
	try {
		await BleClient.initialize();
		this.bleClientInitialized = true;
		return;
	} catch(e) {}
}

connectToggle() {
	if (this.connectedBLE == 2)
		this.disconnect();
	else if (this.connectedBLE == 1) {
		this.connectedBLE = 0;
		this.global.refreshUI.next(true);
		this.stopScan();
	} else
		this.tryConnect();
}


async stopScan() {
	console.log("Stop Scan");
	await BleClient.stopLEScan();
}

async disconnect() {
	await BleClient.disconnect(this.deviceID);
	this.connectedBLE = 0;
	this.global.refreshUI.next(true);
	await this.stopScan();
}

async tryConnect() {
	console.log("tryConnect");
	this.connectedBLE = 1;
	this.global.refreshUI.next(true);
	if (!this.bleClientInitialized)
		await this.bleClientInitialize();
	try {
		if (!await BleClient.isEnabled())
			await BleClient.requestEnable();
	} catch(e) {
		console.log("BLE enable() not working " + e.message);
	}
	console.log("BLE isEnabled:" + await BleClient.isEnabled());
	if (!this.global.plt.is("android") && !this.global.plt.is("ios")) {
		try {
			const bled = await BleClient.requestDevice({optionalServices:[UUID_GATT]});
		this.deviceID = bled.deviceId;
		this.deviceName = bled.name;
			await this.connectToBluetoothDevice(bled.deviceId);
		} catch(e) {
			this.connectedBLE = 0;
			this.global.refreshUI.next(true);
		}
	} else {
		await BleClient.requestLEScan({services:[]}, this.onBluetoothDeviceFound.bind(this));
	}
}

async onBluetoothDeviceFound(result) {
	console.log("onBluetoothDeviceFound " + JSON.stringify(result));
	if (result.device.name != undefined && result.device.name.startsWith(BLE_NAME)) {
		if (this.deviceID != "") {
			await this.stopScan();
			this.global.refreshUI.next(true);
			return;
		}
		this.deviceID = result.device.deviceId;
		this.deviceName = result.device.name;
		console.log("onBluetoothDeviceFound " + result.device.deviceId);
		console.log("Found device Connecting to " + this.deviceID + " (" + this.deviceName + ")...");
		await this.stopScan();
		await this.connectToBluetoothDevice(result.device.deviceId);
	}
}

async writeData(a) {
	await BleClient.write(this.deviceID, UUID_GATT, UUID_DATA, textToDataView(a));
}

bleNotifyDataCb = ((value) => {
	var buf = new Uint8Array(value.buffer);
	var bufSt = "";
	const len = buf.byteLength;
	for (let i = 0; i < len; i++)
		bufSt += String.fromCharCode(buf[i]);
	const b = JSON.parse(bufSt);
	if (b.a === "state")
		appServerReceive(bufSt, 0);
});

async connectToBluetoothDevice(devId: string) {
	let err = 0;
	const a = await BleClient.connect(devId, (dId) => {this.onDeviceDisconnected(dId);}).catch(error => {
		console.log("connect error: ", error);
		err = 1;
	});
	if (err == 1) {
		this.connectedBLE = 0;
		this.global.refreshUI.next(true);
		return;
	}
	appCommunicationStatus(1);
	console.log("connectToBluetoothDevice device success!");
	this.connectedBLE = 2;
	this.global.refreshUI.next(true);
	this.listServices(devId);
	BleClient.startNotifications(devId, UUID_GATT, UUID_DATA, this.bleNotifyDataCb).catch(error => {
		console.log("startNotifications UUID_DATA error: ", error)
	});
	await this.version();
	await this.syncDate();
	if (this.global.plt.is("android")) {
		console.log("Request HIGH ConnectionPriority for interval");
		BleClient.requestConnectionPriority(devId, ConnectionPriority.CONNECTION_PRIORITY_HIGH);
	}
}

onDeviceDisconnected(disconnectedDeviceId: string) {
	this.deviceID = "";
	this.deviceName = "";
	this.connectedBLE = 0;
	this.global.refreshUI.next(true);
	console.log("Disconnected device " + disconnectedDeviceId);
	appCommunicationStatus(0);
}

async listServices(deviceId: string) {
	try {
		await BleClient.getServices(deviceId).then((value) => {
			console.log("services " + JSON.stringify(value));
		});
	} catch (e) {
	  console.log("getServices Error " + e.message());
	}
}

async requestPasscode() {
	await this.writeData(JSON.stringify({ a:"passcode" }));
}

async shutdown() {
	await this.writeData(JSON.stringify({ a:"shutdown" }));
}

async checkVersion() {
		if (this.firmwareDeviceVersion < this.global.firmwareServerVersion && this.global.currentUrl != "upgrade") {
			let ret = await this.global.presentQuestion("Upgrade needed", "Do you want to upgrade the device now?", "The firmware of the device is too old and not compatible with this app.");
			if (ret)
				this.global.openPage("upgrade", false);
		}
}

async syncDate() {
	const d = new Date();
	const localTime = Math.floor(d.getTime() / 1000 - (d.getTimezoneOffset() * 60));
	await this.writeData(JSON.stringify({ a:"date", v:localTime }));
}

async version() {
	await BleClient.read(this.deviceID, UUID_GATT, UUID_VERSION).then(value => {
		const ar = new Uint8Array(value.buffer);
		this.firmwareDeviceVersion = new TextDecoder().decode(ar);
		console.log("Device version " + this.firmwareDeviceVersion);
		this.global.refreshUI.next(true);
		this.checkVersion();
	});
}

}
