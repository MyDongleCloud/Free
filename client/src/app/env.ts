import { Injectable } from '@angular/core';
import { Router } from '@angular/router';
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
import { Observable, Subject } from 'rxjs';
import { environment } from '../environments/environment';
import { Settings, Config } from './myinterface';
import { VERSION } from './version';
//import { toASCII } from 'punycode';
import * as ACME from '@root/acme';
import * as AA from '@root/acme/maintainers';
import * as Keypairs from '@root/keypairs';
import * as CSR from '@root/csr';
import * as PEM from '@root/pem';
import * as ENC from '@root/encoding/base64';

enum PostMsg {
ERROR,
PLACEHOLDER
};

@Injectable({
	providedIn: "root"
})

export class Global {
VERSION: string = VERSION;
WEBURL: string = "https://mydongle.cloud";
DONGLEURL: string = "";
space;
postMsg = Object();
currentUrl: string;
settings: Settings = {} as Settings;
config: Config = {} as Config;
refreshUI:Subject<any> = new Subject();
refreshO;
refreshObs;
firmwareServerVersion;

constructor(public plt: Platform, private router: Router, private navCtrl: NavController, private alertCtrl: AlertController, private menu: MenuController, private translate: TranslateService, public popoverController: PopoverController, private httpClient: HttpClient) {
	this.getSpace();
	if (environment.production || this.isPlatform("androidios")) {
		;
	} else
		this.DONGLEURL = "http://localhost";
	if (typeof (<any>window).electron != "undefined") {
		(<any>window).electron.ipc.invoke("isDev").then((r) => {
			this.settings.isDev = 2;
		});
	}
	Device.getId().then((info) => {
		this.settings.deviceId = info["identifier"];
		console.log("deviceId: " + this.settings.deviceId);
	}).catch((e) => { console.log(e); });
	navCtrl.setDirection("forward");
	translate.setDefaultLang("en");
	console.log("Default browser language is: " + translate.getBrowserLang());
	this.changeLanguage(this.translate.getBrowserLang());
	if (typeof (<any>window).electron != "undefined") {
		(<any>window).electron.ipc.log((err: any, v: string) => {
			console.log(v);
		});
		(<any>window).electron.ipc.open_page((err: any, v: string) => {
			this.openPage(v, false);
		});
	}
	this.refreshO = Observable.create((obs) => {
		this.refreshObs = obs;
		return () => {}
	});
	this.configLoad();
	this.settingsLoad();
	//setTimeout(() => { this.getCertificate("test101"); }, 2000);
}

async getSpace() {
	try {
		this.space = await this.httpClient.get("/data/space.json").toPromise();
	} catch(e) {
		console.log("Failed to download /data/space.json");
		this.space = { name:"placeholder" };
	}
	this.DONGLEURL = "https://" + (this.space["name"] ?? "") + ".mydongle.cloud";
}

initMsg() {
	for (let i = 0; i < Object.keys(PostMsg).length / 2; i++)
		this.postMsg[PostMsg[i]] = Object.keys(this.translate.instant("assistant"))[i];
}

async checkFolder(d, p) {
	if (typeof (<any>window).electron != "undefined") {//Electron
	} else if (!this.plt.is("electron") && this.plt.is("desktop")) {//Web
	} else {//Android, iOS
		try {
			const ret = await Filesystem.stat({
				directory: d,
				path: p,
			});
		} catch(e) {
			try {
				await Filesystem.mkdir({
					directory: d,
					path: p,
				});
			} catch(e) {
				console.log("Error mkdir " + e);
			}
		}
	}
}

async configLoad() {
	this.config.authorization = "";
}

async settingsLoad() {
	console.log("settingsLoad: Enter");
	if (typeof (<any>window).electron != "undefined") {
		const useKeyring = true;
		await (<any>window).electron.ipc.invoke("keyringEnable", useKeyring);
		const value = await (<any>window).electron.ipc.invoke("load", "settings");
		if (value != null)
			this.settings = value as Settings;
	} else if (true || this.plt.is("android") || this.plt.is("ios")) {
		const { value } = await Preferences.get({key: "settings"});
		if (value != null)
			this.settings = JSON.parse(value);
	}

	if (this.settings.powerUser === undefined)
		this.settings.powerUser = false;
	if (this.settings.isDev === undefined)
		this.settings.isDev = 0;
	if (this.settings.userId === undefined)
		this.settings.userId = "";
	if (this.settings.email === undefined)
		this.settings.email = "";
	if (this.settings.welcomeSeen === undefined)
		this.settings.welcomeSeen = false;
	if (this.settings.reviewRequestLastTime === undefined)
		this.settings.reviewRequestLastTime = 0;
	if (this.settings.dontShowAgain === undefined)
		this.settings.dontShowAgain = Object();

	await this.translate.use(this.settings.language);
	if (typeof (<any>window).electron != "undefined")
		(<any>window).electron.ipc.invoke("menuLanguage", this.settings.language);

	this.initMsg();
}

async settingsSave() {
	console.log("settingsSave: Enter");
	let st = JSON.stringify(this.settings);
	if (this.plt.is("electron"))
		(<any>window).electron.ipc.invoke("save", "settings", st);
	else if (true || this.plt.is("android") || this.plt.is("ios"))
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
		if (typeof (<any>window).electron != "undefined")
			(<any>window).electron.ipc.invoke("menuLanguage", this.settings.language);
	}
}

changeLanguageAndRefresh(l) {
	this.changeLanguage(l);
	this.refreshObs.next();
}

mytranslateP(page, st) {
	const inp = page + "." + st;
	const ret = this.translate.instant(page + "." + st);
	return ret == "" ? (this.settings.isDev == 2 && this.settings.language != "en" ? ("##" + st + "##") : st) : ret == inp ? (this.settings.isDev == 2 ? ("##" + st + "##") : st) : ret;
}

mytranslate(st) {
	return this.mytranslateP(this.currentUrl, st);
}

mytranslateM(st) {
	return this.mytranslateP("modules", st);
}

async sleepms(ms) {
	return new Promise(resolve => setTimeout(resolve, ms));
}

openBrowser(url: string) {
	if (typeof (<any>window).electron != "undefined")
		(<any>window).electron.ipc.invoke("openBrowser", url);
	else
		window.open(url, "_blank");
}

openPage(url: string, close: boolean) {
	if (!close)
		this.navCtrl.setDirection('root');
	this.router.navigate(["/" + url]);
	if (close)
	this.popoverController.dismiss();
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

review() {
	this.settings.reviewRequestLastTime = Date.now();
	this.settingsSave();
	InAppReview.requestReview();
}

async getCertificate(space) {
	AA.init = function () {};
	const DOMAIN = "mydongle.cloud";
	const STAGING = false;
	let acme = ACME.create({ maintainerEmail: "acme@" + DOMAIN, packageAgent: "MDC/2025-01-01", notify: function (ev, msg) { /*console.log(msg);*/ }, skipDryRun: true });
	await acme.init("https://acme" + (STAGING ? "-staging" : "") + "-v02.api.letsencrypt.org/directory");

	const accountKeypair = await Keypairs.generate({ kty: "EC", format: "jwk" });
	const accountKey = accountKeypair.private;
	const accountKeyPEM = await Keypairs.export({ jwk: accountKey });
	console.log("# account.pem #################################");
	console.log(accountKeyPEM);
	console.log("##################################");
	console.info("CERTIFICATE: Registering ACME account...");
	const account = await acme.accounts.create({ subscriberEmail: "certificate@" + DOMAIN, agreeToTerms: true, accountKey: accountKey });
	console.info("CERTIFICATE: Created account with id ", account.key.kid);

	const serverKeypair = await Keypairs.generate({ kty: "RSA", format: "jwk" });
	const serverKey = serverKeypair.private;
	const serverKeyPEM = await Keypairs.export({ jwk: serverKey });
	console.log("# privkey.pem #################################");
	console.log(serverKeyPEM);
	console.log("##################################");

	let domains = [space + "." + DOMAIN, "*." + space + "." + DOMAIN];
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
				console.log("CERTIFICATE: Zones: ", challenge.dnsHosts);
				return [DOMAIN];
			},
			set: async ({ challenge }) => {
				for (let i = 0; i < challenge.challenges.length; i++)
					if (challenge.challenges[i]["type"] == "dns-01") {
						console.log("CERTIFICATE: Set" + ("wildcard" in challenge ? " (.*)" : ""), challenge);
					}
				console.log(challenge.keyAuthorizationDigest);
				const data = { space: space, action: "set", text: challenge.keyAuthorizationDigest };
				const ret = await this.httpClient.post("https://mydongle.cloud/master/domain.json", data).toPromise();
				console.log(ret);
				//alert(challenge.keyAuthorizationDigest);
			},
			get: async (args) => {
				console.log("CERTIFICATE Get: ", args);
			},
			remove: async ({ challenge }) => {
				for (let i = 0; i < challenge.challenges.length; i++)
					if (challenge.challenges[i]["type"] == "dns-01") {
						console.log("CERTIFICATE: Remove" + ("wildcard" in challenge ? " (.*)" : ""), challenge);
					}
				console.log(challenge.keyAuthorizationDigest);
				const data = { space: space, action: "remove", text: challenge.keyAuthorizationDigest };
				const ret = await this.httpClient.post("https://mydongle.cloud/master/domain.json", data).toPromise();
				console.log(ret);
				//alert(challenge.keyAuthorizationDigest);
			},
			propagationDelay: 5000
		}
	};
	console.info("Validating domain authorization for " + domains.join(" "));
	const pems = await acme.certificates.create({ account: account, accountKey: accountKey, csr: csr, domains: domains, challenges: challenges });
	const fullchain = pems.cert + "\n" + pems.chain + "\n";
	console.log("CERTIFICATE: " + pems.expires, pems);
	console.log("# fullchain.pem #################################");
	console.log(fullchain);
	console.log("##################################");
}

}
