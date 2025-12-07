import { Component, ChangeDetectorRef } from '@angular/core';
import { Global } from '../env';
import { HttpClient } from '@angular/common/http';

@Component({
	selector: 'app-settings',
	templateUrl: './settings.page.html',
	standalone: false
})

export class Settings {
L(st) { return this.global.mytranslate(st); }
LG(st) { return this.global.mytranslateG(st); }
activeTab = "domains";
adminSudo;
ssh;
signInNotification;

constructor(public global: Global, private cdr: ChangeDetectorRef, private httpClient: HttpClient) {
	global.refreshUI.subscribe(event => {
		this.cdr.detectChanges();
	});
}

async changeLanguageAndRefresh(l: string) {
	await this.global.changeLanguage(l);
	this.cdr.detectChanges();
	setTimeout(() => {this.cdr.detectChanges();}, 500);
}

}
