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
language;
currentUrl: string = "login";
settings: Settings = {} as Settings;
DONGLEURL: string;
session;

constructor(public plt: Platform, private router: Router, private navCtrl: NavController, private translate: TranslateService, private httpClient: HttpClient) {
	console.log("%c⛅ MyDongle.Cloud: my data, my cloud, my sovereignty 🚀", "font-weight:bold; font-size:x-large;");
	console.log("%cDocs: https://docs.mydongle.cloud", "font-weight:bold; font-size:large;");
	console.log("%cVersion: " + this.VERSION, "background-color:rgb(100, 100, 100); border-radius:5px; padding:5px;");
	console.log("Platform: " + this.plt.platforms());
	this.getSession();
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

setCookie(name, value, domain) {
	//console.log("setCookie: " + `${name}=${value}; domain=${domain}; path=/;`);
	document.cookie = `${name}=${value}; domain=${domain}; path=/;`;
}

domainFromFqdn(fqdn) {
	const parts = fqdn.split('.');
	if (parts.length <= 2)
		return fqdn;
	if (!isNaN(parts[parts.length - 1]))
		return fqdn;
	const sliceIndex = (parts[parts.length - 2] === "mydongle" || parts[parts.length - 2] === "myd") ? -3 : -2;
	return parts.slice(sliceIndex).join('.');
}

setCookieSpecial(name, value) {
	const host = window.location.hostname.replace(/^([^:]*)(?::\d+)?$/i, '$1');
	const domain = this.domainFromFqdn(host);
	this.setCookie(name, value, domain);
}

async AuthHealth() {
	const ret = await this.httpClient.get("/MyDongleCloud/Auth/", {headers:{"content-type": "application/json"}}).toPromise();
	console.log("Auth Health: ", ret);
}

async getSession() {
	this.session = await this.httpClient.get("/MyDongleCloud/Auth/get-session", {headers:{"content-type": "application/json"}}).toPromise();
	console.log("Auth get-session: ", this.session);
	if (this.session != null) {
		const jwt = await this.httpClient.get("/MyDongleCloud/Auth/token", {headers:{"content-type": "application/json"}}).toPromise();
		this.setCookieSpecial("jwt", jwt["token"]);
	}
}

async logout() {
	console.log("logout");
	document.location.href = "/";
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
