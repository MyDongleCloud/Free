import { Component, ChangeDetectorRef, inject } from '@angular/core';
import { BleService } from '../ble';
import { Global } from '../env';

declare var appInit: any;
declare var appButton: any;
declare var appRequestPasscode: any;

@Component({
	selector: 'app-setup',
	templateUrl: './setup.page.html',
	styleUrls: ['./setup.page.scss'],
	standalone: false
})

export class Setup {

constructor(public global: Global, private cdr: ChangeDetectorRef, public ble: BleService) {
	global.refreshUI.subscribe(event => {
		this.cdr.detectChanges();
	});
}

}
