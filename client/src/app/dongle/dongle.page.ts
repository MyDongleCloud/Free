import { Component, ChangeDetectorRef } from '@angular/core';
import { BleService } from '../ble';
import { Global } from '../env';

declare var socket: any;
declare var appInit: any;
declare var appButton: any;
declare var appConnectToggle: any;
declare var appShutdown: any;
declare var appPasscode: any;

@Component({
	selector: 'app-dongle',
	templateUrl: './dongle.page.html',
	styleUrls: ['./dongle.page.scss'],
	standalone: false
})

export class Dongle {
typeBluetooth: boolean = true;

constructor(public global: Global, private cdr: ChangeDetectorRef, public ble: BleService) {
	global.refreshUI.subscribe(event => {
		this.cdr.detectChanges();
	});
	ble.communicationEvent.subscribe((event) => {
	});
}

ngOnInit() {
	appInit(this.ble, "assets/app.js", true, true);
}

button(k, l) {
	appButton(k, l);
}

ionViewWillLeave() {
	if (this.ble.connectedBLE == 2)
		this.ble.connectToggle();
	if (this.ble.connectedWS == 2)
		appConnectToggle();
}

connectToggle() {
	if (this.typeBluetooth)
		this.ble.connectToggle();
	else
		appConnectToggle();
}

shutdown() {
	if (this.typeBluetooth)
		this.ble.shutdown();
	else
		appShutdown();
}

passcode() {
	if (this.typeBluetooth)
		this.ble.passcode();
	else
		appPasscode();
}

}
