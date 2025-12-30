import { Component, ChangeDetectorRef } from '@angular/core';
import { Global } from '../env';

@Component({
	selector: 'app-help',
	templateUrl: 'help.page.html',
	standalone: false
})

export class Help {
L(st) { return this.global.mytranslate(st); }
LG(st) { return this.global.mytranslateG(st); }

constructor(public global: Global, private cdr: ChangeDetectorRef) {}

welcomeTour() {
	this.global.settings.welcomeTourShown = false;
	this.global.settingsSave();
	this.global.openPage("");
}

}
