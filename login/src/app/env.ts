import { Injectable } from '@angular/core';
import { Router } from '@angular/router';
import { HttpClient } from '@angular/common/http';
import { Platform } from '@ionic/angular';
import { NavController } from '@ionic/angular';
import { App } from '@capacitor/app';
import { TranslateService } from '@ngx-translate/core';
import { Observable, Subject } from 'rxjs';
import { environment } from '../environments/environment';
import { VERSION } from './version';

@Injectable({
	providedIn: "root"
})

export class Global {
VERSION: string = VERSION;
MASTERURL: string;
token;
language;
currentUrl: string = "login";
refreshUI:Subject<any> = new Subject();
refreshO;
refreshObs;

constructor(public plt: Platform, private router: Router, private navCtrl: NavController, private translate: TranslateService, private httpClient: HttpClient) {
	if (environment.production || this.isPlatform("androidios")) {
		this.MASTERURL = "";
	} else
		this.MASTERURL = "http://localhost:8080";
	this.token = this.getCookie("token");
	console.log("Cookie token: " + this.token);
	navCtrl.setDirection("forward");
	translate.setDefaultLang("en");
	console.log("Default browser language is: " + translate.getBrowserLang());
	this.changeLanguage(this.translate.getBrowserLang());
	if (typeof (<any>window).electron != "undefined") {
		(<any>window).electron.ipc.log((err: any, v: string) => {
			console.log(v);
		});
		(<any>window).electron.ipc.open_page((err: any, v: string) => {
			;//this.openPage(v, false);
		});
	}
	this.refreshO = Observable.create((obs) => {
		this.refreshObs = obs;
		return () => {}
	});
}

getCookie(name) {
	const allCookies = document.cookie;
	const nameEQ = name + "=";
	const ca = document.cookie.split(';');
	for(let i=0; i < ca.length; i++) {
		let c = ca[i];
		while (c.charAt(0) === ' ') c = c.substring(1, c.length);
		if (c.indexOf(nameEQ) === 0)
			return c.substring(nameEQ.length, c.length);
	}
	return null;
}

async backButtonAlert() {
}

async changeLanguage(st) {
	if (st != this.language) {
		this.language = st;
		await this.translate.use(this.language);
		if (typeof (<any>window).electron != "undefined")
			(<any>window).electron.ipc.invoke("menuLanguage", this.language);
	}
}

changeLanguageAndRefresh(l) {
	this.changeLanguage(l);
	this.refreshObs.next();
}

mytranslateP(page, st) {
	const inp = page + "." + st;
	const ret = this.translate.instant(page + "." + st);
	return ret == "" || ret == inp ? st : ret;
}

mytranslate(st) {
	return this.mytranslateP(this.currentUrl == "" ? "login" : this.currentUrl, st);
}

async sleepms(ms) {
	return new Promise(resolve => setTimeout(resolve, ms));
}

async platform() {
	if (typeof (<any>window).electron != "undefined")
		return await (<any>window).electron.ipc.invoke("os");
	else if (this.plt.is("ios") && this.plt.is("cordova"))
		return "ios";
	else if (this.plt.is("android") && this.plt.is("cordova"))
		return "android";
	else
		return "web";
}

isPlatform(a) {
	if (a == "electron")
		return typeof (<any>window).electron != "undefined";
	else if (a == "androidios")
		return (this.plt.is("android") || this.plt.is("ios")) && this.plt.is("cordova");
	else if (a == "android")
		return this.plt.is("android") && this.plt.is("cordova");
	else if (a == "ios")
		return this.plt.is("ios") && this.plt.is("cordova");
	else if (a == "web")
		return !(typeof (<any>window).electron != "undefined" || (this.plt.is("android") && this.plt.is("cordova")) || (this.plt.is("ios") && this.plt.is("cordova")));
	else
		return false;
}

}
