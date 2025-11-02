import { Injectable } from '@angular/core';
import { CanActivate, ActivatedRouteSnapshot, RouterStateSnapshot, Router } from '@angular/router';
import { HttpClient } from '@angular/common/http';
import { Platform } from '@ionic/angular';
import { NavController, AlertController, MenuController } from '@ionic/angular';
import { PopoverController } from '@ionic/angular';
import { Filesystem, Directory } from '@capacitor/filesystem';
import { App } from '@capacitor/app';
import { Device } from '@capacitor/device';
import { Preferences } from '@capacitor/preferences';
import { InAppReview } from '@capacitor-community/in-app-review';
import { TranslateService } from '@ngx-translate/core';
import { Subject } from 'rxjs';
import { environment } from '../environments/environment';
import { Settings } from './myinterface';
import { VERSION } from './version';
//import { toASCII } from 'punycode';
import * as ACME from '@root/acme';
import * as Keypairs from '@root/keypairs';
import * as CSR from '@root/csr';
import * as PEM from '@root/pem';
import * as ENC from '@root/encoding/base64';

@Injectable({
	providedIn: "root"
})

export class Global implements CanActivate {
developer: boolean = false;
demo: boolean = false;
splashDone = false;
VERSION: string = VERSION;
SERVERURL: string = "https://mydongle.cloud";
currentUrl: string;
activateUrl: string;
settings: Settings = {} as Settings;
refreshUI:Subject<any> = new Subject();
firmwareServerVersion;
session;

constructor(public plt: Platform, private router: Router, private navCtrl: NavController, private alertCtrl: AlertController, private menu: MenuController, private translate: TranslateService, public popoverController: PopoverController, private httpClient: HttpClient) {
	this.developer = window.location.hostname == "localhost" && window.location.port == "8100";
	this.consolelog(0, "%c⛅ MyDongle.Cloud: my data, my cloud, my sovereignty 🚀", "font-weight:bold; font-size:x-large;");
	this.consolelog(0, "%cDocs: https://docs.mydongle.cloud", "font-weight:bold; font-size:large;");
	this.consolelog(0, "%cVersion: " + this.VERSION, "background-color:rgb(100, 100, 100); border-radius:5px; padding:5px;");
	this.consolelog(1, "Platform: " + this.plt.platforms());
	navCtrl.setDirection("forward");
	translate.setDefaultLang("en");
	this.consolelog(1, "Default browser language: " + translate.getBrowserLang());
	if (window.location.hostname.indexOf("mondongle.cloud") != -1)
		this.changeLanguage("fr");
	else
		this.changeLanguage(this.translate.getBrowserLang());
	this.AuthStatus();
	this.getSession();
	this.settingsLoad();
}

consolelog(level, ...st) {
	if (level == 0 || this.developer)
		console.log(...st);
}

getCookie(name) {
	const allCookies = document.cookie;
	const nameEQ = name + "=";
	const ca = document.cookie.split(';');
	for(let i=0; i < ca.length; i++) {
		let c = ca[i];
		while (c.charAt(0) === ' ') c = c.substring(1, c.length);
		if (c.indexOf(nameEQ) === 0)
			return decodeURIComponent(c.substring(nameEQ.length, c.length));
	}
	return null;
}

domainFromFqdn(fqdn) {
	const parts = fqdn.split('.');
	if (parts.length <= 2)
		return fqdn;
	if (!isNaN(parts[parts.length - 1]))
		return fqdn;
	const sliceIndex = (parts[parts.length - 2] === "mydongle" || parts[parts.length - 2] === "mondongle" || parts[parts.length - 2] === "myd") ? -3 : -2;
	return parts.slice(sliceIndex).join('.');
}

setCookie(name, value, domain = null) {
	const host = window.location.hostname.replace(/^([^:]*)(?::\d+)?$/i, '$1');
	if (domain == null)
		domain = this.domainFromFqdn(host);
	if (value == "")
		document.cookie = `${name}=; Domain=${domain}; Path=/; Expires=Thu, 01 Jan 1970 00:00:01 GMT;`;
	else
		document.cookie = `${name}=${value}; Domain=${domain}; Path=/;`;
}

async AuthStatus() {
	const ret = await this.httpClient.get("/MyDongleCloud/Auth/status", {headers:{"content-type": "application/json"}}).toPromise();
	if (ret["demo"] === true)
		this.demo = true;
	this.consolelog(2, "Auth Status: ", ret);
}

async getSession() {
	this.session = await this.httpClient.get("/MyDongleCloud/Auth/get-session", {headers:{"content-type": "application/json"}}).toPromise();
	this.consolelog(2, "Auth get-session: ", this.session);
	if (this.session != null) {
		const jwt = await this.httpClient.get("/MyDongleCloud/Auth/token", {headers:{"content-type": "application/json"}}).toPromise();
		this.setCookie("jwt", jwt["token"]);
	}
}

async logout() {
	this.setCookie("jwt", "");
	const data = { token:this.session?.session?.token };
	try {
		const ret = await this.httpClient.post("/MyDongleCloud/Auth/revoke-session", JSON.stringify(data), {headers:{"content-type": "application/json"}}).toPromise();
		this.consolelog(2, "Auth revoke-session: ", ret);
	} catch(e) {}
	this.session = null;
	this.settingsSave();
}

async logoutRedirect() {
	await this.logout();
	this.openPage("/login");
}

async settingsLoad() {
	this.consolelog(1, "settingsLoad: Enter");
	const { value } = await Preferences.get({key: "settings"});
	if (value != null)
		this.settings = JSON.parse(value);
	if (this.settings.powerUser === undefined)
		this.settings.powerUser = false;
	if (this.settings.isDev === undefined)
		this.settings.isDev = 0;
	if (this.settings.dontShowAgain === undefined)
		this.settings.dontShowAgain = Object();
	if (this.settings.deviceId === undefined)
		this.settings.deviceId = await Device.getId()["identifier"];
	const e = this.getCookie("email");
	if (e !== undefined)
		this.settings.email = e;
	await this.translate.use(this.settings.language);
}

async settingsSave() {
	this.consolelog(1, "settingsSave: Enter");
	let st = JSON.stringify(this.settings);
	await Preferences.set({key: "settings", value: st});
}

async backButtonAlert() {
	const alert = await this.alertCtrl.create({
		message: this.mytranslateP("splash", "Do you want to leave this application?"),
		buttons: [{
			text: "cancel",
			role: "cancel"
		},{
			text: "Close App",
			handler: () => {
				App.exitApp();
			}
		}]
	});
	await alert.present();
}

async presentQuestion(hd, st, msg, key:string = "") {
	if (this.settings.dontShowAgain[key] !== undefined)
		return false;
	let checked = false;
	let yesClicked = false;
	const question = await this.alertCtrl.create({
		cssClass: "basic-alert",
		header: hd,
		subHeader: st,
		message: msg,
		buttons: [{
			text: "Yes", handler: (data) => { yesClicked = true; if (data !== undefined && data.length > 0 && data[0]) checked = true; }
		}, {
			text: "No", cssClass: "secondary", handler: (data) => { yesClicked = false; if (data !== undefined && data.length > 0 && data[0]) checked = true; }
		}],
		inputs: key != "" ? [{ label:"Don't show again", type:"checkbox", checked:false, value:true }] : []
	});
	await question.present();
	await question.onDidDismiss();
	if (checked) {
		this.settings.dontShowAgain[key] = true;
		this.settingsSave();
	}
	return yesClicked;
}

async changeLanguage(st) {
	if (st != this.settings.language) {
		this.settings.language = st;
		await this.translate.use(this.settings.language);
	}
}

changeLanguageAndRefresh(l) {
	this.changeLanguage(l);
	this.refreshUI.next(true);
}

mytranslateP(page, st) {
	const inp = page + "." + st;
	const ret = this.translate.instant(page + "." + st);
	return ret == "" ? (this.developer && this.settings.language != "en" ? ("##" + st + "##") : st) : ret == inp ? (this.developer ? ("##" + st + "##") : st) : ret;
}

mytranslate(st) {
	return this.mytranslateP(this.currentUrl, st);
}

mytranslateG(st) {
	return this.mytranslateP("global", st);
}

mytranslateMT(st) {
	return this.mytranslateP("modules.title", st);
}

mytranslateMD(st) {
	return this.mytranslateP("modules.description", st);
}

async sleepms(ms) {
	return new Promise(resolve => setTimeout(resolve, ms));
}

openBrowser(url: string) {
	window.open(url, "_blank");
}

openPage(url: string) {
	this.navCtrl.setDirection('root');
	this.router.navigate(["/" + url]);
}

openModule(module, page, extract) {
	if (extract && !this.demo)
		window.open(location.protocol + "//" + location.host + "/m/" + module + (page ?? ""), "_blank");
	else {
		this.navCtrl.setDirection('root');
		this.router.navigate(["/wrapper"], { queryParams:{ module, page } });
	}
}

async presentAlert(hd, st, msg, key:string = "") {
	let checked = false;
	if (this.settings.dontShowAgain[key] !== undefined)
		return;
	const alert = await this.alertCtrl.create({
		cssClass: "basic-alert",
		header: hd,
		subHeader: st,
		message: msg,
		buttons: [{ text:"OK", handler: data => { if (data !== undefined && data.length > 0 && data[0]) checked = true; } }],
		inputs: key != "" ? [{label:"Don't show again", type:"checkbox", checked:false, value:true}] : []
	});
	await alert.present();
	await alert.onDidDismiss();
	if (checked) {
		this.settings.dontShowAgain[key] = true;
		this.settingsSave();
	}
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

getMonth(a) {
	if (typeof a == "undefined")
		return "";
	else if (a.indexOf("01.01") != -1 || a.indexOf("01-01") != -1)
		return "January";
	else if (a.indexOf("02.02") != -1 || a.indexOf("02-02") != -1)
		return "February";
	else if (a.indexOf("03.03") != -1 || a.indexOf("03-03") != -1)
		return "March";
	else if (a.indexOf("04.04") != -1 || a.indexOf("04-04") != -1)
		return "April";
	else if (a.indexOf("05.05") != -1 || a.indexOf("05-05") != -1)
		return "May";
	else if (a.indexOf("06.06") != -1 || a.indexOf("06-06") != -1)
		return "June";
	else if (a.indexOf("07.07") != -1 || a.indexOf("07-07") != -1)
		return "July";
	else if (a.indexOf("08.08") != -1 || a.indexOf("08-08") != -1)
		return "August";
	else if (a.indexOf("09.09") != -1 || a.indexOf("09-09") != -1)
		return "September";
	else if (a.indexOf("10.10") != -1 || a.indexOf("10-10") != -1)
		return "October";
	else if (a.indexOf("11.11") != -1 || a.indexOf("11-11") != -1)
		return "November";
	else if (a.indexOf("12.12") != -1 || a.indexOf("12-12") != -1)
		return "December";
	return "";
}

iconCategory(st) {
	if (st == "Backend")
		return "aperture-outline";
	else if (st == "Developer")
		return "code-slash-outline";
	else if (st == "Essential")
		return "star-outline";
	else if (st == "Personal")
		return "person-outline";
	else if (st == "Productivity")
		return "attach-outline";
	else if (st == "Utils")
		return "hammer-outline";
	else
		return null;
}

colorWord(st) {
	if (st == "_disabled_")
		return "bg-red-100 text-red-800";

	else if (st == "_public_")
		return "bg-green-100 text-green-800";
	else if (st == "_localnetwork_")
		return "bg-orange-100 text-orange-800";
	else if (st == "_groupadmin_" || st == "admin")
		return "bg-yellow-100 text-yellow-800";
	else if (st == "_groupuser_" || st == "users")
		return "bg-purple-100 text-purple-800";

	else if (st == "Essential")
		return "bg-purple-100 text-purple-800";
	else if (st == "Personal")
		return "bg-blue-100 text-blue-800";
	else if (st == "Productivity")
		return "bg-indigo-100 text-indigo-800";
	else if (st == "Utils")
		return "bg-cyan-100 text-cyan-800";
	else if (st == "Developer")
		return "bg-red-100 text-red-800";

	else
		return "bg-gray-100 text-gray-800";
}

review() {
	this.settings.reviewRequestLastTime = Date.now();
	this.settingsSave();
	InAppReview.requestReview();
}

async getCertificate(cloud) {
	const ret = { accountKey:"", accountKeyId:"", fullChain:"", privateKey:"" };
	const DOMAIN = "mydongle.cloud";
	const STAGING = true;
	let acme = ACME.create({ maintainerEmail: "acme@" + DOMAIN, packageAgent: "MDC/2025-01-01", notify: function (ev, msg) { /*this.consolelog(1, msg);*/ }, skipDryRun: true });
	await acme.init("https://acme" + (STAGING ? "-staging" : "") + "-v02.api.letsencrypt.org/directory");

	const accountKeypair = await Keypairs.generate({ kty: "EC", format: "jwk" });
	const accountKey = accountKeypair.private;
	ret.accountKey = await Keypairs.export({ jwk: accountKey });
	console.info("CERTIFICATE: Registering ACME account...");
	const account = await acme.accounts.create({ subscriberEmail: "certificate@" + DOMAIN, agreeToTerms: true, accountKey: accountKey });
	console.info("CERTIFICATE: Created account with id ", account.key.kid);
	ret.accountKeyId = account.key.kid;

	const serverKeypair = await Keypairs.generate({ kty: "RSA", format: "jwk" });
	const serverKey = serverKeypair.private;
	ret.privateKey = await Keypairs.export({ jwk: serverKey });

	let domains = [cloud + "." + DOMAIN, "*." + cloud + "." + DOMAIN];
	domains = domains.map(function(name) {
		return name;//toASCII(name);
	});
	const csrDer = await CSR.csr({ jwk: serverKey, domains: domains, encoding: "der" });
	const csr = PEM.packBlock({ type: "CERTIFICATE REQUEST", bytes: csrDer });
	const challenges = {
		"dns-01": {
			init: async (args) => {
				return null;
			},
			zones: async ({ challenge }) => {
				this.consolelog(1, "CERTIFICATE: Zones: ", challenge.dnsHosts);
				return [DOMAIN];
			},
			set: async ({ challenge }) => {
				for (let i = 0; i < challenge.challenges.length; i++)
					if (challenge.challenges[i]["type"] == "dns-01") {
						this.consolelog(1, "CERTIFICATE: Set" + ("wildcard" in challenge ? " (.*)" : ""), challenge);
					}
				this.consolelog(1, challenge.keyAuthorizationDigest);
				const data = { cloud: cloud, action: "set", text: challenge.keyAuthorizationDigest };
				const ret = await this.httpClient.post(this.SERVERURL + "/master/domain.json", data).toPromise();
				this.consolelog(1, ret);
				//alert(challenge.keyAuthorizationDigest);
			},
			get: async (args) => {
				this.consolelog(1, "CERTIFICATE Get: ", args);
			},
			remove: async ({ challenge }) => {
				for (let i = 0; i < challenge.challenges.length; i++)
					if (challenge.challenges[i]["type"] == "dns-01") {
						this.consolelog(1, "CERTIFICATE: Remove" + ("wildcard" in challenge ? " (.*)" : ""), challenge);
					}
				this.consolelog(1, challenge.keyAuthorizationDigest);
				const data = { cloud: cloud, action: "remove", text: challenge.keyAuthorizationDigest };
				const ret = await this.httpClient.post(this.SERVERURL + "/master/domain.json", data).toPromise();
				this.consolelog(1, ret);
				//alert(challenge.keyAuthorizationDigest);
			},
			propagationDelay: 5000
		}
	};
	console.info("Validating domain authorization for " + domains.join(" "));
	const pems = await acme.certificates.create({ account: account, accountKey: accountKey, csr: csr, domains: domains, challenges: challenges });
	ret.fullChain = pems.cert + "\n" + pems.chain + "\n";
	this.consolelog(1, "##################################");
	this.consolelog(1, "CERTIFICATE: " + pems.expires, pems);
	this.consolelog(1, ret);
	this.consolelog(1, "##################################");
	return ret;
}

canActivate(route: ActivatedRouteSnapshot, state: RouterStateSnapshot) {
	this.activateUrl = state.url;
	return this.splashDone ? true : this.router.navigate(["/splash"]);
}

}
