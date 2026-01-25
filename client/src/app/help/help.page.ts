import { Component, ChangeDetectorRef } from '@angular/core';
import { Router } from '@angular/router';
import { Global } from '../env';

@Component({
	selector: 'app-help',
	templateUrl: 'help.page.html',
	standalone: false
})

export class Help {
L(st) { return this.global.mytranslate(st); }

constructor(public global: Global, private router: Router, private cdr: ChangeDetectorRef) {}

welcomeTour() {
	this.global.settings.welcomeTourShown = false;
	this.global.settingsSave();
	this.router.navigate(["/"]);
}

}
