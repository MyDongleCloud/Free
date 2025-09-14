import { Component, ChangeDetectorRef } from '@angular/core';
import { BleService } from '../ble';
import { Global } from '../env';

declare var appInit: any;
declare var appButton: any;
declare var appRequestPasscode: any;

@Component({
	selector: 'app-dongle',
	templateUrl: './dongle.page.html',
	styleUrls: ['./dongle.page.scss'],
	standalone: false
})

export class Dongle {

constructor(public global: Global, private cdr: ChangeDetectorRef, public ble: BleService) {
	global.refreshUI.subscribe(event => {
		this.cdr.detectChanges();
	});
}

ngOnInit() {
	appInit(this.ble, 1);
}

button(k, l) {
	appButton(k, l);
}

ionViewWillLeave() {
	this.global.settingsSave();
}

ionViewDidEnter() {
	this.global.settingsSave();
}

}
