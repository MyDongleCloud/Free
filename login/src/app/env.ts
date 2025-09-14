import { Injectable } from '@angular/core';
import { Router } from '@angular/router';
import { HttpClient } from '@angular/common/http';
import { Platform } from '@ionic/angular';
import { NavController } from '@ionic/angular';
import { App } from '@capacitor/app';
import { TranslateService } from '@ngx-translate/core';
import { environment } from '../environments/environment';
import { Settings } from './myinterface';
import { VERSION } from './version';

@Injectable({
	providedIn: "root"
})

export class Global {
VERSION: string = VERSION;
loggedIn = 0;
MASTERURL: string;
language;
currentUrl: string = "login";
settings: Settings = {} as Settings;
DONGLEURL: string;

constructor(public plt: Platform, private router: Router, private navCtrl: NavController, private translate: TranslateService, private httpClient: HttpClient) {
	if (environment.production || this.isPlatform("androidios")) {
		this.MASTERURL = "";
	} else
		this.MASTERURL = "http://localhost:8080/";
	this.checkLogin();
	navCtrl.setDirection("forward");
	translate.setDefaultLang("en");
	console.log("Default browser language is: " + translate.getBrowserLang());
	this.changeLanguage(this.translate.getBrowserLang());
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

async checkLogin() {
	try {
		const ret = await this.httpClient.post(this.MASTERURL + "master/login.json", "", {headers:{"content-type": "application/x-www-form-urlencoded"}}).toPromise();
		//console.log("checkLogin: " + JSON.stringify(ret));
		if (ret["error"] === 0) {
			if (confirm("You are already loggedin. Do you want to logout?")) {
				this.loggedIn = -1;
				document.cookie = "user=; Path=/; Expires=Thu, 01 Jan 1970 00:00:01 GMT;";
				document.cookie = "token=; Path=/; Expires=Thu, 01 Jan 1970 00:00:01 GMT;";
			} else
				this.openPage("", false);
			return;
		}
	} catch(e) {
		console.log("Failed to download " + this.MASTERURL + "/master/login.json");
	}
	this.loggedIn = -1;
}

async logout() {
}

async settingsSave() {
	
}

openPage(url: string, close: boolean) {
	console.log("openPage");
	document.location.href = "/";
}

async backButtonAlert() {
}

async changeLanguage(st) {
	if (st != this.language) {
		this.language = st;
		await this.translate.use(this.language);
	}
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
	if (this.plt.is("ios") && this.plt.is("cordova"))
		return "ios";
	else if (this.plt.is("android") && this.plt.is("cordova"))
		return "android";
	else
		return "web";
}

isPlatform(a) {
	if (a == "androidios")
		return (this.plt.is("android") || this.plt.is("ios")) && this.plt.is("cordova");
	else if (a == "android")
		return this.plt.is("android") && this.plt.is("cordova");
	else if (a == "ios")
		return this.plt.is("ios") && this.plt.is("cordova");
	else if (a == "web")
		return !((this.plt.is("android") && this.plt.is("cordova")) || (this.plt.is("ios") && this.plt.is("cordova")));
	else
		return false;
}

}
