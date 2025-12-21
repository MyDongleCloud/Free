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
dSave: boolean = true;

constructor(public global: Global, private cdr: ChangeDetectorRef, private httpClient: HttpClient) {
	global.refreshUI.subscribe(event => {
		this.cdr.detectChanges();
	});
	this.adminSudo = true;
	this.signInNotification = true;
	this.dSave = true;
}

async save() {
	const data = { security:{ adminSudo:this.adminSudo, signInNotification:this.signInNotification } };
	const ret = await this.httpClient.post("/_app_/auth/settings/save", JSON.stringify(data), { headers:{ "content-type": "application/json" } }).toPromise();
	this.global.consolelog(2, "Auth settings/save: ", ret);
}

async refresh() {
	const ret = await this.httpClient.get("/_app_/auth/refresh", {headers:{"content-type": "application/json"}}).toPromise();
	this.global.consolelog(2, "Auth Refresh: ", ret);
	this.global.presentToast("Modules have been refreshed.", "help-outline");
	this.global.modulesDataPrepare();
}

async wifiApply() {
	const ret = await this.httpClient.get("/_app_/auth/wifi/apply", {headers:{"content-type": "application/json"}}).toPromise();
	this.global.consolelog(2, "Auth wifi/apply: ", ret);
}

}
