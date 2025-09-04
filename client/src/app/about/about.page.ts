import { Component, ChangeDetectorRef } from '@angular/core';
import { BleService } from '../ble';
import { Global } from '../env';

@Component({
	selector: 'app-about',
	templateUrl: 'about.page.html',
	styleUrls: ['about.page.scss'],
	standalone: false
})

export class About {
isDevCheckbox: boolean;

constructor(public global: Global, private cdr: ChangeDetectorRef, public ble: BleService) {}

ionViewWillLeave() {
	this.global.settingsSave();
}

ionViewDidEnter() {
	this.isDevCheckbox = this.global.settings.isDev == 2;
}

async changeLanguageAndRefresh(l: string) {
	await this.global.changeLanguage(l);
	this.cdr.detectChanges();
	setTimeout(() => {this.cdr.detectChanges();}, 500);
}

isDevChange() {
	this.global.settings.isDev = this.isDevCheckbox ? 2 : 1;
}

async longCopyright() {
	if (this.global.settings.isDev == 2) {
		await this.global.presentAlert("Info", "Development Mode", "You need to disable Development Mode.");
		return;
	} else if (this.global.settings.isDev == 1)
		this.global.settings.isDev = 0;
	else
		this.global.settings.isDev = 1;
}

}
